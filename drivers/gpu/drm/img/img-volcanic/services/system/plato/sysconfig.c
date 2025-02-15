/*************************************************************************/ /*!
@File
@Title			System Configuration
@Copyright		Copyright (c) Imagination Technologies Ltd. All Rights Reserved
@Description	System Configuration functions
@License        Dual MIT/GPLv2

The contents of this file are subject to the MIT license as set out below.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

Alternatively, the contents of this file may be used under the terms of
the GNU General Public License Version 2 ("GPL") in which case the provisions
of GPL are applicable instead of those above.

If you wish to allow use of your version of this file only under the terms of
GPL, and not to allow others to use your version of this file under the terms
of the MIT license, indicate your decision by deleting the provisions above
and replace them with the notice and other provisions required by GPL as set
out in the file called "GPL-COPYING" included in this distribution. If you do
not delete the provisions above, a recipient may use your version of this file
under the terms of either the MIT license or GPL.

This License is also included in this distribution in the file called
"MIT-COPYING".

EXCEPT AS OTHERWISE STATED IN A NEGOTIATED AGREEMENT: (A) THE SOFTWARE IS
PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
PURPOSE AND NONINFRINGEMENT; AND (B) IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/ /**************************************************************************/

#include "img_defs.h"
#include "pvr_debug.h"
#include "osfunc.h"
#include "allocmem.h"
#include "pvrsrv_device.h"
#include "pvrsrv_memallocflags.h"
#include "syscommon.h"
#include "power.h"
#include "sysinfo.h"
#include "plato.h"
#include "sysconfig.h"
#include "physheap.h"
#include "pci_support.h"
#include "interrupt_support.h"
#if defined(__linux__)
#include <linux/dma-mapping.h>
#endif

#include "plato_aon_regs.h"
#include "plato_top_regs.h"
#include "plato_ddr_ctrl_regs.h"
#include "plato_ddr_publ_regs.h"

/* BUP flags */
#define PLATO_DDR_TRAINING_RETRY_MAX 5
#define PLATO_DDR_KINGSTON 1

#if defined(PLATO_LOG_CHECKPOINTS)
#define PLATO_CHECKPOINT PVR_LOG(("- %s: %d", __func__, __LINE__))
#else
#define PLATO_CHECKPOINT
#endif

#if (PLATO_MEMORY_CONFIG == PLATO_MEMORY_LOCAL)
#define	HOST_PCI_INIT_FLAGS	0
#else
#define	HOST_PCI_INIT_FLAGS	HOST_PCI_INIT_FLAG_BUS_MASTER
#endif

#if (PLATO_MEMORY_CONFIG == PLATO_MEMORY_LOCAL) || (PLATO_MEMORY_CONFIG == PLATO_MEMORY_HYBRID)
static PHYS_HEAP_FUNCTIONS gsLocalPhysHeapFuncs =
{
	/* pfnCpuPAddrToDevPAddr */
	PlatoLocalCpuPAddrToDevPAddr,
	/* pfnDevPAddrToCpuPAddr */
	PlatoLocalDevPAddrToCpuPAddr,
};
#endif

#if (PLATO_MEMORY_CONFIG == PLATO_MEMORY_HOST) || (PLATO_MEMORY_CONFIG == PLATO_MEMORY_HYBRID)
static PHYS_HEAP_FUNCTIONS gsHostPhysHeapFuncs =
{
	/* pfnCpuPAddrToDevPAddr */
	PlatoSystemCpuPAddrToDevPAddr,
	/* pfnDevPAddrToCpuPAddr */
	PlatoSystemDevPAddrToCpuPAddr,
};
#endif

/* Clock speed module parameters */
static IMG_UINT32 ui32MemClockSpeed  = PLATO_MEM_CLOCK_SPEED;
static IMG_UINT32 ui32CoreClockSpeed = PLATO_RGX_CORE_CLOCK_SPEED;

/* Helpers for getting DDR/GPU/PLL clock speed */
IMG_UINT32 SysGetPlatoMemClockSpeed(void)
{
	IMG_UINT32 ret = ui32MemClockSpeed;

	(ui32MemClockSpeed > PLATO_MAX_MEM_CLOCK_SPEED) ? ret = PLATO_MAX_MEM_CLOCK_SPEED : 0;
	(ui32MemClockSpeed < PLATO_MIN_MEM_CLOCK_SPEED) ? ret = PLATO_MIN_MEM_CLOCK_SPEED : 0;

	return ret;
}

IMG_UINT32 SysGetPlatoCoreClockSpeed(void)
{
	IMG_UINT32 ret = ui32CoreClockSpeed;

	(ui32CoreClockSpeed > PLATO_RGX_MAX_CORE_CLOCK_SPEED) ? ret = PLATO_RGX_MAX_CORE_CLOCK_SPEED : 0;
	(ui32CoreClockSpeed < PLATO_RGX_MIN_CORE_CLOCK_SPEED) ? ret = PLATO_RGX_MIN_CORE_CLOCK_SPEED : 0;

	return ret;
}

/*
 * This function returns a PLL clock value which can be easily reduced
 * (using integer dividers) to get 148.5 MHz HDMI pixel clock.
 *
 * NOTE: Accurate values are returned only in the cases
 *       which are known by the function.
 */
IMG_UINT32 SysGetPlatoPLLClockSpeed(IMG_UINT32 ui32ClockSpeed)
{
    /*
     * Force the lowest possible PLL clock in case when the requested
     * clock speed is higher than the largest value known by the function.
     */
    IMG_UINT32 ui32PLLClockSpeed = ui32ClockSpeed;
    IMG_UINT32 ui32AccClockSpeed = ui32ClockSpeed;

    /* Tweak the values if the requested clock speed is a supported one */
    (ui32ClockSpeed <= 742500000) ? (ui32PLLClockSpeed = 742500000,  ui32AccClockSpeed = 742500000) : 0;
    (ui32ClockSpeed <= 668250000) ? (ui32PLLClockSpeed = 1336500000, ui32AccClockSpeed = 668250000) : 0;
    (ui32ClockSpeed <= 594000000) ? (ui32PLLClockSpeed = 1188000000, ui32AccClockSpeed = 594000000) : 0;
    (ui32ClockSpeed <= 544500000) ? (ui32PLLClockSpeed = 1633500000, ui32AccClockSpeed = 544500000) : 0;
    (ui32ClockSpeed <= 519750000) ? (ui32PLLClockSpeed = 1039500000, ui32AccClockSpeed = 519750000) : 0;
    (ui32ClockSpeed <= 495000000) ? (ui32PLLClockSpeed = 1485000000, ui32AccClockSpeed = 495000000) : 0;
    (ui32ClockSpeed <= 445500000) ? (ui32PLLClockSpeed = 891000000,  ui32AccClockSpeed = 445500000) : 0;
    (ui32ClockSpeed <= 408375000) ? (ui32PLLClockSpeed = 1633500000, ui32AccClockSpeed = 408375000) : 0;
    (ui32ClockSpeed <= 396000000) ? (ui32PLLClockSpeed = 1188000000, ui32AccClockSpeed = 396000000) : 0;

    /*
     * If the requested core clock speed matches the expected one, there is
     * no need to do any additional adjustments of the PLL clock speed.
     */
    if (ui32ClockSpeed == ui32AccClockSpeed)
    {
        return ui32PLLClockSpeed;
    }

    /* Do fine grained adjustment if the requested clock speed is different than expected */
    return ((((IMG_UINT64)ui32PLLClockSpeed << 32) / ui32AccClockSpeed) >> 32) * ui32ClockSpeed;
}

#if defined(__linux__)
#include <linux/module.h>
#include <linux/moduleparam.h>
module_param_named(sys_mem_clk_speed,  ui32MemClockSpeed,  uint, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(sys_mem_clk_speed, "Plato memory clock speed in Hz (600000000 - 800000000)");
module_param_named(sys_core_clk_speed, ui32CoreClockSpeed, uint, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(sys_core_clk_speed, "Plato core clock speed in Hz (396000000 - 742500000)");

EXPORT_SYMBOL(SysGetPlatoMemClockSpeed);
EXPORT_SYMBOL(SysGetPlatoCoreClockSpeed);
EXPORT_SYMBOL(SysGetPlatoPLLClockSpeed);
#endif

#define PLATO_HAS_NON_MAPPABLE(dev) (dev->ui64MappedMemSize < SYS_DEV_MEM_REGION_SIZE)

static IMG_BOOL PollPr(IMG_UINT32* base, IMG_UINT32 reg, IMG_UINT32 val, IMG_UINT32 msk, IMG_UINT32 cnt, IMG_UINT32 intrvl)
{
	IMG_UINT32 polnum;

	for (polnum = 0; polnum < cnt; polnum++)
	{
		if ((OSReadHWReg32(base, reg) & msk) == val)
		{
			break;
		}
		OSSleepms(intrvl);
	}
	if (polnum == cnt)
	{
		PVR_DPF((PVR_DBG_ERROR, "Poll failed for register: 0x%08X", (unsigned int)reg));
		return false;
	}

	return true;
}

#define Poll(base,reg,val,msk) PollPr(base,reg,val,msk,10,10)
static IMG_CHAR *GetDeviceVersionString(SYS_DATA *psSysData)
{
	return NULL;
}

static
IMG_BOOL SystemISRHandler(void *pvData)
{
	SYS_DATA *psDevData = (SYS_DATA *)pvData;
	IMG_UINT32 ui32InterruptStatus;
	IMG_UINT32 i;
	IMG_BOOL bInterruptServiced = IMG_FALSE;

#if defined(VIRTUAL_PLATFORM)
	/* On virtual platform all interrupt handlers need to be called */
	ui32InterruptStatus = (0x1 << PLATO_GPU_INT_SHIFT) | (0x1 << PLATO_PDP_INT_SHIFT);
#else
	{
		void* pvSystemDbgPeripRegs = psDevData->pvSystemRegCpuVBase + SYS_PLATO_REG_PERIP_OFFSET;
		ui32InterruptStatus = OSReadHWReg32(pvSystemDbgPeripRegs , PLATO_TOP_CR_INT_STATUS);
		PVR_DPF((PVR_DBG_MESSAGE, "%s:%d Interrupt Status (%u)",
				 __func__, __LINE__, ui32InterruptStatus));
	}
#endif

	for (i = 0; i < ARRAY_SIZE(psDevData->sInterruptData); i++)
	{
		if ((ui32InterruptStatus & psDevData->sInterruptData[i].ui32InterruptFlag) != 0)
		{
			bInterruptServiced = psDevData->sInterruptData[i].pfnLISR(psDevData->sInterruptData[i].pvData);
		}
	}

	return bInterruptServiced;
}

static void EnableInterrupt(SYS_DATA *psDevData, IMG_UINT32 ui32InterruptFlag)
{
#if !defined(VIRTUAL_PLATFORM)
	IMG_UINT32 ui32Value;

	void* pvSystemDbgPeripRegs = psDevData->pvSystemRegCpuVBase + SYS_PLATO_REG_PERIP_OFFSET;

	ui32Value = OSReadHWReg32(pvSystemDbgPeripRegs, PLATO_TOP_CR_PCI_INT_MASK);
	ui32Value |= ui32InterruptFlag;
	OSWriteHWReg32(pvSystemDbgPeripRegs, PLATO_TOP_CR_PCI_INT_MASK, ui32Value);

	(void) OSReadHWReg32(pvSystemDbgPeripRegs, PLATO_TOP_CR_PCI_INT_MASK);
	OSWaitus(10);
#endif
}

static void DisableInterrupt(SYS_DATA *psDevData, IMG_UINT32 ui32InterruptFlag)
{
#if !defined(VIRTUAL_PLATFORM)
	IMG_UINT32 ui32Value;

	void* pvSystemDbgPeripRegs = psDevData->pvSystemRegCpuVBase + SYS_PLATO_REG_PERIP_OFFSET;

	ui32Value = OSReadHWReg32(pvSystemDbgPeripRegs, PLATO_TOP_CR_PCI_INT_MASK);
	ui32Value &= ~ui32InterruptFlag;
	OSWriteHWReg32(pvSystemDbgPeripRegs, PLATO_TOP_CR_PCI_INT_MASK, ui32Value);

	(void) OSReadHWReg32(pvSystemDbgPeripRegs, PLATO_TOP_CR_PCI_INT_MASK);
	OSWaitus(10);
#endif
}

PVRSRV_ERROR SysGetPlatoGPUTemp(SYS_DATA * psDevData, IMG_UINT32 *ui32Temp);

static IMG_UINT32 HashNOCRegisterAddress(IMG_UINT32 ui32Addr)
{
	IMG_UINT32 ui32Hash_bits = (ui32Addr & 0x4000 ? 1 : 0);
	if (ui32Addr & 0x2000)
		++ui32Hash_bits;
	if (ui32Addr & 0x1000)
		++ui32Hash_bits;
	if (ui32Addr & 0x0800)
		++ui32Hash_bits;
	if ((ui32Hash_bits == 1) || (ui32Hash_bits == 3))
		ui32Addr ^= 0x100;

	return ui32Addr;
}


typedef struct PLATO_NOC_DBG_REG
{
	IMG_UINT32 ui32Offset;
	IMG_CHAR *pszName;
}PLATO_NOC_DBG_REGS;

static PLATO_NOC_DBG_REGS asPlatoNOCDbgRegs[] = {
	{SYS_PLATO_REG_NOC_DBG_DDR_A_CTRL_OFFSET, "DDR A Ctrl"},
	{SYS_PLATO_REG_NOC_DBG_DDR_A_DATA_OFFSET, "DDR A Data"},
	{SYS_PLATO_REG_NOC_DBG_DDR_A_PUBL_OFFSET, "DDR A Publ"},
	{SYS_PLATO_REG_NOC_DBG_DDR_B_CTRL_OFFSET, "DDR B Ctrl"},
	{SYS_PLATO_REG_NOC_DBG_DDR_B_DATA_OFFSET, "DDR B Data"},
	{SYS_PLATO_REG_NOC_DBG_DDR_B_PUBL_OFFSET, "DDR B Publ"},

	{SYS_PLATO_REG_NOC_DBG_DISPLAY_S_OFFSET, "Display S"},
	{SYS_PLATO_REG_NOC_DBG_GPIO_0_S_OFFSET,  "GPIO 0 S"},
	{SYS_PLATO_REG_NOC_DBG_GPIO_1_S_OFFSET,  "GPIO 1 S"},
	{SYS_PLATO_REG_NOC_DBG_GPU_S_OFFSET,	 "GPU S"},
	{SYS_PLATO_REG_NOC_DBG_PCI_PHY_OFFSET,	 "PCI PHY"},
	{SYS_PLATO_REG_NOC_DBG_PCI_REG_OFFSET,	 "PCI Reg"},
	{SYS_PLATO_REG_NOC_DBG_PCI_S_OFFSET,	 "PCI S"},
	{SYS_PLATO_REG_NOC_DBG_PERIPH_S_OFFSET,  "Periph S"},
	{SYS_PLATO_REG_NOC_DBG_RET_REG_OFFSET,	 "Ret Reg"},
	{SYS_PLATO_REG_NOC_DBG_SERVICE_OFFSET,	 "Service"}
};

PVRSRV_ERROR SysDebugInfo(PVRSRV_DEVICE_CONFIG *psDevConfig,
				DUMPDEBUG_PRINTF_FUNC *pfnDumpDebugPrintf,
				void *pvDumpDebugFile)
{
	SYS_DATA *psDevData = (SYS_DATA *)psDevConfig->hSysData;
	void *pvDbgPeripRegs = psDevData->pvSystemRegCpuVBase + SYS_PLATO_REG_PERIP_OFFSET;
	void *pvNocRegs = psDevData->pvSystemRegCpuVBase + SYS_PLATO_REG_NOC_OFFSET;
	void *pvAonRegs = psDevData->pvSystemRegCpuVBase + SYS_PLATO_REG_AON_OFFSET;
	IMG_UINT32 ui32Val;
	PVRSRV_ERROR eError = PVRSRV_OK;
	IMG_UINT32 i;
	IMG_UINT32 ui32GPUTemp;

	PVR_DUMPDEBUG_LOG("------[ System Debug - Device ID:%u ]------", psDevConfig->psDevNode->sDevId.ui32InternalID);

	if ((eError = SysGetPlatoGPUTemp(psDevData, &ui32GPUTemp)) != PVRSRV_OK)
	{
		return eError;
	}
	PVR_DUMPDEBUG_LOG("GPU Temp: %u °C", ui32GPUTemp);
	PVR_DUMPDEBUG_LOG("PCI_PHY_STATUS: 0x%x",
					   OSReadHWReg32(pvDbgPeripRegs, PLATO_TOP_CR_PCI_PHY_STATUS));
	for (i = 0; i <= 0x1C; i += 4)
	{
		PVR_DUMPDEBUG_LOG("NOC Offset 0x%x: 0x%x", i,
						   OSReadHWReg32(pvNocRegs, i));
	}
	PVR_DUMPDEBUG_LOG("NOC Offset 0x%x: 0x%x", 0x50,
					   OSReadHWReg32(pvNocRegs, 0x50));
	PVR_DUMPDEBUG_LOG("NOC Offset 0x%x: 0x%x", 0x54,
					   OSReadHWReg32(pvNocRegs, 0x54));
	PVR_DUMPDEBUG_LOG("NOC Offset 0x%x: 0x%x", 0x58,
					   OSReadHWReg32(pvNocRegs, 0x58));
	PVR_DUMPDEBUG_LOG("AON Offset 0x%x: 0x%x", PLATO_AON_CR_BM_STATUS,
					   OSReadHWReg32(pvAonRegs, PLATO_AON_CR_BM_STATUS));
	PVR_DUMPDEBUG_LOG("AON Offset 0x%x: 0x%x", PLATO_AON_CR_PLL_STATUS,
					   OSReadHWReg32(pvAonRegs, PLATO_AON_CR_PLL_STATUS));

	PVR_DUMPDEBUG_LOG("NOC Error Status: ");
	for (i = 0; i < ARRAY_SIZE(asPlatoNOCDbgRegs); i++)
	{
		IMG_UINT32 ui32Offset = asPlatoNOCDbgRegs[i].ui32Offset + 0xC;
		IMG_UINT32 ui32HashedOffset = HashNOCRegisterAddress(ui32Offset);

		ui32Val = OSReadHWReg32(pvNocRegs, ui32HashedOffset);
		if (ui32Val == 0)
		{
			PVR_DUMPDEBUG_LOG("%s ok", asPlatoNOCDbgRegs[i].pszName);
		}
		else
		{
			PVR_DUMPDEBUG_LOG("%s error val 0x%x (offset 0x%x)", asPlatoNOCDbgRegs[i].pszName, ui32Val, ui32Offset);
			PVR_DUMPDEBUG_LOG("%s error val 0x%x (offset 0x%x)", asPlatoNOCDbgRegs[i].pszName, OSReadHWReg32(pvNocRegs, ui32HashedOffset + 0x8), ui32Offset + 0x8);
			PVR_DUMPDEBUG_LOG("%s error val 0x%x (offset 0x%x)", asPlatoNOCDbgRegs[i].pszName, OSReadHWReg32(pvNocRegs, ui32HashedOffset + 0xC), ui32Offset + 0xC);
			PVR_DUMPDEBUG_LOG("%s error val 0x%x (offset 0x%x)", asPlatoNOCDbgRegs[i].pszName, OSReadHWReg32(pvNocRegs, ui32HashedOffset + 0x14), ui32Offset + 0x14);
			PVR_DUMPDEBUG_LOG("%s error val 0x%x (offset 0x%x)", asPlatoNOCDbgRegs[i].pszName, OSReadHWReg32(pvNocRegs, ui32HashedOffset + 0x18), ui32Offset + 0x18);
			PVR_DUMPDEBUG_LOG("%s error val 0x%x (offset 0x%x)", asPlatoNOCDbgRegs[i].pszName, OSReadHWReg32(pvNocRegs, ui32HashedOffset + 0x1C), ui32Offset + 0x1C);
		}

	}

	return eError;
}

PVRSRV_ERROR PlatoMapRegisters(SYS_DATA *psDevData, IMG_UINT32 ui32BaseNum, IMG_UINT32 ui32Offset, IMG_UINT32 ui32Size, void ** pvCpuVAddr)
{
	PVRSRV_ERROR eError = PVRSRV_OK;
	IMG_CPU_PHYADDR	sCpuPBase;

	sCpuPBase.uiAddr = OSPCIAddrRangeStart(psDevData->hRGXPCI, ui32BaseNum) + ui32Offset;

#if !defined(VIRTUAL_PLATFORM)
	/* Reserve all register banks up to HDMI bank */
	eError = OSPCIRequestAddrRegion(psDevData->hRGXPCI, ui32BaseNum, ui32Offset, ui32Size);
	if (eError != PVRSRV_OK)
	{
		PVR_DPF((PVR_DBG_ERROR,
				 "%s: Failed to request the system address region(BAR %d)",
				 __func__, ui32BaseNum));
		goto ErrorReturn;
	}

	*pvCpuVAddr = OSMapPhysToLin(sCpuPBase, ui32Size, PVRSRV_MEMALLOCFLAG_CPU_UNCACHED);
	if (*pvCpuVAddr == NULL)
	{
		PVR_DPF((PVR_DBG_ERROR, "%s: Failed to map system registers (BAR %d)",
				 __func__, ui32BaseNum));
		eError = PVRSRV_ERROR_OUT_OF_MEMORY;
		goto ErrorSystemRegReleaseAddrRegion;
	}
#endif

	return eError;

#if !defined(VIRTUAL_PLATFORM)
ErrorSystemRegReleaseAddrRegion:
	OSPCIReleaseAddrRegion(psDevData->hRGXPCI, SYS_PLATO_REG_PCI_BASENUM, SYS_PLATO_REG_SYS_OFFSET, SYS_PLATO_REG_PDP_OFFSET - SYS_PLATO_REG_SYS_OFFSET);

ErrorReturn:
	return eError;
#endif
}

void PlatoUnmapRegisters(SYS_DATA * psDevData, void * pvBase, IMG_UINT32 ui32BaseNum, IMG_UINT32 ui32Offset, IMG_UINT32 ui32Size)
{
	OSUnMapPhysToLin(pvBase, ui32Size);
	OSPCIReleaseAddrRegion(psDevData->hRGXPCI, ui32BaseNum, ui32Offset, ui32Size);
}


static PVRSRV_ERROR ConfigPlatoDram(void * pvPublRegs, void * pvCtrlRegs, void * pvAonRegs,
									IMG_UINT32 pvBLDRData[PLATO_DDR_PUBL_DATX_LANE_COUNT][PLATO_DDR_PUBL_DXBDLR_REGS_PER_LANE],
									IMG_UINT32 ui32ResetFlags)
{
	/*

	Phase 1: Program the DWC_ddr_umctl2 registers

	*/

	/* Single rank only for Kingston DDRs*/
	OSWriteHWReg32(pvCtrlRegs, PLATO_DDR_CTRL_MSTR, 0x41040001);

	OSWaitus(100);

	/*refresh timings*/
#if defined(PLATO_DDR_KINGSTON)
	OSWriteHWReg32(pvCtrlRegs, PLATO_DDR_CTRL_RFSHTMG, 0x0081008B);
#else
	OSWriteHWReg32(pvCtrlRegs, PLATO_DDR_CTRL_RFSHTMG, 0x006100BB);
#endif

	// Train DDR sequence
	OSWriteHWReg32(pvCtrlRegs, PLATO_DDR_CTRL_INIT0, 0x00020100);
	OSWriteHWReg32(pvCtrlRegs, PLATO_DDR_CTRL_INIT1, 0x00010000);

#if defined(PLATO_DDR_KINGSTON)
	/*write recovery */
	OSWriteHWReg32(pvCtrlRegs, PLATO_DDR_CTRL_INIT3, 0x01700000);
	OSWriteHWReg32(pvCtrlRegs, PLATO_DDR_CTRL_INIT4, 0x00280000);
	OSWriteHWReg32(pvCtrlRegs, PLATO_DDR_CTRL_INIT5, 0x0012000c);
	OSWriteHWReg32(pvCtrlRegs, PLATO_DDR_CTRL_DRAMTMG0, 0x0f0e2112);
	OSWriteHWReg32(pvCtrlRegs, PLATO_DDR_CTRL_DRAMTMG1, 0x00040618);
	OSWriteHWReg32(pvCtrlRegs, PLATO_DDR_CTRL_DRAMTMG2, 0x0506040B);

	OSWriteHWReg32(pvCtrlRegs, PLATO_DDR_CTRL_DRAMTMG3, 0x00002008);

	OSWriteHWReg32(pvCtrlRegs, PLATO_DDR_CTRL_DRAMTMG4, 0x06020307);
	OSWriteHWReg32(pvCtrlRegs, PLATO_DDR_CTRL_DRAMTMG5, 0x090e0403);

	OSWriteHWReg32(pvCtrlRegs, PLATO_DDR_CTRL_DRAMTMG6, 0x0d0e000e);
	OSWriteHWReg32(pvCtrlRegs, PLATO_DDR_CTRL_DRAMTMG7, 0x00000c0e);

	OSWriteHWReg32(pvCtrlRegs, PLATO_DDR_CTRL_DRAMTMG8, 0x01010a05);

	/*impedance registers */
	OSWriteHWReg32(pvCtrlRegs, PLATO_DDR_CTRL_ZQCTL0, 0x30ab002b);
	OSWriteHWReg32(pvCtrlRegs, PLATO_DDR_CTRL_ZQCTL1, 0x00000070);
	OSWriteHWReg32(pvCtrlRegs, PLATO_DDR_CTRL_ZQCTL2, 0x00000000);

	/*refresh control*/
	OSWriteHWReg32(pvCtrlRegs, PLATO_DDR_CTRL_RFSHCTL0, 0x00e01020);
	OSWriteHWReg32(pvCtrlRegs, PLATO_DDR_CTRL_RFSHCTL1, 0x0078007e);
	OSWriteHWReg32(pvCtrlRegs, PLATO_DDR_CTRL_RFSHCTL2, 0x0057000e);
	OSWriteHWReg32(pvCtrlRegs, PLATO_DDR_CTRL_RFSHCTL3, 0x00000000);
	/*DFI Timings*/
	OSWriteHWReg32(pvCtrlRegs, PLATO_DDR_CTRL_DFITMG0, 0x02878208);

	OSWriteHWReg32(pvCtrlRegs, PLATO_DDR_CTRL_DFITMG1, 0x00020202);

	OSWriteHWReg32(pvCtrlRegs, PLATO_DDR_CTRL_DFIUPD0, 0x00400003);
	OSWriteHWReg32(pvCtrlRegs, PLATO_DDR_CTRL_DFIUPD1, 0x00f000ff);
	OSWriteHWReg32(pvCtrlRegs, PLATO_DDR_CTRL_DFIUPD2, 0x80100010);
	OSWriteHWReg32(pvCtrlRegs, PLATO_DDR_CTRL_DFIUPD3, 0x088105c3);

	OSWriteHWReg32(pvCtrlRegs, PLATO_DDR_CTRL_DFIMISC, 0x00000000);
#else
	OSWriteHWReg32(pvCtrlRegs, PLATO_DDR_CTRL_INIT3, 0x0d700000);
	OSWriteHWReg32(pvCtrlRegs, PLATO_DDR_CTRL_INIT4, 0x00180000);
	OSWriteHWReg32(pvCtrlRegs, PLATO_DDR_CTRL_INIT5, 0x00090009);
	OSWriteHWReg32(pvCtrlRegs, PLATO_DDR_CTRL_DRAMTMG0, 0x0c101a0e);
	OSWriteHWReg32(pvCtrlRegs, PLATO_DDR_CTRL_DRAMTMG1, 0x000a0313);
	OSWriteHWReg32(pvCtrlRegs, PLATO_DDR_CTRL_DRAMTMG2, 0x04050509);

	OSWriteHWReg32(pvCtrlRegs, PLATO_DDR_CTRL_DRAMTMG3, 0x00002008);

	OSWriteHWReg32(pvCtrlRegs, PLATO_DDR_CTRL_DRAMTMG4, 0x06020306);
	OSWriteHWReg32(pvCtrlRegs, PLATO_DDR_CTRL_DRAMTMG5, 0x070c0202);

	OSWriteHWReg32(pvCtrlRegs, PLATO_DDR_CTRL_DRAMTMG6, 0x0d0e000e);
	OSWriteHWReg32(pvCtrlRegs, PLATO_DDR_CTRL_DRAMTMG7, 0x00000c0e);

	OSWriteHWReg32(pvCtrlRegs, PLATO_DDR_CTRL_DRAMTMG8, 0x01010a07);
	/*impedance registers */
	OSWriteHWReg32(pvCtrlRegs, PLATO_DDR_CTRL_ZQCTL0, 0x10800020);
	OSWriteHWReg32(pvCtrlRegs, PLATO_DDR_CTRL_ZQCTL1, 0x00000070);
	OSWriteHWReg32(pvCtrlRegs, PLATO_DDR_CTRL_ZQCTL2, 0x00000000);

	/*refresh control*/
	OSWriteHWReg32(pvCtrlRegs, PLATO_DDR_CTRL_RFSHCTL0, 0x00e06010);
	OSWriteHWReg32(pvCtrlRegs, PLATO_DDR_CTRL_RFSHCTL1, 0x00600031);
	OSWriteHWReg32(pvCtrlRegs, PLATO_DDR_CTRL_RFSHCTL2, 0x0004002a);
	OSWriteHWReg32(pvCtrlRegs, PLATO_DDR_CTRL_RFSHCTL3, 0x00000000);

	/*DFI Timings*/
	OSWriteHWReg32(pvCtrlRegs, PLATO_DDR_CTRL_DFITMG0, 0x02878206);

	OSWriteHWReg32(pvCtrlRegs, PLATO_DDR_CTRL_DFITMG1, 0x00020202);

	OSWriteHWReg32(pvCtrlRegs, PLATO_DDR_CTRL_DFILPCFG0, 0x07111031);
	OSWriteHWReg32(pvCtrlRegs, PLATO_DDR_CTRL_DFILPCFG1, 0x00000050);

	OSWriteHWReg32(pvCtrlRegs, PLATO_DDR_CTRL_DFIUPD0, 0x00400003);
	OSWriteHWReg32(pvCtrlRegs, PLATO_DDR_CTRL_DFIUPD1, 0x006a006f);
	OSWriteHWReg32(pvCtrlRegs, PLATO_DDR_CTRL_DFIUPD2, 0x0d0b02b6);
	OSWriteHWReg32(pvCtrlRegs, PLATO_DDR_CTRL_DFIUPD3, 0x00100010);

	OSWriteHWReg32(pvCtrlRegs, PLATO_DDR_CTRL_DFIMISC, 0x00000001);
#endif

	/* Single rank only on Kingston */
	OSWriteHWReg32(pvCtrlRegs, PLATO_DDR_CTRL_ADDRMAP0, 0x00001F1F);
	OSWriteHWReg32(pvCtrlRegs, PLATO_DDR_CTRL_ADDRMAP1, 0x00070707);
	OSWriteHWReg32(pvCtrlRegs, PLATO_DDR_CTRL_ADDRMAP2, 0x00000000);
	OSWriteHWReg32(pvCtrlRegs, PLATO_DDR_CTRL_ADDRMAP3, 0x0F000000);
	OSWriteHWReg32(pvCtrlRegs, PLATO_DDR_CTRL_ADDRMAP4, 0x00000F0F);
	OSWriteHWReg32(pvCtrlRegs, PLATO_DDR_CTRL_ADDRMAP5, 0x06060606);
	OSWriteHWReg32(pvCtrlRegs, PLATO_DDR_CTRL_ADDRMAP6, 0x06060606);
	OSWriteHWReg32(pvCtrlRegs, PLATO_DDR_CTRL_ADDRMAP7, 0x00000F0F);
	OSWriteHWReg32(pvCtrlRegs, PLATO_DDR_CTRL_ADDRMAP8, 0x00000707);

#if defined(PLATO_DDR_KINGSTON)
	OSWriteHWReg32(pvCtrlRegs, PLATO_DDR_CTRL_ODTCFG, 0x06000604);
	OSWriteHWReg32(pvCtrlRegs, PLATO_DDR_CTRL_ODTMAP, 0x99c5b050);
#else
	OSWriteHWReg32(pvCtrlRegs, PLATO_DDR_CTRL_ODTCFG, 0x0d0f0740);
	OSWriteHWReg32(pvCtrlRegs, PLATO_DDR_CTRL_ODTMAP, 0x99c5b050);
#endif

	OSWriteHWReg32(pvCtrlRegs, PLATO_DDR_CTRL_PERFHPR1, 0x9f008f23);
	OSWriteHWReg32(pvCtrlRegs, PLATO_DDR_CTRL_SCHED, 0x00003f00);
	OSWriteHWReg32(pvCtrlRegs, PLATO_DDR_CTRL_PERFLPR1, 0x18000064);
	OSWriteHWReg32(pvCtrlRegs, PLATO_DDR_CTRL_PERFWR1, 0x18000096);

	/* Setup the virtual channels */
	OSWriteHWReg32(pvCtrlRegs, 0x00000410, 0x0000000F);
	OSWriteHWReg32(pvCtrlRegs, 0x00000414, 0x00000000);
	OSWriteHWReg32(pvCtrlRegs, 0x00000418, 0x0000000F);
	OSWriteHWReg32(pvCtrlRegs, 0x0000041C, 0x00000001);
	OSWriteHWReg32(pvCtrlRegs, 0x00000420, 0x0000000F);
	OSWriteHWReg32(pvCtrlRegs, 0x00000424, 0x00000002);
	OSWriteHWReg32(pvCtrlRegs, 0x00000428, 0x0000000F);
	OSWriteHWReg32(pvCtrlRegs, 0x0000042C, 0x00000003);
	OSWriteHWReg32(pvCtrlRegs, 0x00000430, 0x0000000F);
	OSWriteHWReg32(pvCtrlRegs, 0x00000434, 0x00000004);
	OSWriteHWReg32(pvCtrlRegs, 0x00000438, 0x0000000F);
	OSWriteHWReg32(pvCtrlRegs, 0x0000043C, 0x00000005);
	OSWriteHWReg32(pvCtrlRegs, 0x00000440, 0x0000000F);
	OSWriteHWReg32(pvCtrlRegs, 0x00000444, 0x00000006);
	OSWriteHWReg32(pvCtrlRegs, 0x00000448, 0x0000000F);
	OSWriteHWReg32(pvCtrlRegs, 0x0000044C, 0x00000007);
	OSWriteHWReg32(pvCtrlRegs, 0x00000450, 0x0000000F);
	OSWriteHWReg32(pvCtrlRegs, 0x00000454, 0x00000008);
	OSWriteHWReg32(pvCtrlRegs, 0x00000458, 0x0000000F);
	OSWriteHWReg32(pvCtrlRegs, 0x0000045C, 0x00000009);
	OSWriteHWReg32(pvCtrlRegs, 0x00000460, 0x0000000F);
	OSWriteHWReg32(pvCtrlRegs, 0x00000464, 0x0000000A);
	OSWriteHWReg32(pvCtrlRegs, 0x00000468, 0x0000000F);
	OSWriteHWReg32(pvCtrlRegs, 0x0000046C, 0x0000000B);
	OSWriteHWReg32(pvCtrlRegs, 0x00000470, 0x0000000F);
	OSWriteHWReg32(pvCtrlRegs, 0x00000474, 0x0000000C);
	OSWriteHWReg32(pvCtrlRegs, 0x00000478, 0x0000000F);
	OSWriteHWReg32(pvCtrlRegs, 0x0000047C, 0x0000000D);
	OSWriteHWReg32(pvCtrlRegs, 0x00000480, 0x0000000F);
	OSWriteHWReg32(pvCtrlRegs, 0x00000484, 0x0000000E);

	Poll(pvCtrlRegs, 0x484, 0x0000000E, 0x0000000F);

	OSWaitus(1000);

	PLATO_CHECKPOINT;

	/*

	Phase 2: Deassert soft reset signal core_ddrc_rstn

	*/

	/* Now getting DRAM controller out of reset */
	OSWriteHWReg32(pvAonRegs, PLATO_AON_CR_RESET_CTRL, ui32ResetFlags | OSReadHWReg32(pvAonRegs, PLATO_AON_CR_RESET_CTRL));

	OSWaitus(1000);

	/* ECC disable */
	OSWriteHWReg32(pvCtrlRegs, PLATO_DDR_CTRL_DBG1, 0x00000000);
	/* power related */
	OSWriteHWReg32(pvCtrlRegs, PLATO_DDR_CTRL_PWRCTL, 0x00000000);
	/* Enabling AXI input port (Port control) */
	OSWriteHWReg32(pvCtrlRegs, PLATO_DDR_CTRL_PCTRL, 0x00000001);

	PLATO_CHECKPOINT;
	/*

	Phase 7: Set DFIMISC.dfi_init_complete_en to 1

	(skipped on emu?)
	*/

	/*

	Phase 3: Start PHY initialization by accessing relevant PUB registers

	*/
	OSWriteHWReg32(pvPublRegs, PLATO_DDR_PUBL_DCR_OFFSET, 0x0000040B);

#if defined(PLATO_DDR_KINGSTON)
	/* IF DDR_RISE_RISE */
	OSWriteHWReg32(pvPublRegs, PLATO_DDR_PUBL_DSGCR_OFFSET, 0x0064641F);

		/*MR Registers */
	OSWriteHWReg32(pvPublRegs, PLATO_DDR_PUBL_MR0_OFFSET, 0x170);
	OSWriteHWReg32(pvPublRegs, PLATO_DDR_PUBL_MR1_OFFSET, 0x00000400);
	OSWriteHWReg32(pvPublRegs, PLATO_DDR_PUBL_MR2_OFFSET, 0x00000228);
	OSWriteHWReg32(pvPublRegs, PLATO_DDR_PUBL_MR3_OFFSET, 0x00000000);

	OSWriteHWReg32(pvPublRegs, PLATO_DDR_PUBL_DTPR0_OFFSET, 0x06220308);
	OSWriteHWReg32(pvPublRegs, PLATO_DDR_PUBL_DTPR1_OFFSET, 0x281b1004);
	OSWriteHWReg32(pvPublRegs, PLATO_DDR_PUBL_DTPR2_OFFSET, 0x00060120);
	OSWriteHWReg32(pvPublRegs, PLATO_DDR_PUBL_DTPR3_OFFSET, 0x02000101);
	OSWriteHWReg32(pvPublRegs, PLATO_DDR_PUBL_DTPR4_OFFSET, 0x01200807);
	OSWriteHWReg32(pvPublRegs, PLATO_DDR_PUBL_DTPR5_OFFSET, 0x00300c08);

	OSWriteHWReg32(pvPublRegs, PLATO_DDR_PUBL_PGCR1_OFFSET, 0x020046A0);
	OSWriteHWReg32(pvPublRegs, PLATO_DDR_PUBL_PGCR2_OFFSET, 0x00F09088);
#else
	OSWriteHWReg32(pvPublRegs, PLATO_DDR_PUBL_PTR0_OFFSET, 0x10000010);
	OSWriteHWReg32(pvPublRegs, PLATO_DDR_PUBL_PTR1_OFFSET, 0x271012c0);
	//OSWriteHWReg32(pvPublRegs, PLATO_DDR_PUBL_PTR2_OFFSET, 0x00080421);

	/* IF DDR_RISE_RISE */
	OSWriteHWReg32(pvPublRegs, PLATO_DDR_PUBL_DSGCR_OFFSET, 0x0064641F);

	OSWriteHWReg32(pvPublRegs, PLATO_DDR_PUBL_MR0_OFFSET, 0xd70);
	OSWriteHWReg32(pvPublRegs, PLATO_DDR_PUBL_MR1_OFFSET, 0x00000000);
	OSWriteHWReg32(pvPublRegs, PLATO_DDR_PUBL_MR2_OFFSET, 0x00000018);
	OSWriteHWReg32(pvPublRegs, PLATO_DDR_PUBL_MR3_OFFSET, 0x00000000);

	OSWriteHWReg32(pvPublRegs, PLATO_DDR_PUBL_DTPR0_OFFSET, 0x061c0B06);
	OSWriteHWReg32(pvPublRegs, PLATO_DDR_PUBL_DTPR1_OFFSET, 0x28200400);
	OSWriteHWReg32(pvPublRegs, PLATO_DDR_PUBL_DTPR2_OFFSET, 0x00040005);
	OSWriteHWReg32(pvPublRegs, PLATO_DDR_PUBL_DTPR3_OFFSET, 0x02000101);
	OSWriteHWReg32(pvPublRegs, PLATO_DDR_PUBL_DTPR4_OFFSET, 0x01180805);
	OSWriteHWReg32(pvPublRegs, PLATO_DDR_PUBL_DTPR5_OFFSET, 0x00250B06);
	/* IF TRAIN_DDR */
	OSWriteHWReg32(pvPublRegs, PLATO_DDR_PUBL_PGCR2_OFFSET, 0x00F09088);
	//OSWriteHWReg32(pvPublRegs, PLATO_DDR_PUBL_PGCR1_OFFSET, 0x020046A0);
#endif

	/* DISABLE VT COMPENSATION */
	//OSWriteHWReg32(pvPublRegs, PLATO_DDR_PUBL_PGCR6_OFFSET, 0x00013001);
	//OSWriteHWReg32(pvPublRegs, PLATO_DDR_PUBL_PGCR4_OFFSET, 0x40000000);
	OSWriteHWReg32(pvPublRegs, PLATO_DDR_PUBL_DXCCR_OFFSET, 0x20C01884);

	/* VREF CHANGE */
	/*
	OSWriteHWReg32(pvPublRegs, 0x0710, 0x0E00083C);
	OSWriteHWReg32(pvPublRegs, 0x0810, 0x0E00083C);
	OSWriteHWReg32(pvPublRegs, 0x0910, 0x0E00083C);
	OSWriteHWReg32(pvPublRegs, 0x0A10, 0x0E00083C);
	OSWriteHWReg32(pvPublRegs, 0x0B10, 0x0E00083C);
	OSWriteHWReg32(pvPublRegs, 0x0C10, 0x0E00083C);
	OSWriteHWReg32(pvPublRegs, 0x0D10, 0x0E00083C);
	OSWriteHWReg32(pvPublRegs, 0x0E10, 0x0E00083C);
	*/

	PLATO_CHECKPOINT;
	/*

	Phase 4: Trigger PHY initialization: Impedance, PLL, and DDL; assert PHY reset

	*/
	OSWriteHWReg32(pvPublRegs, PLATO_DDR_PUBL_PIR_OFFSET, 0x00000073);

	/*

	Phase 5: Monitor PHY initialization status by polling the PUB register PGSR0

	*/
	Poll(pvPublRegs, PLATO_DDR_PUBL_PGSR0_OFFSET, 0xF, 0xF);

	OSWriteHWReg32(pvCtrlRegs, PLATO_DDR_CTRL_SWCTL, 0x00000000);
	OSWriteHWReg32(pvCtrlRegs, PLATO_DDR_CTRL_DFIMISC, 0x00000001);
	OSWriteHWReg32(pvCtrlRegs, PLATO_DDR_CTRL_SWCTL, 0x00000001);

	PLATO_CHECKPOINT;

	/*

	Phase 6: Indicate to the PUB that the controller will perform SDRAM
	initialization by setting PIR.INIT and PIR.CTLDINIT, and
	poll PGSR0.IDONE

	*/
	OSWriteHWReg32(pvPublRegs, PLATO_DDR_PUBL_PIR_OFFSET, 0x00040001);
	Poll(pvPublRegs, PLATO_DDR_PUBL_PGSR0_OFFSET, 0x11, 0x11);

	/*

	Phase 8: Wait for DWC_ddr_umctl2 to move to "normal" operating
	mode by monitoring STAT.operating_mode signal

	*/
	PollPr(pvCtrlRegs, PLATO_DDR_CTRL_STAT, 0x01, 0x01, 10, 100);

	OSWaitus(100);

	/* IF TRAIN_DDR */
	OSWriteHWReg32(pvPublRegs, PLATO_DDR_PUBL_DTCR0_OFFSET, 0x8000B1C7);
	/*single rank only for Kingston */
	OSWriteHWReg32(pvPublRegs, PLATO_DDR_PUBL_DTCR1_OFFSET, 0x00010237);

#if defined(PLATO_DDR_BDLR_TRAINING)
	/* Bit delay line register training */
	{
		IMG_UINT8 ui8Lane = 0;
		IMG_UINT8 ui8Reg = 0;
		for (ui8Lane = 0; ui8Lane < PLATO_DDR_PUBL_DATX_LANE_COUNT; ui8Lane++)
		{
			for (ui8Reg = 0; ui8Reg < PLATO_DDR_PUBL_DXBDLR_REGS_PER_LANE; ui8Reg++)
			{
				// PVR_LOG(("- %s: BDLR Lane %d Register %d: 0x%08x", __func__, ui8Lane, ui8Reg, pvBLDRData[ui8Lane][ui8Reg]));
				OSWriteHWReg32(pvPublRegs, PLATO_DDR_PUBL_DXnBDLR_OFFSET(ui8Lane, ui8Reg), pvBLDRData[ui8Lane][ui8Reg]);
			}
		}
	}

	/* Poll on general status register 2 for each lane */
	{
		IMG_UINT8 ui8Lane;
		for (ui8Lane = 0; ui8Lane < PLATO_DDR_PUBL_DATX_LANE_COUNT; ui8Lane++)
		{
			Poll(pvPublRegs, PLATO_DDR_PUBL_DXnGSR_OFFSET(ui8Lane, 2), 0, 0x001FFFFF);
		}
	}
#endif

	OSWriteHWReg32(pvPublRegs, PLATO_DDR_PUBL_PIR_OFFSET, 0x0000ff72);
	OSWriteHWReg32(pvPublRegs, PLATO_DDR_PUBL_PIR_OFFSET, 0x0000ff73);
	Poll(pvPublRegs, PLATO_DDR_PUBL_PGSR0_OFFSET, 0x80000fff, 0xfff80fff);
	Poll(pvCtrlRegs, PLATO_DDR_CTRL_STAT, 0x1, 0x1);

	/* Setting the Anti Glitch OFF (?), Disabling On Die pullup/pulldowns */
#if defined(PLATO_DDR_KINGSTON)
	OSWriteHWReg32(pvPublRegs, PLATO_DDR_PUBL_DXCCR_OFFSET, 0x02401884);
#else
	OSWriteHWReg32(pvPublRegs, PLATO_DDR_PUBL_DXCCR_OFFSET, 0x02400004);
#endif

	if (OSReadHWReg32(pvPublRegs, PLATO_DDR_PUBL_PGSR0_OFFSET) != 0x80000fff)
	{
		PVR_DPF((PVR_DBG_ERROR, "%s: DDR Training failed", __func__));
		return PVRSRV_ERROR_INIT_FAILURE;
	}

	OSWaitus(100);

#if defined(DEBUG)
	{
		IMG_UINT8 lane = 0;
		IMG_UINT8 reg = 0;
		for (lane = 0; lane < PLATO_DDR_PUBL_DATX_LANE_COUNT; lane++)
		{
			for (reg = 0; reg < PLATO_DDR_PUBL_DXGSR_REGS_PER_LANE; reg++)
			{
				PVR_LOG(("DX%dGSR%d: 0x%08x", lane, reg,
					OSReadHWReg32(pvPublRegs, PLATO_DDR_PUBL_DXnGSR_OFFSET(lane, reg))));
			}
		}
	}
#endif

	return PVRSRV_OK;
}

#if defined(EMULATOR)
static PVRSRV_ERROR ConfigPlatoEmu(SYS_DATA *psDevData)
{
	IMG_BOOL bPollPassed;
	/* To be obtained differently */
	void* pvSystemDbgPeripRegs		= psDevData->pvSystemRegCpuVBase + SYS_PLATO_REG_PERIP_OFFSET;
	void* pvSystemDbgAonRegs		= psDevData->pvSystemRegCpuVBase + SYS_PLATO_REG_AON_OFFSET;
	void* pvSystemDbgDdrACtrlRegs	= psDevData->pvSystemRegCpuVBase + SYS_PLATO_REG_DDR_A_CTRL_OFFSET;
	void* pvSystemDbgDdrAPublRegs	= psDevData->pvSystemRegCpuVBase + SYS_PLATO_REG_DDR_A_PUBL_OFFSET;
	void* pvSystemDbgDdrBCtrlRegs	= psDevData->pvSystemRegCpuVBase + SYS_PLATO_REG_DDR_B_CTRL_OFFSET;
	void* pvSystemDbgDdrBPublRegs	= psDevData->pvSystemRegCpuVBase + SYS_PLATO_REG_DDR_B_PUBL_OFFSET;
	void* pvSystemDbgNocRegs		= psDevData->pvSystemRegCpuVBase + SYS_PLATO_REG_NOC_OFFSET;

#if defined(ENABLE_PLATO_HDMI)
	OSWriteHWReg32(pvSystemDbgPeripRegs, PLATO_TOP_CR_HDMI_CEC_CLK_CTRL, 0x3370A03);
	Poll(pvSystemDbgPeripRegs, PLATO_TOP_CR_HDMI_CEC_CLK_CTRL, 0x3370A03, 0x3370A03);
	OSWriteHWReg32(pvSystemDbgPeripRegs, PLATO_TOP_CR_I2C_CLK_CTRL, 0x1);
	Poll(pvSystemDbgPeripRegs, PLATO_TOP_CR_I2C_CLK_CTRL, 0x1, 0x1);
#endif

	OSWriteHWReg32(pvSystemDbgAonRegs, PLATO_AON_CR_NOC_CLK_CTRL, 0x1);
	Poll(pvSystemDbgAonRegs, PLATO_AON_CR_NOC_CLK_CTRL, 0x1, 0x1);

	OSWriteHWReg32(pvSystemDbgAonRegs, PLATO_AON_CR_DDR_PLL_CTRL_0, 0x01101037);
	Poll(pvSystemDbgAonRegs, PLATO_AON_CR_DDR_PLL_CTRL_0, 0x01101037, 0x01101037);
	OSWriteHWReg32(pvSystemDbgAonRegs, PLATO_AON_CR_DDR_PLL_CTRL_1, 0x00780000);
	Poll(pvSystemDbgAonRegs, PLATO_AON_CR_DDR_PLL_CTRL_1, 0x00780000, 0x00780000);

	/* Waiting for DDR PLL getting locked */
	PollPr(pvSystemDbgAonRegs, PLATO_AON_CR_PLL_STATUS, 0x2, 0x2, -1 , 10);

	OSWriteHWReg32(pvSystemDbgAonRegs, PLATO_AON_CR_PLL_BYPASS, 0x01);
	Poll(pvSystemDbgAonRegs, PLATO_AON_CR_PLL_BYPASS, 0x01, 0x01);

	OSWaitus(100);

#if defined(PLATO_DUAL_CHANNEL_DDR)
	OSWriteHWReg32(pvSystemDbgPeripRegs, PLATO_TOP_CR_DDR_CLK_CTRL, 0x011);
	Poll(pvSystemDbgPeripRegs, PLATO_TOP_CR_DDR_CLK_CTRL, 0x011, 0x011);
#else
	OSWriteHWReg32(pvSystemDbgPeripRegs, PLATO_TOP_CR_DDR_CLK_CTRL, 0x01);
	Poll(pvSystemDbgPeripRegs, PLATO_TOP_CR_DDR_CLK_CTRL, 0x01, 0x01);
#endif
	/* PLL Lock is done */

	OSWaitus(1000);

	/* Enabling PDP gated clock output - 198 MHz */

	OSWriteHWReg32(pvSystemDbgPeripRegs, PLATO_TOP_CR_PDP_CLK_CTRL, 0x00001210);
	Poll(pvSystemDbgPeripRegs, PLATO_TOP_CR_PDP_CLK_CTRL, 0x00001210, 0x00001210);

	OSWaitus(100);

	/* PDP needs HDMI clocks on for framegrabber, start them here */
	/* Enabling HDMI gated clock output - 148.5 MHz */
	OSWriteHWReg32(pvSystemDbgPeripRegs, PLATO_TOP_CR_HDMI_CLK_CTRL, 0x00001310);
	Poll(pvSystemDbgPeripRegs, PLATO_TOP_CR_HDMI_CLK_CTRL, 0x00001310, 0x00001310);

	OSWaitus(100);
	PVR_LOG(("%s: Enabled PDP and HDMI clocks", __func__));

	/* GPU PLL configuration */
	OSWriteHWReg32(pvSystemDbgAonRegs, PLATO_AON_CR_GPU_PLL_CTRL_0, 0x0110103D);
	Poll(pvSystemDbgAonRegs, PLATO_AON_CR_GPU_PLL_CTRL_0, 0x0110103D, 0x0110103D);

	OSWriteHWReg32(pvSystemDbgAonRegs, PLATO_AON_CR_GPU_PLL_CTRL_1, 0x00E00000);
	Poll(pvSystemDbgAonRegs, PLATO_AON_CR_GPU_PLL_CTRL_1, 0x00E00000, 0x00E00000);
	/* Waiting for GPU PLL getting locked */

	PollPr(pvSystemDbgAonRegs, PLATO_AON_CR_PLL_STATUS, 0x3, 0x3, -1, 10);
	/* GPU and DDR PLL Locked */

	OSWriteHWReg32(pvSystemDbgAonRegs, PLATO_AON_CR_PLL_BYPASS, 0x01);
	Poll(pvSystemDbgAonRegs, PLATO_AON_CR_PLL_BYPASS, 0x1, 0x1);

	OSWaitus(100);

	/* Enabling gated clock output */
	OSWriteHWReg32(pvSystemDbgPeripRegs, PLATO_TOP_CR_GPU_CLK_CTRL, 0x011);
	Poll(pvSystemDbgPeripRegs, PLATO_TOP_CR_GPU_CLK_CTRL, 0x011, 0x011);

	OSWaitus(1000);

#if defined(PLATO_DUAL_CHANNEL_DDR)
	/* Setting dual memory interleaved mode */
	OSWriteHWReg32(pvSystemDbgNocRegs, 0x00000050, 0x01);
	PollPr(pvSystemDbgNocRegs, 0x00000058, 0x01, 0x1, 1, 10);

	/* Now putting DRAM controller out of reset */
	OSWriteHWReg32(pvSystemDbgAonRegs, PLATO_AON_CR_RESET_CTRL, 0x0F30);
	bPollPassed = Poll(pvSystemDbgAonRegs, PLATO_AON_CR_RESET_CTRL, 0x0F30, 0x0F30);
	if (!bPollPassed)
	{
		return PVRSRV_ERROR_INIT_FAILURE;
	}
	OSWaitus(10);

	OSWriteHWReg32(pvSystemDbgAonRegs, PLATO_AON_CR_RESET_CTRL, 0x0A30);
	bPollPassed = Poll(pvSystemDbgAonRegs, PLATO_AON_CR_RESET_CTRL, 0x0A30, 0xA30);

#else
	OSWriteHWReg32(pvSystemDbgAonRegs, PLATO_AON_CR_RESET_CTRL, 0x0330);
	bPollPassed = Poll(pvSystemDbgAonRegs, PLATO_AON_CR_RESET_CTRL, 0x0330, 0x0330);
	if (!bPollPassed)
	{
		return PVRSRV_ERROR_INIT_FAILURE;
	}
	OSWaitus(10);

	/* Now putting DRAM controller into reset */
	OSWriteHWReg32(pvSystemDbgAonRegs, PLATO_AON_CR_RESET_CTRL, 0x0230);
	bPollPassed = Poll(pvSystemDbgAonRegs, PLATO_AON_CR_RESET_CTRL, 0x0230, 0x0230);

#endif
	if (!bPollPassed)
	{
		return PVRSRV_ERROR_INIT_FAILURE;
	}

	/*

	Phase 1: Program the DWC_ddr_umctl2 registers

	*/
	OSWriteHWReg32(pvSystemDbgDdrACtrlRegs, PLATO_DDR_CTRL_MSTR, 0x8F040001);
	Poll(pvSystemDbgDdrACtrlRegs, PLATO_DDR_CTRL_MSTR, 0x8F040001, 0x8F040001);

#if defined(PLATO_DUAL_CHANNEL_DDR)
	OSWriteHWReg32(pvSystemDbgDdrBCtrlRegs, PLATO_DDR_CTRL_MSTR, 0x8F040001);
	Poll(pvSystemDbgDdrBCtrlRegs, PLATO_DDR_CTRL_MSTR, 0x8F040001, 0x8F040001);
#endif

	OSWaitus(100);

	OSWriteHWReg32(pvSystemDbgDdrACtrlRegs, PLATO_DDR_CTRL_RFSHTMG, 0x007f0056);
	OSWriteHWReg32(pvSystemDbgDdrBCtrlRegs, PLATO_DDR_CTRL_RFSHTMG, 0x007f0056);

	OSWriteHWReg32(pvSystemDbgDdrACtrlRegs, PLATO_DDR_CTRL_INIT3, 0x01140000);
	OSWriteHWReg32(pvSystemDbgDdrBCtrlRegs, PLATO_DDR_CTRL_INIT3, 0x01140000);

	OSWriteHWReg32(pvSystemDbgDdrACtrlRegs, PLATO_DDR_CTRL_INIT1, 0x00010000);
	OSWriteHWReg32(pvSystemDbgDdrBCtrlRegs, PLATO_DDR_CTRL_INIT1, 0x00010000);

	OSWriteHWReg32(pvSystemDbgDdrACtrlRegs, PLATO_DDR_CTRL_INIT0, 0x00020001);
	OSWriteHWReg32(pvSystemDbgDdrBCtrlRegs, PLATO_DDR_CTRL_INIT0, 0x00020001);

	OSWriteHWReg32(pvSystemDbgDdrACtrlRegs, PLATO_DDR_CTRL_INIT4, 0x00280000);
	OSWriteHWReg32(pvSystemDbgDdrBCtrlRegs, PLATO_DDR_CTRL_INIT4, 0x00280000);

	OSWriteHWReg32(pvSystemDbgDdrACtrlRegs, PLATO_DDR_CTRL_INIT5, 0x000C000C);
	OSWriteHWReg32(pvSystemDbgDdrBCtrlRegs, PLATO_DDR_CTRL_INIT5, 0x000C000C);

	OSWriteHWReg32(pvSystemDbgDdrACtrlRegs, PLATO_DDR_CTRL_DRAMTMG0, 0x0f132312);
	OSWriteHWReg32(pvSystemDbgDdrBCtrlRegs, PLATO_DDR_CTRL_DRAMTMG0, 0x0f132312);

	OSWriteHWReg32(pvSystemDbgDdrACtrlRegs, PLATO_DDR_CTRL_DRAMTMG1, 0x00080419);
	OSWriteHWReg32(pvSystemDbgDdrBCtrlRegs, PLATO_DDR_CTRL_DRAMTMG1, 0x00080419);

	OSWriteHWReg32(pvSystemDbgDdrACtrlRegs, PLATO_DDR_CTRL_DRAMTMG2, 0x0507050b);
	OSWriteHWReg32(pvSystemDbgDdrBCtrlRegs, PLATO_DDR_CTRL_DRAMTMG2, 0x0507050b);

	OSWriteHWReg32(pvSystemDbgDdrACtrlRegs, PLATO_DDR_CTRL_DRAMTMG3, 0x00002008);
	OSWriteHWReg32(pvSystemDbgDdrBCtrlRegs, PLATO_DDR_CTRL_DRAMTMG3, 0x00002008);

	OSWriteHWReg32(pvSystemDbgDdrACtrlRegs, PLATO_DDR_CTRL_DRAMTMG4, 0x07020407);
	OSWriteHWReg32(pvSystemDbgDdrBCtrlRegs, PLATO_DDR_CTRL_DRAMTMG4, 0x07020407);

	OSWriteHWReg32(pvSystemDbgDdrACtrlRegs, PLATO_DDR_CTRL_DRAMTMG5, 0x090e0403);
	OSWriteHWReg32(pvSystemDbgDdrBCtrlRegs, PLATO_DDR_CTRL_DRAMTMG5, 0x090e0403);

	OSWriteHWReg32(pvSystemDbgDdrACtrlRegs, PLATO_DDR_CTRL_DRAMTMG6, 0x020e000e);
	OSWriteHWReg32(pvSystemDbgDdrBCtrlRegs, PLATO_DDR_CTRL_DRAMTMG6, 0x020e000e);

	OSWriteHWReg32(pvSystemDbgDdrACtrlRegs, PLATO_DDR_CTRL_DRAMTMG7, 0x00000c0e);
	OSWriteHWReg32(pvSystemDbgDdrBCtrlRegs, PLATO_DDR_CTRL_DRAMTMG7, 0x00000c0e);

	OSWriteHWReg32(pvSystemDbgDdrACtrlRegs, PLATO_DDR_CTRL_DRAMTMG8, 0x01010a05);
	OSWriteHWReg32(pvSystemDbgDdrBCtrlRegs, PLATO_DDR_CTRL_DRAMTMG8, 0x01010a05);

	OSWriteHWReg32(pvSystemDbgDdrACtrlRegs, PLATO_DDR_CTRL_ZQCTL0, 0x30ab002b);
	OSWriteHWReg32(pvSystemDbgDdrBCtrlRegs, PLATO_DDR_CTRL_ZQCTL0, 0x30ab002b);

	OSWriteHWReg32(pvSystemDbgDdrACtrlRegs, PLATO_DDR_CTRL_ZQCTL1, 0x00000070);
	OSWriteHWReg32(pvSystemDbgDdrBCtrlRegs, PLATO_DDR_CTRL_ZQCTL1, 0x00000070);

	OSWriteHWReg32(pvSystemDbgDdrACtrlRegs, PLATO_DDR_CTRL_ZQCTL2, 0x00000000);
	OSWriteHWReg32(pvSystemDbgDdrBCtrlRegs, PLATO_DDR_CTRL_ZQCTL2, 0x00000000);

	OSWriteHWReg32(pvSystemDbgDdrACtrlRegs, PLATO_DDR_CTRL_RFSHCTL0, 0x00e01020);
	OSWriteHWReg32(pvSystemDbgDdrBCtrlRegs, PLATO_DDR_CTRL_RFSHCTL0, 0x00e01020);

	OSWriteHWReg32(pvSystemDbgDdrACtrlRegs, PLATO_DDR_CTRL_RFSHCTL1, 0x0078007e);
	OSWriteHWReg32(pvSystemDbgDdrBCtrlRegs, PLATO_DDR_CTRL_RFSHCTL1, 0x0078007e);

	OSWriteHWReg32(pvSystemDbgDdrACtrlRegs, PLATO_DDR_CTRL_RFSHCTL2, 0x0057000e);
	OSWriteHWReg32(pvSystemDbgDdrBCtrlRegs, PLATO_DDR_CTRL_RFSHCTL2, 0x0057000e);

	OSWriteHWReg32(pvSystemDbgDdrACtrlRegs, PLATO_DDR_CTRL_RFSHCTL3, 0x00000000);
	OSWriteHWReg32(pvSystemDbgDdrBCtrlRegs, PLATO_DDR_CTRL_RFSHCTL3, 0x00000000);

	OSWriteHWReg32(pvSystemDbgDdrACtrlRegs, PLATO_DDR_CTRL_DFITMG0, 0x028A8208);
	OSWriteHWReg32(pvSystemDbgDdrBCtrlRegs, PLATO_DDR_CTRL_DFITMG0, 0x028A8208);

	OSWriteHWReg32(pvSystemDbgDdrACtrlRegs, PLATO_DDR_CTRL_DFITMG1, 0x00020202);
	OSWriteHWReg32(pvSystemDbgDdrBCtrlRegs, PLATO_DDR_CTRL_DFITMG1, 0x00020202);

	OSWriteHWReg32(pvSystemDbgDdrACtrlRegs, PLATO_DDR_CTRL_DFIUPD0, 0x00400003);
	OSWriteHWReg32(pvSystemDbgDdrBCtrlRegs, PLATO_DDR_CTRL_DFIUPD0, 0x00400003);

	OSWriteHWReg32(pvSystemDbgDdrACtrlRegs, PLATO_DDR_CTRL_DFIUPD1, 0x00F000FF);
	OSWriteHWReg32(pvSystemDbgDdrBCtrlRegs, PLATO_DDR_CTRL_DFIUPD1, 0x00F000FF);

	OSWriteHWReg32(pvSystemDbgDdrACtrlRegs, PLATO_DDR_CTRL_DFIUPD2, 0x80100010);
	OSWriteHWReg32(pvSystemDbgDdrBCtrlRegs, PLATO_DDR_CTRL_DFIUPD2, 0x80100010);

	OSWriteHWReg32(pvSystemDbgDdrACtrlRegs, PLATO_DDR_CTRL_DFIUPD3, 0x00100010);
	OSWriteHWReg32(pvSystemDbgDdrBCtrlRegs, PLATO_DDR_CTRL_DFIUPD3, 0x00100010);

	OSWriteHWReg32(pvSystemDbgDdrACtrlRegs, PLATO_DDR_CTRL_DFIMISC, 0x00000000);
	OSWriteHWReg32(pvSystemDbgDdrBCtrlRegs, PLATO_DDR_CTRL_DFIMISC, 0x00000000);

	OSWriteHWReg32(pvSystemDbgDdrACtrlRegs, PLATO_DDR_CTRL_ADDRMAP0, 0x00001414);
	OSWriteHWReg32(pvSystemDbgDdrACtrlRegs, PLATO_DDR_CTRL_ADDRMAP1, 0x00070707);
	OSWriteHWReg32(pvSystemDbgDdrACtrlRegs, PLATO_DDR_CTRL_ADDRMAP2, 0x00000000);
	OSWriteHWReg32(pvSystemDbgDdrACtrlRegs, PLATO_DDR_CTRL_ADDRMAP3, 0x0F000000);
	OSWriteHWReg32(pvSystemDbgDdrACtrlRegs, PLATO_DDR_CTRL_ADDRMAP4, 0x00000F0F);
	OSWriteHWReg32(pvSystemDbgDdrACtrlRegs, PLATO_DDR_CTRL_ADDRMAP5, 0x06060606);
	OSWriteHWReg32(pvSystemDbgDdrACtrlRegs, PLATO_DDR_CTRL_ADDRMAP6, 0x0F0F0606);

	OSWriteHWReg32(pvSystemDbgDdrBCtrlRegs, PLATO_DDR_CTRL_ADDRMAP0, 0x00001414);
	OSWriteHWReg32(pvSystemDbgDdrBCtrlRegs, PLATO_DDR_CTRL_ADDRMAP1, 0x00070707);
	OSWriteHWReg32(pvSystemDbgDdrBCtrlRegs, PLATO_DDR_CTRL_ADDRMAP2, 0x00000000);
	OSWriteHWReg32(pvSystemDbgDdrBCtrlRegs, PLATO_DDR_CTRL_ADDRMAP3, 0x0F000000);
	OSWriteHWReg32(pvSystemDbgDdrBCtrlRegs, PLATO_DDR_CTRL_ADDRMAP4, 0x00000F0F);
	OSWriteHWReg32(pvSystemDbgDdrBCtrlRegs, PLATO_DDR_CTRL_ADDRMAP5, 0x06060606);
	OSWriteHWReg32(pvSystemDbgDdrBCtrlRegs, PLATO_DDR_CTRL_ADDRMAP6, 0x0F0F0606);


	OSWriteHWReg32(pvSystemDbgDdrACtrlRegs, PLATO_DDR_CTRL_ODTCFG, 0x04000400);
	OSWriteHWReg32(pvSystemDbgDdrBCtrlRegs, PLATO_DDR_CTRL_ODTCFG, 0x04000400);

	OSWriteHWReg32(pvSystemDbgDdrACtrlRegs, PLATO_DDR_CTRL_SCHED, 0x00003F00);
	OSWriteHWReg32(pvSystemDbgDdrBCtrlRegs, PLATO_DDR_CTRL_SCHED, 0x00003F00);

	OSWriteHWReg32(pvSystemDbgDdrACtrlRegs, PLATO_DDR_CTRL_PERFHPR1, 0x0F0000FF);
	OSWriteHWReg32(pvSystemDbgDdrBCtrlRegs, PLATO_DDR_CTRL_PERFHPR1, 0x0F0000FF);

	OSWriteHWReg32(pvSystemDbgDdrACtrlRegs, PLATO_DDR_CTRL_PERFLPR1, 0x0F0000FF);
	OSWriteHWReg32(pvSystemDbgDdrBCtrlRegs, PLATO_DDR_CTRL_PERFLPR1, 0x0F0000FF);

	OSWriteHWReg32(pvSystemDbgDdrACtrlRegs, PLATO_DDR_CTRL_PERFWR1, 0x0F0000FF);
	OSWriteHWReg32(pvSystemDbgDdrBCtrlRegs, PLATO_DDR_CTRL_PERFWR1, 0x0F0000FF);

	/* Setup the virtual channels */
	OSWriteHWReg32(pvSystemDbgDdrACtrlRegs, 0x00000410, 0x0000000F);
	OSWriteHWReg32(pvSystemDbgDdrACtrlRegs, 0x00000414, 0x00000000);
	OSWriteHWReg32(pvSystemDbgDdrACtrlRegs, 0x00000418, 0x0000000F);
	OSWriteHWReg32(pvSystemDbgDdrACtrlRegs, 0x0000041C, 0x00000001);
	OSWriteHWReg32(pvSystemDbgDdrACtrlRegs, 0x00000420, 0x0000000F);
	OSWriteHWReg32(pvSystemDbgDdrACtrlRegs, 0x00000424, 0x00000002);
	OSWriteHWReg32(pvSystemDbgDdrACtrlRegs, 0x00000428, 0x0000000F);
	OSWriteHWReg32(pvSystemDbgDdrACtrlRegs, 0x0000042C, 0x00000003);
	OSWriteHWReg32(pvSystemDbgDdrACtrlRegs, 0x00000430, 0x0000000F);
	OSWriteHWReg32(pvSystemDbgDdrACtrlRegs, 0x00000434, 0x00000004);
	OSWriteHWReg32(pvSystemDbgDdrACtrlRegs, 0x00000438, 0x0000000F);
	OSWriteHWReg32(pvSystemDbgDdrACtrlRegs, 0x0000043C, 0x00000005);
	OSWriteHWReg32(pvSystemDbgDdrACtrlRegs, 0x00000440, 0x0000000F);
	OSWriteHWReg32(pvSystemDbgDdrACtrlRegs, 0x00000444, 0x00000006);
	OSWriteHWReg32(pvSystemDbgDdrACtrlRegs, 0x00000448, 0x0000000F);
	OSWriteHWReg32(pvSystemDbgDdrACtrlRegs, 0x0000044C, 0x00000007);
	OSWriteHWReg32(pvSystemDbgDdrACtrlRegs, 0x00000450, 0x0000000F);
	OSWriteHWReg32(pvSystemDbgDdrACtrlRegs, 0x00000454, 0x00000008);
	OSWriteHWReg32(pvSystemDbgDdrACtrlRegs, 0x00000458, 0x0000000F);
	OSWriteHWReg32(pvSystemDbgDdrACtrlRegs, 0x0000045C, 0x00000009);
	OSWriteHWReg32(pvSystemDbgDdrACtrlRegs, 0x00000460, 0x0000000F);
	OSWriteHWReg32(pvSystemDbgDdrACtrlRegs, 0x00000464, 0x0000000A);
	OSWriteHWReg32(pvSystemDbgDdrACtrlRegs, 0x00000468, 0x0000000F);
	OSWriteHWReg32(pvSystemDbgDdrACtrlRegs, 0x0000046C, 0x0000000B);
	OSWriteHWReg32(pvSystemDbgDdrACtrlRegs, 0x00000470, 0x0000000F);
	OSWriteHWReg32(pvSystemDbgDdrACtrlRegs, 0x00000474, 0x0000000C);
	OSWriteHWReg32(pvSystemDbgDdrACtrlRegs, 0x00000478, 0x0000000F);
	OSWriteHWReg32(pvSystemDbgDdrACtrlRegs, 0x0000047C, 0x0000000D);
	OSWriteHWReg32(pvSystemDbgDdrACtrlRegs, 0x00000480, 0x0000000F);
	OSWriteHWReg32(pvSystemDbgDdrACtrlRegs, 0x00000484, 0x0000000E);

	OSWriteHWReg32(pvSystemDbgDdrBCtrlRegs, 0x00000410, 0x0000000F);
	OSWriteHWReg32(pvSystemDbgDdrBCtrlRegs, 0x00000414, 0x00000000);
	OSWriteHWReg32(pvSystemDbgDdrBCtrlRegs, 0x00000418, 0x0000000F);
	OSWriteHWReg32(pvSystemDbgDdrBCtrlRegs, 0x0000041C, 0x00000001);
	OSWriteHWReg32(pvSystemDbgDdrBCtrlRegs, 0x00000420, 0x0000000F);
	OSWriteHWReg32(pvSystemDbgDdrBCtrlRegs, 0x00000424, 0x00000002);
	OSWriteHWReg32(pvSystemDbgDdrBCtrlRegs, 0x00000428, 0x0000000F);
	OSWriteHWReg32(pvSystemDbgDdrBCtrlRegs, 0x0000042C, 0x00000003);
	OSWriteHWReg32(pvSystemDbgDdrBCtrlRegs, 0x00000430, 0x0000000F);
	OSWriteHWReg32(pvSystemDbgDdrBCtrlRegs, 0x00000434, 0x00000004);
	OSWriteHWReg32(pvSystemDbgDdrBCtrlRegs, 0x00000438, 0x0000000F);
	OSWriteHWReg32(pvSystemDbgDdrBCtrlRegs, 0x0000043C, 0x00000005);
	OSWriteHWReg32(pvSystemDbgDdrBCtrlRegs, 0x00000440, 0x0000000F);
	OSWriteHWReg32(pvSystemDbgDdrBCtrlRegs, 0x00000444, 0x00000006);
	OSWriteHWReg32(pvSystemDbgDdrBCtrlRegs, 0x00000448, 0x0000000F);
	OSWriteHWReg32(pvSystemDbgDdrBCtrlRegs, 0x0000044C, 0x00000007);
	OSWriteHWReg32(pvSystemDbgDdrBCtrlRegs, 0x00000450, 0x0000000F);
	OSWriteHWReg32(pvSystemDbgDdrBCtrlRegs, 0x00000454, 0x00000008);
	OSWriteHWReg32(pvSystemDbgDdrBCtrlRegs, 0x00000458, 0x0000000F);
	OSWriteHWReg32(pvSystemDbgDdrBCtrlRegs, 0x0000045C, 0x00000009);
	OSWriteHWReg32(pvSystemDbgDdrBCtrlRegs, 0x00000460, 0x0000000F);
	OSWriteHWReg32(pvSystemDbgDdrBCtrlRegs, 0x00000464, 0x0000000A);
	OSWriteHWReg32(pvSystemDbgDdrBCtrlRegs, 0x00000468, 0x0000000F);
	OSWriteHWReg32(pvSystemDbgDdrBCtrlRegs, 0x0000046C, 0x0000000B);
	OSWriteHWReg32(pvSystemDbgDdrBCtrlRegs, 0x00000470, 0x0000000F);
	OSWriteHWReg32(pvSystemDbgDdrBCtrlRegs, 0x00000474, 0x0000000C);
	OSWriteHWReg32(pvSystemDbgDdrBCtrlRegs, 0x00000478, 0x0000000F);
	OSWriteHWReg32(pvSystemDbgDdrBCtrlRegs, 0x0000047C, 0x0000000D);
	OSWriteHWReg32(pvSystemDbgDdrBCtrlRegs, 0x00000480, 0x0000000F);
	OSWriteHWReg32(pvSystemDbgDdrBCtrlRegs, 0x00000484, 0x0000000E);

	/* DRAM controller registers configuration done */

	OSWaitus(100);

	/*

	Phase 2: Deassert soft reset signal core_ddrc_rstn

	*/

	/* Now getting DRAM controller out of reset */
#if defined(PLATO_DUAL_CHANNEL_DDR)
	OSWriteHWReg32(pvSystemDbgAonRegs, PLATO_AON_CR_RESET_CTRL, 0x00000F30);
#else
	OSWriteHWReg32(pvSystemDbgAonRegs, PLATO_AON_CR_RESET_CTRL, 0x00000330);
#endif

	OSWaitus(100);

	/* ECC disable */
	OSWriteHWReg32(pvSystemDbgDdrACtrlRegs, PLATO_DDR_CTRL_DBG1, 0x00000000);
	/* power related */
	OSWriteHWReg32(pvSystemDbgDdrACtrlRegs, PLATO_DDR_CTRL_PWRCTL, 0x00000000);
	/* Enabling AXI input port (Port control) */
	OSWriteHWReg32(pvSystemDbgDdrACtrlRegs, PLATO_DDR_CTRL_PCTRL, 0x00000001);

	OSWriteHWReg32(pvSystemDbgDdrBCtrlRegs, PLATO_DDR_CTRL_DBG1, 0x00000000);
	OSWriteHWReg32(pvSystemDbgDdrBCtrlRegs, PLATO_DDR_CTRL_PWRCTL, 0x00000000);
	OSWriteHWReg32(pvSystemDbgDdrBCtrlRegs, PLATO_DDR_CTRL_PCTRL, 0x00000001);

	/*

	Phase 7: Set DFIMISC.dfi_init_complete_en to 1

	(skipped on emu?)
	*/

	/*

	Phase 3: Start PHY initialization by accessing relevant PUB registers

	*/
	OSWriteHWReg32(pvSystemDbgDdrAPublRegs, PLATO_DDR_PUBL_DCR_OFFSET, 0x0000040B);
	OSWriteHWReg32(pvSystemDbgDdrBPublRegs, PLATO_DDR_PUBL_DCR_OFFSET, 0x0000040B);

	OSWriteHWReg32(pvSystemDbgDdrAPublRegs, PLATO_DDR_PUBL_PTR0_OFFSET, 0x10000010);
	OSWriteHWReg32(pvSystemDbgDdrBPublRegs, PLATO_DDR_PUBL_PTR0_OFFSET, 0x10000010);

	OSWriteHWReg32(pvSystemDbgDdrAPublRegs, PLATO_DDR_PUBL_PTR1_OFFSET, 0x00800080);
	OSWriteHWReg32(pvSystemDbgDdrBPublRegs, PLATO_DDR_PUBL_PTR1_OFFSET, 0x00800080);

	OSWriteHWReg32(pvSystemDbgDdrAPublRegs, PLATO_DDR_PUBL_PTR2_OFFSET, 0x00080421);
	OSWriteHWReg32(pvSystemDbgDdrBPublRegs, PLATO_DDR_PUBL_PTR2_OFFSET, 0x00080421);

	OSWriteHWReg32(pvSystemDbgDdrAPublRegs, PLATO_DDR_PUBL_DSGCR_OFFSET, 0x0020641F);
	OSWriteHWReg32(pvSystemDbgDdrBPublRegs, PLATO_DDR_PUBL_DSGCR_OFFSET, 0x0020641F);

	OSWriteHWReg32(pvSystemDbgDdrAPublRegs, PLATO_DDR_PUBL_MR0_OFFSET, 0x00000114);
	OSWriteHWReg32(pvSystemDbgDdrAPublRegs, PLATO_DDR_PUBL_MR1_OFFSET, 0x00000000);
	OSWriteHWReg32(pvSystemDbgDdrAPublRegs, PLATO_DDR_PUBL_MR2_OFFSET, 0x00000028);
	OSWriteHWReg32(pvSystemDbgDdrAPublRegs, PLATO_DDR_PUBL_MR3_OFFSET, 0x00000000);

	OSWriteHWReg32(pvSystemDbgDdrBPublRegs, PLATO_DDR_PUBL_MR0_OFFSET, 0x00000114);
	OSWriteHWReg32(pvSystemDbgDdrBPublRegs, PLATO_DDR_PUBL_MR1_OFFSET, 0x00000000);
	OSWriteHWReg32(pvSystemDbgDdrBPublRegs, PLATO_DDR_PUBL_MR2_OFFSET, 0x00000028);
	OSWriteHWReg32(pvSystemDbgDdrBPublRegs, PLATO_DDR_PUBL_MR3_OFFSET, 0x00000000);

	OSWriteHWReg32(pvSystemDbgDdrAPublRegs, PLATO_DDR_PUBL_DTPR0_OFFSET, 0x040F0406);
	OSWriteHWReg32(pvSystemDbgDdrBPublRegs, PLATO_DDR_PUBL_DTPR0_OFFSET, 0x040F0406);

	OSWriteHWReg32(pvSystemDbgDdrAPublRegs, PLATO_DDR_PUBL_DTPR1_OFFSET, 0x28110402);
	OSWriteHWReg32(pvSystemDbgDdrBPublRegs, PLATO_DDR_PUBL_DTPR1_OFFSET, 0x28110402);

	OSWriteHWReg32(pvSystemDbgDdrAPublRegs, PLATO_DDR_PUBL_DTPR2_OFFSET, 0x00030002);
	OSWriteHWReg32(pvSystemDbgDdrBPublRegs, PLATO_DDR_PUBL_DTPR2_OFFSET, 0x00030002);

	OSWriteHWReg32(pvSystemDbgDdrAPublRegs, PLATO_DDR_PUBL_DTPR3_OFFSET, 0x02000101);
	OSWriteHWReg32(pvSystemDbgDdrBPublRegs, PLATO_DDR_PUBL_DTPR3_OFFSET, 0x02000101);

	OSWriteHWReg32(pvSystemDbgDdrAPublRegs, PLATO_DDR_PUBL_DTPR4_OFFSET, 0x00190602);
	OSWriteHWReg32(pvSystemDbgDdrBPublRegs, PLATO_DDR_PUBL_DTPR4_OFFSET, 0x00190602);

	OSWriteHWReg32(pvSystemDbgDdrAPublRegs, PLATO_DDR_PUBL_DTPR5_OFFSET, 0x0018040B);
	OSWriteHWReg32(pvSystemDbgDdrBPublRegs, PLATO_DDR_PUBL_DTPR5_OFFSET, 0x0018040B);

	OSWriteHWReg32(pvSystemDbgDdrAPublRegs, PLATO_DDR_PUBL_PGCR1_OFFSET, 0x020046A0);
	OSWriteHWReg32(pvSystemDbgDdrBPublRegs, PLATO_DDR_PUBL_PGCR1_OFFSET, 0x020046A0);

	/*

	Phase 4: Trigger PHY initialization: Impedance, PLL, and DDL; assert PHY reset

	*/
	OSWriteHWReg32(pvSystemDbgDdrAPublRegs, PLATO_DDR_PUBL_PIR_OFFSET, 0x00000073);
	OSWriteHWReg32(pvSystemDbgDdrBPublRegs, PLATO_DDR_PUBL_PIR_OFFSET, 0x00000073);

	/*

	Phase 5: Monitor PHY initialization status by polling the PUB register PGSR0

	*/
	OSWriteHWReg32(pvSystemDbgDdrACtrlRegs, PLATO_DDR_CTRL_SWCTL, 0x00000000);
	OSWriteHWReg32(pvSystemDbgDdrBCtrlRegs, PLATO_DDR_CTRL_SWCTL, 0x00000000);

	OSWriteHWReg32(pvSystemDbgDdrACtrlRegs, PLATO_DDR_CTRL_DFIMISC, 0x00000001);
	OSWriteHWReg32(pvSystemDbgDdrBCtrlRegs, PLATO_DDR_CTRL_DFIMISC, 0x00000001);

	OSWriteHWReg32(pvSystemDbgDdrACtrlRegs, PLATO_DDR_CTRL_SWCTL, 0x00000001);
	OSWriteHWReg32(pvSystemDbgDdrBCtrlRegs, PLATO_DDR_CTRL_SWCTL, 0x00000001);

	/*

	Phase 6: Indicate to the PUB that the controller will perform SDRAM
	initialization by setting PIR.INIT and PIR.CTLDINIT, and
	poll PGSR0.IDONE

	*/
	OSWriteHWReg32(pvSystemDbgDdrAPublRegs, PLATO_DDR_PUBL_PIR_OFFSET, 0x00040001);
	OSWriteHWReg32(pvSystemDbgDdrBPublRegs, PLATO_DDR_PUBL_PIR_OFFSET, 0x00040001);

	/*

	Phase 8: Wait for DWC_ddr_umctl2 to move to "normal" operating
	mode by monitoring STAT.operating_mode signal

	*/
	PollPr(pvSystemDbgDdrACtrlRegs, PLATO_DDR_CTRL_STAT, 0x01, 0x01, 10, 100);
#if defined(PLATO_DUAL_CHANNEL_DDR)
	PollPr(pvSystemDbgDdrBCtrlRegs, PLATO_DDR_CTRL_STAT, 0x01, 0x01, 10, 100);
#endif
	OSWaitus(100);

	/* Getting GPU And DDR A out of reset */
#if defined(PLATO_DUAL_CHANNEL_DDR)
	OSWriteHWReg32(pvSystemDbgAonRegs, PLATO_AON_CR_RESET_CTRL, 0x00000F12);
	bPollPassed = PollPr(pvSystemDbgAonRegs, PLATO_AON_CR_RESET_CTRL, 0x00000F12, 0x00000F12, -1, 100);
#else
	OSWriteHWReg32(pvSystemDbgAonRegs, PLATO_AON_CR_RESET_CTRL, 0x00000312);
	bPollPassed = PollPr(pvSystemDbgAonRegs, PLATO_AON_CR_RESET_CTRL, 0x00000312, 0x00000312, -1, 100);
#endif
	if (!bPollPassed)
	{
		return PVRSRV_ERROR_INIT_FAILURE;
	}
	/* setting CR_ISO_CTRL:CR_GPU_CLK_E */
	OSWriteHWReg32(pvSystemDbgAonRegs, PLATO_AON_CR_ISO_CTRL, 0x000001F);

	return PVRSRV_OK;
}

#else // HW

static PVRSRV_ERROR CheckPDPAccessibility(SYS_DATA *psDevData)
{
	#define PLATO_PDP_REGISTER_CORE_ID_OFFSET (0x04e0)
	#define VALID_PDP_CORE_ID (0x7010003)

	void* pvPDPRegs;
	IMG_UINT32 ui32CoreId;

	if (PlatoMapRegisters(psDevData, SYS_PLATO_REG_PCI_BASENUM, SYS_PLATO_REG_PDP_OFFSET,
		SYS_PLATO_REG_PDP_SIZE, &pvPDPRegs) != PVRSRV_OK || pvPDPRegs == NULL)
	{
		return PVRSRV_ERROR_DC_DEVICE_INACCESSIBLE;
	}

	ui32CoreId = OSReadHWReg32(pvPDPRegs, PLATO_PDP_REGISTER_CORE_ID_OFFSET);
	if (ui32CoreId != VALID_PDP_CORE_ID)
	{
		PVR_LOG(("PDP Access failed, expected 0x%x but got 0x%x", VALID_PDP_CORE_ID, ui32CoreId));
		PlatoUnmapRegisters(psDevData, pvPDPRegs, SYS_PLATO_REG_PCI_BASENUM,
			SYS_PLATO_REG_PDP_OFFSET, SYS_PLATO_REG_PDP_SIZE);
		return PVRSRV_ERROR_DC_DEVICE_INACCESSIBLE;
	}

	PlatoUnmapRegisters(psDevData, pvPDPRegs, SYS_PLATO_REG_PCI_BASENUM,
		SYS_PLATO_REG_PDP_OFFSET, SYS_PLATO_REG_PDP_SIZE);
	return PVRSRV_OK;
}

/*
 * Helpers for getting integer and fractional values which are needed when
 * programming PLATO_AON_CR_{GPU|DDR}_PLL_CTRL_{0|1} registers for DDR/GPU PLLs.
 *
 * PLL_CLOCK_SPEED = (PLL_REF_CLOCK_SPEED * REFDIV) *
 *                   (PLL_INT + (PLL_FRAC / 2^24)) / POSTDIV1 / POSTDIV2
 *
 * NOTE: It's assumed that REFDIV, POSTDIV1 and POSTDIV2 are '1' in all cases.
 */
static IMG_UINT32 GetPlatoPLLInt(IMG_UINT32 ui32PLLClock)
{
	return ui32PLLClock / PLATO_PLL_REF_CLOCK_SPEED;
}

static IMG_UINT32 GetPlatoPLLFrac(IMG_UINT32 ui32PLLClock)
{
	/* Shift to get 24 bits for fractional part after div */
	IMG_UINT64 shift = (IMG_UINT64)ui32PLLClock << 24;

	/* Div and return only fractional part of the result */
	return (shift / PLATO_PLL_REF_CLOCK_SPEED) & ((1 << 24) - 1);
}

/*
 * Helper for getting value of integer divider for GPU clock.
 */
static IMG_UINT32 GetPlatoGPUVDiv0(IMG_UINT32 ui32PLLClock, IMG_UINT32 ui32CoreClockSpeed)
{
	IMG_UINT32 div, ret;

	div = ui32PLLClock / ui32CoreClockSpeed;

	/* Bias the result by (-1) with saturation, then clip it */
	ret = (div - (div > 0)) & (PLATO_CR_GPUV_DIV_0_MASK >> PLATO_CR_GPUV_DIV_0_SHIFT);

	/* Check for lost result after clipping, saturate if so */
	return (div > 1) && (ret != (div - (div > 0))) ?
			(PLATO_CR_GPUV_DIV_0_MASK >> PLATO_CR_GPUV_DIV_0_SHIFT) : ret;
}

/*
 * Helpers for getting values of integer dividers for PDP clock.
 *
 * NOTE: Use only if PLL clock speed > ~350 MHz.
 */
static IMG_UINT32 GetPlatoPDPV0Div0(IMG_UINT32 ui32PLLClock)
{
	IMG_UINT32 div, ret;

	div = ui32PLLClock / PLATO_MIN_PDP_CLOCK_SPEED;

	/* Bias the result by (-1) with saturation, then clip it */
	ret = (div - (div > 0)) & (PLATO_CR_PDPV0_DIV_0_MASK >> PLATO_CR_PDPV0_DIV_0_SHIFT);

	/* Check for lost result after clipping, saturate if so */
	return (div > 1) && (ret != (div - (div > 0))) ?
			(PLATO_CR_PDPV0_DIV_0_MASK >> PLATO_CR_PDPV0_DIV_0_SHIFT) : ret;
}

static IMG_UINT32 GetPlatoPDPV1Div0(IMG_UINT32 ui32PLLClock)
{
	IMG_UINT32 div, ret;

	div = (ui32PLLClock / (GetPlatoPDPV0Div0(ui32PLLClock) + 1)) / PLATO_MIN_PDP_CLOCK_SPEED;

	/* Bias the result by (-1) with saturation, then clip it */
	ret = (div - (div > 0)) & (PLATO_CR_PDPV1_DIV_0_MASK >> PLATO_CR_PDPV1_DIV_0_SHIFT);

	/* Check for lost result after clipping, saturate if so */
	return (div > 1) && (ret != (div - (div > 0))) ?
			(PLATO_CR_PDPV1_DIV_0_MASK >> PLATO_CR_PDPV1_DIV_0_SHIFT) : ret;
}

#if defined(ENABLE_PLATO_HDMI)
/*
 * Helpers for getting values of integer dividers for HDMICEC clocks.
 *
 * NOTE: They strive to get clock speed of HDMI-SFR as close to
 *       27 MHz as possible.
 */
static IMG_UINT32 GetPlatoHDMICECV0Div0(IMG_UINT32 ui32PLLClock)
{
    IMG_UINT32 ui32HDMICECV0Div0, ui32HDMICECV0Div0Limit;
    IMG_UINT32 ui32HDMICECV1Div0, ui32HDMICECV1Div0Limit;
    IMG_UINT32 ui32HDMISFRClockSpeed;

    ui32HDMICECV0Div0Limit = PLATO_CR_HDMICECV0_DIV_0_MASK >> PLATO_CR_HDMICECV0_DIV_0_SHIFT;
    ui32HDMICECV1Div0Limit = PLATO_CR_HDMICECV1_DIV_0_MASK >> PLATO_CR_HDMICECV1_DIV_0_SHIFT;

    ui32HDMICECV0Div0 = 0;
    while (ui32HDMICECV0Div0 < ui32HDMICECV0Div0Limit)
    {
        ui32HDMICECV1Div0 = 0;
        while (ui32HDMICECV1Div0 < ui32HDMICECV1Div0Limit)
        {
            ui32HDMISFRClockSpeed = ui32PLLClock / (ui32HDMICECV0Div0 + 1) / (ui32HDMICECV1Div0 + 1);

            if (ui32HDMISFRClockSpeed <= PLATO_TARGET_HDMI_SFR_CLOCK_SPEED)
            {
                /* Done, value of the divider found */
                return ui32HDMICECV0Div0;
            }

            ui32HDMICECV1Div0++;
        }

        ui32HDMICECV0Div0++;
    }

    /* Here the function returns the highest possible value of the divider */
    return ui32HDMICECV0Div0;
}

static IMG_UINT32 GetPlatoHDMICECV1Div0(IMG_UINT32 ui32PLLClock)
{
    IMG_UINT64 div, ret;

    /* Calculate the divider using 32.32 fixed point math */
    div = (IMG_UINT64)ui32PLLClock << 32;
    div /= GetPlatoHDMICECV0Div0(ui32PLLClock) + 1;
    div /= PLATO_TARGET_HDMI_SFR_CLOCK_SPEED;

    /* Round up if the fractional part is present */
    div = (div >> 32) + ((div & 0xFFFFFFFF) > 0);

    /* Bias the result by (-1) with saturation, then clip it */
    ret = (div - (div > 0)) & (PLATO_CR_HDMICECV1_DIV_0_MASK >> PLATO_CR_HDMICECV1_DIV_0_SHIFT);

    /* Check for lost result after clipping, saturate if so */
    return (div > 1) && (ret != (div - (div > 0))) ?
            (PLATO_CR_HDMICECV1_DIV_0_MASK >> PLATO_CR_HDMICECV1_DIV_0_SHIFT) : ret;
}

static IMG_UINT32 GetPlatoHDMICECV2Div0(IMG_UINT32 ui32PLLClock)
{
    IMG_UINT64 div, ret;

    /* Calculate the divider using 32.32 fixed point math */
    div = (IMG_UINT64)ui32PLLClock << 32;
    div /= GetPlatoHDMICECV0Div0(ui32PLLClock) + 1;
    div /= GetPlatoHDMICECV1Div0(ui32PLLClock) + 1;
    div /= PLATO_TARGET_HDMI_CEC_CLOCK_SPEED;

    /* Round up if the fractional part is present */
    div = (div >> 32) + ((div & 0xFFFFFFFF) > 0);

    /* Bias the result by (-1) with saturation, then clip it */
    ret = (div - (div > 0)) & (PLATO_CR_HDMICECV2_DIV_0_MASK >> PLATO_CR_HDMICECV2_DIV_0_SHIFT);

    /* Check for lost result after clipping, saturate if so */
    return (div > 1) && (ret != (div - (div > 0))) ?
            (PLATO_CR_HDMICECV2_DIV_0_MASK >> PLATO_CR_HDMICECV2_DIV_0_SHIFT) : ret;
}
#endif

#if defined(PLATO_DUAL_CHANNEL_DDR)

static PVRSRV_ERROR ConfigPlatoDualChannel(SYS_DATA *psDevData)
{
	PVRSRV_ERROR eError = PVRSRV_OK;
	IMG_BOOL bPollPassed;

	void* pvSystemDbgPeripRegs	  = psDevData->pvSystemRegCpuVBase + SYS_PLATO_REG_PERIP_OFFSET;
	void* pvSystemDbgAonRegs	  = psDevData->pvSystemRegCpuVBase + SYS_PLATO_REG_AON_OFFSET;
	void* pvSystemDbgDdrACtrlRegs = psDevData->pvSystemRegCpuVBase + SYS_PLATO_REG_DDR_A_CTRL_OFFSET;
	void* pvSystemDbgDdrAPublRegs = psDevData->pvSystemRegCpuVBase + SYS_PLATO_REG_DDR_A_PUBL_OFFSET;
	void* pvSystemDbgDdrBCtrlRegs = psDevData->pvSystemRegCpuVBase + SYS_PLATO_REG_DDR_B_CTRL_OFFSET;
	void* pvSystemDbgDdrBPublRegs = psDevData->pvSystemRegCpuVBase + SYS_PLATO_REG_DDR_B_PUBL_OFFSET;
	void* pvSystemDbgNocRegs	  = psDevData->pvSystemRegCpuVBase + SYS_PLATO_REG_NOC_OFFSET;

	IMG_UINT32 ui32BDLRSetupDddrA[PLATO_DDR_PUBL_DATX_LANE_COUNT][PLATO_DDR_PUBL_DXBDLR_REGS_PER_LANE] =
		{{0x0F0F0F0F, 0x0F0F0F0F, 0x000F0F0F},
		 {0x0F0F0F0F, 0x0F0F0F0F, 0x000F0F0F},
		 {0x14141414, 0x14141414, 0x00141414},
		 {0x0F0F0F0F, 0x0F0F0F0F, 0x000F0F0F},
		 {0x14141414, 0x14141414, 0x00141414},
		 {0x14141414, 0x14141414, 0x00141414},
		 {0x0F0F0F0F, 0x0F0F0F0F, 0x000F0F0F},
		 {0x0F0F0F0F, 0x0F0F0F0F, 0x000F0F0F}};
	IMG_UINT32 ui32BDLRSetupDddrB[PLATO_DDR_PUBL_DATX_LANE_COUNT][PLATO_DDR_PUBL_DXBDLR_REGS_PER_LANE] =
		{{0x0F0F0F0F, 0x0F0F0F0F, 0x000F0F0F},
		 {0x14141414, 0x14141414, 0x00141414},
		 {0x14141414, 0x14141414, 0x00141414},
		 {0x0F0F0F0F, 0x0F0F0F0F, 0x000F0F0F},
		 {0x0F0F0F0F, 0x0F0F0F0F, 0x000F0F0F},
		 {0x14141414, 0x14141414, 0x00141414},
		 {0x14141414, 0x14141414, 0x00141414},
		 {0x0F0F0F0F, 0x0F0F0F0F, 0x000F0F0F}};

	IMG_UINT32 ui32MemClockSpeed, ui32MemPLLClockSpeed;
	IMG_UINT32 ui32MemClockPLLCtrl0, ui32MemClockPLLCtrl1, ui32MemClockCtrl;

	IMG_UINT32 ui32CoreClockSpeed, ui32CorePLLClockSpeed;
	IMG_UINT32 ui32CoreClockPLLCtrl0, ui32CoreClockPLLCtrl1, ui32CoreClockCtrl;

#if defined(ENABLE_PLATO_HDMI)
	IMG_UINT32 ui32HDMICECClockCtrl;
#endif

	IMG_UINT32 ui32PDPClockCtrl, ui32HDMIClockCtrl;

	/* Plato Soft reset */
	OSWriteHWReg32(pvSystemDbgAonRegs, PLATO_AON_CR_RESET_CTRL, 0x40);
	OSSleepms(100);

	/* Temporary fix for non-32GB PCI BAR Plato boards that seem to not signal getting out reset */
	if (!PLATO_HAS_NON_MAPPABLE(psDevData))
	{
		bPollPassed = Poll(pvSystemDbgAonRegs, PLATO_AON_CR_RESET_CTRL, 0x030, 0xF0);
		if (!bPollPassed)
		{
			return PVRSRV_ERROR_INIT_FAILURE;
		}
	}

	/* On non-32GB PCI BAR Plato boards, bring display subsystem out of reset with PLL bypassed. */
	if (PLATO_HAS_NON_MAPPABLE(psDevData))
	{
		OSWriteHWReg32(pvSystemDbgAonRegs, PLATO_AON_CR_PLL_BYPASS, 0x0);
		OSWriteHWReg32(pvSystemDbgAonRegs, PLATO_AON_CR_RESET_CTRL, PLATO_CR_DISPLAY_RESET_MASK);
		OSWriteHWReg32(pvSystemDbgAonRegs, PLATO_AON_CR_PLL_BYPASS, 0x1);
	}

	/* Setting dual memory interleaved mode */
	OSWriteHWReg32(pvSystemDbgNocRegs, 0x00000050, 0x01);
	PollPr(pvSystemDbgNocRegs, 0x00000058, 0x01, 0x1, 1, 10);

	OSWriteHWReg32(pvSystemDbgAonRegs, PLATO_AON_CR_NOC_CLK_CTRL, 0x1);
	Poll(pvSystemDbgAonRegs, PLATO_AON_CR_NOC_CLK_CTRL, 0x1, 0x1);

	/* Setup DDR PLL's */
	ui32MemClockSpeed = SysGetPlatoMemClockSpeed();
	ui32MemPLLClockSpeed = ui32MemClockSpeed;

	ui32MemClockPLLCtrl0 = (GetPlatoPLLInt(ui32MemPLLClockSpeed) << PLATO_CR_DDR_PLL_FBDIV_SHIFT);
	ui32MemClockPLLCtrl0 &= PLATO_CR_DDR_PLL_FBDIV_MASK;
	ui32MemClockPLLCtrl0 |= (1 << PLATO_CR_DDR_PLL_REFDIV_SHIFT);
	ui32MemClockPLLCtrl0 |= (1 << PLATO_CR_DDR_PLL_POSTDIV1_SHIFT);
	ui32MemClockPLLCtrl0 |= (1 << PLATO_CR_DDR_PLL_POSTDIV2_SHIFT);

	ui32MemClockPLLCtrl1 = (GetPlatoPLLFrac(ui32MemPLLClockSpeed) << PLATO_CR_DDR_PLL_FRAC_SHIFT);
	ui32MemClockPLLCtrl1 &= PLATO_CR_DDR_PLL_FRAC_MASK;

	OSWriteHWReg32(pvSystemDbgAonRegs, PLATO_AON_CR_DDR_PLL_CTRL_0, ui32MemClockPLLCtrl0);
	Poll(pvSystemDbgAonRegs, PLATO_AON_CR_DDR_PLL_CTRL_0, ui32MemClockPLLCtrl0, ui32MemClockPLLCtrl0);

	OSWriteHWReg32(pvSystemDbgAonRegs, PLATO_AON_CR_DDR_PLL_CTRL_1, ui32MemClockPLLCtrl1);
	Poll(pvSystemDbgAonRegs, PLATO_AON_CR_DDR_PLL_CTRL_1, ui32MemClockPLLCtrl1, ui32MemClockPLLCtrl1);

	PVR_LOG(("- %s: DDR clock: %u Hz (%u MHz)",
			 __func__, ui32MemClockSpeed, ui32MemClockSpeed/1000000));

	/* Setup GPU PLL's */
	ui32CoreClockSpeed = SysGetPlatoCoreClockSpeed();
	ui32CorePLLClockSpeed = SysGetPlatoPLLClockSpeed(ui32CoreClockSpeed);

	ui32CoreClockPLLCtrl0 = (GetPlatoPLLInt(ui32CorePLLClockSpeed) << PLATO_CR_GPU_PLL_FBDIV_SHIFT);
	ui32CoreClockPLLCtrl0 &= PLATO_CR_GPU_PLL_FBDIV_MASK;
	ui32CoreClockPLLCtrl0 |= (1 << PLATO_CR_GPU_PLL_REFDIV_SHIFT);
	ui32CoreClockPLLCtrl0 |= (1 << PLATO_CR_GPU_PLL_POSTDIV1_SHIFT);
	ui32CoreClockPLLCtrl0 |= (1 << PLATO_CR_GPU_PLL_POSTDIV2_SHIFT);

	ui32CoreClockPLLCtrl1 = (GetPlatoPLLFrac(ui32CorePLLClockSpeed) << PLATO_CR_GPU_PLL_FRAC_SHIFT);
	ui32CoreClockPLLCtrl1 &= PLATO_CR_GPU_PLL_FRAC_MASK;

	OSWriteHWReg32(pvSystemDbgAonRegs, PLATO_AON_CR_GPU_PLL_CTRL_0, ui32CoreClockPLLCtrl0);
	Poll(pvSystemDbgAonRegs, PLATO_AON_CR_GPU_PLL_CTRL_0, ui32CoreClockPLLCtrl0, ui32CoreClockPLLCtrl0);

	OSWriteHWReg32(pvSystemDbgAonRegs, PLATO_AON_CR_GPU_PLL_CTRL_1, ui32CoreClockPLLCtrl1);
	Poll(pvSystemDbgAonRegs, PLATO_AON_CR_GPU_PLL_CTRL_1, ui32CoreClockPLLCtrl1, ui32CoreClockPLLCtrl1);

	PVR_LOG(("- %s: GPU clock: %u Hz (%u MHz)",
			 __func__, ui32CoreClockSpeed,
			 ui32CoreClockSpeed / 1000000));
	PVR_LOG(("- %s: GPU-PLL clock: %u Hz (%u MHz)",
			 __func__, ui32CorePLLClockSpeed,
			 ui32CorePLLClockSpeed / 1000000));

#if defined(ENABLE_PLATO_HDMI)
	{
		IMG_UINT32 ui32HdmiCecV0 = GetPlatoHDMICECV0Div0(ui32CorePLLClockSpeed);
		IMG_UINT32 ui32HdmiCecV1 = GetPlatoHDMICECV1Div0(ui32CorePLLClockSpeed);
		IMG_UINT32 ui32HdmiCecV2 = GetPlatoHDMICECV2Div0(ui32CorePLLClockSpeed);

		PVR_LOG(("- %s: Setting HDMI CEC dividers:", __func__));
		PVR_LOG(("- %s:		Hdmi CEC div0: %u(+1)", __func__, ui32HdmiCecV0));
		PVR_LOG(("- %s:		Hdmi CEC div1: %u(+1)", __func__, ui32HdmiCecV1));
		PVR_LOG(("- %s:		Hdmi CEC div2: %u(+1)", __func__, ui32HdmiCecV2));

		/* Setup HDMI CEC clock outputs */
		ui32HDMICECClockCtrl = 0;
		ui32HDMICECClockCtrl |= (ui32HdmiCecV0 << PLATO_CR_HDMICECV0_DIV_0_SHIFT);
		ui32HDMICECClockCtrl |= (ui32HdmiCecV1 << PLATO_CR_HDMICECV1_DIV_0_SHIFT);
		ui32HDMICECClockCtrl |= (ui32HdmiCecV2 << PLATO_CR_HDMICECV2_DIV_0_SHIFT);

		OSWriteHWReg32(pvSystemDbgPeripRegs, PLATO_TOP_CR_HDMI_CEC_CLK_CTRL, ui32HDMICECClockCtrl);
		Poll(pvSystemDbgPeripRegs, PLATO_TOP_CR_HDMI_CEC_CLK_CTRL, ui32HDMICECClockCtrl, ui32HDMICECClockCtrl);

		OSWriteHWReg32(pvSystemDbgPeripRegs, PLATO_TOP_CR_I2C_CLK_CTRL, 0x1);
		Poll(pvSystemDbgPeripRegs, PLATO_TOP_CR_I2C_CLK_CTRL, 0x1, 0x1);
	}
#endif

	PLATO_CHECKPOINT;

	/* Waiting for DDR and GPU PLL's to lock */
	PollPr(pvSystemDbgAonRegs, PLATO_AON_CR_PLL_STATUS, 0x3, 0x3, -1 , 10);

	OSWriteHWReg32(pvSystemDbgAonRegs, PLATO_AON_CR_PLL_BYPASS, 0x01);
	Poll(pvSystemDbgAonRegs, PLATO_AON_CR_PLL_BYPASS, 0x01, 0x01);
	/* PLL Lock is done */

	OSWaitus(1000);

	/* Enabling gated clock output for DDR A/B */
	ui32MemClockCtrl = (1 << PLATO_CR_DDRAG_GATE_EN_SHIFT) | (1 << PLATO_CR_DDRBG_GATE_EN_SHIFT);

	OSWriteHWReg32(pvSystemDbgPeripRegs, PLATO_TOP_CR_DDR_CLK_CTRL, ui32MemClockCtrl);
	Poll(pvSystemDbgPeripRegs, PLATO_TOP_CR_DDR_CLK_CTRL, ui32MemClockCtrl, ui32MemClockCtrl);

	/* Enabling gated clock output for GPU and dividing the clock */
	ui32CoreClockCtrl = (1 << PLATO_CR_GPUG_GATE_EN_SHIFT);
	ui32CoreClockCtrl |= (GetPlatoGPUVDiv0(ui32CorePLLClockSpeed, ui32CoreClockSpeed) << PLATO_CR_GPUV_DIV_0_SHIFT);

	OSWriteHWReg32(pvSystemDbgPeripRegs, PLATO_TOP_CR_GPU_CLK_CTRL, ui32CoreClockCtrl);
	Poll(pvSystemDbgPeripRegs, PLATO_TOP_CR_GPU_CLK_CTRL, ui32CoreClockCtrl, ui32CoreClockCtrl);

	PLATO_CHECKPOINT;

	OSWaitus(100);

	/* Enabling PDP gated clock output >= 165 MHz for <= 1080p */
	ui32PDPClockCtrl = (1 << PLATO_CR_PDPG_GATE_EN_SHIFT);
	ui32PDPClockCtrl |= (GetPlatoPDPV0Div0(ui32CorePLLClockSpeed) << PLATO_CR_PDPV0_DIV_0_SHIFT);
	ui32PDPClockCtrl |= (GetPlatoPDPV1Div0(ui32CorePLLClockSpeed) << PLATO_CR_PDPV1_DIV_0_SHIFT);

	OSWriteHWReg32(pvSystemDbgPeripRegs, PLATO_TOP_CR_PDP_CLK_CTRL, ui32PDPClockCtrl);
	Poll(pvSystemDbgPeripRegs, PLATO_TOP_CR_PDP_CLK_CTRL, ui32PDPClockCtrl, ui32PDPClockCtrl);

	OSWaitus(100);

	/*
	 * Enabling HDMI gated clock output. PDP needs HDMI clocks on for framegrabber.
	 *
	 * NOTE: The dividers will be reconfigured in video.c,
	 *       for now they are set to their highest values.
	 */
	ui32HDMIClockCtrl = (1 << PLATO_CR_HDMIG_GATE_EN_SHIFT);
	ui32HDMIClockCtrl |= (PLATO_CR_HDMIV0_DIV_0_MASK);
	ui32HDMIClockCtrl |= (PLATO_CR_HDMIV1_DIV_0_MASK);

	OSWriteHWReg32(pvSystemDbgPeripRegs, PLATO_TOP_CR_HDMI_CLK_CTRL, ui32HDMIClockCtrl);
	Poll(pvSystemDbgPeripRegs, PLATO_TOP_CR_HDMI_CLK_CTRL, ui32HDMIClockCtrl, ui32HDMIClockCtrl);

	OSWaitus(100);

	PVR_LOG(("%s: Enabled PDP and HDMI clocks", __func__));
	PLATO_CHECKPOINT;

	/* Now putting DRAM controller out of reset */
	OSWriteHWReg32(pvSystemDbgAonRegs, PLATO_AON_CR_RESET_CTRL,
		PLATO_CR_DDR_A_DATA_RESET_N_MASK | PLATO_CR_DDR_A_CTRL_RESET_N_MASK |
		PLATO_CR_DDR_B_DATA_RESET_N_MASK | PLATO_CR_DDR_B_CTRL_RESET_N_MASK);
	OSWaitus(100);

	/* Now putting DRAM controller into reset */
	OSWriteHWReg32(pvSystemDbgAonRegs, PLATO_AON_CR_RESET_CTRL, PLATO_CR_DDR_A_CTRL_RESET_N_MASK | PLATO_CR_DDR_B_CTRL_RESET_N_MASK);

	/* Always configure DDR A */
	eError = ConfigPlatoDram(pvSystemDbgDdrAPublRegs, pvSystemDbgDdrACtrlRegs, pvSystemDbgAonRegs,
		ui32BDLRSetupDddrA, PLATO_CR_DDR_A_DATA_RESET_N_MASK | PLATO_CR_DDR_A_CTRL_RESET_N_MASK);
	if (eError != PVRSRV_OK)
	{
		PVR_LOG(("DDR Bank A setup failed. Init cannot proceed."));
		return eError;
	}

	PVR_LOG(("- %s: Finished DDR A Setup", __func__));

	/* Configure DDR B */
	eError = ConfigPlatoDram(pvSystemDbgDdrBPublRegs, pvSystemDbgDdrBCtrlRegs, pvSystemDbgAonRegs,
		ui32BDLRSetupDddrB, PLATO_CR_DDR_B_DATA_RESET_N_MASK | PLATO_CR_DDR_B_CTRL_RESET_N_MASK);
	if (eError != PVRSRV_OK)
	{
		PVR_LOG(("DDR Bank B setup failed. Init cannot proceed."));
		return eError;
	}

	PVR_LOG(("- %s: Finished DDR B Setup", __func__));

	/* Getting GPU And DDR A/B out of reset */
	OSWriteHWReg32(pvSystemDbgAonRegs, PLATO_AON_CR_RESET_CTRL, 0x00000F12);
	bPollPassed = PollPr(pvSystemDbgAonRegs, PLATO_AON_CR_RESET_CTRL, 0x00000F12, 0x00000F12, -1, 100);
	if (!bPollPassed)
	{
		return PVRSRV_ERROR_INIT_FAILURE;
	}

	/* setting CR_ISO_CTRL:CR_GPU_CLK_E */
	OSWriteHWReg32(pvSystemDbgAonRegs, PLATO_AON_CR_ISO_CTRL, 0x000001F);

	if (PLATO_HAS_NON_MAPPABLE(psDevData))
	{
		eError = CheckPDPAccessibility(psDevData);
	}

	return eError;

}
#else

static PVRSRV_ERROR ConfigPlatoSingleChannel(SYS_DATA *psDevData)
{

	PVRSRV_ERROR eError = PVRSRV_OK;
	IMG_BOOL bPollPassed;

	void* pvSystemDbgPeripRegs	  = psDevData->pvSystemRegCpuVBase + SYS_PLATO_REG_PERIP_OFFSET;
	void* pvSystemDbgAonRegs	  = psDevData->pvSystemRegCpuVBase + SYS_PLATO_REG_AON_OFFSET;
	void* pvSystemDbgDdrACtrlRegs = psDevData->pvSystemRegCpuVBase + SYS_PLATO_REG_DDR_A_CTRL_OFFSET;
	void* pvSystemDbgDdrAPublRegs = psDevData->pvSystemRegCpuVBase + SYS_PLATO_REG_DDR_A_PUBL_OFFSET;

	IMG_UINT32 ui32MemClockSpeed, ui32MemPLLClockSpeed;
	IMG_UINT32 ui32MemClockPLLCtrl0, ui32MemClockPLLCtrl1, ui32MemClockCtrl;

	IMG_UINT32 ui32CoreClockSpeed;
	IMG_UINT32 ui32CoreClockPLLCtrl0, ui32CoreClockPLLCtrl1, ui32CoreClockCtrl;

#if defined(ENABLE_PLATO_HDMI)
	IMG_UINT32 ui32HDMICECClockCtrl;
#endif

	IMG_UINT32 ui32PDPClockCtrl, ui32HDMIClockCtrl;

	/* Plato Soft reset */
	OSWriteHWReg32(pvSystemDbgAonRegs, PLATO_AON_CR_RESET_CTRL, 0x40);
	OSSleepms(1000);
	bPollPassed = Poll(pvSystemDbgAonRegs, PLATO_AON_CR_RESET_CTRL, 0x030, 0xF0);
	if (!bPollPassed)
	{
		return PVRSRV_ERROR_INIT_FAILURE;
	}
	PLATO_CHECKPOINT;

	OSWriteHWReg32(pvSystemDbgAonRegs, PLATO_AON_CR_NOC_CLK_CTRL, 0x1);
	Poll(pvSystemDbgAonRegs, PLATO_AON_CR_NOC_CLK_CTRL, 0x1, 0x1);

	/* Setup DDR PLL's */
	ui32MemClockSpeed = SysGetPlatoMemClockSpeed();
	ui32MemPLLClockSpeed = ui32MemClockSpeed;

	ui32MemClockPLLCtrl0 = (GetPlatoPLLInt(ui32MemPLLClockSpeed) << PLATO_CR_DDR_PLL_FBDIV_SHIFT);
	ui32MemClockPLLCtrl0 &= PLATO_CR_DDR_PLL_FBDIV_MASK;
	ui32MemClockPLLCtrl0 |= (1 << PLATO_CR_DDR_PLL_REFDIV_SHIFT);
	ui32MemClockPLLCtrl0 |= (1 << PLATO_CR_DDR_PLL_POSTDIV1_SHIFT);
	ui32MemClockPLLCtrl0 |= (1 << PLATO_CR_DDR_PLL_POSTDIV2_SHIFT);

	ui32MemClockPLLCtrl1 = (GetPlatoPLLFrac(ui32MemPLLClockSpeed) << PLATO_CR_DDR_PLL_FRAC_SHIFT);
	ui32MemClockPLLCtrl1 &= PLATO_CR_DDR_PLL_FRAC_MASK;

	OSWriteHWReg32(pvSystemDbgAonRegs, PLATO_AON_CR_DDR_PLL_CTRL_0, ui32MemClockPLLCtrl0);
	Poll(pvSystemDbgAonRegs, PLATO_AON_CR_DDR_PLL_CTRL_0, ui32MemClockPLLCtrl0, ui32MemClockPLLCtrl0);

	OSWriteHWReg32(pvSystemDbgAonRegs, PLATO_AON_CR_DDR_PLL_CTRL_1, ui32MemClockPLLCtrl1);
	Poll(pvSystemDbgAonRegs, PLATO_AON_CR_DDR_PLL_CTRL_1, ui32MemClockPLLCtrl1, ui32MemClockPLLCtrl1);

	PVR_LOG(("- %s: DDR clock: %u", __func__, ui32MemClockSpeed));

	/* Setup GPU PLL's */
	ui32CoreClockSpeed = SysGetPlatoCoreClockSpeed();
	ui32CorePLLClockSpeed = SysGetPlatoPLLClockSpeed(ui32CoreClockSpeed);

	ui32CoreClockPLLCtrl0 = (GetPlatoPLLInt(ui32CorePLLClockSpeed) << PLATO_CR_GPU_PLL_FBDIV_SHIFT);
	ui32CoreClockPLLCtrl0 &= PLATO_CR_GPU_PLL_FBDIV_MASK;
	ui32CoreClockPLLCtrl0 |= (1 << PLATO_CR_GPU_PLL_REFDIV_SHIFT);
	ui32CoreClockPLLCtrl0 |= (1 << PLATO_CR_GPU_PLL_POSTDIV1_SHIFT);
	ui32CoreClockPLLCtrl0 |= (1 << PLATO_CR_GPU_PLL_POSTDIV2_SHIFT);

	ui32CoreClockPLLCtrl1 = (GetPlatoPLLFrac(ui32CorePLLClockSpeed) << PLATO_CR_GPU_PLL_FRAC_SHIFT);
	ui32CoreClockPLLCtrl1 &= PLATO_CR_GPU_PLL_FRAC_MASK;

	OSWriteHWReg32(pvSystemDbgAonRegs, PLATO_AON_CR_GPU_PLL_CTRL_0, ui32CoreClockPLLCtrl0);
	Poll(pvSystemDbgAonRegs, PLATO_AON_CR_GPU_PLL_CTRL_0, ui32CoreClockPLLCtrl0, ui32CoreClockPLLCtrl0);

	OSWriteHWReg32(pvSystemDbgAonRegs, PLATO_AON_CR_GPU_PLL_CTRL_1, ui32CoreClockPLLCtrl1);
	Poll(pvSystemDbgAonRegs, PLATO_AON_CR_GPU_PLL_CTRL_1, ui32CoreClockPLLCtrl1, ui32CoreClockPLLCtrl1);

	PVR_LOG(("- %s: GPU clock: %u", __func__, ui32CoreClockSpeed));

#if defined(ENABLE_PLATO_HDMI)
	/* Setup HDMI CEC clock outputs */
	ui32HDMICECClockCtrl = 0;
	ui32HDMICECClockCtrl |= (GetPlatoHDMICECV0Div0(ui32CorePLLClockSpeed) << PLATO_CR_HDMICECV0_DIV_0_SHIFT);
	ui32HDMICECClockCtrl |= (GetPlatoHDMICECV1Div0(ui32CorePLLClockSpeed) << PLATO_CR_HDMICECV1_DIV_0_SHIFT);
	ui32HDMICECClockCtrl |= (GetPlatoHDMICECV2Div0(ui32CorePLLClockSpeed) << PLATO_CR_HDMICECV2_DIV_0_SHIFT);

	OSWriteHWReg32(pvSystemDbgPeripRegs, PLATO_TOP_CR_HDMI_CEC_CLK_CTRL, ui32HDMICECClockCtrl);
	Poll(pvSystemDbgPeripRegs, PLATO_TOP_CR_HDMI_CEC_CLK_CTRL, ui32HDMICECClockCtrl, ui32HDMICECClockCtrl);

	OSWriteHWReg32(pvSystemDbgPeripRegs, PLATO_TOP_CR_I2C_CLK_CTRL, 0x1);
	Poll(pvSystemDbgPeripRegs, PLATO_TOP_CR_I2C_CLK_CTRL, 0x1, 0x1);
#endif

	PLATO_CHECKPOINT;

	/* Waiting for DDR and GPU PLL's to lock */
	PollPr(pvSystemDbgAonRegs, PLATO_AON_CR_PLL_STATUS, 0x3, 0x3, -1 , 10);

	OSWriteHWReg32(pvSystemDbgAonRegs, PLATO_AON_CR_PLL_BYPASS, 0x01);
	Poll(pvSystemDbgAonRegs, PLATO_AON_CR_PLL_BYPASS, 0x01, 0x01);
	/* PLL Lock is done */

	OSWaitus(100);

	/* Enabling gated clock output for DDR A/B */
	ui32MemClockCtrl = (1 << PLATO_CR_DDRAG_GATE_EN_SHIFT) | (1 << PLATO_CR_DDRBG_GATE_EN_SHIFT);

	OSWriteHWReg32(pvSystemDbgPeripRegs, PLATO_TOP_CR_DDR_CLK_CTRL, ui32MemClockCtrl);
	Poll(pvSystemDbgPeripRegs, PLATO_TOP_CR_DDR_CLK_CTRL, ui32MemClockCtrl, ui32MemClockCtrl);

	/* Enabling gated clock output for GPU and dividing the clock */
	ui32CoreClockCtrl = (1 << PLATO_CR_GPUG_GATE_EN_SHIFT);
	ui32CoreClockCtrl |= (GetPlatoGPUVDiv0(ui32CorePLLClockSpeed, ui32CoreClockSpeed) << PLATO_CR_GPUV_DIV_0_SHIFT);

	OSWriteHWReg32(pvSystemDbgPeripRegs, PLATO_TOP_CR_GPU_CLK_CTRL, ui32CoreClockCtrl);
	Poll(pvSystemDbgPeripRegs, PLATO_TOP_CR_GPU_CLK_CTRL, ui32CoreClockCtrl, ui32CoreClockCtrl);

	PLATO_CHECKPOINT;

	OSWaitus(1000);

	/* Enabling PDP gated clock output >= 165 MHz for <= 1080p */
	ui32PDPClockCtrl = (1 << PLATO_CR_PDPG_GATE_EN_SHIFT);
	ui32PDPClockCtrl |= (GetPlatoPDPV0Div0(ui32CorePLLClockSpeed) << PLATO_CR_PDPV0_DIV_0_SHIFT);
	ui32PDPClockCtrl |= (GetPlatoPDPV1Div0(ui32CorePLLClockSpeed) << PLATO_CR_PDPV1_DIV_0_SHIFT);

	OSWriteHWReg32(pvSystemDbgPeripRegs, PLATO_TOP_CR_PDP_CLK_CTRL, ui32PDPClockCtrl);
	Poll(pvSystemDbgPeripRegs, PLATO_TOP_CR_PDP_CLK_CTRL, ui32PDPClockCtrl, ui32PDPClockCtrl);

	OSWaitus(100);

	/*
	 * Enabling HDMI gated clock output. PDP needs HDMI clocks on for framegrabber.
	 *
	 * NOTE: The dividers will be reconfigured in video.c,
	 *       for now they are set to their highest values.
	 */
	ui32HDMIClockCtrl = (1 << PLATO_CR_HDMIG_GATE_EN_SHIFT);
	ui32HDMIClockCtrl |= (PLATO_CR_HDMIV0_DIV_0_MASK);
	ui32HDMIClockCtrl |= (PLATO_CR_HDMIV1_DIV_0_MASK);

	OSWriteHWReg32(pvSystemDbgPeripRegs, PLATO_TOP_CR_HDMI_CLK_CTRL, ui32HDMIClockCtrl);
	Poll(pvSystemDbgPeripRegs, PLATO_TOP_CR_HDMI_CLK_CTRL, ui32HDMIClockCtrl, ui32HDMIClockCtrl);

	OSWaitus(100);

	PVR_LOG(("%s: Enabled PDP and HDMI clocks", __func__));

	PLATO_CHECKPOINT;

	/* Now putting DRAM A controller out of reset */
	OSWriteHWReg32(pvSystemDbgAonRegs, PLATO_AON_CR_RESET_CTRL, 0x0300);
	OSWaitus(100);

	/* Now putting DRAM A controller into reset */
	OSWriteHWReg32(pvSystemDbgAonRegs, PLATO_AON_CR_RESET_CTRL, 0x0200);

	/* Configure DRAM A control and publ regs */
	eError = ConfigPlatoDram(pvSystemDbgDdrAPublRegs, pvSystemDbgDdrACtrlRegs, pvSystemDbgAonRegs, PLATO_CR_DDR_A_DATA_RESET_N_MASK | PLATO_CR_DDR_A_CTRL_RESET_N_MASK);
	if (eError != PVRSRV_OK)
	{
		PVR_LOG(("DDR Bank setup failed. Init cannot proceed."));
		return eError;
	}
	PVR_LOG(("- %s: Finished DDR A Setup", __func__));

	/* Getting GPU and DDR A out of reset */
	OSWriteHWReg32(pvSystemDbgAonRegs, PLATO_AON_CR_RESET_CTRL, 0x00000312);
	bPollPassed = PollPr(pvSystemDbgAonRegs, PLATO_AON_CR_RESET_CTRL, 0x00000312, 0x00000312, -1, 100);
	if (!bPollPassed)
	{
		return PVRSRV_ERROR_INIT_FAILURE;
	}
	/* setting CR_ISO_CTRL:CR_GPU_CLK_E */
	OSWriteHWReg32(pvSystemDbgAonRegs, PLATO_AON_CR_ISO_CTRL, 0x000001F);

	if (PLATO_HAS_NON_MAPPABLE(psDevData))
	{
		eError = CheckPDPAccessibility(psDevData);
	}

	return eError;
}

#endif

static PVRSRV_ERROR ConfigPlatoHw(SYS_DATA *psDevData)
{
	PVRSRV_ERROR eError = PVRSRV_OK;
	IMG_UINT32 ui32MaxRestartCount = PLATO_PDP_RELOADS_MAX;
	IMG_UINT32 ui32RestartCount = 0;

	PLATO_CHECKPOINT;

	/* Config Plato until PDP registers become accessible */
	do
	{
		#if defined(PLATO_DUAL_CHANNEL_DDR)
		eError = ConfigPlatoDualChannel(psDevData);
		#else
		eError = ConfigPlatoSingleChannel(psDevData);
		#endif

		ui32RestartCount++;

		if (eError == PVRSRV_OK)
		{
			break;
		}

		PVR_LOG(("PDP registers inaccessible after %d out of %d tries.",
					ui32RestartCount, ui32MaxRestartCount));

	} while (ui32RestartCount < ui32MaxRestartCount &&
			 eError == PVRSRV_ERROR_DC_DEVICE_INACCESSIBLE);

	if (PLATO_HAS_NON_MAPPABLE(psDevData) && eError == PVRSRV_OK)
	{
		PVR_LOG(("Got correct PDP register values after %d tries!",
					ui32RestartCount));
	}

	if (ui32RestartCount == ui32MaxRestartCount && eError == PVRSRV_ERROR_DC_DEVICE_INACCESSIBLE)
	{
		PVR_LOG(("Failed to access PDP registers after %d tries!",
					ui32RestartCount));
	}

	return eError;
}

#endif

static PVRSRV_ERROR ConfigPlato(SYS_DATA *psDevData)
{
	#if defined(EMULATOR)
	return ConfigPlatoEmu(psDevData);
	#elif defined(VIRTUAL_PLATFORM)
	return PVRSRV_OK;
	#else
	return ConfigPlatoHw(psDevData);
	#endif
}

static PVRSRV_ERROR PlatoLocalMemoryTest(PVRSRV_DEVICE_CONFIG *psDevConfig)
{
	IMG_UINT64 i, j = 0;
	IMG_UINT32 tmp = 0;
	IMG_UINT32 chunk = sizeof(IMG_UINT32) * 10;

	IMG_UINT64 ui64TestMemoryBase = psDevConfig->pasPhysHeaps[PHYS_HEAP_ID_GPU_LOCAL].sStartAddr.uiAddr;
	IMG_UINT64 ui64TestMemorySize = psDevConfig->pasPhysHeaps[PHYS_HEAP_ID_GPU_LOCAL].uiSize;

	PVR_LOG(("- %s: Starting Local memory test from "
			 "0x%llx to 0x%llx (in CPU space)",
			 __func__, ui64TestMemoryBase,
			 ui64TestMemoryBase + ui64TestMemorySize));

	while (j < ui64TestMemorySize)
	{
		IMG_CPU_PHYADDR myPaddr;
		IMG_UINT32* pui32Virt;

		myPaddr.uiAddr = ui64TestMemoryBase + j;
		pui32Virt = OSMapPhysToLin(myPaddr, chunk, PVRSRV_MEMALLOCFLAG_CPU_UNCACHED);

		for (i = 0; i < chunk/sizeof(IMG_UINT32); i++)
		{
			*(pui32Virt + i) = 0xdeadbeef;
			OSWriteMemoryBarrier(pui32Virt);
			tmp = *(pui32Virt + i);
			if (tmp != 0xdeadbeef)
			{
				PVR_DPF((PVR_DBG_ERROR,
						"Local memory read-write test failed at address=0x%llx: written 0x%x, read 0x%x",
						ui64TestMemoryBase + ((i * sizeof(IMG_UINT32)) + j) , (IMG_UINT32) 0xdeadbeef, tmp));

				OSUnMapPhysToLin(pui32Virt, chunk);
				return PVRSRV_ERROR_SYSTEM_LOCAL_MEMORY_INIT_FAIL;
			}
		}

		OSUnMapPhysToLin(pui32Virt, chunk);

		j+= (1024 * 1024 * 500);
	}

	PVR_LOG(("Local memory read-write test passed!"));

	return PVRSRV_OK;
}

#if (PLATO_MEMORY_CONFIG == PLATO_MEMORY_LOCAL) || (PLATO_MEMORY_CONFIG == PLATO_MEMORY_HYBRID)
/* Queries PCI BAR register, reads its base address and returns the reported size */
static PVRSRV_ERROR AcquireLocalMappableMemory(IMG_HANDLE hPCI,
											   IMG_PUINT64 pui64MemCpuPAddr,
											   IMG_PUINT64 pui64MemSize)
{
	IMG_UINT16 uiVendorID, uiDevID;
	IMG_UINT64 uiBarSize;

	PVRSRV_ERROR eError;

	OSPCIGetVendorDeviceIDs(hPCI, &uiVendorID, &uiDevID);
	if (uiDevID != SYS_RGX_DEV_DEVICE_ID)
	{
		PVR_DPF((PVR_DBG_ERROR, "%s: Unexpected device ID 0x%X",
				 __func__, uiDevID));

		return PVRSRV_ERROR_PCI_DEVICE_NOT_FOUND;
	}

	uiBarSize = OSPCIAddrRangeLen(hPCI, SYS_DEV_MEM_PCI_BASENUM);

	/* Reserve the address region for whole BAR */
	eError = OSPCIRequestAddrRegion(hPCI, SYS_DEV_MEM_PCI_BASENUM, 0, uiBarSize);
	if (eError != PVRSRV_OK)
	{
		PVR_DPF((PVR_DBG_ERROR, "%s: Device memory region not available",
				 __func__));

		return eError;
	}

	/* Clear any BIOS-configured MTRRs */
	eError = OSPCIClearResourceMTRRs(hPCI, SYS_DEV_MEM_PCI_BASENUM);
	if (eError != PVRSRV_OK)
	{
		PVR_DPF((PVR_DBG_WARNING, "%s: Failed to clear BIOS MTRRs (Error %d)",
				 __func__, eError));
		/* Soft-fail, the driver can limp along. */
	}

	*pui64MemCpuPAddr = OSPCIAddrRangeStart(hPCI, SYS_DEV_MEM_PCI_BASENUM);
	*pui64MemSize = uiBarSize;

	return PVRSRV_OK;
}

static INLINE void ReleaseLocalMappableMemory(IMG_HANDLE hPCI,
											  IMG_UINT64 ui64MemCpuPAddr,
											  IMG_UINT64 ui64MemSize)
{
	PVR_UNREFERENCED_PARAMETER(ui64MemCpuPAddr);

	OSPCIReleaseAddrRegion(hPCI, SYS_DEV_MEM_PCI_BASENUM, 0, ui64MemSize);

	OSPCIReleaseResourceMTRRs(hPCI, SYS_DEV_MEM_PCI_BASENUM);
}
#endif /* (PLATO_MEMORY_CONFIG == PLATO_MEMORY_LOCAL) || (PLATO_MEMORY_CONFIG == PLATO_MEMORY_HYBRID) */

#if (PLATO_MEMORY_CONFIG == PLATO_MEMORY_LOCAL) || (PLATO_MEMORY_CONFIG == PLATO_MEMORY_HYBRID)

static PVRSRV_ERROR InitLocalMemory(SYS_DATA *psDevData,
									PHYS_HEAP_CONFIG *pasPhysHeaps,
									IMG_UINT32 uiPhysHeapCount,
									IMG_HANDLE hPhysHeapPrivData)
{
	IMG_UINT64 ui64MappedMemSize = psDevData->ui64MappedMemSize;
	IMG_UINT64 ui64MappedMemCpuPAddr = psDevData->ui64MappedMemCpuPAddr;
	IMG_UINT64 ui64MappedMemDevPAddr;
	PHYS_HEAP_CONFIG *psPhysHeap;

#if (RGX_NUM_OS_SUPPORTED > 1)
	IMG_UINT64 uiFwCarveoutSize;
#if defined(SUPPORT_AUTOVZ)
	/* Carveout out enough LMA memory to hold the heaps of
	 * all supported OSIDs and the FW page tables */
	uiFwCarveoutSize = (RGX_NUM_OS_SUPPORTED * RGX_FIRMWARE_RAW_HEAP_SIZE) +
						RGX_FIRMWARE_MAX_PAGETABLE_SIZE;
#elif defined(RGX_VZ_STATIC_CARVEOUT_FW_HEAPS)
	/* Carveout out enough LMA memory to hold the heaps of all supported OSIDs */
	uiFwCarveoutSize = (RGX_NUM_OS_SUPPORTED * RGX_FIRMWARE_RAW_HEAP_SIZE);
#else
	/* Create a memory carveout just for the Host's Firmware heap.
	 * Guests will allocate their own physical memory. */
	uiFwCarveoutSize = RGX_FIRMWARE_RAW_HEAP_SIZE;
#endif
#endif /* (RGX_NUM_OS_SUPPORTED > 1) */


	psPhysHeap = &pasPhysHeaps[PHYS_HEAP_ID_GPU_LOCAL];
	psPhysHeap->eType = PHYS_HEAP_TYPE_LMA;
	psPhysHeap->pszPDumpMemspaceName = "LMA";
	psPhysHeap->psMemFuncs = &gsLocalPhysHeapFuncs;
	psPhysHeap->hPrivData = hPhysHeapPrivData;
	psPhysHeap->ui32UsageFlags = PHYS_HEAP_USAGE_GPU_LOCAL;

	/* Configure mappable heap region */
	psPhysHeap->sCardBase.uiAddr = PLATO_DDR_DEV_PHYSICAL_BASE;

	if (!PLATO_HAS_NON_MAPPABLE(psDevData))
	{
		/*
		 * With a BAR size that's greater than the actual memory size,
		 * move base CPU address of the heap region to use last
		 * aliased part of odd region and first aliased part of even region.
		 *
		 * This allows to use full available memory in one contiguous heap region.
		 */
		ui64MappedMemCpuPAddr +=
			PLATO_DRAM_SPLIT_ADDR - (SYS_DEV_MEM_REGION_SIZE >> 1) - PLATO_DDR_DEV_PHYSICAL_BASE;

		/*
		 * As we can't read the actual memory size from the BAR we limit it to
		 * a value set during build time.
		 */
		ui64MappedMemSize = SYS_DEV_MEM_REGION_SIZE;

		ui64MappedMemDevPAddr = PLATO_DDR_ALIASED_DEV_PHYSICAL_BASE;
	}
	else
	{
		ui64MappedMemDevPAddr = PLATO_DDR_DEV_PHYSICAL_BASE;

		/*
		 * With a BAR size less than the actual memory size (8GB),
		 * we need to dynamically calculate the device base address
		 * that the PCI memory window is pointing to. The address depends on
		 * what address in host memory space the memory BAR was assigned by the BIOS.
		 *
		 * Bits x to 34 in host CPU base address decide on where
		 * within the DRAM region the BAR points to, where x is the shift calculated
		 * below based on the mapped memory size (BAR).
		 */
		{
			IMG_UINT32 ui32Shift = 0;
			IMG_UINT64 ui64Mask;
			ui32Shift = __builtin_ffsll(psDevData->ui64MappedMemSize)-1;
			PVR_ASSERT(ui32Shift < 35U);
			ui64Mask = (1ULL << (35U-ui32Shift)) - 1ULL;
			PVR_ASSERT(ui64Mask != 0);

			ui64MappedMemDevPAddr += ui64MappedMemCpuPAddr & (ui64Mask << ui32Shift);
		}

		/*
		 * Our aliased address can point anywhere in the 32GB - 64GB range,
		 * we now need to determine the closest aliased address to the split point (48GB).
		 * This is done by first finding the offset from the previous segment (mod below)
		 * and adding it to either the DRAM split point or the start of the aliased
		 * region that's closest to the split point.
		 */
		if (ui64MappedMemDevPAddr >= PLATO_DRAM_SPLIT_ADDR)
		{
			ui64MappedMemDevPAddr = PLATO_DRAM_SPLIT_ADDR + (ui64MappedMemDevPAddr % PLATO_DDR_ALIASED_DEV_SEGMENT_SIZE);
		}
		else
		{
			ui64MappedMemDevPAddr = PLATO_DDR_ALIASED_DEV_PHYSICAL_BASE + (ui64MappedMemDevPAddr % PLATO_DDR_ALIASED_DEV_SEGMENT_SIZE);
		}
	}

	/* Confidence check on mapped device physical address */
	PVR_ASSERT(ui64MappedMemDevPAddr >= PLATO_DDR_ALIASED_DEV_PHYSICAL_BASE && ui64MappedMemDevPAddr < PLATO_DDR_ALIASED_DEV_PHYSICAL_END);

	psPhysHeap->sStartAddr.uiAddr = ui64MappedMemCpuPAddr;
	psPhysHeap->sCardBase.uiAddr = ui64MappedMemDevPAddr;
	psPhysHeap->uiSize = ui64MappedMemSize;
#if (RGX_NUM_OS_SUPPORTED > 1)
	psPhysHeap->uiSize -= uiFwCarveoutSize;
#endif
#if defined(SUPPORT_DISPLAY_CLASS)
	psPhysHeap->uiSize -= RGX_PLATO_RESERVE_DC_MEM_SIZE;
#endif

	/* Setup non-mappable region if BAR size is less than actual memory size (8GB) */
	if (PLATO_HAS_NON_MAPPABLE(psDevData))
	{
		IMG_UINT64 ui64PrecedingRegionBase = 0;
		IMG_UINT64 ui64PrecedingRegionSize = 0;
		IMG_UINT64 ui64FollowingRegionBase = 0;
		IMG_UINT64 ui64FollowingRegionSize = 0;

		psPhysHeap = &pasPhysHeaps[PHYS_HEAP_ID_NON_MAPPABLE];
		psPhysHeap->eType = PHYS_HEAP_TYPE_LMA;
		psPhysHeap->pszPDumpMemspaceName = "LMA";
		psPhysHeap->psMemFuncs = &gsLocalPhysHeapFuncs;
		psPhysHeap->hPrivData = hPhysHeapPrivData;
		psPhysHeap->ui32UsageFlags = PHYS_HEAP_USAGE_GPU_PRIVATE;
		psPhysHeap->sCardBase.uiAddr = PLATO_DDR_DEV_PHYSICAL_BASE;

		/*
		 * If mapped region is not at the base of memory,
		 * then it is preceded by a non-mappable region
		 */
		 ui64PrecedingRegionBase = PLATO_DDR_ALIASED_DEV_PHYSICAL_BASE;
		 ui64PrecedingRegionSize = ui64MappedMemDevPAddr - ui64PrecedingRegionBase;

		/*
		 * If mapped region is not at the end of memory,
		 * then it is followed by a non-mappable region
		 */
		ui64FollowingRegionBase = ui64MappedMemDevPAddr + psDevData->ui64MappedMemSize;
		ui64FollowingRegionSize =
			PLATO_DDR_ALIASED_DEV_PHYSICAL_END - (ui64MappedMemDevPAddr + psDevData->ui64MappedMemSize);

		/* Use only bigger region for now */
		if (ui64FollowingRegionSize > ui64PrecedingRegionSize)
		{
			psPhysHeap->sCardBase.uiAddr = ui64FollowingRegionBase;
			psPhysHeap->uiSize = ui64FollowingRegionSize;
		}
		else
		{
			psPhysHeap->sCardBase.uiAddr = ui64PrecedingRegionBase;
			psPhysHeap->uiSize = ui64PrecedingRegionSize;
		}

		psPhysHeap->sStartAddr.uiAddr = 0;

		PVR_LOG(("Added non-mappable local memory region. Base = 0x%016llx, Size=0x%016llx",
					psPhysHeap->sCardBase.uiAddr,
					psPhysHeap->uiSize));

		PVR_ASSERT(psPhysHeap->uiSize < SYS_DEV_MEM_REGION_SIZE);
	}

#if (RGX_NUM_OS_SUPPORTED > 1)
	psPhysHeap = &pasPhysHeaps[PHYS_HEAP_ID_FW_LOCAL];
	psPhysHeap->eType = PHYS_HEAP_TYPE_LMA;
	psPhysHeap->pszPDumpMemspaceName = "LMA";
	psPhysHeap->psMemFuncs = &gsLocalPhysHeapFuncs;
	psPhysHeap->hPrivData = hPhysHeapPrivData;
	psPhysHeap->ui32UsageFlags = PHYS_HEAP_USAGE_FW_MAIN;

	psPhysHeap->sCardBase.uiAddr =
		pasPhysHeaps[PHYS_HEAP_ID_GPU_LOCAL].sCardBase.uiAddr +
		pasPhysHeaps[PHYS_HEAP_ID_GPU_LOCAL].uiSize;

	psPhysHeap->sStartAddr.uiAddr =
		pasPhysHeaps[PHYS_HEAP_ID_GPU_LOCAL].sStartAddr.uiAddr +
		pasPhysHeaps[PHYS_HEAP_ID_GPU_LOCAL].uiSize;
	psPhysHeap->uiSize = RGX_FIRMWARE_RAW_HEAP_SIZE;
#endif

#if defined(SUPPORT_DISPLAY_CLASS)
	psPhysHeap = &pasPhysHeaps[PHYS_HEAP_ID_PDP_LOCAL];
	psPhysHeap->eType = PHYS_HEAP_TYPE_LMA;
	psPhysHeap->pszPDumpMemspaceName = "LMA";
	psPhysHeap->psMemFuncs = &gsLocalPhysHeapFuncs;
	psPhysHeap->hPrivData = hPhysHeapPrivData;
	psPhysHeap->ui32UsageFlags = PHYS_HEAP_USAGE_DISPLAY;

	psPhysHeap->sCardBase.uiAddr = PLATO_DDR_DEV_PHYSICAL_BASE;

	psPhysHeap->sStartAddr.uiAddr =
#if (RGX_NUM_OS_SUPPORTED > 1)
		pasPhysHeaps[PHYS_HEAP_ID_FW_LOCAL].sStartAddr.uiAddr +
		pasPhysHeaps[PHYS_HEAP_ID_FW_LOCAL].uiSize;
#else
		pasPhysHeaps[PHYS_HEAP_ID_GPU_LOCAL].sStartAddr.uiAddr +
		pasPhysHeaps[PHYS_HEAP_ID_GPU_LOCAL].uiSize;
#endif
	psPhysHeap->uiSize = RGX_PLATO_RESERVE_DC_MEM_SIZE;
#endif

	return PVRSRV_OK;
}
#endif /* (PLATO_MEMORY_CONFIG == PLATO_MEMORY_LOCAL) || (PLATO_MEMORY_CONFIG == PLATO_MEMORY_HYBRID) */

#if (PLATO_MEMORY_CONFIG == PLATO_MEMORY_HOST) || (PLATO_MEMORY_CONFIG == PLATO_MEMORY_HYBRID)
static PVRSRV_ERROR InitHostMemory(SYS_DATA *psDevData,
								   PHYS_HEAP_CONFIG *pasPhysHeaps,
								   IMG_UINT32 uiPhysHeapCount,
								   IMG_HANDLE hPhysHeapPrivData)
{
	PHYS_HEAP_CONFIG *psPhysHeap;

	PVR_ASSERT(uiPhysHeapCount == 1);

	psPhysHeap = &pasPhysHeaps[PHYS_HEAP_ID_CPU_LOCAL];
	psPhysHeap->eType = PHYS_HEAP_TYPE_UMA;
	psPhysHeap->pszPDumpMemspaceName = "SYSMEM";
	psPhysHeap->psMemFuncs = &gsHostPhysHeapFuncs;
	psPhysHeap->hPrivData = hPhysHeapPrivData;
#if (PLATO_MEMORY_CONFIG == PLATO_MEMORY_HOST)
	psPhysHeap->ui32UsageFlags = PHYS_HEAP_USAGE_GPU_LOCAL;
#elif (PLATO_MEMORY_CONFIG == PLATO_MEMORY_HYBRID)
	psPhysHeap->ui32UsageFlags = PHYS_HEAP_USAGE_CPU_LOCAL;
	PVR_DPF((PVR_DBG_WARNING, "Initialising CPU_LOCAL UMA Host PhysHeaps"));
#endif

	psPhysHeap->sCardBase.uiAddr = PLATO_HOSTRAM_DEV_PHYSICAL_BASE;

	return PVRSRV_OK;
}
#endif /* (PLATO_MEMORY_CONFIG == PLATO_MEMORY_HOST) */

#if (PLATO_MEMORY_CONFIG == PLATO_MEMORY_HYBRID)
static PVRSRV_ERROR InitHybridMemory(SYS_DATA *psDevData,
									 PHYS_HEAP_CONFIG *pasPhysHeaps,
									 IMG_UINT32 uiPhysHeapCount,
									 IMG_HANDLE hPhysHeapPrivData)
{
	PVRSRV_ERROR eError;

	PVR_ASSERT(uiPhysHeapCount >= PHYS_HEAP_ID_PDP_LOCAL);

	eError = InitHostMemory(psDevData, pasPhysHeaps,
							1, hPhysHeapPrivData);
	if (eError != PVRSRV_OK)
	{
		return eError;
	}

	/*
	 * InitLocalMemory should set up the correct heaps regardless of whether the
	 * memory configuration is 'local' or 'hybrid'.
	 */
	eError = InitLocalMemory(psDevData, pasPhysHeaps,
							 uiPhysHeapCount, hPhysHeapPrivData);
	if (eError != PVRSRV_OK)
	{
		return eError;
	}

	/* Adjust the pdump memory space names */
	pasPhysHeaps[PHYS_HEAP_ID_GPU_LOCAL].pszPDumpMemspaceName = "LMA";
	pasPhysHeaps[PHYS_HEAP_ID_PDP_LOCAL].pszPDumpMemspaceName = "LMA";

	return PVRSRV_OK;
}
#endif /* (PLATO_MEMORY_CONFIG == PLATO_MEMORY_HYBRID) */

static PVRSRV_ERROR InitMemory(SYS_DATA *psDevData,
							   PVRSRV_DEVICE_CONFIG *psDevConfig,
							   PHYS_HEAP_CONFIG **ppasPhysHeapsOut,
							   IMG_UINT32 *puiPhysHeapCountOut)
{
	IMG_UINT32 uiHeapCount = 1;
	PHYS_HEAP_CONFIG *pasPhysHeaps;
	PVRSRV_ERROR eError;

#if (PLATO_MEMORY_CONFIG == PLATO_MEMORY_HYBRID)
	uiHeapCount++;
#endif

#if defined(SUPPORT_DISPLAY_CLASS)
	uiHeapCount++;
#endif

#if (RGX_NUM_OS_SUPPORTED > 1)
#if (PLATO_MEMORY_CONFIG != PLATO_MEMORY_HYBRID)
#error "VZ support implemented only hybrid memory configuration (PLATO_MEMORY_HYBRID)"
#endif
	uiHeapCount++;
#endif

	if (PLATO_HAS_NON_MAPPABLE(psDevData))
	{
		uiHeapCount++;
	}

	pasPhysHeaps = OSAllocZMem(sizeof(*pasPhysHeaps) * uiHeapCount);
	if (!pasPhysHeaps)
	{
		return PVRSRV_ERROR_OUT_OF_MEMORY;
	}

#if (PLATO_MEMORY_CONFIG == PLATO_MEMORY_LOCAL)
	eError = InitLocalMemory(psDevData, pasPhysHeaps,
							 uiHeapCount, psDevConfig);
#elif (PLATO_MEMORY_CONFIG == PLATO_MEMORY_HOST)
	eError = InitHostMemory(psDevData, pasPhysHeaps,
							uiHeapCount, psDevConfig);
#elif (PLATO_MEMORY_CONFIG == PLATO_MEMORY_HYBRID)
	eError = InitHybridMemory(psDevData, pasPhysHeaps,
							  uiHeapCount, psDevConfig);
#endif
	if (eError != PVRSRV_OK)
	{
		OSFreeMem(pasPhysHeaps);
		return eError;
	}

	*ppasPhysHeapsOut = pasPhysHeaps;
	*puiPhysHeapCountOut = uiHeapCount;

	return PVRSRV_OK;
}

static INLINE void DeInitMemory(PHYS_HEAP_CONFIG *pasPhysHeaps,
								IMG_UINT32 uiPhysHeapCount)
{
	OSFreeMem(pasPhysHeaps);
}

#if (PLATO_MEMORY_CONFIG == PLATO_MEMORY_LOCAL) || (PLATO_MEMORY_CONFIG == PLATO_MEMORY_HYBRID)
void PlatoLocalCpuPAddrToDevPAddr(IMG_HANDLE hPrivData,
					  IMG_UINT32 ui32NumOfAddr,
					  IMG_DEV_PHYADDR *psDevPAddr,
					  IMG_CPU_PHYADDR *psCpuPAddr)
{
	PVRSRV_DEVICE_CONFIG *psDevConfig = (PVRSRV_DEVICE_CONFIG *)hPrivData;

	/* Optimise common case */
	psDevPAddr[0].uiAddr = psCpuPAddr[0].uiAddr -
		psDevConfig->pasPhysHeaps[PHYS_HEAP_ID_GPU_LOCAL].sStartAddr.uiAddr +
		psDevConfig->pasPhysHeaps[PHYS_HEAP_ID_GPU_LOCAL].sCardBase.uiAddr;

	if (ui32NumOfAddr > 1)
	{
		IMG_UINT32 ui32Idx;
		for (ui32Idx = 1; ui32Idx < ui32NumOfAddr; ++ui32Idx)
		{
			psDevPAddr[ui32Idx].uiAddr = psCpuPAddr[ui32Idx].uiAddr -
				psDevConfig->pasPhysHeaps[PHYS_HEAP_ID_GPU_LOCAL].sStartAddr.uiAddr +
				psDevConfig->pasPhysHeaps[PHYS_HEAP_ID_GPU_LOCAL].sCardBase.uiAddr;
		}
	}
}

void PlatoLocalDevPAddrToCpuPAddr(IMG_HANDLE hPrivData,
					  IMG_UINT32 ui32NumOfAddr,
					  IMG_CPU_PHYADDR *psCpuPAddr,
					  IMG_DEV_PHYADDR *psDevPAddr)
{
	IMG_BOOL bInvalidAddress=IMG_FALSE;
	IMG_DEV_PHYADDR sMappableRegionCardBase;
	IMG_UINT64	uiMaxSize;
	PVRSRV_DEVICE_CONFIG *psDevConfig = (PVRSRV_DEVICE_CONFIG *)hPrivData;

	sMappableRegionCardBase = psDevConfig->pasPhysHeaps[PHYS_HEAP_ID_GPU_LOCAL].sCardBase;
	uiMaxSize = psDevConfig->pasPhysHeaps[PHYS_HEAP_ID_GPU_LOCAL].uiSize;

#if defined(SUPPORT_DISPLAY_CLASS)
	uiMaxSize += RGX_PLATO_RESERVE_DC_MEM_SIZE;
#endif

	if (psDevPAddr[0].uiAddr > (sMappableRegionCardBase.uiAddr + uiMaxSize) ||
	    psDevPAddr[0].uiAddr < sMappableRegionCardBase.uiAddr)
	{
		bInvalidAddress = IMG_TRUE;
	}

	/* Optimise common case */
	psCpuPAddr[0].uiAddr = psDevPAddr[0].uiAddr -
		psDevConfig->pasPhysHeaps[PHYS_HEAP_ID_GPU_LOCAL].sCardBase.uiAddr +
		psDevConfig->pasPhysHeaps[PHYS_HEAP_ID_GPU_LOCAL].sStartAddr.uiAddr;

	if (ui32NumOfAddr > 1)
	{
		IMG_UINT32 ui32Idx;
		for (ui32Idx = 1; ui32Idx < ui32NumOfAddr; ++ui32Idx)
		{
			if (psDevPAddr[ui32Idx].uiAddr > (sMappableRegionCardBase.uiAddr + uiMaxSize) ||
			    psDevPAddr[ui32Idx].uiAddr < sMappableRegionCardBase.uiAddr)
			{
				bInvalidAddress = IMG_TRUE;
			}

			psCpuPAddr[ui32Idx].uiAddr = psDevPAddr[ui32Idx].uiAddr -
				psDevConfig->pasPhysHeaps[PHYS_HEAP_ID_GPU_LOCAL].sCardBase.uiAddr +
				psDevConfig->pasPhysHeaps[PHYS_HEAP_ID_GPU_LOCAL].sStartAddr.uiAddr;
		}
	}

	if (bInvalidAddress)
	{
		PVR_DPF((PVR_DBG_ERROR,"/*****************************WARNING**********************************************/"));
		PVR_DPF((PVR_DBG_ERROR,"%s: Given Device Physical Address belongs to non-mappable region",__func__));
		PVR_DPF((PVR_DBG_ERROR,"Accessing the buffer may lead to unexpected behaviour"));
#if defined(DEBUG)
		PVR_DPF((PVR_DBG_ERROR,"Input Device Physical Address count: %d", ui32NumOfAddr));
		{
			IMG_UINT32 ui32Idx;
			for (ui32Idx = 0; ui32Idx < ui32NumOfAddr; ++ui32Idx)
			{
				if (psDevPAddr[ui32Idx].uiAddr > (sMappableRegionCardBase.uiAddr + uiMaxSize) ||
				    psDevPAddr[ui32Idx].uiAddr < sMappableRegionCardBase.uiAddr)
				{
					PVR_DPF((PVR_DBG_ERROR,"Device Physical Address : 0x%llx", psDevPAddr[ui32Idx].uiAddr));
				}
			}
		}
		dump_stack();
#endif
		PVR_DPF((PVR_DBG_ERROR,"/**********************************************************************************/"));
	}

}
#endif

#if (PLATO_MEMORY_CONFIG == PLATO_MEMORY_HOST) || (PLATO_MEMORY_CONFIG == PLATO_MEMORY_HYBRID)
void PlatoSystemCpuPAddrToDevPAddr(IMG_HANDLE hPrivData,
					   IMG_UINT32 ui32NumOfAddr,
					   IMG_DEV_PHYADDR *psDevPAddr,
					   IMG_CPU_PHYADDR *psCpuPAddr)
{
	PVR_UNREFERENCED_PARAMETER(hPrivData);

	/* Optimise common case */
	psDevPAddr[0].uiAddr = psCpuPAddr[0].uiAddr + PLATO_HOSTRAM_DEV_PHYSICAL_BASE;
	if (ui32NumOfAddr > 1)
	{
		IMG_UINT32 ui32Idx;
		for (ui32Idx = 1; ui32Idx < ui32NumOfAddr; ++ui32Idx)
		{
			psDevPAddr[ui32Idx].uiAddr = psCpuPAddr[ui32Idx].uiAddr + PLATO_HOSTRAM_DEV_PHYSICAL_BASE;
		}
	}
}

void PlatoSystemDevPAddrToCpuPAddr(IMG_HANDLE hPrivData,
					   IMG_UINT32 ui32NumOfAddr,
					   IMG_CPU_PHYADDR *psCpuPAddr,
					   IMG_DEV_PHYADDR *psDevPAddr)
{
	PVR_UNREFERENCED_PARAMETER(hPrivData);

	/* Optimise common case */
	psCpuPAddr[0].uiAddr = IMG_CAST_TO_CPUPHYADDR_UINT(psDevPAddr[0].uiAddr - PLATO_HOSTRAM_DEV_PHYSICAL_BASE);
	if (ui32NumOfAddr > 1)
	{
		IMG_UINT32 ui32Idx;
		for (ui32Idx = 1; ui32Idx < ui32NumOfAddr; ++ui32Idx)
		{
			psCpuPAddr[ui32Idx].uiAddr = IMG_CAST_TO_CPUPHYADDR_UINT(psDevPAddr[ui32Idx].uiAddr - PLATO_HOSTRAM_DEV_PHYSICAL_BASE);
		}
	}
}
#endif /* (PLATO_MEMORY_CONFIG == PLATO_MEMORY_HOST) || (PLATO_MEMORY_CONFIG == PLATO_MEMORY_HYBRID) */

static PVRSRV_ERROR DeviceConfigCreate(void *pvOSDevice,
									   SYS_DATA *psDevData,
									   PVRSRV_DEVICE_CONFIG **ppsDevConfigOut)
{
	PVRSRV_DEVICE_CONFIG *psDevConfig;
	RGX_DATA *psRGXData;
	RGX_TIMING_INFORMATION *psRGXTimingInfo;
	PVRSRV_ERROR eError;

	psDevConfig = OSAllocZMem(sizeof(*psDevConfig) +
							  sizeof(*psRGXData) +
							  sizeof(*psRGXTimingInfo));
	if (!psDevConfig)
	{
		return PVRSRV_ERROR_OUT_OF_MEMORY;
	}

	psRGXData = (RGX_DATA *)((IMG_CHAR *)psDevConfig + sizeof(*psDevConfig));
	psRGXTimingInfo = (RGX_TIMING_INFORMATION *)((IMG_CHAR *)psRGXData + sizeof(*psRGXData));

	/* Set up the RGX timing information */
	psRGXTimingInfo->ui32CoreClockSpeed = ui32CoreClockSpeed;
	psRGXTimingInfo->bEnableActivePM = IMG_FALSE;
	psRGXTimingInfo->bEnableRDPowIsland = IMG_FALSE;
	psRGXTimingInfo->ui32ActivePMLatencyms = SYS_RGX_ACTIVE_POWER_LATENCY_MS;

	/* Set up the RGX data */
	psRGXData->psRGXTimingInfo = psRGXTimingInfo;

	/* Set up the device config */
	eError = InitMemory(psDevData, psDevConfig, &psDevConfig->pasPhysHeaps,
						&psDevConfig->ui32PhysHeapCount);
	if (eError != PVRSRV_OK)
	{
		OSFreeMem(psDevConfig);

		return eError;
	}

	psDevConfig->pvOSDevice = pvOSDevice;
	psDevConfig->pszName = PLATO_SYSTEM_NAME;
	psDevConfig->pszVersion = GetDeviceVersionString(psDevData);

	psDevConfig->sRegsCpuPBase.uiAddr =
		OSPCIAddrRangeStart(psDevData->hRGXPCI, SYS_PLATO_REG_PCI_BASENUM);
	psDevConfig->sRegsCpuPBase.uiAddr += SYS_PLATO_REG_RGX_OFFSET;

	psDevConfig->ui32RegsSize = SYS_PLATO_REG_RGX_SIZE;
	psDevConfig->eDefaultHeap = PVRSRV_PHYS_HEAP_GPU_LOCAL;

	psDevConfig->eCacheSnoopingMode = PVRSRV_DEVICE_SNOOP_NONE;
	psDevConfig->bHasNonMappableLocalMemory = PLATO_HAS_NON_MAPPABLE(psDevData);
	psDevConfig->bHasFBCDCVersion31 = IMG_FALSE;

	psDevConfig->hDevData = psRGXData;
	psDevConfig->hSysData = psDevData;

	psDevConfig->pfnSysDevFeatureDepInit = NULL;

	/* device error notify callback function */
	psDevConfig->pfnSysDevErrorNotify = NULL;

	*ppsDevConfigOut = psDevConfig;

	return PVRSRV_OK;
}

static void DeviceConfigDestroy(PVRSRV_DEVICE_CONFIG *psDevConfig)
{
	if (psDevConfig->pszVersion)
	{
		OSFreeMem(psDevConfig->pszVersion);
	}

	DeInitMemory(psDevConfig->pasPhysHeaps, psDevConfig->ui32PhysHeapCount);

	/*
	 * The device config, RGX data and RGX timing info are part of the same
	 * allocation so do only one free.
	 */
	OSFreeMem(psDevConfig);
}

PVRSRV_ERROR SysDevInit(void *pvOSDevice, PVRSRV_DEVICE_CONFIG **ppsDevConfig)
{
	PVRSRV_DEVICE_CONFIG *psDevConfig;
	SYS_DATA *psDevData;
	PVRSRV_ERROR eError;

	PVR_ASSERT(pvOSDevice);

#if defined(__linux__)
	dma_set_mask(pvOSDevice, DMA_BIT_MASK(40));
#endif

	psDevData = OSAllocZMem(sizeof(*psDevData));
	if (psDevData == NULL)
	{
		return PVRSRV_ERROR_OUT_OF_MEMORY;
	}

	psDevData->hRGXPCI = OSPCISetDev(TO_PCI_COOKIE(pvOSDevice), HOST_PCI_INIT_FLAGS);
	if (!psDevData->hRGXPCI)
	{
		PVR_DPF((PVR_DBG_ERROR, "%s: Failed to acquire PCI device",
				 __func__));
		eError = PVRSRV_ERROR_PCI_DEVICE_NOT_FOUND;
		goto ErrorFreeDevData;
	}

	/*
		Reserve and map system registers (does not include DMA) up to PDP
	 */
	psDevData->uiSystemRegSize = SYS_PLATO_REG_PDP_OFFSET - SYS_PLATO_REG_SYS_OFFSET;

	eError = PlatoMapRegisters(psDevData, SYS_PLATO_REG_PCI_BASENUM, SYS_PLATO_REG_SYS_OFFSET,
		psDevData->uiSystemRegSize, &psDevData->pvSystemRegCpuVBase);
	if (eError != PVRSRV_OK)
	{
		PVR_DPF((PVR_DBG_ERROR, "%s: Failed to map system registers",
				 __func__));
		goto ErrorPCIReleaseDevice;
	}

#if (PLATO_MEMORY_CONFIG == PLATO_MEMORY_LOCAL) || (PLATO_MEMORY_CONFIG == PLATO_MEMORY_HYBRID)
	eError = AcquireLocalMappableMemory(psDevData->hRGXPCI,
										&psDevData->ui64MappedMemCpuPAddr,
										&psDevData->ui64MappedMemSize);
	if (eError != PVRSRV_OK)
	{
		goto ErrorPlatoRegMap;
	}

	PVR_LOG(("Detected Plato mappable memory size: 0x%llx",
			 psDevData->ui64MappedMemSize));
#endif

#if !defined(VIRTUAL_PLATFORM)
	eError = ConfigPlato(psDevData);
	if (eError != PVRSRV_OK)
	{
		goto ErrorReleaseLocalMappableMemory;
	}
#endif

	eError = OSPCIIRQ(psDevData->hRGXPCI, &psDevData->ui32IRQ);
	if (eError != PVRSRV_OK)
	{
		PVR_DPF((PVR_DBG_ERROR, "%s: Couldn't get IRQ", __func__));

		goto ErrorReleaseLocalMappableMemory;
	}

	/* Register our handler */
	eError = OSInstallSystemLISR(&psDevData->hLISR,
								 psDevData->ui32IRQ,
								 PLATO_SYSTEM_NAME,
								 SystemISRHandler,
								 psDevData,
								 SYS_IRQ_FLAG_TRIGGER_DEFAULT | SYS_IRQ_FLAG_SHARED);
	if (eError != PVRSRV_OK)
	{
		PVR_DPF((PVR_DBG_ERROR,
				 "%s: Failed to install the system device interrupt handler",
				 __func__));

		goto ErrorReleaseLocalMappableMemory;
	}

	eError = DeviceConfigCreate(pvOSDevice, psDevData, &psDevConfig);
	if (eError != PVRSRV_OK)
	{
		goto ErrorOSUninstallSystemLISR;
	}

#if (PLATO_MEMORY_CONFIG == PLATO_MEMORY_LOCAL) || (PLATO_MEMORY_CONFIG == PLATO_MEMORY_HYBRID)
	/*
	 * When using local memory perform a read-write test
	 * to confirm that the memory has been configured correctly
	 */
	eError = PlatoLocalMemoryTest(psDevConfig);
	if (eError != PVRSRV_OK)
	{
		goto ErrorDeviceConfigDestroy;
	}
#endif

	*ppsDevConfig = psDevConfig;

	return PVRSRV_OK;

#if (PLATO_MEMORY_CONFIG == PLATO_MEMORY_LOCAL) || (PLATO_MEMORY_CONFIG == PLATO_MEMORY_HYBRID)
ErrorDeviceConfigDestroy:
	DeviceConfigDestroy(psDevConfig);
#endif

ErrorOSUninstallSystemLISR:
	OSUninstallSystemLISR(psDevData->hLISR);

ErrorReleaseLocalMappableMemory:
#if (PLATO_MEMORY_CONFIG == PLATO_MEMORY_LOCAL) || (PLATO_MEMORY_CONFIG == PLATO_MEMORY_HYBRID)
	ReleaseLocalMappableMemory(psDevData->hRGXPCI,
							   psDevData->ui64MappedMemCpuPAddr,
							   psDevData->ui64MappedMemSize);

ErrorPlatoRegMap:
#endif
#if !defined(VIRTUAL_PLATFORM)
	PlatoUnmapRegisters(psDevData, psDevData->pvSystemRegCpuVBase, SYS_PLATO_REG_PCI_BASENUM,
		SYS_PLATO_REG_SYS_OFFSET, psDevData->uiSystemRegSize);
#endif

ErrorPCIReleaseDevice:
	OSPCIReleaseDev(psDevData->hRGXPCI);

ErrorFreeDevData:
	OSFreeMem(psDevData);

	return eError;
}

void SysDevDeInit(PVRSRV_DEVICE_CONFIG *psDevConfig)
{
	SYS_DATA *psDevData = (SYS_DATA *)psDevConfig->hSysData;

	DeviceConfigDestroy(psDevConfig);

	OSUninstallSystemLISR(psDevData->hLISR);

#if (PLATO_MEMORY_CONFIG == PLATO_MEMORY_LOCAL) || (PLATO_MEMORY_CONFIG == PLATO_MEMORY_HYBRID)
	ReleaseLocalMappableMemory(psDevData->hRGXPCI,
							   psDevData->ui64MappedMemCpuPAddr,
							   psDevData->ui64MappedMemSize);
#endif

#if !defined(VIRTUAL_PLATFORM)
	PlatoUnmapRegisters(psDevData, psDevData->pvSystemRegCpuVBase, SYS_PLATO_REG_PCI_BASENUM,
		SYS_PLATO_REG_SYS_OFFSET, psDevData->uiSystemRegSize);
#endif

	OSPCIReleaseDev(psDevData->hRGXPCI);
	OSFreeMem(psDevData);
}

PVRSRV_ERROR SysInstallDeviceLISR(IMG_HANDLE hSysData,
				  IMG_UINT32 ui32IRQ,
				  const IMG_CHAR *pszName,
				  PFN_LISR pfnLISR,
				  void *pvData,
				  IMG_HANDLE *phLISRData)
{
	SYS_DATA *psDevData = (SYS_DATA *)hSysData;
	IMG_UINT32 ui32InterruptFlag;

	switch (ui32IRQ)
	{
		case PLATO_IRQ_GPU:
			ui32InterruptFlag = (0x1 << PLATO_GPU_INT_SHIFT);
			break;
		case PLATO_IRQ_PDP:
			ui32InterruptFlag = (0x1 << PLATO_PDP_INT_SHIFT);
			break;
		case PLATO_IRQ_HDMI:
			ui32InterruptFlag = (0x1 << PLATO_HDMI_INT_SHIFT);
			break;
		default:
			PVR_DPF((PVR_DBG_ERROR, "%s: No device matching IRQ %d",
					 __func__, ui32IRQ));
			return PVRSRV_ERROR_UNABLE_TO_INSTALL_ISR;
	}

	if (psDevData->sInterruptData[ui32IRQ].pfnLISR)
	{
		PVR_DPF((PVR_DBG_ERROR, "%s: ISR for %s already installed!",
				 __func__, pszName));
		return PVRSRV_ERROR_ISR_ALREADY_INSTALLED;
	}

	psDevData->sInterruptData[ui32IRQ].psSysData		= psDevData;
	psDevData->sInterruptData[ui32IRQ].pszName		= pszName;
	psDevData->sInterruptData[ui32IRQ].pfnLISR		= pfnLISR;
	psDevData->sInterruptData[ui32IRQ].pvData		= pvData;
	psDevData->sInterruptData[ui32IRQ].ui32InterruptFlag	= ui32InterruptFlag;

	*phLISRData = &psDevData->sInterruptData[ui32IRQ];

	EnableInterrupt(psDevData, psDevData->sInterruptData[ui32IRQ].ui32InterruptFlag);

	PVR_LOG(("Installed device LISR %s on IRQ %d", pszName, psDevData->ui32IRQ));

	return PVRSRV_OK;

}

PVRSRV_ERROR SysUninstallDeviceLISR(IMG_HANDLE hLISRData)
{

	SYS_INTERRUPT_DATA *psInterruptData = (SYS_INTERRUPT_DATA *)hLISRData;

	PVR_ASSERT(psInterruptData);

	PVR_LOG(("Uninstalling device LISR %s on IRQ %d", psInterruptData->pszName, ((SYS_DATA *)psInterruptData->psSysData)->ui32IRQ));

	/* Disable interrupts for this device */
#if !defined(VIRTUAL_PLATFORM)
	DisableInterrupt(psInterruptData->psSysData, psInterruptData->ui32InterruptFlag);
#endif

	/* Reset the interrupt data */
	psInterruptData->pszName = NULL;
	psInterruptData->psSysData = NULL;
	psInterruptData->pfnLISR = NULL;
	psInterruptData->pvData = NULL;
	psInterruptData->ui32InterruptFlag = 0;

	return PVRSRV_OK;
}

PVRSRV_ERROR SysGetPlatoGPUTemp(SYS_DATA * psDevData, IMG_UINT32 *ui32Temp)
{
	IMG_UINT32 ui32Value = 0;
#if defined(PLATO_TEMPA_SENSOR_IMPLEMENTED)
	void* pvSystemTempRegs;

	/* Temp sensor A starts at 0x80000 within peripheral regs */
	pvSystemTempRegs = psDevData->pvSystemRegCpuVBase + SYS_PLATO_REG_TEMPA_OFFSET;

	OSWriteHWReg32(pvSystemTempRegs, 0x04, 0x3);
	Poll(pvSystemTempRegs, 0x10, (1 << 16), (1 << 16));
	ui32Value = OSReadHWReg32(pvSystemTempRegs, 0x10) & 0xFFF;

	ui32Value = ((ui32Value * 352) / 4096) - 109;
#endif  // PLATO_TEMPA_SENSOR_IMPLEMENTED

	*ui32Temp = ui32Value;

	return PVRSRV_OK;
}
