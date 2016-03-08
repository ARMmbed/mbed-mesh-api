/*
 * Copyright (c) 2015 ARM Limited. All rights reserved.
 * SPDX-License-Identifier: Apache-2.0
 * Licensed under the Apache License, Version 2.0 (the License); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef __INCLUDE_STATIC_CONFIG__
#define __INCLUDE_STATIC_CONFIG__
#include "ns_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Channel list is defined as 1 shifted by the channel number
 * f.e.:
 * channel 4 = 1<<4.
 * channel 10 = 1<<10
 *
 * Channel page.
 * -for scan channels 0-10 use value 2 = CHANNEL_PAGE_2
 * -for scan channels 11-26 use value 0 = CHANNEL_PAGE_0
 * Possible channels are 0,1,2,3,4,5,6,9,10, see arm_hal_phy.h for details
 */

#ifndef YOTTA_CFG_MBED_MESH_API_6LOWPAN_ND_CHANNEL_MASK
#ifdef YOTTA_CFG_MBED_MESH_API_6LOWPAN_ND_CHANNEL
#define YOTTA_CFG_MBED_MESH_API_6LOWPAN_ND_CHANNEL_MASK (1<<YOTTA_CFG_MBED_MESH_API_6LOWPAN_ND_CHANNEL)
#else
#define YOTTA_CFG_MBED_MESH_API_6LOWPAN_ND_CHANNEL_MASK (1<<12)
#endif
#endif

#ifndef YOTTA_CFG_MBED_MESH_API_6LOWPAN_ND_CHANNEL_PAGE
#define YOTTA_CFG_MBED_MESH_API_6LOWPAN_ND_CHANNEL_PAGE 0
#endif

#ifndef YOTTA_CFG_MBED_MESH_API_6LOWPAN_ND_CHANNEL
#define YOTTA_CFG_MBED_MESH_API_6LOWPAN_ND_CHANNEL 0
#endif

/* Thread configuration */
#ifndef YOTTA_CFG_MBED_MESH_API_THREAD_PSKD
// PSKd, must be longer than 6
#define YOTTA_CFG_MBED_MESH_API_THREAD_PSKD "Secret password"
#endif

#ifndef YOTTA_CFG_MBED_MESH_API_THREAD_DEVICE_TYPE
// Operating mode, values: Router, SED
#define YOTTA_CFG_MBED_MESH_API_THREAD_DEVICE_TYPE MESH_DEVICE_TYPE_THREAD_ROUTER
#endif

#ifndef YOTTA_CFG_MBED_MESH_API_THREAD_CONFIG_CHANNEL_MASK
// channel mask, enable all channels
#define YOTTA_CFG_MBED_MESH_API_THREAD_CONFIG_CHANNEL_MASK 0x07fff800
#endif

#ifndef YOTTA_CFG_MBED_MESH_API_THREAD_CONFIG_CHANNEL_PAGE
#define YOTTA_CFG_MBED_MESH_API_THREAD_CONFIG_CHANNEL_PAGE 0
#endif

#ifndef YOTTA_CFG_MBED_MESH_API_THREAD_CONFIG_CHANNEL
#define YOTTA_CFG_MBED_MESH_API_THREAD_CONFIG_CHANNEL 12
#endif

#ifndef YOTTA_CFG_MBED_MESH_API_THREAD_CONFIG_PANID
// Default PANID, 0xDEFA=57082
#define YOTTA_CFG_MBED_MESH_API_THREAD_CONFIG_PANID 0xDEFA
#endif

#ifndef YOTTA_CFG_MBED_MESH_API_THREAD_MASTER_KEY
#define YOTTA_CFG_MBED_MESH_API_THREAD_MASTER_KEY {0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff}
#endif

#ifndef YOTTA_CFG_MBED_MESH_API_THREAD_CONFIG_ML_PREFIX
#define YOTTA_CFG_MBED_MESH_API_THREAD_CONFIG_ML_PREFIX {0xfd, 0x00, 0x0d, 0xb8, 0x00, 0x00, 0x00, 0x00}
#endif

#ifndef YOTTA_CFG_MBED_MESH_API_THREAD_CONFIG_PSKC
#define YOTTA_CFG_MBED_MESH_API_THREAD_CONFIG_PSKC {0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff}
#endif

#ifdef __cplusplus
}
#endif
#endif /* __INCLUDE_STATIC_CONFIG__ */
