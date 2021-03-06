/******************************************************************************
 *
 * Copyright (C) 2014 Microchip Technology Inc. and its
 *                    subsidiaries (“Microchip”).
 *
 * All rights reserved.
 *
 * You are permitted to use the Aurea software, 3DTouchPad SDK, and other
 * accompanying software with Microchip products.  Refer to the license
 * agreement accompanying this software, if any, for additional info regarding
 * your rights and obligations.
 *
 * SOFTWARE AND DOCUMENTATION ARE PROVIDED “AS IS” WITHOUT WARRANTY OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION, ANY WARRANTY OF
 * MERCHANTABILITY, TITLE, NON-INFRINGEMENT AND FITNESS FOR A PARTICULAR
 * PURPOSE. IN NO EVENT SHALL MICROCHIP, SMSC, OR ITS LICENSORS BE LIABLE OR
 * OBLIGATED UNDER CONTRACT, NEGLIGENCE, STRICT LIABILITY, CONTRIBUTION, BREACH
 * OF WARRANTY, OR OTHER LEGAL EQUITABLE THEORY FOR ANY DIRECT OR INDIRECT
 * DAMAGES OR EXPENSES INCLUDING BUT NOT LIMITED TO ANY INCIDENTAL, SPECIAL,
 * INDIRECT OR CONSEQUENTIAL DAMAGES, OR OTHER SIMILAR COSTS.
 *
 ******************************************************************************/
#ifndef HMI_ARCH_DEFAULT_H
#define HMI_ARCH_DEFAULT_H

#include "hmi_api.h"

/* ======== Packing / Unpacking ======== */

/* Defines: Data Packing Macros
 *
 * Macros for operation on values in messages.
 *
 * GET_S8  - Loads 8-bit signed integer
 * GET_S16 - Loads 16-bit signed integer
 * GET_S32 - Loads 32-bit signed integer
 * GET_U8  - Loads 8-bit unsigned integer
 * GET_U16 - Loads 16-bit unsigned integer
 * GET_U32 - Loads 32-bit unsigned integer
 * GET_F32 - Loads 32-bit float number
 * SET_S8  - Stores 8-bit signed integer
 * SET_S16 - Stores 16-bit signed integer
 * SET_S32 - Stores 32-bit signed integer
 * SET_U8  - Stores 8-bit unsigned integer
 * SET_U16 - Stores 16-bit unsigned integer
 * SET_U32 - Stores 32-bit unsigned integer
 * SET_F32 - Stores 32-bit float number
 *
 * The values are stored in messages as a sequence of unsigned chars
 * in little-endian order.
 *
 * Note:
 *    In contrast to earlier versions P might be stored in any location and
 *    is *not* aligned to the size of the contained variable.
 * Note:
 *    Those macros have to be replaced for big endian platforms.
 */
#ifndef GET_U32
#   ifdef HMI_UNALIGNED_LITTLEENDIAN
#      define GET_S8(P) (*(char*)(P))
#      define GET_S16(P) (*(short*)(P))
#      define GET_S32(P) (*(int*)(P))
#      define GET_U8(P) (*(unsigned char*)(P))
#      define GET_U16(P) (*(unsigned short*)(P))
#      define GET_U32(P) (*(unsigned int*)(P))
#      define GET_F32(P) (*(float*)(P))
#      define SET_S8(P, X) (*(char*)(P) = (char)(X))
#      define SET_S16(P, X) (*(short*)(P) = (short)(X))
#      define SET_S32(P, X) (*(int*)(P) = (int)(X))
#      define SET_U8(P, X) (*(unsigned char*)(P) = (unsigned char)(X))
#      define SET_U16(P, X) (*(unsigned short*)(P) = (unsigned short)(X))
#      define SET_U32(P, X) (*(unsigned int*)(P) = (unsigned int)(X))
#      define SET_F32(P, X) (*(float*)(P) = (float)(X))
#   else
#      error "Missing defines for packing/unpacking of messages"
#   endif
#endif

/* ======== Assertion ======== */

#ifndef HMI_ASSERT
#   error "HMI_ASSERT not defined"
#endif

/* ======== Memory Operations ======== */

#if defined(HMI_HAS_DYNAMIC) && !defined(HMI_NO_ALLOCATION)
#   if !defined(HMI_MALLOC) || !defined(HMI_FREE)
#       error "Missing HMI_MALLOC and/or HMI_FREE defines"

#       include <stdlib.h>
#       ifndef HMI_MALLOC
#           define HMI_MALLOC malloc
#       endif
#       ifndef HMI_FREE
#           define HMI_FREE free
#       endif
#   endif
#endif

#if !defined(HMI_MEMSET) || !defined(HMI_MEMCPY)
#   error "Missing HMI_MEMSET and/or HMI_MEMCPY defines"

#   include <string.h>
#   define HMI_MEMSET memset
#   define HMI_MEMCPY memcpy
#endif

/* ======== Synchronisation etc. ======== */

#if defined(HMI_SYNC_INTERRUPT) || defined(HMI_SYNC_THREADING)
#   if !defined(HMI_SYNC_INIT) || !defined(HMI_SYNC_LOCK) || !defined(HMI_SYNC_UNLOCK) || !defined(HMI_SYNC_RELEASE)
#       error "Missing definition of locking mechanisms"
#   endif
#endif

/* ======== Logging (not implemented by default). ======== */

#ifndef HMI_BAD_DATA
#   define HMI_BAD_DATA(FUNC, MSG, VALUE1, VALUE2) ((void)0)
#endif

#endif /* HMI_ARCH_DEFAULT_H */
