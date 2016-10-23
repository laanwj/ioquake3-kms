kmsquake3
==========

A version of ioquake3 that runs on bare Linux KMS, with raw evdev for
input: no X11, no Wayland. OpenGL ES 1.0 is used for rendering, through EGL.

Configuring input
-------------------

Currently event sources have to be configured explicitly for input to work.

Find device ids for keyboard and mouse:
```
$ dmesg|grep input
[    2.236565] input: Power Button as /devices/LNXSYSTM:00/LNXSYBUS:00/PNP0C0C:00/input/input0
[    2.236748] input: Sleep Button as /devices/LNXSYSTM:00/LNXSYBUS:00/PNP0C0E:00/input/input1
[    2.236934] input: Lid Switch as /devices/LNXSYSTM:00/LNXSYBUS:00/PNP0C0D:00/input/input2
[    2.237175] input: Power Button as /devices/LNXSYSTM:00/LNXPWRBN:00/input/input3
[    2.408107] input: AT Translated Set 2 keyboard as /devices/platform/i8042/serio0/input/input4
[    3.438979] input: Video Bus as /devices/LNXSYSTM:00/LNXSYBUS:00/PNP0A08:00/LNXVIDEO:00/input/input7
[    4.985779] input: SynPS/2 Synaptics TouchPad as /devices/platform/i8042/serio1/input/input6
[   23.509602] input: HDA Intel Mic as /devices/pci0000:00/0000:00:1b.0/sound/card0/input8
[   23.510098] input: HDA Intel Headphone as /devices/pci0000:00/0000:00:1b.0/sound/card0/input9
[   25.182000] input: CNF9011 as /devices/pci0000:00/0000:00:1d.7/usb1/1-4/1-4:1.0/input/input10
```
Set these in `~/.q3a/baseq3/q3config.cfg`:
```
seta usbm "10"
seta usbk "4"
```
E.g. `10` for `input10`, `4` for `input4` etc.

Make sure that the user has direct access to the input devices
(group `input` in many distributions).

Blocking console input
-----------------------

Another setting relevant to input is `kmsterm`. This is useful when launching
ioquake3 from the Linux console, to prevent keyboard input from ending up in
the terminal: it will set up the terminal to eat all console input during the
kms event loop. It defaults to `0`.

```
seta kmsterm "1"
```

X11 mode
----------
Optionally (for debugging) the client can run in X11 mode.
This will render into a full-screen X window instead of KMS.

The option will only be effective if the client was built with `USE_X11`
compilation flag.

```
seta x11 "1"
```
Note that this does not affect the input subsystem, just the rendering.

Game data
------------
Install a suitable set of game data (pk3) like the original Quake3 or
openarena and put it in `~/.q3a/baseq3`.

Note:

- Tested with Quake3 1.32 point release.

- Tested with following pk3s:

```
7ce8b3910620cd50a09e4f1100f426e8c6180f68895d589f80e6bd95af54bcae  pak0.pk3
d4ffd60b4b414c3419499e321b6f5c2e933cf082df85823ad2d6ae2f803e1682  pak1.pk3
ccae938a2f13a03b24902d675181d516a431699701ed88023a307f34b5bcd58c  pak2.pk3
d03c0a0e06b99f9ecca2be7389f57faed406e85f7c09b9c56afdfa53ba25e312  pak3.pk3
af5f6d5c82fe4440ae0bb660f0648d1fa1731a9e8305a9eb652aa243428697f1  pak4.pk3
69f87070ca7719e252a3ba97e6483f6663939c987ede550d1268d4d9a07b45bc  pak5.pk3
bb4f0ae2bf603b050fb665436d3178ce7c1c20360e67bacf7c14d93daff38daf  pak6.pk3
de6283ce23e3486a2622c5dbf73d3721a59f24debd380e90f43a97d952fea283  pak7.pk3
812c9e97f231e89cefede3848c6110b7bd34245093af6f22c2cacde3e6b15663  pak8.pk3
```

Debian
----------

May need these dependencies:

    apt-get install libgles1-mesa-dev libgbm-dev

