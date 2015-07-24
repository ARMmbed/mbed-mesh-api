# mbed mesh API
The ARM mbed mesh API allows the client to use the IPv6 mesh network.

The client can use `Mesh6LoWPAN_ND` object for connecting to the mesh network 
and once successfully connected, the client can create a socket by using the 
[mbed C++ socket API](https://github.com/ARMmbed/mbed-net-sockets) to start 
communication with a remote peer. When the connection is no longer needed the client 
can close the connection by using the `disconnect`method.

### Supported mesh networking modes
Supported mesh network modes are:

* 6LoWPAN ND (neighbor discovery)

### Mesh socket
The mbed mesh API cooperates with sockets in the [mbed mesh socket](https://github.com/ARMmbed/mbed-mesh-socket) module. 
The module lists the known mesh socket limitations. Full documentation of the IPv6/6LoWPAN 
stack can be found in [IPv6/6LoWPAN stack](https://github.com/ARMmbed/mbed-6lowpan-private).


## Usage notes
This module should not be used directly by applications. Applications should 
use the following modules when they are available:

* `connection manager`for handling network connections.
* `device config API` for configuring the network.

This module is under construction and therefore, there are some limitations as follows:

* Node is statically configured to router mode.
* Beacon scan takes place on channel 4 (subGhz). You can change this by setting 
 `CONFIGURED_SCAN_CHANNEL` to some other channel in file `mesh_tasklet.c`

### Network connection states
After the initialization, the network state is `MESH_DISCONNECTED`. After a successful connection, 
the state changes to `MESH_CONNECTED` and when disconnected from the network the 
state is changed back to `MESH_DISCONNECTED`.

In case of connection errors, the state is changed to some of the connection error 
states. In an error state, there is no need to make a `disconnect` request and the
client is allowed to attempt connecting again.

## Getting started
This module contains an example application in the `./test/6lowpan_nd/main.cpp` folder. 
To build and run the example, see the [instructions](https://github.com/ARMmbed/mbed-mesh-api/tree/master/test/6lowpan_nd).

## Usage examples

Create a callback function to catch the network status:
```
void mesh_api_callback(mesh_connection_status_t mesh_status)
{
    network_state = mesh_status;
}
```
Create a Mesh6LoWPAN_ND class:
```C++
Mesh6LoWPAN_ND *meshApi = Mesh6LoWPAN_ND::getInstance();
```
Initialize the object with a registered RF device and create callback:
```C++
meshApi->init(rf_device_register(), mesh_api_callback);
```
Connect to the mesh network:
```C++
meshApi->connect();
```
Wait for the callback to be called and once successfully connected to the mesh network, create a socket and 
start communication with the remote end:
```
if (network_state == MESH_CONNECTED) {
    UDPaSocket s(SOCKET_STACK_NANOSTACK_IPV6);
    s.open(SOCKET_AF_INET6);
	/* start communication */
	s.send_to(data, dlen, addr, port);
}

```

## Testing
The test application is located in `./test/system_test`.

Use the same setup as in the example application. 

Compile the test:
```
yt target frdm-k64f-gcc
yt build
```
1. Flash the FRDM-K64F with the software.
2. Start the serial terminal emulator.
3. Press the **Reset** button on the board.
4. Check the test results in the traces.

