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

#ifndef HMI2D_NO_UPDATE

void hmi2d_handle_update_response(hmi_t *hmi, const unsigned char *msg)
{
    int size = msg[1];
    if(size == 1)
        hmi->bootloader_2d_error_code = msg[2];
}

static int update_wait_response(hmi_t *hmi)
{
    int result = HMI_NO_ERROR;
    int timeout = 100;

    hmi->bootloader_2d_error_code = -1;

    for(;;) {
        /* Receive and handle message */
        result = hmi_message_receive(hmi, &timeout);
        if(result != HMI_NO_ERROR) {
            if(result == HMI_NO_DATA)
                result = HMI_NO_RESPONSE_ERROR;
            break;
        }

        /* Check whether we got a response */
        if(hmi->bootloader_2d_error_code >= 0) {
            if(hmi->bootloader_2d_error_code == 0)
                result = HMI_NO_ERROR;
            else
                result = HMI_2D_BOOTLOADER_ERROR;
            break;
        }
    }

    return result;
}

int hmi2d_update_enter_bootloader(hmi_t *hmi)
{
    unsigned char cmd[] = { 0xF0 };
    int result;

    HMI_ASSERT(hmi && HMI_CONNECTED(hmi));

    result = hmi2d_message_write(hmi, hmi2d_msg_t_update, sizeof(cmd), cmd);

    if(result == HMI_NO_ERROR)
        result = update_wait_response(hmi);

    return result;
}

int hmi2d_update_unlock(hmi_t *hmi)
{
    unsigned char cmd[] = { 0xF7 };
    int result;

    HMI_ASSERT(hmi && HMI_CONNECTED(hmi));

    result = hmi2d_message_write(hmi, hmi2d_msg_t_update, sizeof(cmd), cmd);

    if(result == HMI_NO_ERROR)
        result = update_wait_response(hmi);

    return result;
}

int hmi2d_update_erase_memory(hmi_t *hmi)
{
    unsigned char cmd[5] = { 0xF2 };
    int result;
    int addr;

    HMI_ASSERT(hmi && HMI_CONNECTED(hmi));

    for(addr = HMI2D_PROG_MEM_START;
        addr < HMI2D_PROG_MEM_END;
        addr += HMI2D_PROG_PAGE_SIZE)
    {
        SET_U32(cmd + 1, addr);

        result = hmi2d_message_write(hmi, hmi2d_msg_t_update, sizeof(cmd), cmd);
        if(result != HMI_NO_ERROR)
            break;

        result = update_wait_response(hmi);
        if(result != HMI_NO_ERROR)
            break;
    }

    return result;
}

int hmi2d_update_flash_block(hmi_t *hmi, int addr, hmi2d_update_block_t *block)
{
    unsigned char addrCmd[5] = { 0xF4 };
    unsigned char dataCmd[35] = { 0xF5, 0x08 };
    int result;
    int i, j;

    HMI_ASSERT(hmi && HMI_CONNECTED(hmi));
    HMI_ASSERT(addr >= HMI2D_PROG_MEM_START);
    HMI_ASSERT(addr + HMI2D_PROG_BLOCK_SIZE <= HMI2D_PROG_MEM_END);

    SET_U32(addrCmd + 1, addr);
    result = hmi2d_message_write(hmi, hmi2d_msg_t_update,
                                 sizeof(addrCmd), addrCmd);

    if(result == HMI_NO_ERROR)
        result = update_wait_response(hmi);

    for(i = 0; (result == HMI_NO_ERROR) && (i < HMI2D_PROG_BLOCK_SIZE/32); ++i)
    {
        SET_U8(dataCmd + 2, i);

        for(j = 0; j < 32; ++j)
            SET_U8(dataCmd + 3 + j, (*block)[i*32 + j]);

        result = hmi2d_message_write(hmi, hmi2d_msg_t_update,
                                     sizeof(dataCmd), dataCmd);
        if(result != HMI_NO_ERROR)
            break;

        result = update_wait_response(hmi);
    }

    return result;
}

int hmi2d_update_flash_memory(hmi_t *hmi, hmi2d_update_image_t *image)
{
    int result;
    int i;

    HMI_ASSERT(hmi && HMI_CONNECTED(hmi));

    for(i = 0; i < HMI2D_PROG_BLOCK_COUNT; ++i) {
        int addr = HMI2D_PROG_MEM_START + i * HMI2D_PROG_BLOCK_SIZE;
        result = hmi2d_update_flash_block(hmi, addr, (*image) + i);

        if(result != HMI_NO_ERROR)
            break;
    }
    return result;
}

int hmi2d_update_exit_bootloader(hmi_t *hmi)
{
    unsigned char cmd[] = { 0xF6 };
    int result;

    HMI_ASSERT(hmi && HMI_CONNECTED(hmi));

    result = hmi2d_message_write(hmi, hmi2d_msg_t_update, sizeof(cmd), cmd);

    if(result == HMI_NO_ERROR)
        result = update_wait_response(hmi);

    return result;
}

#endif
