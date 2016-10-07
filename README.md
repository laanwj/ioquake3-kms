kmsquake3
==========

A version of ioquake3 that runs on bare Linux KMS, with raw evdev for
input: no X11, no Wayland. OpenGL ES 1.0 is used for rendering, through EGL.

Configuring
-------------

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

X11 mode
----------
Optionally (for debugging) the client can run in X11 mode.
This will render into a full-screen X window instead of KMS.
```
seta x11 "1"
```
Note that this does not affect the input subsystem, just the rendering.

