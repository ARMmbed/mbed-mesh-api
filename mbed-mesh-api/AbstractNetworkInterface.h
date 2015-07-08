/*
 * Copyright (c) 2015 ARM. All rights reserved.
 */

#ifndef __ABSTRACTNETWORKINTERFACE_H__
#define __ABSTRACTNETWORKINTERFACE_H__

/**
 * \brief Abstract base class for network interfaces.
 * This is a pure virtual class; it can't instantiated directly, but
 * it provides common functionality for derived classes.
 *
 * Note! This class will be replaced by connection manager when it is available.
 */

class AbstractNetworkInterface {

public:
    /**
     * Virtual desctructor.
     */
    virtual ~AbstractNetworkInterface() = 0;

    /**
     * Initialize the interface.
     */
    virtual int8_t init() = 0;

    /**
     * Connect the interface.
     */
    virtual int8_t connect() = 0;

    /**
     * Disconnect the interface.
     */
    virtual int8_t disconnect() = 0;
};

#endif /* __ABSTRACTNETWORKINTERFACE_H__ */
