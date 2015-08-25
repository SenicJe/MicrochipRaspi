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

#if (HMI_IO == HMI_IO_CDC_SERIAL) && defined(_WIN32)

#include <Windows.h>
#include <SetupAPI.h>

int hmi_open(hmi_t *hmi) {
    int error = HMI_NO_ERROR;
    int i, j;
    SP_DEVINFO_DATA devInfoData = { sizeof(SP_DEVINFO_DATA) };
    SP_DEVICE_INTERFACE_DATA devIfaceData = { sizeof(SP_DEVICE_INTERFACE_DATA) };
    GUID usbClassGuid = { 0xa5dcbf10, 0x6530, 0x11d2, {0x90, 0x1f, 0x00, 0xc0, 0x4f, 0xb9, 0x51, 0xed} };
    HDEVINFO devInfoSet = NULL;

    PTSTR devInstId = NULL;
    DWORD devInstIdAlloc = 0;
    SP_DEVICE_INTERFACE_DETAIL_DATA *devIfaceDetailData = NULL;
    DWORD devIfaceDetailDataAlloc = 0;
    HANDLE handle = INVALID_HANDLE_VALUE;

    HMI_ASSERT(hmi);

    // Get all devices from the PORTS class {4d36e978-e325-11ce-bfc1-08002be10318}
    devInfoSet = SetupDiGetClassDevs(&usbClassGuid, NULL, NULL, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);
    if(devInfoSet == INVALID_HANDLE_VALUE)
        error = HMI_IO_ENUM_ERROR;

    for(i = 0; !error; ++i) {
        DWORD required = 0;

        if(!SetupDiEnumDeviceInfo(devInfoSet, i, &devInfoData))
            break;

        // Get device instance ID
        SetupDiGetDeviceInstanceId(devInfoSet, &devInfoData, NULL, 0, &required);
        // The 2*required is needed as SetupDiGetDeviceInstanceId has a bug
        // resulting in required being set to only the half of required memory
        if(devInstIdAlloc < 2 * required) {
            devInstIdAlloc = 3 * required;
            if(devInstId) LocalFree(devInstId);
            devInstId = (PTSTR)LocalAlloc(LPTR, devInstIdAlloc);
        }
        if(!SetupDiGetDeviceInstanceId(devInfoSet, &devInfoData, devInstId, required, NULL))
            continue;

        // Test whether device is a usb device with correct vendorId and productId
        if(_wcsnicmp(devInstId, L"usb\\vid_04d8&pid_000a", 21))
            continue;

        // Try to get interface of device
        for(j = 0; ; ++j) {
            if(!SetupDiEnumDeviceInterfaces(devInfoSet, &devInfoData, &usbClassGuid, j, &devIfaceData))
                break;

            SetupDiGetDeviceInterfaceDetail(devInfoSet, &devIfaceData, NULL, 0, &required, NULL);
            if(devIfaceDetailDataAlloc < required) {
                devIfaceDetailDataAlloc = 2 * required;
                if(devIfaceDetailData) LocalFree(devIfaceDetailData);
                devIfaceDetailData = (SP_DEVICE_INTERFACE_DETAIL_DATA*)LocalAlloc(LPTR, devIfaceDetailDataAlloc);
                devIfaceDetailData->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);
            }

            if(!SetupDiGetDeviceInterfaceDetail(devInfoSet, &devIfaceData, devIfaceDetailData, required, NULL, NULL))
                continue;

            // Try to open file
            handle = CreateFile(devIfaceDetailData->DevicePath, GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
            if(handle != INVALID_HANDLE_VALUE)
                break;
        }

        if(handle != INVALID_HANDLE_VALUE)
            break;
    }
    SetupDiDestroyDeviceInfoList(devInfoSet);
    if(devIfaceDetailData) LocalFree(devIfaceDetailData);
    if(devInstId) LocalFree(devInstId);

    if(handle == INVALID_HANDLE_VALUE)
        error = HMI_IO_OPEN_ERROR;

    /* Set COM-port parameters */
    {
        DCB dcbSerialParams;

        if(!error) {
            memset(&dcbSerialParams, 0, sizeof(dcbSerialParams));
            dcbSerialParams.DCBlength = sizeof(dcbSerialParams);
            if(!GetCommState(handle, &dcbSerialParams))
                error = HMI_IO_CTL_ERROR;
        }

        if(!error) {
            dcbSerialParams.ByteSize = 8;
            dcbSerialParams.StopBits = ONESTOPBIT;
            dcbSerialParams.Parity = NOPARITY;
            dcbSerialParams.fDtrControl = DTR_CONTROL_ENABLE;
            if(!SetCommState(handle, &dcbSerialParams))
                error = HMI_IO_CTL_ERROR;
        }
    }

    /* Set timeouts */
    if(!error) {
        COMMTIMEOUTS timeouts;

        memset(&timeouts, 0, sizeof(timeouts));
        timeouts.ReadIntervalTimeout = MAXDWORD;

        if(!SetCommTimeouts(handle, &timeouts))
            error = HMI_IO_CTL_ERROR;
    }

    if(error) {
        if(handle != INVALID_HANDLE_VALUE)
            CloseHandle(handle);
    } else {
        hmi->io.cdc_serial = handle;
    }

    return error;
}

void hmi_close(hmi_t *hmi) {
    HMI_ASSERT(hmi && HMI_CONNECTED(hmi));

    CloseHandle((HANDLE)hmi->io.cdc_serial);
    hmi->io.cdc_serial = NULL;
}

int hmi3d_reset(hmi_t *hmi) {
    const unsigned char reset_msg[] = {
        0xFE, 0xFF, 0x00, 0x11, 0x00, 0x00, 0x00, 0x00
    };
    DWORD bytesWritten = 0;
    HANDLE handle = INVALID_HANDLE_VALUE;
    int error = HMI_NO_ERROR;

    HMI_ASSERT(hmi && HMI_CONNECTED(hmi));

    handle = (HANDLE)hmi->io.cdc_serial;
    if(!WriteFile(handle, reset_msg, sizeof(reset_msg), &bytesWritten, NULL))
        error = HMI_IO_ERROR;

    return error;
}

int hmi3d_serial_read(hmi_t *hmi, void *buffer, int maxsize) {
    DWORD bytesRead = 0;
    HANDLE handle = INVALID_HANDLE_VALUE;

    HMI_ASSERT(hmi && HMI_CONNECTED(hmi));

    handle = (HANDLE)hmi->io.cdc_serial;
    if(!ReadFile(handle, buffer, maxsize, &bytesRead, NULL))
        return HMI_IO_ERROR;
    return bytesRead;
}

int hmi3d_serial_write(hmi_t *hmi, void *buffer, int size) {
    DWORD bytesWritten = 0;
    HANDLE handle = INVALID_HANDLE_VALUE;

    HMI_ASSERT(hmi && HMI_CONNECTED(hmi));

    handle = (HANDLE)hmi->io.cdc_serial;
    if(!WriteFile(handle, buffer, size, &bytesWritten, NULL))
        return HMI_IO_ERROR;
    return bytesWritten;

}

#endif /* (HMI_IO == HMI_IO_CDC_SERIAL) && defined(_WIN32) */
