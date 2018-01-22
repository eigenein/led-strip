# ESP8266-based LED strip controller

Intended to be controlled by https://github.com/eigenein/smart-home.

## Parts

* [WeMos D1 mini compatible board](https://www.aliexpress.com/item/WeMos-D1-mini-PRO-analog-WiFi-D1-mini-PRO-integration-of-ESP8266-32Mb-flash-and-USB/32806566263.html).
* Three [STP16NF06L](https://www.aliexpress.com/item/STP16NF06L-STP16NE06-STP16NF06-5pcs-lot/32375609225.html).
* [5V LED strip](https://www.aliexpress.com/item/RGB-LED-Strip-Waterproof-5050-DC-5V-USB-LED-Light-Strips-Flexible-bias-Lighting-50cm-1m/32826222420.html)

## Building

Built with [PlatformIO](http://platformio.org/).

Before building, create `src/settings.h` and configure your Wi-Fi access point:

```cpp
static const char SSID[] = "Your Wi-Fi name";
static const char KEY[] = "Your Wi-Fi key";
static const char UUID[] = "00000000000000000000000000000000";
```

## mDNS

Device registers itself in [mDNS](https://en.wikipedia.org/wiki/Multicast_DNS) under a default name of `ESP_XXXXXX` where `XXXXXX` is the chip number:

```
$ dns-sd -B _smart_home._udp
Browsing for _smart_home._udp
DATE: ---Tue 16 Jan 2018---
20:11:17.202  ...STARTING...
Timestamp     A/R    Flags  if Domain               Service Type         Instance Name
20:11:17.430  Add        2   7 local.               _smart-home._udp.    ESP_380D93
```

## Communication

At application level the device uses JSON over UDP. Entire `Message` must be сontained within one datagram.

The following example demonstrates how communication is done:

```python
In [18]: import socket

In [19]: s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

In [20]: s.sendto(b'{"type": "ping"}', ('ESP_380D93.local', 5555))
Out[20]: 16

In [21]: print(s.recv(1000).decode())
{"millis":1110940}
```

### Ping

```json
{
    "messageId": 42,
    "type": "ping"
}
```

#### Response

```json
{
    "messageId": 42,
    "millis": 1110940,
    "uuid": "00000000000000000000000000000000",
    "deviceType": "LED"
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

This message has no response.
