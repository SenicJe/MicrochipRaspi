/******************************************************************************
 *
 * Copyright (C) 2014 Microchip Technology Inc. and its
 *                    subsidiaries ("Microchip").
 *
 * All rights reserved.
 *
 * You are permitted to use the Aurea software, 3DTouchPad SDK, and other
 * accompanying software with Microchip products.  Refer to the license
 * agreement accompanying this software, if any, for additional info regarding
 * your rights and obligations.
 *
 * SOFTWARE AND DOCUMENTATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION, ANY WARRANTY OF
 * MERCHANTABILITY, TITLE, NON-INFRINGEMENT AND FITNESS FOR A PARTICULAR
 * PURPOSE. IN NO EVENT SHALL MICROCHIP, SMSC, OR ITS LICENSORS BE LIABLE OR
 * OBLIGATED UNDER CONTRACT, NEGLIGENCE, STRICT LIABILITY, CONTRIBUTION, BREACH
 * OF WARRANTY, OR OTHER LEGAL EQUITABLE THEORY FOR ANY DIRECT OR INDIRECT
 * DAMAGES OR EXPENSES INCLUDING BUT NOT LIMITED TO ANY INCIDENTAL, SPECIAL,
 * INDIRECT OR CONSEQUENTIAL DAMAGES, OR OTHER SIMILAR COSTS.
 *
 ******************************************************************************/
#include "3d.h"

#if !defined(HMI3D_NO_FW_VERSION) || !defined(HMI3D_NO_UPDATE)
void hmi3d_handle_version_info(hmi_t *hmi,
                                const unsigned char *data,
                                int size)
{
    hmi3d_version_request_t *request;
    int v_size;
    if(size != 132) {
        HMI_BAD_DATA("hmi3d_handle_version_info",
                     "Expected message size of 132 bytes",
                     size, 0);
        return;
    }

#ifdef HMI_SYNC_THREADING
    /* Synchronize against hmi3d_query_fw_version calls from application */
    HMI_SYNC_LOCK(hmi->io_sync);
#endif

    hmi->fw_valid = GET_U8(data + 4);
    request = hmi->version_request;
    if(request) {
        v_size = request->size > 120 ? 120 : request->size;
        HMI_MEMCPY(request->version, data + 12, v_size);
        request->received = 1;
    }

#ifdef HMI_SYNC_THREADING
    /* Release synchronization against Application-Layer */
    HMI_SYNC_UNLOCK(hmi->io_sync);
#endif
}
#endif

#ifndef HMI3D_NO_FW_VERSION
int hmi3d_query_fw_version(hmi_t *hmi, char *version,
                           int v_length)
{
    hmi3d_version_request_t request;
    int error;

    request.version = version;
    request.size = v_length;
    request.received = 0;
    hmi->version_request = &request;
    error = hmi3d_request_message(hmi, hmi3d_msg_Fw_Version_Info, 0, 100);

#ifdef HMI_SYNC_THREADING
    HMI_SYNC_LOCK(hmi->io_sync);
    hmi->version_request = 0;
    HMI_SYNC_UNLOCK(hmi->io_sync);
#else
    hmi->version_request = 0;
#endif

    if(!error && !request.received)
        error = HMI_MSG_MISSING_ERROR;
    return error;
}
#endif
