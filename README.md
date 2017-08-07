airplaned
===

An userspace daemon for managing airplane mode. It listens for KEY_RFKILL on a given event device and automatically saves and restores rfkill device state when entering and leaving airplane mode respectively.
It's currently a work in progress and many imperfections are present.
The LED name is currently hardcoded to `asus-wireless::airplane`.

_If you wish to use rfkill's integrated input handler and only manage the LED, check out the ledonly branch. (see what I did there?)_

How to use
---
Adjust the `#define LEDNAME` in airplaned.c to whatever your airplane mode LED is called. (Look into `/sys/class/leds`)
Find your airplane mode button input device by running `grep -B5 rfkill /proc/bus/input/devices`. You are interested in eventXX listed in the line
starting with `H:`. You may get multiple results, but unless
you have multiple airplane mode buttons (contact me) only one of them will be correct. To find it, you could use `evtest` and monitor the
event device for output when pressing the button. On my ASUS zenbook 2 devices are detected and the correct one is called
`Asus Wireless Radio Control`.
```
make
sudo make install
sudo systemctl enable airplaned@eventXX.service
```

reboot or `sudo systemctl start airplaned@eventXX.service`.

The daemon should now disable rfkill's integrated input handler and listen for input.
