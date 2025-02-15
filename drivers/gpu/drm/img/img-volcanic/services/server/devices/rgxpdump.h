/*************************************************************************/ /*!
@File
@Title          RGX pdump Functionality
@Copyright      Copyright (c) Imagination Technologies Ltd. All Rights Reserved
@Description    RGX pdump functionality
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
#include "pvrsrv_error.h"
#include "rgxdevice.h"
#include "device.h"
#include "devicemem.h"
#include "pdump_km.h"
#include "pvr_debug.h"

#if defined(PDUMP)
/*!
*******************************************************************************

 @Function	PVRSRVPDumpSignatureBufferKM

 @Description

 Dumps TA and 3D signature and checksum buffers

 @Return   PVRSRV_ERROR

******************************************************************************/
PVRSRV_ERROR PVRSRVPDumpSignatureBufferKM(CONNECTION_DATA * psConnection,
                                          PVRSRV_DEVICE_NODE * psDeviceNode,
                                          IMG_UINT32 ui32PDumpFlags);


/*!
*******************************************************************************

 @Function	PVRSRVPDumpCRCSignatureCheckKM

 @Description

 Poll on FBC/FBDC end-to-end signature status

 @Return   PVRSRV_ERROR

******************************************************************************/
PVRSRV_ERROR PVRSRVPDumpCRCSignatureCheckKM(CONNECTION_DATA * psConnection,
                                            PVRSRV_DEVICE_NODE * psDeviceNode,
                                            IMG_UINT32 ui32PDumpFlags);

/*!
*******************************************************************************

 @Function	PVRSRVPDumpValCheckPreCommandKM

 @Description

 Poll on various GPU status/signature status for validation, before
 sending the GPU command.

 @Return   PVRSRV_ERROR

******************************************************************************/
PVRSRV_ERROR PVRSRVPDumpValCheckPreCommandKM(CONNECTION_DATA * psConnection,
                                             PVRSRV_DEVICE_NODE * psDeviceNode,
                                             IMG_UINT32 ui32PDumpFlags);

/*!
*******************************************************************************

 @Function	PVRSRVPDumpValCheckPostCommandKM

 @Description

 Poll on various GPU status/signature status for validation, after
 sending the GPU command.

 @Return   PVRSRV_ERROR

******************************************************************************/
PVRSRV_ERROR PVRSRVPDumpValCheckPostCommandKM(CONNECTION_DATA * psConnection,
                                              PVRSRV_DEVICE_NODE * psDeviceNode,
                                              IMG_UINT32 ui32PDumpFlags);

/*!
*******************************************************************************

 @Function	PVRSRVPDumpTraceBufferKM

 @Description

 Dumps TA and 3D signature and checksum buffers

 @Return   PVRSRV_ERROR

******************************************************************************/
PVRSRV_ERROR PVRSRVPDumpTraceBufferKM(CONNECTION_DATA * psConnection,
                                      PVRSRV_DEVICE_NODE *psDeviceNode,
                                      IMG_UINT32 ui32PDumpFlags);

/*!
*******************************************************************************

 @Function	RGXPDumpPrepareOutputImageDescriptorHdr

 @Description

 Dumps the header for an OutputImage command

 @Return   PVRSRV_ERROR

******************************************************************************/
PVRSRV_ERROR RGXPDumpPrepareOutputImageDescriptorHdr(PVRSRV_DEVICE_NODE *psDeviceNode,
									IMG_UINT32 ui32HeaderSize,
									IMG_UINT32 ui32DataSize,
									IMG_UINT32 ui32LogicalWidth,
									IMG_UINT32 ui32LogicalHeight,
									IMG_UINT32 ui32PhysicalWidth,
									IMG_UINT32 ui32PhysicalHeight,
									PDUMP_PIXEL_FORMAT ePixFmt,
									IMG_MEMLAYOUT eMemLayout,
									IMG_FB_COMPRESSION eFBCompression,
									const IMG_UINT32 *paui32FBCClearColour,
									PDUMP_FBC_SWIZZLE eFBCSwizzle,
									IMG_PBYTE abyPDumpDesc);

/*!
*******************************************************************************

 @Function	RGXPDumpPrepareOutputDataDescriptorHdr

 @Description

 Dumps the header for an OutputData command

 @Return   PVRSRV_ERROR

******************************************************************************/
PVRSRV_ERROR RGXPDumpPrepareOutputDataDescriptorHdr(PVRSRV_DEVICE_NODE *psDeviceNode,
									IMG_UINT32 ui32HeaderType,
									IMG_UINT32 ui32DataSize,
									IMG_UINT32 ui32ElementType,
									IMG_UINT32 ui32ElementCount,
									IMG_PBYTE pbyPDumpDataHdr);

#else /* PDUMP */

#ifdef INLINE_IS_PRAGMA
#pragma inline(PVRSRVPDumpSignatureBufferKM)
#endif
static INLINE PVRSRV_ERROR
PVRSRVPDumpSignatureBufferKM(CONNECTION_DATA * psConnection,
                             PVRSRV_DEVICE_NODE *psDeviceNode,
                             IMG_UINT32 ui32PDumpFlags)
{
	PVR_UNREFERENCED_PARAMETER(psConnection);
	PVR_UNREFERENCED_PARAMETER(psDeviceNode);
	PVR_UNREFERENCED_PARAMETER(ui32PDumpFlags);
	return PVRSRV_OK;
}

#ifdef INLINE_IS_PRAGMA
#pragma inline(PVRSRVPDumpTraceBufferKM)
#endif
static INLINE PVRSRV_ERROR
PVRSRVPDumpTraceBufferKM(CONNECTION_DATA * psConnection,
                         PVRSRV_DEVICE_NODE *psDeviceNode,
                         IMG_UINT32 ui32PDumpFlags)
{
	PVR_UNREFERENCED_PARAMETER(psConnection);
	PVR_UNREFERENCED_PARAMETER(psDeviceNode);
	PVR_UNREFERENCED_PARAMETER(ui32PDumpFlags);
	return PVRSRV_OK;
}
#endif /* PDUMP */
/******************************************************************************
 End of file (rgxpdump.h)
******************************************************************************/
