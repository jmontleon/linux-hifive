/*******************************************************************************
@File
@Title          Common bridge header for ri
@Copyright      Copyright (c) Imagination Technologies Ltd. All Rights Reserved
@Description    Declares common defines and structures used by both the client
                and server side of the bridge for ri
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
*******************************************************************************/

#ifndef COMMON_RI_BRIDGE_H
#define COMMON_RI_BRIDGE_H

#include <powervr/mem_types.h>

#include "img_defs.h"
#include "img_types.h"
#include "pvrsrv_error.h"

#include "ri_typedefs.h"

#define PVRSRV_BRIDGE_RI_CMD_FIRST			0
#define PVRSRV_BRIDGE_RI_RIWRITEPMRENTRY			PVRSRV_BRIDGE_RI_CMD_FIRST+0
#define PVRSRV_BRIDGE_RI_RIWRITEMEMDESCENTRY			PVRSRV_BRIDGE_RI_CMD_FIRST+1
#define PVRSRV_BRIDGE_RI_RIWRITEPROCLISTENTRY			PVRSRV_BRIDGE_RI_CMD_FIRST+2
#define PVRSRV_BRIDGE_RI_RIUPDATEMEMDESCADDR			PVRSRV_BRIDGE_RI_CMD_FIRST+3
#define PVRSRV_BRIDGE_RI_RIDELETEMEMDESCENTRY			PVRSRV_BRIDGE_RI_CMD_FIRST+4
#define PVRSRV_BRIDGE_RI_RIDUMPLIST			PVRSRV_BRIDGE_RI_CMD_FIRST+5
#define PVRSRV_BRIDGE_RI_RIDUMPALL			PVRSRV_BRIDGE_RI_CMD_FIRST+6
#define PVRSRV_BRIDGE_RI_RIDUMPPROCESS			PVRSRV_BRIDGE_RI_CMD_FIRST+7
#define PVRSRV_BRIDGE_RI_RIWRITEPMRENTRYWITHOWNER			PVRSRV_BRIDGE_RI_CMD_FIRST+8
#define PVRSRV_BRIDGE_RI_CMD_LAST			(PVRSRV_BRIDGE_RI_CMD_FIRST+8)

/*******************************************
            RIWritePMREntry
 *******************************************/

/* Bridge in structure for RIWritePMREntry */
typedef struct PVRSRV_BRIDGE_IN_RIWRITEPMRENTRY_TAG
{
	IMG_HANDLE hPMRHandle;
} __packed PVRSRV_BRIDGE_IN_RIWRITEPMRENTRY;

/* Bridge out structure for RIWritePMREntry */
typedef struct PVRSRV_BRIDGE_OUT_RIWRITEPMRENTRY_TAG
{
	PVRSRV_ERROR eError;
} __packed PVRSRV_BRIDGE_OUT_RIWRITEPMRENTRY;

/*******************************************
            RIWriteMEMDESCEntry
 *******************************************/

/* Bridge in structure for RIWriteMEMDESCEntry */
typedef struct PVRSRV_BRIDGE_IN_RIWRITEMEMDESCENTRY_TAG
{
	IMG_UINT64 ui64Flags;
	IMG_UINT64 ui64Offset;
	IMG_UINT64 ui64Size;
	IMG_HANDLE hPMRHandle;
	const IMG_CHAR *puiTextB;
	IMG_UINT32 ui32TextBSize;
} __packed PVRSRV_BRIDGE_IN_RIWRITEMEMDESCENTRY;

/* Bridge out structure for RIWriteMEMDESCEntry */
typedef struct PVRSRV_BRIDGE_OUT_RIWRITEMEMDESCENTRY_TAG
{
	IMG_HANDLE hRIHandle;
	PVRSRV_ERROR eError;
} __packed PVRSRV_BRIDGE_OUT_RIWRITEMEMDESCENTRY;

/*******************************************
            RIWriteProcListEntry
 *******************************************/

/* Bridge in structure for RIWriteProcListEntry */
typedef struct PVRSRV_BRIDGE_IN_RIWRITEPROCLISTENTRY_TAG
{
	IMG_UINT64 ui64DevVAddr;
	IMG_UINT64 ui64Size;
	const IMG_CHAR *puiTextB;
	IMG_UINT32 ui32TextBSize;
} __packed PVRSRV_BRIDGE_IN_RIWRITEPROCLISTENTRY;

/* Bridge out structure for RIWriteProcListEntry */
typedef struct PVRSRV_BRIDGE_OUT_RIWRITEPROCLISTENTRY_TAG
{
	IMG_HANDLE hRIHandle;
	PVRSRV_ERROR eError;
} __packed PVRSRV_BRIDGE_OUT_RIWRITEPROCLISTENTRY;

/*******************************************
            RIUpdateMEMDESCAddr
 *******************************************/

/* Bridge in structure for RIUpdateMEMDESCAddr */
typedef struct PVRSRV_BRIDGE_IN_RIUPDATEMEMDESCADDR_TAG
{
	IMG_DEV_VIRTADDR sAddr;
	IMG_HANDLE hRIHandle;
} __packed PVRSRV_BRIDGE_IN_RIUPDATEMEMDESCADDR;

/* Bridge out structure for RIUpdateMEMDESCAddr */
typedef struct PVRSRV_BRIDGE_OUT_RIUPDATEMEMDESCADDR_TAG
{
	PVRSRV_ERROR eError;
} __packed PVRSRV_BRIDGE_OUT_RIUPDATEMEMDESCADDR;

/*******************************************
            RIDeleteMEMDESCEntry
 *******************************************/

/* Bridge in structure for RIDeleteMEMDESCEntry */
typedef struct PVRSRV_BRIDGE_IN_RIDELETEMEMDESCENTRY_TAG
{
	IMG_HANDLE hRIHandle;
} __packed PVRSRV_BRIDGE_IN_RIDELETEMEMDESCENTRY;

/* Bridge out structure for RIDeleteMEMDESCEntry */
typedef struct PVRSRV_BRIDGE_OUT_RIDELETEMEMDESCENTRY_TAG
{
	PVRSRV_ERROR eError;
} __packed PVRSRV_BRIDGE_OUT_RIDELETEMEMDESCENTRY;

/*******************************************
            RIDumpList
 *******************************************/

/* Bridge in structure for RIDumpList */
typedef struct PVRSRV_BRIDGE_IN_RIDUMPLIST_TAG
{
	IMG_HANDLE hPMRHandle;
} __packed PVRSRV_BRIDGE_IN_RIDUMPLIST;

/* Bridge out structure for RIDumpList */
typedef struct PVRSRV_BRIDGE_OUT_RIDUMPLIST_TAG
{
	PVRSRV_ERROR eError;
} __packed PVRSRV_BRIDGE_OUT_RIDUMPLIST;

/*******************************************
            RIDumpAll
 *******************************************/

/* Bridge in structure for RIDumpAll */
typedef struct PVRSRV_BRIDGE_IN_RIDUMPALL_TAG
{
	IMG_UINT32 ui32EmptyStructPlaceholder;
} __packed PVRSRV_BRIDGE_IN_RIDUMPALL;

/* Bridge out structure for RIDumpAll */
typedef struct PVRSRV_BRIDGE_OUT_RIDUMPALL_TAG
{
	PVRSRV_ERROR eError;
} __packed PVRSRV_BRIDGE_OUT_RIDUMPALL;

/*******************************************
            RIDumpProcess
 *******************************************/

/* Bridge in structure for RIDumpProcess */
typedef struct PVRSRV_BRIDGE_IN_RIDUMPPROCESS_TAG
{
	IMG_PID ui32Pid;
} __packed PVRSRV_BRIDGE_IN_RIDUMPPROCESS;

/* Bridge out structure for RIDumpProcess */
typedef struct PVRSRV_BRIDGE_OUT_RIDUMPPROCESS_TAG
{
	PVRSRV_ERROR eError;
} __packed PVRSRV_BRIDGE_OUT_RIDUMPPROCESS;

/*******************************************
            RIWritePMREntryWithOwner
 *******************************************/

/* Bridge in structure for RIWritePMREntryWithOwner */
typedef struct PVRSRV_BRIDGE_IN_RIWRITEPMRENTRYWITHOWNER_TAG
{
	IMG_HANDLE hPMRHandle;
	IMG_PID ui32Owner;
} __packed PVRSRV_BRIDGE_IN_RIWRITEPMRENTRYWITHOWNER;

/* Bridge out structure for RIWritePMREntryWithOwner */
typedef struct PVRSRV_BRIDGE_OUT_RIWRITEPMRENTRYWITHOWNER_TAG
{
	PVRSRV_ERROR eError;
} __packed PVRSRV_BRIDGE_OUT_RIWRITEPMRENTRYWITHOWNER;

#endif /* COMMON_RI_BRIDGE_H */
