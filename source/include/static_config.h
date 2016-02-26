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
 * There are 26 channels from 1 to 26. 1 to 10 in sub-GHz and 11 to 26 in 2.4 GHz band
 */
#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)

#ifdef YOTTA_CFG_MBED_MESH_API_SELECTED_RF_CHANNEL
#define USE_CHANNEL STR(YOTTA_CFG_MBED_MESH_API_SELECTED_RF_CHANNEL)
#else
#define USE_CHANNEL       1<<12
#define ALL_CHANNELS    0x07fff800
#endif

#ifdef  YOTTA_CFG_MBED_MESH_API_SCAN_CHANNEL_LIST
// use mbed OS configuration if it is available
#define SCAN_CHANNEL_LIST   YOTTA_CFG_MBED_MESH_API_SCAN_CHANNEL_LIST
#else
#define SCAN_CHANNEL_LIST   USE_CHANNEL
#endif

/*
 * Channel page.
 * -for scan channels 0-10 use value 2 = CHANNEL_PAGE_2
 * -for scan channels 11-26 use value 0 = CHANNEL_PAGE_0
 * Possible channels are 0,1,2,3,4,5,6,9,10, see arm_hal_phy.h for details
 */
#ifdef YOTTA_CFG_MBED_MESH_API_CHANNEL_PAGE
// use mbed OS configuration if it is available
#define FHSS_CHANNEL_PAGE   YOTTA_CFG_MBED_MESH_API_CHANNEL_PAGE
#else
#define FHSS_CHANNEL_PAGE   0
//#define FHSS_CHANNEL_PAGE   2
#endif

/*
 * RF channel in Thread configuration
 */
#ifdef YOTTA_CFG_MBED_MESH_API_THREAD_CHANNEL
#define THREAD_RF_CHANNEL   YOTTA_CFG_MBED_MESH_API_THREAD_CHANNEL
#else
#define THREAD_RF_CHANNEL   12
#endif

#define THREAD_PANID        0xFACE

#ifdef __cplusplus
}
#endif
#endif /* __INCLUDE_STATIC_CONFIG__ */
