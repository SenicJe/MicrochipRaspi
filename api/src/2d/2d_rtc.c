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

void hmi2d_handle_parameter(hmi_t *hmi, const unsigned char *msg)
{
    hmi_param_request_t *request;
    int size = msg[1];
    const unsigned char *data = msg + 2;
    int param_id;
    int value = 0;
    int i;

    if(size < 3 || size > 6) {
        HMI_BAD_DATA("hmi2d_handle_parameter",
                     "Expected message size of 3 to 6 bytes",
                     size, 0);
        return;
    }

#ifdef HMI_SYNC_THREADING
    /* Synchronize against calls to hmi2d_get_param from application */
    HMI_SYNC_LOCK(hmi->io_sync);
#endif

    request = hmi->param2d_request;
    param_id = GET_U16(data);
    if(request && request->param == param_id) {
        for(i = 0; i < size-2; ++i)
            value |= GET_U8(data + 2 + i) << (8*i);

        if(request->arg0)
            *request->arg0 = value;

        request->param = 0;
    }

#ifdef HMI_SYNC_THREADING
    /* Release synchronization against hmi2d_get_param */
    HMI_SYNC_UNLOCK(hmi->io_sync);
#endif
}

int hmi2d_set_param(hmi_t *hmi, hmi2d_parameter_id_t param,
                    int arg0, int arg1)
{
    unsigned char msg[10];

    HMI_ASSERT(hmi && HMI_CONNECTED(hmi));

    SET_U16(msg, param);
    SET_U32(msg+2, arg0);
    SET_U32(msg+6, arg1);

    return hmi2d_send_message(hmi, hmi2d_msg_t_set_param,
                              sizeof(msg), msg, 100);
}

int hmi2d_get_param(hmi_t *hmi,
                    hmi2d_parameter_id_t param,
                    unsigned int *arg0)
{
    char request_msg[2];
    hmi_param_request_t request;
    int result;

    HMI_ASSERT(hmi && HMI_CONNECTED(hmi));

    /* Enable receiving of parameters */
    request.param = param;
    request.arg0 = arg0;
    request.arg1 = 0;
    hmi->param2d_request = &request;

    /* Do actual receiving */
    SET_U16(request_msg, param);
    result = hmi2d_send_message(hmi, hmi2d_msg_t_get_param,
                                sizeof(request_msg), request_msg, 100);

    /* Disable receiving of parameters */
#ifdef HMI_SYNC_THREADING
    HMI_SYNC_LOCK(hmi->io_sync);
    hmi->param2d_request = 0;
    HMI_SYNC_UNLOCK(hmi->io_sync);
#else
    hmi->param2d_request = 0;
#endif

    /* Check whether parameter was received */
    if(result == HMI_NO_ERROR && request.param)
        result = HMI_MSG_MISSING_ERROR;
    return result;
}

#ifndef HMI2D_NO_RTC

int hmi2d_set_active_mask(hmi_t *hmi, hmi2d_active_mask_t active,
                          hmi2d_active_mask_t mask)
{
    return hmi2d_set_param(hmi, hmi2d_param_active_mask, active, mask);
}

int hmi2d_get_active_mask(hmi_t *hmi, hmi2d_active_mask_t *active)
{
    int value;
    int result = hmi2d_get_param(hmi, hmi2d_param_active_mask, &value);
    *active = value;
    return result;
}

int hmi2d_set_full_mutual(hmi_t *hmi, int enabled)
{
    int value = enabled ? hmi2d_active_full_mutual : 0;
    return hmi2d_set_active_mask(hmi, value, hmi2d_active_full_mutual);
}

int hmi2d_get_full_mutual(hmi_t *hmi, int *enabled)
{
    hmi2d_active_mask_t mask;
    int result = hmi2d_get_active_mask(hmi, &mask);
    *enabled = mask & hmi2d_active_full_mutual ? 1 : 0;
    return result;
}

int hmi2d_set_operation_mode(hmi_t *hmi, hmi2d_operation_mode_t mode)
{
    return hmi2d_set_param(hmi, hmi2d_param_operation_mode, mode, ~0);
}

int hmi2d_get_operation_mode(hmi_t *hmi, hmi2d_operation_mode_t *mode)
{
    int value;
    int result = hmi2d_get_param(hmi, hmi2d_param_operation_mode, &value);
    *mode = value;
    return result;
}

int hmi2d_set_com_mask(hmi_t *hmi,
                       hmi2d_com_mask_t enabled,
                       hmi2d_com_mask_t mask)
{
    return hmi2d_set_param(hmi, hmi2d_param_com_mask, enabled, mask);
}

int hmi2d_get_com_mask(hmi_t *hmi, hmi2d_com_mask_t *mask)
{
    int value;
    int result = hmi2d_get_param(hmi, hmi2d_param_com_mask, &value);
    *mask = value;
    return result;
}

int hmi2d_set_key_combo(hmi_t *hmi, int param, hmi2d_key_combo_t *combo)
{
    int result = HMI_NO_ERROR;

    HMI_ASSERT(hmi && HMI_CONNECTED(hmi));
    HMI_ASSERT((param & 0xF) == 0);
    HMI_ASSERT((param >= hmi2d_param_outkeyFlickL_send) &&
               (param <= hmi2d_param_outkeyApproach_send));

    result = hmi2d_set_param(hmi, param + 0, combo->cond, -1);

    if(result == HMI_NO_ERROR)
        result = hmi2d_set_param(hmi, param + 1, combo->key[0], -1);

    if(result == HMI_NO_ERROR)
        result = hmi2d_set_param(hmi, param + 2, combo->key[1], -1);

    if(result == HMI_NO_ERROR)
        result = hmi2d_set_param(hmi, param + 3, combo->key[2], -1);

    return result;
}

int hmi2d_get_key_combo(hmi_t *hmi, int param, hmi2d_key_combo_t *combo)
{
    int value;
    int result = HMI_NO_ERROR;

    HMI_ASSERT(hmi && HMI_CONNECTED(hmi));
    HMI_ASSERT((param & 0xF) == 0);
    HMI_ASSERT((param >= hmi2d_param_outkeyFlickL_send) &&
               (param <= hmi2d_param_outkeyApproach_send));

    /* Fetch first parameter (condition) */
    result = hmi2d_get_param(hmi, param + 0, &value);

    if(result == HMI_NO_ERROR) {
        combo->cond = value;

        /* Fetch next parameter (first key) */
        result = hmi2d_get_param(hmi, param + 1, &value);
    }
    if(result == HMI_NO_ERROR) {
        combo->key[0] = value;

        /* Fetch next parameter (second key) */
        result = hmi2d_get_param(hmi, param + 2, &value);
    }
    if(result == HMI_NO_ERROR) {
        combo->key[1] = value;

        /* Fetch next parameter (third key) */
        result = hmi2d_get_param(hmi, param + 3, &value);
    }
    if(result == HMI_NO_ERROR) {
        combo->key[2] = value;
    }

    return result;
}

#endif
