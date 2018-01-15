### ESP8266-based LED strip controller

#### Building

Built with PlatformIO.

Before building, create `src/settings.h` and configure your Wi-Fi access point:

```cpp
static const char SSID[] = "Your Wi-Fi name";
static const char KEY[] = "Your Wi-Fi key";
```

#### mDNS

Device registers itself in mDNS under a name `ESP_XXXXXX` where `XXXXXX` is the chip number:

```
$ dns-sd -B _led._udp
Browsing for _led._udp
DATE: ---Mon 15 Jan 2018---
21:09:31.707  ...STARTING...
Timestamp     A/R    Flags  if Domain               Service Type         Instance Name
21:09:31.913  Add        2   7 local.               _led._udp.           ESP_380D93
```
