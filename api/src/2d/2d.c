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

void hmi2d_handle_ack(hmi_t *hmi, const unsigned char *msg)
{
    int size = msg[1];
    const unsigned char *data = msg + 2;
    if(size == 1) {
        if(GET_U8(data) == hmi->resp2d_msg_id) {
            hmi->resp2d_msg_id = 0;
            hmi->resp2d_ack = 1;
        }
    } else {
        HMI_BAD_DATA("hmi2d_handle_ack", "Expected message size of 1 byte",
                     size, 0);
    }
}

int hmi2d_wait_response(hmi_t *hmi, int id, int timeout)
{
    int result = HMI_NO_ERROR;

    hmi->resp2d_ack = 0;
    hmi->resp2d_msg_id = id;

    for(;;) {
        /* Receive and handle message */
        result = hmi_message_receive(hmi, &timeout);
        if(result != HMI_NO_ERROR) {
            if(result == HMI_NO_DATA)
                result = HMI_NO_RESPONSE_ERROR;
            break;
        }

        /* Check whether we got a response */
        if(hmi->resp2d_ack)
            break;
    }

    return result;
}

int hmi2d_send_message(hmi_t *hmi, int id, int size, void *msg, int timeout)
{
    int retries;
    int result = HMI_NO_ERROR;

    for(retries = 5; retries > 0; --retries) {
        result = hmi2d_message_write(hmi, id, size, msg);
        if(result != HMI_NO_ERROR)
            continue;

        result = hmi2d_wait_response(hmi, id, timeout);
        if(result == HMI_NO_ERROR)
            break;
    }
    return result;
}

void hmi2d_message_handle(hmi_t *hmi, const unsigned char *msg)
{
    int id = GET_U8(msg);

    if(0) {
#ifndef HMI2D_NO_UPDATE
    } else if(id == hmi2d_msg_r_update) {
        hmi2d_handle_update_response(hmi, msg);
#endif
    } else if(id == hmi2d_msg_r_parameter) {
        hmi2d_handle_parameter(hmi, msg);
#ifndef HMI2D_NO_DATA_RETRIEVAL
    } else if((id >= hmi2d_msg_r_mutual_raw_0) &&
              (id <= hmi2d_msg_r_mutual_raw_f))
    {
        int row_idx = id - hmi2d_msg_r_mutual_raw_0;
        hmi2d_row_t *row = &hmi->internal2d.mutual_raw[row_idx];
        hmi2d_handle_data_row(hmi, row, msg);
    } else if((id >= hmi2d_msg_r_mutual_cal_0) &&
              (id <= hmi2d_msg_r_mutual_cal_f))
    {
        int row_idx = id - hmi2d_msg_r_mutual_cal_0;
        hmi2d_row_t *row = &hmi->internal2d.mutual_cal[row_idx];
        hmi2d_handle_data_row(hmi, row, msg);
    } else if(id == hmi2d_msg_r_mouse_btns) {
        hmi2d_handle_mouse_btns(hmi, msg);
    } else if(id == hmi2d_msg_r_gesture) {
        hmi2d_handle_gesture(hmi, msg);
#endif
    } else if(id == hmi2d_msg_r_ack) {
        hmi2d_handle_ack(hmi, msg);
#ifndef HMI2D_NO_DATA_RETRIEVAL
    } else if(id == hmi2d_msg_r_finger_pos) {
        hmi2d_handle_finger_pos(hmi, msg);
    } else if(id == hmi2d_msg_r_self_raw) {
        hmi2d_handle_data_row(hmi, &hmi->internal2d.self_raw, msg);
    } else if(id == hmi2d_msg_r_self_measure) {
        hmi2d_handle_data_row(hmi, &hmi->internal2d.self_cal, msg);
#endif
    } else if(id == hmi2d_msg_r_fw_version) {
        hmi2d_handle_fw_version(hmi, msg);
    }
}
