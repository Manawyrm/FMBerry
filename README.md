![FMBerry Logo](http://tbspace.de/holz/uzsjpoghdq.png)
FMBerry
=======
> Written by Tobias Mädel (t.maedel@alfeld.de)

> http://tbspace.de

What is this? 
-------------
FMBerry is a piece of software that allows you to transmit FM radio with your Raspberry Pi.

[YouTube-Video](http://youtu.be/NJRADd7C6rs)

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

The hardware is explained here:
[HARDWARE.md](https://github.com/Manawyrm/FMBerry/blob/master/HARDWARE.md#fmberry---hardware)

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
``sudo apt-get install i2c-tools build-essential git libconfuse-dev``

Build and install the bcm2835 lib by issuing these commands:
```
wget http://www.airspayce.com/mikem/bcm2835/bcm2835-1.25.tar.gz
tar zxvf bcm2835-1.25.tar.gz
cd bcm2835-1.25
./configure
make
sudo make install
```

###Step 3: Finding out your hardware revision

Run 
``cat /proc/cpuinfo | grep "CPU revision"``
in your terminal.

All Raspberry Pi's with a revision newer than rev. 2 have their i2c port connected up to /dev/i2c-1.

Older devices (beta, alpha, early 256MB Model B's) have it connected up to /dev/i2c-0. 

###Step 4: Checking the hardware

You can check your wiring with the following command:

``i2cdetect -y 1``

Please remember that you need to run the command on another port on older revisions!

``i2cdetect -y 0``

You should then see your transmitter at 0x66. 

If you are not able to see your transmitter please double check your wiring! 
![Output of i2cdetect](http://tbspace.de/holz/csuqzygpwb.png)

###Step 5: Building the software
To build the software execute the following commands (in your homefolder):

```
git clone https://github.com/Manawyrm/FMBerry/
cd FMBerry
```

If you have got an old revision board, please open i2c.c and change the variable fileName to ``/dev/i2c-0``! 

``make``

Compiling the software will take a couple of seconds.
###Step 6: Installing the software
FMBerry is essentially a daemon called fmberryd.
To install it into your system path type 
```sudo make install```. 

You can start it by typing ``sudo /etc/init.d/fmberry start``.

To control the daemon you have to use ctlfmberry.

It currently allows the following commands:
* ``ctlfmberry set freq 99000`` -- Frequency in kHz (76000 - 108000)
* ``ctlfmberry poweron``
* ``ctlfmberry poweroff``
* ``ctlfmberry set rdsid DEADBEEF`` (8 chars!)
* ``ctlfmberry set rdstext Mike Oldfield - Pictures in the Dark`` (max. 64 chars)

That's it! :)
###Step 7: Debugging
FMBerry writes debugging output to /var/log/syslog.

You can watch the information by running ``ctlfmberry log``. It's essentially just a ```cat /var/log/syslog | grep fmberryd```

It will tell you what's wrong. 

###Updating the software
Please check for new dependencies. You can safely just run the ```apt-get install``` command again. It will only install new dependencies if necessary.

First stop the daemon by typing ```/etc/init.d/fmberry stop```. 

Then run ```git pull``` followed by a ```make``` and a ```sudo make install```.

You can then start FMBerry again with ```/etc/init.d/fmberry start```.
##Notes
* WARNING! I am not a professional C programmer. Please expect this software to have major security flaws. Please don't expose it's control port to the internet! I'm fairly certain that this software is vulnerable to buffer overflows. 
* If you are a C programmer, please help by securing this software and sending a pull request. 
* 
* The Daemon itself is essentially a simple TCP server. It is listening to Port 42516. (set in fmberryd.h) You can control it by sending the exact same commands you would give to ctlfmberry.
* For information on How to control the Daemon have a look into ctlfmberry. It's a simple shell script.

* You can also stream song information from a music player daemon via https://github.com/Manawyrm/FMBerryRDSMPD

* Feel free to contact me: t.maedel@alfeld.de (english and german) 

##Common problems
__The daemon does not show anything.__

That's normal. You have to use ./ctlfmberry to control the daemon.

__I can't seem to hear music.__

Turn up the volume/unmute your raspi with alsamixer.

__I am getting compile errors.__

Did you install all dependencies? (All lines with apt-get)

__The transmissions dies after a couple of minutes.__

You didn't disable the internal processor of the MMR70. Do this by connecting TP18 to GND.
