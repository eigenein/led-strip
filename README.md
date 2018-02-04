# ESP8266-based LED strip controller

## Parts

* [Printed circuit board](http://s.click.aliexpress.com/e/bA27UN7?fromSns=Telegram).
* [WeMos D1 mini compatible board](http://s.click.aliexpress.com/e/u7Ubune?fromSns=Telegram).
* Three [IRLB3034PBF](http://s.click.aliexpress.com/e/yRnQNzR?fromSns=Telegram).
* [5V LED strip](http://s.click.aliexpress.com/e/emimIQJ?fromSns=Telegram).
* Three 220μF capacitors.

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

## Device unique identifier

The device mounts SPIFFS and looks for the file `/uuid.txt`. If the file is missing, a new random UUID is generated and written to the file. *Do not share the UUID!*

## Communication

At application level the device uses JSON over UDP. Entire message must be сontained within one datagram. Each message may contain the `messageId` integer field which is simply sent back in the response.

The following example demonstrates how the communication is done:

```python
In [18]: import socket

In [19]: s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

In [20]: s.sendto(b'{"type": "ping", "messageId": 42}', ('ESP_380D93.local', 5555))
Out[20]: 16

In [21]: print(s.recv(1000).decode())
```

The device always responds with its current state:

```json
{
    "messageId": 42,
    "millis": 1110940,
    "uuid": "00000000-0000-0000-0000-000000000000",
    "deviceType": "MULTICOLOR_LIGHTING"
}
```

### Ping

Used to discover the device type and to get its current state.

```json
{
    "messageId": 42,
    "type": "ping"
}
```

### Set static color

Used to set a static lighting color.

```json
{
    "type": "setColor",
    "red": 1.0,
    "green": 0.5,
    "blue": 0.5
}
```
