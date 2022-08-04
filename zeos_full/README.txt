Disk Driver v1.0
================
This file documents the driver for an ATA disk[1].
The ATA specification[1] defines the protocol to use to communicate with
a block storage device. This driver may be used inside the ZeOS code to
access a disk following the ATA specification.

The disk is conceived as an array of blocks, each block with 512bytes of
data and identified with a block number. The first block number is 0.

       _0___1___2__________________N_
      |   |   |   |              |   |
      |___|___|___|______________|___|
      \___/
        |
       512 bytes

The driver offers a C-language interface to:
1) Initialize the disk and prepare it to accept the remaining operations.
2) Read a block from disk to memory.
3) Write a block from memory to disk.

Take a look to 'ide.h' for the specific details.

The current implementation is very simple and does NOT use interrupts at all,
instead it uses polling to access the disk.

Bochs Configuration
-------------------
Even Bochs allow to configure a virtual machine with a physical disk, we will
opt to emulate this device because Bochs allows to configure a simple file to
be used as a virtual disk.

1) Virtual disk creation
  The first step is to create the virtual disk. The virtual disk will be a file
  with the size (in bytes) of the disk to be created.
  In Linux to create a ~5MB file 'fs.img' (10000 blocks of 512B) with a
  content full of zeroes, you may do the following:
  '''''
	  dd if=/dev/zero of=fs.img count=10000 bs=512
  '''''

2) Configure Bochs
  The next step is to configure Bochs[2] to use the previous file as a disk.
  The required information  to use in order to access the
  disk are the IO addresses to access the disk (0x1f0 and 0x3f0), the
  interrupt number to use (14) and a description of the disk (in this case the file
  'fs.img'). Add the following lines to the _.bochsrc_ file:
  '''''
	ata0: enabled=1, ioaddr1=0x1f0, ioaddr2=0x3f0, irq=14
	ata0-master: type=disk, path="fs.img", mode=flat
  '''''

3) Use the driver
  After the previous steps, the driver may be used. The first thing to do is
  to initialize the disk, and after that the disk may be read and written. Due
  to the current implementation of the driver, there is no way to get
  information about the disk, for example the number of blocks available in
  the disk and therefore this information needs to be "hardcoded" someway.
  Continuing the previous example, you 'know' that the disk has a maximum of
  10000 blocks.
  Any write operation used from the driver will physically write the 'virtual'
  disk and its effects will be observable from outside Bochs. You can observe
  the content of the file (the virtual disk) with Linux tools like 'hexdump':
  '''''
    hd fs.img
  '''''
That's all, have fun :)

[1] ATA specification
[2] Bochs configuration user manual. ATA section. Available online at
<http://bochs.sourceforge.net/doc/docbook/user/bochsrc.html#BOCHSOPT-ATA>.

 vim:tw=78:ts=8:noet:ft=help:norl:
