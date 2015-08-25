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

#ifndef HMI3D_NO_UPDATE

#define CRC_P_32 0xEDB88320L

static unsigned int crc(hmi_t *hmi, const unsigned char *msg, int size)
{
    unsigned int crc = 0xFFFFFFFF;
    int i;
    for(i = 0; i < size; ++i)
        crc = (crc >> 8) ^ hmi->flash.crc_table[(crc ^ msg[i]) & 0xFF];
    return crc ^ 0xFFFFFFFF;
}

static void init_crc(hmi_t *hmi)
{
    int i, j;
    unsigned long crc;

    for (i = 0; i < 256; ++i) {
        crc = i;

        for (j = 0; j < 8; ++j) {
            if ( crc & 0x00000001L )
                crc = (crc >> 1) ^ CRC_P_32;
            else
                crc = crc >> 1;
        }

        hmi->flash.crc_table[i] = crc;
    }

    hmi->flash.crc_intialized = 1;
}

int hmi3d_wait_for_version_info(hmi_t *hmi)
{
    int error = HMI_NO_ERROR;
    /* 100 is a usefull timeout here because the bootloader requires
     * some time to start and will continue with firmware after
     * a timeout of 100 ms
     */
    int timeout = 100;

    for(;;) {
        /* Receive and handle one message */
        error = hmi_message_receive(hmi, &timeout);
        if(error != HMI_NO_ERROR) {
            if(error == HMI_NO_DATA)
                error = HMI_NO_RESPONSE_ERROR;
            break;
        }

        /* Check whether we received version info */
        if(hmi->version_request->received) {
            error = HMI_NO_ERROR;
            break;
        }
    }

    return error;
}

int hmi3d_update_begin(hmi_t *hmi, unsigned int session_id, void *iv,
                       hmi3d_UpdateFunction_t mode)
{
    int error = HMI_NO_ERROR;
    unsigned char msg[28];
    hmi3d_version_request_t v_request;

    /* Init table for CRC32-checksum */
    if(!hmi->flash.crc_intialized)
        init_crc(hmi);

    /* Prepare flash start message */
    HMI_MEMSET(msg, 0, sizeof(msg));
    SET_U8(msg, sizeof(msg));
    SET_U8(msg + 3, hmi3d_msg_Fw_Update_Start);
    SET_U32(msg+8, session_id);
    HMI_MEMCPY(msg+12, iv, 14);
    SET_U8(msg+26, mode);

    HMI_MEMSET(&v_request, 0, sizeof(v_request));

    SET_U32(msg+4, crc(hmi, msg+8, 20));

    hmi->flash.session_id = session_id;

    /* Reset device and wait for the firmware-version */
    hmi->version_request = &v_request;
    error = hmi3d_reset(hmi);

    if(!error)
        error = hmi3d_wait_for_version_info(hmi);
    hmi->version_request = 0;

    /* Send message to start flash-mode */
    if(!error)
        error = hmi3d_send_message(hmi, msg, sizeof(msg), 100);

    return error;
}

int hmi3d_update_write(hmi_t *hmi, unsigned short address,
                       unsigned char length, unsigned char *record,
                       hmi3d_UpdateFunction_t mode)
{
    unsigned char msg[140];

    /* Assert that only verification is used for verification sessions */
    HMI_ASSERT(hmi->flash.session_mode != hmi3d_UpdateFunction_VerifyOnly ||
            mode == hmi3d_UpdateFunction_VerifyOnly);

    HMI_MEMSET(msg, 0, sizeof(msg));
    SET_U8(msg, sizeof(msg));
    SET_U8(msg + 3, hmi3d_msg_Fw_Update_Block);
    SET_U16(msg + 8, address);
    SET_U8(msg + 10, length);
    SET_U8(msg + 11, mode);
    HMI_MEMCPY(msg + 12, record, 128);

    SET_U32(msg + 4, crc(hmi, msg + 8, 132));

    return hmi3d_send_message(hmi, msg, sizeof(msg), 100);
}

int hmi3d_update_end(hmi_t *hmi, unsigned char *version)
{
    int error = HMI_NO_ERROR;
    unsigned char msg[136];
    HMI_MEMSET(msg, 0, sizeof(msg));

    /* Finish by writing the version information */

    SET_U8(msg, sizeof(msg));
    SET_U8(msg + 3, hmi3d_msg_Fw_Update_Completed);
    SET_U32(msg + 8, hmi->flash.session_id);
    SET_U8(msg + 12, hmi->flash.session_mode);
    HMI_MEMCPY(msg + 13, version, 120);

    SET_U32(msg + 4, crc(hmi, msg + 8, 128));

    error = hmi3d_send_message(hmi, msg, sizeof(msg), 100);

    /* Finally restart device */

    if(!error) {
        SET_U8(msg + 12, hmi3d_UpdateFunction_Restart);
        HMI_MEMSET(msg + 13, 0, 120);

        SET_U32(msg + 4, crc(hmi, msg + 8, 128));

        error = hmi3d_send_message(hmi, msg, sizeof(msg), 100);
    }

    return error;
}

int hmi3d_update_image(hmi_t *hmi,
                       unsigned int session_id,
                       hmi3d_update_image_t *image,
                       hmi3d_UpdateFunction_t mode)
{
    int error = HMI_NO_ERROR;
    int i;
    hmi3d_update_record_t *record;

    error = hmi3d_update_begin(hmi, session_id, image->iv, mode);

    for(i = 0; !error && i < image->record_count; ++i) {
        record = image->data + i;
        error = hmi3d_update_write(hmi, record->address, record->length,
                                   record->data, mode);
    }

    if(!error)
        error = hmi3d_update_end(hmi, image->fw_version);

    return error;
}

int hmi3d_update_wait_loader_done(hmi_t *hmi)
{
    int error = HMI_NO_ERROR;
    /* The complete bootloader-update process can take up to 20 seconds */
    int timeout = 20000;

    /* 0xFF is an invalid value for this field in fw-version-info */
    hmi->fw_valid = 0xFF;

    for(;;) {
        /* Receive and handle one message */
        error = hmi_message_receive(hmi, &timeout);
        if(error != HMI_NO_ERROR) {
            if(error == HMI_NO_DATA)
                error = HMI_NO_RESPONSE_ERROR;
            break;
        }

        /* Check whether we got a response */
        if(hmi->fw_valid == 0) {
            error = HMI_NO_ERROR;
            break;
        }
    }

    return error;
}

#endif
