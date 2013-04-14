FMBerry
=======
> Written by Tobias Mädel (t.maedel@alfeld.de)

> http://tbspace.de

What is this? 
-------------
FMBerry is a piece of software that allows you to transmit FM radio with your Raspberry Pi. 
How does it work? 
-------------
It uses the Sony-Ericsson MMR-70 transmitter, which was originally intended for use with Sonys Walkman cellphones from early 2000s.
You can get these for really cheap from ebay or Amazon (link below). (Less than 1 Euro with shipping in Germany)

What do I need to build this? 
-------------
* MMR-70 transmitter (http://www.amazon.de/Sony-DPY901634-Ericsson-MMR-70-FM-Transmitter/dp/B000UTMOF0)
* Raspberry Pi (Model A/B - 256MB or 512MB)
* Soldering equipment (soldering iron and some solder)
* Cable for connecting to your Raspis GPIO port (old IDE cable does work fine!)

Please head over to 
http://tbspace.de/fmberry.html
for detailed building instructions.

Installation
-------------
This software was developed under Raspbian Wheezy 2013-02-09.
###Step 1: Enabling I²C

Open raspi-blacklist.conf:

``sudo nano /etc/modprobe.d/raspi-blacklist.conf``

Comment out the Line "``blacklist i2c-bcm2708``" with a #.
Save with Ctrl+O and close nano with Ctrl+X

To make sure I²C Support is loaded at boottime open /etc/modules.

``sudo nano /etc/modules``

Add the following lines:

``snd-bcm2835``

``i2c-dev``

Then again, Save with Ctrl+O and then close nano with Ctrl+X.

###Step 2: Installing I²C tools and dependencies for the build

First update your local package repository with
``sudo apt-get update``

then install all needed software with the following command:
``sudo apt-get install i2c-tools build-essential ``

###Step 3: Finding out your hardware revision

Run 
``cat /proc/cpuinfo | grep "CPU revision"``
in your terminal.

All Raspberry Pi's with a revision newer than rev. 2 have their i2c port connected up to /dev/i2c-1.
Older devices (beta, alpha, early 256MB Model B's) have it connected up to /dev/i2c-0. 

