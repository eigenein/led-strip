# ESP8266-based LED strip controller

Intended to be controlled by https://github.com/eigenein/smart-home.

## Building

Built with [PlatformIO](http://platformio.org/).

Before building, create `src/settings.h` and configure your Wi-Fi access point:

```cpp
static const char SSID[] = "Your Wi-Fi name";
static const char KEY[] = "Your Wi-Fi key";
```

## mDNS

Device registers itself in [mDNS](https://en.wikipedia.org/wiki/Multicast_DNS) under a default name of `ESP_XXXXXX` where `XXXXXX` is the chip number:

```
$ dns-sd -B _led._udp
Browsing for _led._udp
DATE: ---Mon 15 Jan 2018---
21:09:31.707  ...STARTING...
Timestamp     A/R    Flags  if Domain               Service Type         Instance Name
21:09:31.913  Add        2   7 local.               _led._udp.           ESP_380D93
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
    "type": "ping"
}
```

#### Response

```json
{
    "millis": 1110940
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
