kmsquake3
==========

A version of ioquake3 that runs on bare Linux KMS, with raw evdev for
input: no X11, no Wayland. OpenGL ES 1.0 is used for rendering, through EGL.

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

