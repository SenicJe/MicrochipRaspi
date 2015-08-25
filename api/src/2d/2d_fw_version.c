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
#include "2d.h"

void hmi2d_handle_fw_version(hmi_t *hmi, const unsigned char *msg)
{
    int size = msg[1];
    const unsigned char *data = msg + 2;
    hmi2d_version_request_t *request;
    hmi2d_version_info_t *version;

    if(size != 128) {
        HMI_BAD_DATA("hmi2d_handle_fw_version",
                     "Expected message size of 128 bytes",
                     size, 0);
        return;
    }

#ifdef HMI3D_SYNC_THREADING
    /* Synchronize against hmi2d_query_fw_version calls from application */
    HMI_SYNC_LOCK(hmi->io_sync);
#endif

    request = hmi->version2d_request;
    if(request) {
        version = request->version;
        if(version) {
            version->svn_revision = GET_U16(data + 0);
            version->wc_mixed = GET_U8(data + 2);
            version->wc_modified = GET_U8(data + 3);
            version->build_year = GET_U16(data + 4);
            version->build_month = GET_U8(data + 6);
            version->build_day = GET_U8(data + 7);
            version->build_hour = GET_U8(data + 8);
            version->build_minute = GET_U8(data + 9);
            version->build_second = GET_U8(data + 10);
            HMI_MEMCPY(version->buildBy, data + 11, 9);
            HMI_MEMCPY(version->infoString, data + 20, 76);
            version->checksum = GET_U32(data + 108);
            version->fw_app_id = GET_U32(data + 120);
            HMI_MEMCPY(version->fw_version, data + 124, 4);
        }
        request->received = 1;
    }

#ifdef HMI3D_SYNC_THREADING
    /* Release synchronization against hmi2d_query_fw_version */
    HMI_SYNC_UNLOCK(hmi->io_sync);
#endif
}

int hmi2d_query_fw_version(hmi_t *hmi, hmi2d_version_info_t *version)
{
    hmi2d_version_request_t request;
    int result;

    HMI_ASSERT(hmi && HMI_CONNECTED(hmi));

    /* Enable receiving of version */
    request.version = version;
    request.received = 0;
    hmi->version2d_request = &request;

    result = hmi2d_send_message(hmi, hmi2d_msg_t_fw_version, 0, 0, 100);

#ifdef HMI3D_SYNC_THREADING
    HMI_SYNC_LOCK(hmi->io_sync);
    hmi->version2d_request = 0;
    HMI_SYNC_UNLOCK(hmi->io_sync);
#else
    hmi->version2d_request = 0;
#endif

    if(result == HMI_NO_ERROR && !request.received)
        result = HMI_MSG_MISSING_ERROR;

    return result;
}
