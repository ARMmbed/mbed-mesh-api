/*
 * Copyright (c) 2015 ARM. All rights reserved.
 */

#ifndef __MESH6LOWPAN_ND_H_
#define __MESH6LOWPAN_ND_H_

#include "AbstractMesh.h"

class Mesh6LoWPAN_ND : public AbstractMesh {

public:

    /**
     * \brief Return singleton of this class
     * \return class instance.
    */
    static Mesh6LoWPAN_ND *getInstance();

    ~Mesh6LoWPAN_ND();

    /**
     * \brief Read own global IP address
     *
     * \param address is where the IP address will be copied
     * \param len is the length of the address buffer, must be at least 40 bytes
     * \return true if address is read successfully, false otherwise
     */
    bool getOwnIpAddress(char *address, int8_t len);

    /**
     * \brief Read border router IP address
     *
     * \param address is where the IP address will be copied
     * \param len is the length of the address buffer, must be at least 40 bytes
     * \return true if address is read successfully, false otherwise
     */
    bool getRouterIpAddress(char *address, int8_t len);

private:
    /*
     * \brief private constructor for the 6LoWPAN_ND
     */
    Mesh6LoWPAN_ND();

    /*
     * \brief instance
     */
    static Mesh6LoWPAN_ND *instance;

    /*
     * avoid copy/assign object
     */
    Mesh6LoWPAN_ND(Mesh6LoWPAN_ND const& copy);
    Mesh6LoWPAN_ND & operator=(Mesh6LoWPAN_ND const& copy);
};

#endif /* __MESH6LOWPAN_ND_H_ */
