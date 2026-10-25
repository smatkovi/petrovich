# Petrovich for Maemo Leste

Send any file, or several, over Bluetooth or by e-mail: a port of Faheem
Pervez's [Petrovich](http://maemo.org/downloads/product/Maemo5/petrovich/)
for Maemo 5 to Maemo Leste.

Petrovich shows the Hildon file selector; tapping a file (or selecting
several with "Share" in the menu) opens a "Send via" dialog:

- **Bluetooth**: choose a paired device; the files go there by OBEX Object
  Push through obexd (BlueZ 5). Pair the device in the Bluetooth settings
  first.
- **E-mail**: a new message in Modest with the files attached.

On Maemo 5 Petrovich handed the files to Nokia's sharing dialog, a closed
library that Leste does not have; this port brings its own dialog for the
two ways that work on Leste.

## Install

Download the `.deb` from the [releases](https://github.com/smatkovi/petrovich/releases)
and install it with `apt install ./petrovich_*_arm64.deb` (pulls in
bluez-obexd). Build: `dpkg-buildpackage -b -us -uc` on Leste.

License: GPL-2.0-or-later (COPYING). Tested on the F(x)tec Pro1X with
Maemo Leste.
