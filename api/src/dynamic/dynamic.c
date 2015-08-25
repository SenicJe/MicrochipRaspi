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
#include "../impl.h"

#if defined(HMI_API_DYNAMIC) || defined(HMI_HAS_DYNAMIC)

const char *hmi_version_str(void) {
    return HMI_API_VERSION_STRING;
}

hmi_t *hmi_create_(int hmi_rev) {
    if(hmi_rev != HMI_API_REV)
        return NULL;
    return (hmi_t *)HMI_MALLOC(sizeof(hmi_t));
}

void hmi_free(hmi_t *hmi) {
    HMI_ASSERT(hmi);

    HMI_FREE(hmi);
}

hmi3d_system_error_t hmi3d_get_system_error(hmi_t *hmi)
{
    HMI_ASSERT(hmi);

    return hmi->resp_error_code;
}

#ifndef HMI2D_NO_UPDATE
hmi2d_bootloader_error_t hmi2d_get_bootloader_error(hmi_t *hmi)
{
    HMI_ASSERT(hmi);
    return hmi->bootloader_2d_error_code;
}
#endif

#ifndef HMI3D_NO_DATA_RETRIEVAL

hmi3d_signal_t *hmi3d_get_cic(hmi_t *hmi) {
    HMI_ASSERT(hmi);

    return &hmi->result.cic;
}

hmi3d_signal_t *hmi3d_get_sd(hmi_t *hmi) {
    HMI_ASSERT(hmi);

    return &hmi->result.sd;
}

hmi3d_position_t *hmi3d_get_position(hmi_t *hmi) {
    HMI_ASSERT(hmi);

    return &hmi->result.pos;
}

hmi3d_gesture_t *hmi3d_get_gesture(hmi_t *hmi) {
    HMI_ASSERT(hmi);

    return &hmi->result.gesture;
}

hmi3d_touch_t *hmi3d_get_touch(hmi_t *hmi) {
    HMI_ASSERT(hmi);

    return &hmi->result.touch;
}

hmi3d_air_wheel_t *hmi3d_get_air_wheel(hmi_t *hmi) {
    HMI_ASSERT(hmi);

    return &hmi->result.air_wheel;
}

hmi3d_calib_t *hmi3d_get_calibration(hmi_t *hmi) {
    HMI_ASSERT(hmi);

    return &hmi->result.calib;
}

hmi3d_freq_t *hmi3d_get_frequency(hmi_t *hmi) {
    HMI_ASSERT(hmi);
    return &hmi->result.frequency;
}

hmi3d_noise_power_t *hmi3d_get_noise_power(hmi_t *hmi) {
    HMI_ASSERT(hmi);
    return &hmi->result.noise_power;
}

#endif

#ifndef HMI2D_NO_DATA_RETRIEVAL

hmi2d_block_t *hmi2d_get_mutual_raw(hmi_t *hmi)
{
    return &hmi->result2d.mutual_raw;
}

hmi2d_block_t *hmi2d_get_mutual_cal(hmi_t *hmi)
{
    return &hmi->result2d.mutual_cal;
}

hmi2d_row_t *hmi2d_get_self_raw(hmi_t *hmi)
{
    return &hmi->result2d.self_raw;
}

hmi2d_row_t *hmi2d_get_self_cal(hmi_t *hmi)
{
    return &hmi->result2d.self_cal;
}

hmi2d_finger_pos_list_t *hmi2d_get_finger_positions(hmi_t *hmi)
{
    return &hmi->result2d.fingers;
}

hmi2d_mouse_t *hmi2d_get_mouse(hmi_t *hmi)
{
    return &hmi->result2d.mouse;
}

hmi2d_gesture_t *hmi2d_get_gesture(hmi_t *hmi)
{
    return &hmi->result2d.gesture;
}

#endif

#endif
