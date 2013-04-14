FMBerry
=======

To use a NS741-based transmitter with an Raspberry Pi you have to install some packages.
This software was developed under Raspbian Wheezy 2013-02-09.

Step 1: Enabling I²C

Open raspi-blacklist.conf:

sudo nano /etc/modprobe.d/raspi-blacklist.conf

Comment out the Line "blacklist i2c-bcm2708" with a #.
Save with Ctrl+O and close nano with Ctrl+X

To make sure I²C Support is loaded at boottime open /etc/modules.

sudo nano /etc/modules

Add the following lines:

snd-bcm2835
i2c-dev

Then again, Save with Ctrl+O and then close nano with Ctrl+X.

Step 2: Installing I²C tools and dependencys for the build

First update your local package repository with
sudo apt-get update

then install all needed software with the following command:
sudo apt-get install i2c-tools build-essential 

Step 3: Finding out your hardware revision

Run 
cat /proc/cpuinfo | grep "CPU revision"
in your terminal.

All Raspberry Pi's with a revision newer than rev. 2 have their i2c port connected up to /dev/i2c-1.
Older devices (beta, alpha, early 256MB Model B's) have it connected up to /dev/i2c-0. 

