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

void hmi3d_handle_runtime_parameter(hmi_t *hmi,
                                    const unsigned char *data)
{
    hmi_param_request_t *request;
    int param;

#ifdef HMI_SYNC_THREADING
    /* Synchronize against calls to hmi3d_get_param from application */
    HMI_SYNC_LOCK(hmi->io_sync);
#endif

    request = hmi->param_request;
    param = GET_U16(data + 4);
    if(request && request->param == param) {
        if(request->arg0)
            *request->arg0 = GET_U32(data + 8);
        if(request->arg1)
            *request->arg1 = GET_U32(data + 12);
        request->param = 0;
    }

#ifdef HMI_SYNC_THREADING
    /* Release synchronization against hmi3d_get_param */
    HMI_SYNC_UNLOCK(hmi->io_sync);
#endif
}

int hmi3d_set_param(hmi_t *hmi, unsigned short param,
                    unsigned int arg0, unsigned int arg1)
{
    unsigned char msg[16];
    HMI_MEMSET(msg, 0, sizeof(msg));
    SET_U8(msg, sizeof(msg));
    SET_U8(msg + 3, hmi3d_msg_Set_Runtime_Parameter);
    SET_U16(msg + 4, param);
    SET_U32(msg + 8, arg0);
    SET_U32(msg + 12, arg1);
    return hmi3d_send_message(hmi, msg, sizeof(msg), 100);
}

int hmi3d_get_param(hmi_t *hmi, unsigned short param,
                    unsigned int *arg0, unsigned int *arg1)
{
    hmi_param_request_t request;
    int error;

    /* Enable receiving of parameters */
    request.param = param;
    request.arg0 = arg0;
    request.arg1 = arg1;
    hmi->param_request = &request;

    /* Do actual receiving */
    error = hmi3d_request_message(hmi, hmi3d_msg_Set_Runtime_Parameter,
                                  param, 100);

    /* Disable receiving of parameters */
#ifdef HMI_SYNC_THREADING
    HMI_SYNC_LOCK(hmi->io_sync);
    hmi->param_request = 0;
    HMI_SYNC_UNLOCK(hmi->io_sync);
#else
    hmi->param_request = 0;
#endif

    /* Check wheter parameter was received */
    if(error == HMI_NO_ERROR && request.param)
        error = HMI_MSG_MISSING_ERROR;
    return error;
}

int hmi3d_trigger_action(hmi_t *hmi, unsigned short action)
{
    return hmi3d_set_param(hmi, hmi3d_param_trigger, action, 0);
}

#ifndef HMI3D_NO_DATA_RETRIEVAL

int hmi3d_set_output_enable_mask(hmi_t *hmi, hmi3d_DataOutConfigMask_t flags,
                                 hmi3d_DataOutConfigMask_t lock,
                                 hmi3d_DataOutConfigMask_t mask)
{
    int error;

    HMI_ASSERT(hmi && HMI_CONNECTED(hmi));

    error = hmi3d_set_param(hmi, hmi3d_param_dataOutputLockMask, lock, mask);

    if(!error)
        error = hmi3d_set_param(hmi, hmi3d_param_dataOutputEnableMask,
                                flags, mask);

    return error;
}

int hmi3d_get_output_enable_mask(hmi_t *hmi, hmi3d_DataOutConfigMask_t *flags,
                                 hmi3d_DataOutConfigMask_t *locked)
{
    int error;
    unsigned int value;

    HMI_ASSERT(hmi && HMI_CONNECTED(hmi));

    if(flags) {
        error = hmi3d_get_param(hmi, hmi3d_param_dataOutputEnableMask,
                                &value, 0);

        if(!error)
            *flags = value & hmi3d_DataOutConfigMask_OutputAll;
    }
    if(!error && locked) {
        error = hmi3d_get_param(hmi, hmi3d_param_dataOutputLockMask,
                                &value, 0);
        if(!error)
            *locked = value & hmi3d_DataOutConfigMask_OutputAll;
    }
    return error;
}

#endif

#ifndef HMI3D_NO_RTC

int hmi3d_set_auto_calibration(hmi_t *hmi, int enabled)
{
    int mode = enabled ? 0x00 : 0x3F;

    HMI_ASSERT(hmi && HMI_CONNECTED(hmi));

    return hmi3d_set_param(hmi, hmi3d_param_dspCalOpMode, mode, 0x3F);
}

int hmi3d_get_auto_calibration(hmi_t *hmi, int *enabled)
{
    int error;
    unsigned int value;
    HMI_ASSERT(hmi && HMI_CONNECTED(hmi));

    error = hmi3d_get_param(hmi, hmi3d_param_dspCalOpMode, &value, 0);

    if(!error && enabled)
        *enabled = value & 0x3F;
    return error;
}

int hmi3d_force_calibration(hmi_t *hmi)
{
    HMI_ASSERT(hmi && HMI_CONNECTED(hmi));

    return hmi3d_trigger_action(hmi, hmi3d_trigger_calibration);
}

int hmi3d_select_frequencies(hmi_t *hmi, hmi3d_frequencies_t frequencies)
{
    int error = HMI_BAD_PARAM_ERROR;
    int count = 0;
    int list = 0xFFFFF;
    int i;

    HMI_ASSERT(hmi && HMI_CONNECTED(hmi));

    for(i = 0; i < 5; ++i) {
        if(frequencies & (1 << i)) {
            list = (list << 4) | i;
            ++count;
        }
    }

    list &= 0x000FFFFF;

    if(count)
        error = hmi3d_set_param(hmi, hmi3d_param_transFreqSelect, count, list);

    return error;

}

int hmi3d_set_approach_detection(hmi_t *hmi, int enabled)
{
    HMI_ASSERT(hmi && HMI_CONNECTED(hmi));

    return hmi3d_set_param(hmi, hmi3d_param_dspApproachDetectionMode,
                           enabled ? 0x01 : 0x00, 0x01);
}

int hmi3d_get_approach_detection(hmi_t *hmi, int *enabled)
{
    int error;
    unsigned int value;
    HMI_ASSERT(hmi && HMI_CONNECTED(hmi));

    error = hmi3d_get_param(hmi, hmi3d_param_dspApproachDetectionMode,
                            &value, 0);

    if(!error && enabled)
        *enabled = value & 0x01;
    return error;
}

int hmi3d_set_enabled_gestures(hmi_t *hmi, int gestures) {
    HMI_ASSERT(hmi && HMI_CONNECTED(hmi));

    return hmi3d_set_param(hmi, hmi3d_param_dspGestureMask,
                           gestures & 0x7F, 0x7F);
}

int hmi3d_get_enabled_gestures(hmi_t *hmi, int *gestures)
{
    int error;
    unsigned int value;
    HMI_ASSERT(hmi && HMI_CONNECTED(hmi));

    error = hmi3d_get_param(hmi, hmi3d_param_dspGestureMask, &value, 0);

    if(!error && gestures)
        *gestures = value & 0x7F;
    return error;
}

int hmi3d_set_touch_detection(hmi_t *hmi, int enabled)
{
    unsigned int touchFlag = 0;

    HMI_ASSERT(hmi && HMI_CONNECTED(hmi));

    if(enabled)
        touchFlag = 0x08;
    return hmi3d_set_param(hmi, hmi3d_param_dspTouchConfig, touchFlag, 0x08);
}

int hmi3d_get_touch_detection(hmi_t *hmi, int *enabled)
{
    int error;
    unsigned int value;
    HMI_ASSERT(hmi && HMI_CONNECTED(hmi));

    error = hmi3d_get_param(hmi, hmi3d_param_dspTouchConfig, &value, 0);

    if(!error && enabled)
        *enabled = value & 0x80;
    return error;
}

int hmi3d_set_air_wheel_enabled(hmi_t *hmi, int enabled)
{
    unsigned int airWheelFlag = 0;
    HMI_ASSERT(hmi && HMI_CONNECTED(hmi));

    if(enabled)
        airWheelFlag = 0x20;
    return hmi3d_set_param(hmi, hmi3d_param_dspAirWheelConfig,
                           airWheelFlag, 0x20);
}

int hmi3d_get_air_wheel_enabled(hmi_t *hmi, int *enabled)
{
    int error;
    unsigned int value;
    HMI_ASSERT(hmi && HMI_CONNECTED(hmi));

    error = hmi3d_get_param(hmi, hmi3d_param_dspAirWheelConfig, &value, 0);

    if(!error && enabled)
        *enabled = value & 0x20;
    return error;
}

int hmi3d_make_persistent(hmi_t *hmi, hmi3d_param_category category)
{
    HMI_ASSERT(hmi && HMI_CONNECTED(hmi));

    return hmi3d_set_param(hmi, hmi3d_param_makePersistent, category, 0);
}


#endif
