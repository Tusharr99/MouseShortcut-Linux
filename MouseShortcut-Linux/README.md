MouseShortcut-Linux
A Linux kernel module and user-space program that maps mouse button events (BTN_SIDE: 275, BTN_EXTRA: 276) to custom shortcuts for volume control, copy/paste, and screenshots in Ubuntu 24.04. Built and tested in a VirtualBox VM, it uses Netlink for kernel-user communication.
Features

Captures mouse events (BTN_SIDE, BTN_EXTRA) using the Linux input subsystem.
Triggers shortcuts via a user-space program:
BTN_EXTRA press: Increase volume (amixer set Master 5%+).
BTN_SIDE press: Decrease volume (amixer set Master 5%-).
BTN_EXTRA hold (2s): Copy text (xdotool key ctrl+c).
BTN_SIDE hold (2s): Paste text (xdotool key ctrl+v).
BTN_EXTRA + BTN_SIDE hold (3s): Screenshot (scrot).


Communicates between kernel and user-space using Netlink.

Technologies

C: Kernel module and user-space program implementation.
Linux Kernel (6.11): Input subsystem and Netlink for event handling.
Netlink: Kernel-user communication.
ALSA: Volume control via amixer.
X11: GUI shortcuts with xdotool and scrot.
Ubuntu 24.04: Operating system.
Tools: GCC, Make, Git, VirtualBox, evtest, sudo.

Directory Structure
MouseShortcut-Linux/
├── kernel_module/
│   ├── mouse_shortcut.c    # Kernel module source
│   ├── Makefile            # Kernel module build
├── user_space/
│   ├── mouse_shortcut_user.c      # User-space program
│   ├── mouse_shortcut_user.c.bak  # Backup of earlier version
│   ├── Makefile                   # User-space build
├── .gitignore                     # Ignores build artifacts
├── README.md                      # Project documentation

Prerequisites

Ubuntu 24.04 with kernel 6.11.0-21-generic.
Kernel headers: sudo apt-get install linux-headers-$(uname -r).
Dependencies: sudo apt-get install alsa-utils xdotool scrot.

Setup and Installation

Clone the Repository:
git clone https://github.com/Tusharr99/MouseShortcut-Linux.git
cd MouseShortcut-Linux


Build the Kernel Module:
cd kernel_module
make


Build the User-Space Program:
cd ../user_space
make


Run the Project:
cd ../kernel_module
sudo insmod ./mouse_shortcut.ko debug=1
cd ../user_space
sudo -E ./mouse_shortcut_user 276 275



Testing

Use evtest to verify mouse events:sudo evtest

Select VirtualBox mouse integration (event6) or ImExPS/2 Generic Explorer Mouse (event4).
Check kernel logs:sudo dmesg



Known Issues

ALSA/X11 Errors: Commands (amixer, xdotool, scrot) may fail due to sudo unsetting DISPLAY or ALSA sessions. Use sudo -E or:sudo setcap cap_net_admin+ep ./mouse_shortcut_user
./mouse_shortcut_user 276 275


Device Support: Currently optimized for VirtualBox mouse integration. ImExPS/2 support requires modifying mouse_shortcut.c.

Future Improvements

Add support for additional mouse devices (e.g., USB mice).
Resolve ALSA/X11 environment issues without sudo -E.
Implement a GUI for shortcut configuration.

License
GPL
