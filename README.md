# mbed mesh API
The ARM mbed mesh API allows the client to use the IPv6 mesh network.

The client can use the `Mesh6LoWPAN_ND` or `MeshThread`object for connecting to the 
mesh network and when successfully connected, the client can create a socket by 
using the [mbed C++ socket API](https://github.com/ARMmbed/sockets) to start 
communication with a remote peer. When the connection is no longer needed the client 
can close the connection with the `disconnect` method.

### Supported mesh networking modes
Currently, 6LoWPAN-ND (neighbour discovery) and Thread bootstrap modes are supported.

### Mesh socket
The mbed mesh API cooperates with module [mbed mesh socket](https://github.com/ARMmbed/sal-iface-6lowpan). 
The module lists the known mesh socket limitations. Full documentation of the IPv6/6LoWPAN 
stack can be found in [IPv6/6LoWPAN stack](https://github.com/ARMmbed/sal-stack-nanostack).

### Module Configuration

This module supports static configuration via [yotta configuration](#http://yottadocs.mbed.com/reference/config.html) by using file `config.json`. Application should create the configuration file if it wants to use other than default settings. An example of the configuration file is as follows:

```
  "mbed-mesh-api": {
    "thread": {
      "pskd": "\"Secret password\"",
      "device_type": "\"Router\"",
      "pollrate": 300,
      "config": {
        "channel_mask": "0x07fff800",
        "channel_page": 0,
        "channel": 12,
        "panid": "0xDEFA",
        "master_key": "{0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff}",
        "ml_prefix": "{0xfd, 0x00, 0x0d, 0xb8, 0x00, 0x00, 0x00, 0x00}",    
        "pskc": "{0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff}"
      }
    },
    "6lowpan_nd": {
      "channel_mask": "0x1000",
      "channel_page": 0,
      "channel": 12
    }
  }
```

Configurable parameters in section: mbed-mesh-api/thread

| Parameter name  | Value         | Description |
| --------------- | ------------- | ----------- |
| pskd            | string [6-255 chars] | Human-scaled commissioning credentials |
| device_type     | string ["SED", "Router"] | Set device operating mode to Sleepy End Device (SED) or router (Router) |
| pollrate        | number | **Sleepy End Device** data polling rate in milliseconds |
| channel_mask    | number [0-0x07fff800] | Channel mask, 0x07fff800 scan all channels |
| channel_page    | number [0, 2]| Channel page, 0 for 2,4 GHz and 2 for sub-GHz radios |
| channel         | number [0-27] | RF channel to use |
| panid           | number [0-0xFFFF] | Network identifier |
| master_key      | byte array [16]| Network master key|
| ml_prefix       | byte array [8] | Mesh local prefix |
| pskc            | byte array [16] | Pre-Shared Key for the Commissioner |

Configurable parameters in section: mbed-mesh-api/6lowpan_nd

| Parameter name  | Type     | Description |
| --------------- | ---------| ----------- |
| channel_mask    | number [0-0x07fff800] | Channel mask, bit-mask of channels to use |
| channel_page    | number [0, 2] | 0 for 2,4 GHz and 2 for sub-GHz radios |
| channel         | number [0-27] | RF channel to use when `channel_mask` is not defined |


## Usage notes
This module should not be used directly by the applications. The applications should 
use the following modules when they are available:

* `connection manager`for handling network connections.
* `device config API` for configuring the network.

### Network connection states
After the initialization, the network state is `MESH_DISCONNECTED`. After a successful connection, 
the state changes to `MESH_CONNECTED` and when disconnected from the network the 
state is changed back to `MESH_DISCONNECTED`.

In case of connection errors, the state is changed to some of the connection error 
states. In an error state, there is no need to make a `disconnect` request and the
client is allowed to attempt connecting again.

## Getting started
This module contains an example application in the `./test/6lowpan_nd/main.cpp` folder. 
To build and run the example, read the [instructions](https://github.com/ARMmbed/mbed-mesh-api/tree/master/test/6lowpan_nd).

## Usage example for 6LoWPAN ND mode

Create a callback function to get the network status:
```
void mesh_api_callback(mesh_connection_status_t mesh_status)
{
    network_state = mesh_status;
}
```
Create a Mesh6LoWPAN_ND class:
```C++
Mesh6LoWPAN_ND *meshApi = (Mesh6LoWPAN_ND*)MeshInterfaceFactory::createInterface(MESH_TYPE_6LOWPAN_ND);
```
Initialize the object with a registered RF device and callback:
```C++
meshApi->init(rf_device_register(), mesh_api_callback);
```
Connect to the mesh network:
```C++
meshApi->connect();
```
Wait for the callback to be called and when successfully connected to the mesh network, create a socket and 
start communication with the remote end:
```
if (network_state == MESH_CONNECTED) {
    UDPaSocket s(SOCKET_STACK_NANOSTACK_IPV6);
    s.open(SOCKET_AF_INET6);
	/* start communication */
	s.send_to(data, dlen, addr, port);
}

```
When the network is no longer needed you can disconnect from the network:
```C++
meshApi->disconnect();
```

## Usage example for 6LoWPAN Thread mode

Create a callback function to get the network status:
```
void mesh_api_callback(mesh_connection_status_t mesh_status)
{
    network_state = mesh_status;
}
```
Create a MeshThread class:
```C++
MeshThread *meshApi = (MeshThread*)MeshInterfaceFactory::createInterface(MESH_TYPE_THREAD);
```
Initialize the object with a registered RF device ID, callback, RF device MAC address and private shared key:
```
uint8_t eui64[8];
int8_t rf_device = rf_device_register();
rf_read_mac_address(eui64);
status = meshApi->init(rf_device, mesh_network_callback, eui64);
```
Connect to the mesh network:
```C++
meshApi->connect();
```
Wait for the callback to be called and when successfully connected to the mesh network, create a socket and 
start communication with the remote end:
```
if (network_state == MESH_CONNECTED) {
    UDPaSocket s(SOCKET_STACK_NANOSTACK_IPV6);
    s.open(SOCKET_AF_INET6);
	/* start communication */
	s.send_to(data, dlen, addr, port);
}

```
When the network is no longer needed you can disconnect from the network:
```C++
meshApi->disconnect();
```

## Testing
The test application is located in `./test/system_test`.

Use the same setup as in the example application. 

Compile the test:
```
yt target frdm-k64f-gcc
yt build
```
1. Flash the software to the FRDM-K64F board.
2. Start the serial terminal emulator.
3. Press the **Reset** button on the board.
4. Check the test results in the traces.

