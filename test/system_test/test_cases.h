/*
 * Copyright (c) 2015 ARM. All rights reserved.
 */

#ifndef __TEST_CASES__H__
#define __TEST_CASES__H__

/* positive cases */
int mesh_api_connect_disconnect_loop(int8_t rf_device_id, uint8_t loop_count);

/* failure cases */
int mesh_api_init(int8_t rf_device_id);
int mesh_api_connect(int8_t rf_device_id);
int mesh_api_disconnect(int8_t rf_device_id);

#endif /* __TEST_CASES__H__ */
