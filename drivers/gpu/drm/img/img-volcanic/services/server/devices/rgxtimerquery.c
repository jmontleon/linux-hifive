/*************************************************************************/ /*!
@File
@Title          RGX Timer queries
@Copyright      Copyright (c) Imagination Technologies Ltd. All Rights Reserved
@Description    RGX Timer queries
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

#include "rgxtimerquery.h"
#include "rgxdevice.h"
#include "rgxtimecorr.h"

#include "rgxfwutils.h"
#include "pdump_km.h"

PVRSRV_ERROR
PVRSRVRGXBeginTimerQueryKM(CONNECTION_DATA    * psConnection,
                           PVRSRV_DEVICE_NODE * psDeviceNode,
                           IMG_UINT32         ui32QueryId)
{
	PVRSRV_RGXDEV_INFO * psDevInfo = (PVRSRV_RGXDEV_INFO *)psDeviceNode->pvDevice;

	PVR_UNREFERENCED_PARAMETER(psConnection);

	if (ui32QueryId >= RGX_MAX_TIMER_QUERIES)
	{
		return PVRSRV_ERROR_INVALID_PARAMS;
	}

#if !defined(PVRSRV_USE_BRIDGE_LOCK)
	OSLockAcquire(psDevInfo->hTimerQueryLock);
#endif

	psDevInfo->bSaveStart = IMG_TRUE;
	psDevInfo->bSaveEnd   = IMG_TRUE;

	/* clear the stamps, in case there is no Kick */
	psDevInfo->pui64StartTimeById[ui32QueryId] = 0UL;
	psDevInfo->pui64EndTimeById[ui32QueryId]   = 0UL;
	OSWriteMemoryBarrier(&psDevInfo->pui64EndTimeById[ui32QueryId]);
	RGXFwSharedMemCacheOpValue(psDevInfo->pui64StartTimeById[ui32QueryId], FLUSH);
	RGXFwSharedMemCacheOpValue(psDevInfo->pui64EndTimeById[ui32QueryId], FLUSH);

	/* save of the active query index */
	psDevInfo->ui32ActiveQueryId = ui32QueryId;

#if !defined(PVRSRV_USE_BRIDGE_LOCK)
	OSLockRelease(psDevInfo->hTimerQueryLock);
#endif

	return PVRSRV_OK;
}


PVRSRV_ERROR
PVRSRVRGXEndTimerQueryKM(CONNECTION_DATA    * psConnection,
                         PVRSRV_DEVICE_NODE * psDeviceNode)
{
	PVRSRV_RGXDEV_INFO * psDevInfo = (PVRSRV_RGXDEV_INFO *)psDeviceNode->pvDevice;

	PVR_UNREFERENCED_PARAMETER(psConnection);

#if !defined(PVRSRV_USE_BRIDGE_LOCK)
	OSLockAcquire(psDevInfo->hTimerQueryLock);
#endif

	/* clear off the flags set by Begin(). Note that _START_TIME is
	 * probably already cleared by Kick()
	 */
	psDevInfo->bSaveStart = IMG_FALSE;
	psDevInfo->bSaveEnd   = IMG_FALSE;

#if !defined(PVRSRV_USE_BRIDGE_LOCK)
	OSLockRelease(psDevInfo->hTimerQueryLock);
#endif

	return PVRSRV_OK;
}


PVRSRV_ERROR
PVRSRVRGXQueryTimerKM(CONNECTION_DATA    * psConnection,
                      PVRSRV_DEVICE_NODE * psDeviceNode,
                      IMG_UINT32         ui32QueryId,
                      IMG_UINT64         * pui64StartTime,
                      IMG_UINT64         * pui64EndTime)
{
	PVRSRV_RGXDEV_INFO * psDevInfo = (PVRSRV_RGXDEV_INFO *)psDeviceNode->pvDevice;
	IMG_UINT32         ui32Scheduled;
	IMG_UINT32         ui32Completed;
	PVRSRV_ERROR       eError;

	PVR_UNREFERENCED_PARAMETER(psConnection);

	if (ui32QueryId >= RGX_MAX_TIMER_QUERIES)
	{
		return PVRSRV_ERROR_INVALID_PARAMS;
	}

#if !defined(PVRSRV_USE_BRIDGE_LOCK)
	OSLockAcquire(psDevInfo->hTimerQueryLock);
#endif

	ui32Scheduled = psDevInfo->aui32ScheduledOnId[ui32QueryId];

	RGXFwSharedMemCacheOpValue(psDevInfo->pui32CompletedById[ui32QueryId], INVALIDATE);
	ui32Completed = psDevInfo->pui32CompletedById[ui32QueryId];

	/* if there was no kick since the Begin() on this id we return 0-s as Begin cleared
	 * the stamps. If there was no begin the returned data is undefined - but still
	 * safe from services pov
	 */
	if (ui32Completed >= ui32Scheduled)
	{
		RGXFwSharedMemCacheOpValue(psDevInfo->pui64StartTimeById[ui32QueryId], INVALIDATE);
		RGXFwSharedMemCacheOpValue(psDevInfo->pui64EndTimeById[ui32QueryId], INVALIDATE);
		* pui64StartTime = psDevInfo->pui64StartTimeById[ui32QueryId];
		* pui64EndTime   = psDevInfo->pui64EndTimeById[ui32QueryId];

		eError = PVRSRV_OK;
	}
	else
	{
		eError = PVRSRV_ERROR_RESOURCE_UNAVAILABLE;
	}

#if !defined(PVRSRV_USE_BRIDGE_LOCK)
	OSLockRelease(psDevInfo->hTimerQueryLock);
#endif
	return eError;
}



/******************************************************************************
 NOT BRIDGED/EXPORTED FUNCS
******************************************************************************/

void
RGX_GetTimestampCmdHelper(PVRSRV_RGXDEV_INFO      * psDevInfo,
                          PRGXFWIF_TIMESTAMP_ADDR * ppPreAddr,
                          PRGXFWIF_TIMESTAMP_ADDR * ppPostAddr,
                          PRGXFWIF_UFO_ADDR       * ppUpdate)
{
	if (ppPreAddr != NULL)
	{
		if (psDevInfo->bSaveStart)
		{
			/* drop the SaveStart on the first Kick */
			psDevInfo->bSaveStart = IMG_FALSE;

			RGXSetFirmwareAddress(ppPreAddr,
			                      psDevInfo->psStartTimeMemDesc,
			                      sizeof(IMG_UINT64) * psDevInfo->ui32ActiveQueryId,
			                      RFW_FWADDR_NOREF_FLAG);
		}
		else
		{
			ppPreAddr->ui32Addr = 0;
		}
	}

	if (ppPostAddr != NULL && ppUpdate != NULL)
	{
		if (psDevInfo->bSaveEnd)
		{
			RGXSetFirmwareAddress(ppPostAddr,
			                      psDevInfo->psEndTimeMemDesc,
			                      sizeof(IMG_UINT64) * psDevInfo->ui32ActiveQueryId,
			                      RFW_FWADDR_NOREF_FLAG);

			psDevInfo->aui32ScheduledOnId[psDevInfo->ui32ActiveQueryId]++;

			RGXSetFirmwareAddress(ppUpdate,
			                      psDevInfo->psCompletedMemDesc,
			                      sizeof(IMG_UINT32) * psDevInfo->ui32ActiveQueryId,
			                      RFW_FWADDR_NOREF_FLAG);
		}
		else
		{
			ppUpdate->ui32Addr   = 0;
			ppPostAddr->ui32Addr = 0;
		}
	}
}


/******************************************************************************
 End of file (rgxtimerquery.c)
******************************************************************************/
