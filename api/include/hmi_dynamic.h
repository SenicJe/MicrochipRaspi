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
#ifndef HMI_API_DYNAMIC_H
#define HMI_API_DYNAMIC_H

/* This file is included by hmi_api.h and should not be included directly.
 *
 * As most IDEs don't take definitions from the file that is including the
 * header into consideration when parsing a header this file in turn
 * includes hmi_api.h to help the parser find the correct definitions.
 */
#ifndef HMI_API_H
#include "hmi_api.h"
#endif

/* ======== Version ======== */

/* Function: hmi_version_str
 *
 * Returns the 3DTouchPad SDK version as a string.
 */
HMI_API const char * CDECL hmi_version_str(void);

/* ======== Allocation ======== */

/* Function: hmi_create
 *
 * Creates a new <hmi_t>-instance if the API is compatible.
 *
 * Returns a null pointer if the application was compiled against
 * a different API than the HMI library (see <HMI_API_REV>) or if there
 * was not enough memory available.
 * Otherwise returns the newly allocated <hmi_t>-instance.
 *
 * The returned instance has to be freed with <hmi_free> after use.
 *
 * Before the instance could be used it has to be initialized with
 * <hmi_initialize>.
 *
 * See also:
 *    <hmi_free>, <hmi_initialize>, <hmi_cleanup>
 */
HMI_API hmi_t * CDECL hmi_create_(int api_rev);

#define hmi_create() hmi_create_(HMI_API_REV)

/* Function: hmi_free
 *
 * Releases hmi that was aquired with <hmi_create>.
 *
 * Instances that were initialized with <hmi_initialize> have to be
 * cleaned with <hmi_cleanup> before.
 *
 * See also:
 *    <hmi_create>, <hmi_initialize>, <hmi_cleanup>
 */
HMI_API void CDECL hmi_free(hmi_t *hmi);

/* Function: hmi3d_get_system_error
 *
 * Returns the error code of the response to the last request.
 *
 * See also:
 *    <hmi3d_system_error_t>, <HMI_3D_SYSTEM_ERROR>, <hmi3d_send_message>
 */
HMI_API hmi3d_system_error_t CDECL hmi3d_get_system_error(hmi_t *hmi);

#ifndef HMI2D_NO_UPDATE
/* Function: hmi2d_get_bootloader_error
 *
 * Returns the error code returned by the 2D bootloader device.
 *
 * See also:
 *    <2D Firmware Update>
 */
HMI_API hmi2d_bootloader_error_t CDECL hmi2d_get_bootloader_error(hmi_t *hmi);
#endif

/* ======== Data Access ======== */

#ifndef HMI3D_NO_DATA_RETRIEVAL

/* Function: hmi3d_get_cic
 *
 * Returns the pointer to the CIC-signals.
 */
HMI_API hmi3d_signal_t * CDECL hmi3d_get_cic(hmi_t *hmi);

/* Function: hmi3d_get_sd
 *
 * Returns the pointer to the SD-signals.
 */
HMI_API hmi3d_signal_t * CDECL hmi3d_get_sd(hmi_t *hmi);

/* Function: hmi3d_get_position
 *
 * Returns the pointer to the position-data.
 */
HMI_API hmi3d_position_t * CDECL hmi3d_get_position(hmi_t *hmi);

/* Function: hmi3d_get_gesture
 *
 * Returns the pointer to the gesture-data.
 */
HMI_API hmi3d_gesture_t * CDECL hmi3d_get_gesture(hmi_t *hmi);

/* Function: hmi3d_get_touch
 *
 * Returns the pointer to the touch-related data of hmi.
 */
HMI_API hmi3d_touch_t * CDECL hmi3d_get_touch(hmi_t *hmi);

/* Function: hmi3d_get_air_wheel
 *
 * Return the pointer to the AirWheel-related data of hmi.
 */
HMI_API hmi3d_air_wheel_t * CDECL hmi3d_get_air_wheel(hmi_t *hmi);

/* Function: hmi3d_get_calibration
 *
 * Returns the pointer to the calibration-data.
 *
 * Important:
 *    Calibration data is only communicated when
 *    <hmi3d_DataOutConfigMask_SDData> was selected via
 *    <hmi3d_set_output_enable_mask>. Otherwise calibrations would
 *    not be detected.
 */
HMI_API hmi3d_calib_t * CDECL hmi3d_get_calibration(hmi_t *hmi);

/* Function: hmi3d_get_frequency
 *
 * Returns the pointer to the frequency-data of hmi.
 */
HMI_API hmi3d_freq_t * CDECL hmi3d_get_frequency(hmi_t *hmi);

/* Function: hmi3d_get_noise_power
 *
 * Returns the pointer to the noise-power-data of hmi.
 */
HMI_API hmi3d_noise_power_t * CDECL hmi3d_get_noise_power(hmi_t *hmi);

#endif

#ifndef HMI2D_NO_DATA_RETRIEVAL

/* Function: hmi2d_get_mutual_raw
 *
 * Returns the pointer to the raw mutual 2D sensor data.
 */
HMI_API hmi2d_block_t * CDECL hmi2d_get_mutual_raw(hmi_t *hmi);

/* Function: hmi2d_get_mutual_cal
 *
 * Returns the pointer to the calibrated mutual 2D sensor data.
 */
HMI_API hmi2d_block_t * CDECL hmi2d_get_mutual_cal(hmi_t *hmi);

/* Function: hmi2d_get_self_raw
 *
 * Returns the pointer to the raw self sensor data.
 */
HMI_API hmi2d_row_t * CDECL hmi2d_get_self_raw(hmi_t *hmi);

/* Function: hmi2d_get_self_cal
 *
 * Returns the pointer to the calibrated self sensor data.
 */
HMI_API hmi2d_row_t * CDECL hmi2d_get_self_cal(hmi_t *hmi);

/* Function: hmi2d_get_finger_positions
 *
 * Returns the pointer to the list of the finger positions.
 */
HMI_API hmi2d_finger_pos_list_t * CDECL hmi2d_get_finger_positions(hmi_t *hmi);

/* Function: hmi2d_get_mouse
 *
 * Returns the pointer to the data from the mouse emulation.
 */
HMI_API hmi2d_mouse_t * CDECL hmi2d_get_mouse(hmi_t *hmi);

/* Function: hmi2d_get_gesture
 *
 * Returns the pointer to the gesture data used by the keyboard emulation.
 */
HMI_API hmi2d_gesture_t * CDECL hmi2d_get_gesture(hmi_t *hmi);

#endif

#endif /* HMI_API_DYNAMIC_H */
