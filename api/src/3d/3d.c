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

void hmi3d_message_handle(hmi_t *hmi, const void *msg, int size) {
    /* NOTE messages are ensured to have a size of at least 4 bytes  */
    const unsigned char *data = (const unsigned char*)msg;

    switch(GET_U8(data+3)) {
    case hmi3d_msg_System_Status:
        hmi3d_handle_system_status(hmi, data, size);
        break;
#if !defined(HMI3D_NO_FW_VERSION) || !defined(HMI3D_NO_UPDATE)
    case hmi3d_msg_Fw_Version_Info:
        hmi3d_handle_version_info(hmi, data, size);
        break;
#endif
#ifndef HMI3D_NO_DATA_RETRIEVAL
    case hmi3d_msg_Sensor_Data_Output:
        hmi3d_handle_data_output(hmi, data);
        break;
#endif
#ifndef HMI3D_NO_RTC
    case hmi3d_msg_Set_Runtime_Parameter:
        hmi3d_handle_runtime_parameter(hmi, data);
        break;
#endif
    default:
        break;
    }
}

void hmi3d_handle_system_status(hmi_t *hmi,
                                const unsigned char *data,
                                int size)
{
    if(size == 16) {
        int msg_id = GET_U8(data + 4);
        int error_code = GET_U16(data + 6);
        if(msg_id == hmi->resp_msg_id ||
                error_code == hmi3d_system_WakeupHappened)
        {
            hmi->resp_msg_id = 0;
            hmi->resp_error_code = error_code;
        }
    } else {
        HMI_BAD_DATA("hmi3d_handle_system_status",
                     "Expected message size of 16 bytes",
                     size, 0);
    }
}

static int wait_response(hmi_t *hmi, int msg_id, int timeout) {
    int error = HMI_NO_ERROR;

    hmi->resp_error_code = -1;
    hmi->resp_msg_id = msg_id;

    for(;;) {
        /* Receive and handle message */
        error = hmi_message_receive(hmi, &timeout);
        if(error != HMI_NO_ERROR) {
            if(error == HMI_NO_DATA)
                error = HMI_NO_RESPONSE_ERROR;
            break;
        }

        /* Check whether we got a response */
        if(hmi->resp_error_code >= 0) {
            if(hmi->resp_error_code != 0)
                error = HMI_3D_SYSTEM_ERROR;
            break;
        }
    }

    return error;
}

int hmi3d_send_message(hmi_t *hmi, void *msg, int size, int timeout) {
    int retries;
    int last_error = HMI_NO_ERROR;
    int msg_id = ((unsigned char *)msg)[3];

    /* Retry 2 times before accepting a failure */
    for(retries = 3; retries > 0; --retries) {
        last_error = hmi3d_message_write(hmi, msg, size);
        if(last_error)
            continue;

        last_error = wait_response(hmi, msg_id, timeout);
        if(!last_error)
            break;
    }
    return last_error;
}

int hmi3d_request_message(hmi_t *hmi,
                          unsigned char msgId,
                          unsigned int param,
                          int timeout) {
    unsigned char msg[12];
    HMI_MEMSET(msg, 0, sizeof(msg));
    SET_U8(msg, sizeof(msg));
    SET_U8(msg + 3, hmi3d_msg_Request_Message);
    SET_U8(msg + 4, msgId);
    SET_U32(msg + 8, param);
    return hmi3d_send_message(hmi, msg, sizeof(msg), timeout);
}

