/*
 * Copyright (c) 2015 ARM. All rights reserved.
 */

#ifndef __MESH6LOWPAN_ND_H_
#define __MESH6LOWPAN_ND_H_

#include "AbstractMesh.h"

class Mesh6LoWPAN_ND : public AbstractMesh {

public:
    /*
     * \brief Constructor for the 6LoWPAN_ND interface
     * \param description is the name for the interface
     * \param registered device is physical device registered
     * \param callback_func is callback that is called when network state changes
     */
    Mesh6LoWPAN_ND();

    ~Mesh6LoWPAN_ND();

    /*
     * \brief Initialization of the interface.
     * \param registered device is physical device registered
     * \param callback_func is callback that is called when network state changes
     * \return >=0 on success.
     * \return -2 Driver is already associated to other interface.
     * \return -3 No memory for interface.
     */
    int8_t init(int8_t registered_device_id, mesh_interface_cb callback_func);

    // methods from the abstract class
    int8_t init();
    int8_t connect();
    int8_t disconnect();

    /**
     * \brief Read own global IP address
     *
     * \param address is where the IP address will be copied
     * \param len is the length of the address buffer, must be at least 40 bytes
     */
    bool getOwnIpAddress(char *address, int8_t len);

    /**
     * \brief Read border router IP address
     *
     * \param address is where the IP address will be copied
     * \param len is the length of the address buffer, must be at least 40 bytes
     */
    bool getRouterIpAddress(char *address, int8_t len);

private:
    /*
     * Registered device ID
     */
    int8_t device_id;
};

#endif /* __MESH6LOWPAN_ND_H_ */
