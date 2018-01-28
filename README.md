# ESP8266-based LED strip controller

## Parts

* [WeMos D1 mini compatible board](https://www.aliexpress.com/item/WeMos-D1-mini-PRO-analog-WiFi-D1-mini-PRO-integration-of-ESP8266-32Mb-flash-and-USB/32806566263.html).
* Three [STP16NF06L](https://www.aliexpress.com/item/STP16NF06L-STP16NE06-STP16NF06-5pcs-lot/32375609225.html).
* [5V LED strip](https://www.aliexpress.com/item/RGB-LED-Strip-Waterproof-5050-DC-5V-USB-LED-Light-Strips-Flexible-bias-Lighting-50cm-1m/32826222420.html).

## Getting started

* Build with [PlatformIO](http://platformio.org/).
* Hold the WPS button on your router and then click the WPS button on the device. The built-in LED will light once the device is connected to Wi-Fi and IP address is obtained.
* Use [Android app](https://github.com/eigenein/smart-home) to discover and control the device.

## Device discovery

Device registers itself in [mDNS](https://en.wikipedia.org/wiki/Multicast_DNS) under a default name of `ESP_XXXXXX` where `XXXXXX` is the chip number:

```
$ dns-sd -B _smart_home._udp
Browsing for _smart_home._udp
DATE: ---Tue 16 Jan 2018---
20:11:17.202  ...STARTING...
Timestamp     A/R    Flags  if Domain               Service Type         Instance Name
20:11:17.430  Add        2   7 local.               _smart-home._udp.    ESP_380D93
```

## Device unique identifier

The device mounts SPIFFS and looks for the file `/uuid.txt`. If the file is missing, a new random UUID is generated and written to the file.

## Communication

At application level the device uses JSON over UDP. Entire message must be сontained within one datagram. Each message may contain the `messageId` integer field which is simply sent back in the response.

The following example demonstrates how the communication is done:

```python
In [18]: import socket

In [19]: s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

In [20]: s.sendto(b'{"type": "ping", "messageId": 42}', ('ESP_380D93.local', 5555))
Out[20]: 16

In [21]: print(s.recv(1000).decode())
{...}
```

The device always responds with its current state:

```json
{
    "messageId": 42,
    "millis": 1110940,
    "uuid": "00000000-0000-0000-0000-000000000000",
    "deviceType": "RGB"
}
```

### Ping

```json
{
    "messageId": 42,
    "type": "ping"
}
```

### Set static color

```json
{
    "type": "setColor",
    "red": 1023,
    "green": 512,
    "blue": 512
}
```
