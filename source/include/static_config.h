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
 */
#define CHANNEL_1       1<<1
#define CHANNEL_12      1<<12
#define ALL_CHANNELS    0x07fff800

#define SCAN_CHANNEL_LIST CHANNEL_12

/*
 * RF channel in Thread configuration
 */
#define THREAD_RF_CHANNEL   12
#define THREAD_PANID        0xFACE

#ifdef __cplusplus
}
#endif
#endif /* __INCLUDE_STATIC_CONFIG__ */
