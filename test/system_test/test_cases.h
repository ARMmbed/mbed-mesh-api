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

#ifndef __TEST_CASES__H__
#define __TEST_CASES__H__

/* positive cases */
void test_mesh_api_connect_disconnect_loop(int8_t rf_device_id, uint8_t loop_count);
void test_mesh_api_connect_disconnect_loop_thread(int8_t rf_device_id, uint8_t loop_count);

/* failure cases */
void test_mesh_api_init(int8_t rf_device_id);
void test_mesh_api_init_thread(int8_t rf_device_id);
void test_mesh_api_connect(int8_t rf_device_id);
void test_mesh_api_disconnect(int8_t rf_device_id, MeshNetworkType type);

#endif /* __TEST_CASES__H__ */
