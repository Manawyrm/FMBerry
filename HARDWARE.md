FMBerry - Hardware
=======
To build the FMBerry transmitter, 
you just need to connect specific testpoints on the PCB of the MMR70 to your Raspberry Pi.

Thanks to Oliver J. (skriptkiddy) for the schematics and the photo with all labeled testpoints.

[Schematic of the MMR-70](http://www.mikrocontroller.net/attachment/140251/MMR70.pdf)

![Testpoints](http://tbspace.de/content/images/fmberrypics/testpins.jpg)

![Used raspberry pi pins](http://tbspace.de/content/images/fmberrypics/usedpins.png)

Old IDE or Floppy Cable does an excellent job for connecting to your Pi. 
Begin by simply connecting these pins together:

* GND -- TP18
* GND -- TP2 (Thanks to Thomas H. for the note!) 
* GND -- TP1
* 3V3 -- TP11
* SDA -- TP8
* SCL -- TP9
* #17 -- TP6 (RDS Interrupt)
* 75.76 cm long wire -- TP19 (antenna)

External antenna, a wire (300/(frequency in MHz) * 25) cm long, can be connected to TP19 or to TP20 - the only testpoint on back side of MMR-70

__Very important!__

Now you need to disable the internal processor of the transmitter. This can be done easily by shorting on of it's crystal pins.
To do this __connect TP18 to GND and TP2 to GND!__

Be aware that MMR-70 can consume up to 27 mA, so make sure that you do not overload Raspberry Pi 3.3V supply (recommended maximim 3.3V current ~55mA) 

Get yourself an old 3,5" phono plug. I've got mine from an old pair of cellphone speakers. 
Connect it as following:

* Audio GND -- TP1
* Audio R   -- TP15
* Audio L   -- TP16

[It should look like this.](http://tbspace.de/content/images/fmberrypics/2013-03-27%2016.19.48.jpg)


[After soldering (and checking)](http://tbspace.de/content/images/fmberrypics/2013-03-27%2016.47.20.jpg)

[plug it into your Raspberry Pi](http://tbspace.de/content/images/fmberrypics/2013-03-27%2016.49.48.jpg).

That's it! =)
