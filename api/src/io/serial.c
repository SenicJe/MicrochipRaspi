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
#include "io.h"

#if HMI_IO == HMI_IO_CDC_SERIAL

void hmi3d_init_msg_extract(hmi_t *hmi) {
    hmi->io.msg_extract.state = -2;
}

static void *message_extract(hmi3d_msg_extract_t *extract, int *size) {
    /* This while loop is only left through one of the return statements */
    while(1) {
        switch(extract->state) {
        case -2:
            /* Expect FE */
            if(extract->buffer_cursor >= extract->buffer_size)
                return 0;
            if(extract->buffer[extract->buffer_cursor++] != 0xFE)
                continue;
            extract->state = -1;
        case -1:
            /* Expect FF */
            if(extract->buffer_cursor >= extract->buffer_size)
                return 0;
            if(extract->buffer[extract->buffer_cursor++] != 0xFF) {
                extract->state = -2;
                continue;
            }
            extract->state = 0;
        case 0: case 1: case 2: case 3:
            /* Read header */
            while(extract->state < 4) {
                if(extract->buffer_cursor >= extract->buffer_size)
                    return 0;
                extract->msg[extract->state++] = extract->buffer[extract->buffer_cursor++];
            }
            /* Check header */
            if(extract->msg[0] < 4) {
                extract->state = -2;
                continue;
            }
        default:
            /* Read data */
            while(extract->state < extract->msg[0]) {
                if(extract->buffer_cursor >= extract->buffer_size)
                    return 0;
                extract->msg[extract->state++] = extract->buffer[extract->buffer_cursor++];
            }
            if(size)
                *size = extract->state;
            extract->state = -2;
            return extract->msg;
        }
    }
}

int hmi_message_receive(hmi_t *hmi, int *timeout)
{
    int error = HMI_NO_DATA;
    int msg_size;
    void *msg = 0;

    for(;;) {
        msg = message_extract(&hmi->io.msg_extract, &msg_size);
        if(msg) {
            hmi3d_message_handle(hmi, msg, msg_size);
            error = HMI_NO_ERROR;
            break;
        }

        /* Try to read more data to retry message-extraction */
        hmi->io.msg_extract.buffer_cursor = 0;
        hmi->io.msg_extract.buffer_size = hmi3d_serial_read(hmi, hmi->io.msg_extract.buffer, HMI3D_INPUT_CAPACITY);
        if(hmi->io.msg_extract.buffer_size > 0)
            continue;

        /* Wait a short time before retrying */
        if(!timeout || (*timeout <= 0))
            break;

        HMI_SLEEP(10);
        *timeout -= 10;
    }

    return error;
}

int hmi3d_message_write(hmi_t *hmi, void *msg, int size) {
    static const unsigned char feff[] = { 0xFE, 0xFF };
    int status, error = HMI_NO_ERROR;

    /* Avoid copying and send it in two parts, shouldn't be a problem as we use serial streams */
    if((status = hmi3d_serial_write(hmi, (void*)feff, sizeof(feff))) < 0)
        error = status;
    if(!error && (status = hmi3d_serial_write(hmi, msg, size)) < 0)
        error = status;
    return error;
}

#endif /* HMI_IO == HMI_IO_CDC_SERIAL */
