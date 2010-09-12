The DiskFreezerX Project
------------------------

Introduction
------------

The DiskFreezerX is a tool suite that allows to capture bit cell exact
information from a floppy disk. This approach samples disk information at the
lowest possible level and therefore preserves as much information as possible.

This project is heavily inspired by the ideas presented in the Cyclone 20 [1]
and KryoFlux [2] projects. We share the hardware platform (AT91SAM7x) for
connecting the disk drive and doing the low level sampling of the disk data.
However, all the software in this project was written from scratch.
Furthermore, the host hardware in DiskFreezerX is a beagle board that is
connected via SPI to the AT91SAM7x and not a PC connected via USB.

The goal of the DiskFreezerX project is to provide a modular tool set for
sampling disk information in a versatile way. We want to provide a full open
source solution to exact bit cell level disk archiving that supports lots of
platforms and devices. We start of with a working sample system running on a
beagle board and a SAM7x board and hope to add new supported HW platforms in
the near future.

Currently, the project is in no way an end user project, i.e. you cannot
simply buy some HW and off you go. It is rather a hacker project for you
technically skilled folks out there that know how to handle the HW stuff
described here... So you have been warned ;)

And now have fun!

Chris Vogelgsang aka. lallafa
<chris@vogelgsang.org>

[1] http://eab.abime.net/showthread.php?t=40959
    https://docs.google.com/Doc?id=dgddtc7_102gbzr2mgr&hl=en
[2] http://www.kryoflux.com/


Overview
--------

A DiskFreezerX system consists of the following components:

* A _Sampler_ is a hardware device that connects to the floppy drive, controls
  the floppy and samples the disk information. The sampling process is very
  time critical and therefore has a direct bare-metal implementation on a
  microcontroller. The sampled data contains large amounts of information and
  therefore. The "dfx-sampler" is the firmware running on a sampler. 

* A _Capture_ program that runs on a host system and is connected to the
  Sampler device. It does control the floppy drive (enable motor, step track,
  ...) and receives the bit cells sampled from the sampler. The capture
  program "dfx-capture" stores the sampled tracks for further processing

* _Anaylzer_ programs read the captured data and do the reconstruction of the
  bit information stored on disk and then the high level structures found
  there, e.g. disk formats of operating systems. As a result of analysis you
  either get lots information about the data found on the disk or you can
  directly derive disk images that can be used in emulators. The analyzers
  are currently written in Python and called with "dfx-analyzer".

Current System
--------------

The current reference platform consists of the following components:

* The Sampler is a AT91SAM7x board that directly connects to the floppy drive.
  The dfx-sampler firmware running there is written in C using only bare-metal
  (i.e. register) access to the device. The communication with the capture
  host is done via an SPI link. The SPI link runs at 3 MBit/s and the sampler
  is the slave device. Both the control commands and the sampled track data
  are transferred along this link.

* The capture host is a Beagle Board clone called IGEP v2 [3]. This board has
  an OMAP3 Cortex A8 ARM processor at 720 MHz and runs Poky Linux [4]. The
  board has an extension connector that provides an SPI channel. With a level
  converter attached (1.8V to 3.3V) the SPI can be directly connected to the
  SAM7x board. The dfx-capture software is written for the Linux spidev
  user-level SPI programming interface and requires a patched Linux kernel
  that enables the SPI device on the external connector. The capture software
  currently writes sampled raw tracks to one file per track. A disk is sampled
  as a directory of raw track files.

* The analyzer software is written in Python and runs on any system with a
  Python interpreter available. The analyzer handles single raw tracks and
  decodes them into blocks found on this track. A disk image is currently
  build by concatenating track blocks together. In future the anaylzer
  software will be extended into a Python class library with lots of helper
  classes for doing track analysis.

[3] http://www.igep-platform.com/index.php?option=com_content&view=article&id=46&Itemid=55
[4] http://labs.igep.es/index.php/How_to_setup_a_development_environment


Getting Started
---------------

More documentation of this project is located in the "doc" directory.
You'll find there:





EOF