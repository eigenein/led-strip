# ESP8266-based LED strip controller

## Parts

* [Printed circuit board](http://s.click.aliexpress.com/e/bA27UN7?fromSns=Telegram).
* [WeMos D1 mini compatible board](http://s.click.aliexpress.com/e/u7Ubune?fromSns=Telegram).
* Three [TN0610N3-G](https://www.aliexpress.com/item/TN0610N3-TN0610N3-G-TN0610-TO-92/32814210792.html).
* [5V LED strip](http://s.click.aliexpress.com/e/emimIQJ?fromSns=Telegram).
* [Push button](https://www.aliexpress.com/item/50pcs-lot-6x6x4-3MM-4PIN-G89-Tactile-Tact-Push-Button-Micro-Switch-Direct-Plug-in-Self/32669948621.html)

*Note: you have to short-circuit the diode between VBUS and +5V because it can not take enough current.*

## Getting started

* Build with [PlatformIO](http://platformio.org/).
* Hold the WPS button on your router and then click the WPS button on the device. The built-in LED will light once the device is connected to Wi-Fi and IP address is obtained.
* Use [Android app](https://github.com/eigenein/smart-home) to discover and control the device.

## Device discovery

Device registers itself in [mDNS](https://en.wikipedia.org/wiki/Multicast_DNS) under a default name of `ESP_XXXXXX` where `XXXXXX` is the chip number:

```
$ dns-sd -B _smart-home._udp
Browsing for _smart-home._udp
DATE: ---Tue 16 Jan 2018---
20:11:17.202  ...STARTING...
Timestamp     A/R    Flags  if Domain               Service Type         Instance Name
20:11:17.430  Add        2   7 local.               _smart-home._udp.    ESP_380D93
```

## Communication

At application level the device uses JSON over UDP. Entire message must be сontained within one datagram.

The following example demonstrates how the communication is done:

```python
In [18]: import socket

In [19]: s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

In [20]: s.sendto(b'{"t": "PING", "mid": 42}', ('ESP_380D93.local', 5555))
Out[20]: 16

In [21]: print(s.recv(1000).decode())
```

The device may send a state update in response. State update contains a device identifier and fields which should be updated on a remote side:

```json
{
    "ms": 1110940,
    "id": "00000000-0000-0000-0000-000000000000",
    "t": "MULTICOLOR_LIGHTING",
    "name": "ESP_380D93",
    "r": 1,
    "g": 1,
    "b": 1,
    "on": true
}
```

### Ping

Used to discover a device. The device will send its entire current state in response.

```json
{
    "mid": 42,
    "t": "PING"
}
```

### `SET_COLOR`

Set a static lighting color.

```json
{
    "t": "SET_COLOR",
    "r": 1.0,
    "g": 0.5,
    "b": 0.5
}
```

### `TURN_ON`

Turn on lighting.

```json
{
    "t": "TURN_ON"
}

### `TURN_ON`

Turn off lighting.

```json
{
    "t": "TURN_OFF"
}
