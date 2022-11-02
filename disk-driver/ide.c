/* Simple PIO-based (non-DMA) IDE driver code.
 * ===========================================
 * Juanjo Costa <jcosta@ac.upc.edu>
 * (Based on https://github.com/mit-pdos/xv6-public code)
 * Specification accesible at [1]
 *
 * [1] AT Attachment with Packet Interface - 6 (working draft), ANSI, 
 *     December 2001.  Available online at:
 *     https://pdos.csail.mit.edu/6.828/2018/readings/hardware/ATA-d1410r3a.pdf
 * [2] ATA PIO Mode. Available online at: https://wiki.osdev.org/ATA_PIO_Mode
 * [3] Diferent documents and specifications. Available online at: 
 *     http://www.ata-atapi.com/
 */
#include <ide.h>
#include <io.h>

char* IDE_VERSION="v1.0";

// This address MUST match the .bochsrc ioaddr1 option
#define IDEADDR 0x1F0


// Offset for the different registers
#define OFF_DATAR         0x0	//Data Register
#define OFF_FEATR         0x1	//Features Register (W)
#define OFF_ERROR         0x1	//Error Register (R)
#define OFF_SECCOUNTR     0x2   //Sector Count Register
#define OFF_LBALR         0x3   //LBA LOW Register
#define OFF_LBAMR         0x4   //LBA MID Register
#define OFF_LBAHR         0x5   //LBA HIGH Register
#define OFF_DEVR          0x6   //Device Register
#define OFF_COMMANDR      0x7   //Command Register (W)
#define OFF_STATUSR       0x7   //Status Register (R)

// Port Adresses for the registers
#define STATUS            (IDEADDR | OFF_STATUSR)		//Ex: 0x1F7
#define DATA              (IDEADDR | OFF_DATAR)			//Ex: 0x1F1
#define SECCOUNT          (IDEADDR | OFF_SECCOUNTR)		//Ex: 0x1F2
#define LBAL              (IDEADDR | OFF_LBALR)			//Ex: 0x1F3
#define LBAM              (IDEADDR | OFF_LBAMR)			//Ex: 0x1F4
#define LBAH              (IDEADDR | OFF_LBAHR)			//Ex: 0x1F5
#define DEV               (IDEADDR | OFF_DEVR)			//Ex: 0x1F6
#define CMD               (IDEADDR | OFF_COMMANDR)		//Ex: 0x1F7
#define DEVCTL            (IDEADDR | OFF_DEVR | 0x200)  //Ex: 0x3F6	
#define STATUSALT         DEVCTL

// Bits in STATUS register
#define IDE_BSY       0x80	//Busy								0b 1000 0000
#define IDE_DRDY      0x40	//Device ready						0b 0100 0000
#define IDE_DF        0x20	//Data fault						0b 0010 0000
//dfine IDE_SRV		  0x10	//Overlapped Mode Service Request	0b 0001 0000
#define IDE_DRQ       0x08	//Data ReQuest						0b 0000 1000
//dfine IDE_CORR	  0x04	//Corrected data (always 0)			0b 0000 0100
//dfine IDE_IDX		  0x02	//Index (always 0)					0b 0000 0010
#define IDE_ERR       0x01	//Error								0b 0000 0001

// Comands for COMMAND register
#define IDE_CMD_READ  0x20
#define IDE_CMD_WRITE 0x30


int havedisk0 = 0;
int havedisk1 = 0;

unsigned proso_blocks = 0;	// #Total de blocs al disk

/* ide_debug - If enabled shows debug information for register access */
volatile unsigned ide_debug = 0;


#define X(s)			\
	do {				\
		if (ide_debug) {\
			printk(s);	\
		}				\
	} while (0)

void decodeStatusRegister(Byte b);

// Wait for IDE disk to become ready.
// This is needed BEFORE issuing any new command.
// If 'checkerr' is disabled, then always return 0
// Otherwise, returns -1 if disk returns a fault or an error
static int idewait(int checkerr)
{
	int r;

	// Alternate Status Register: 0x3F6 (R)
	// --------------------------
	// (Same as Status Register but without clearing pending interrupts)

	// Status Register content [1]: 0x1F7 (R)
	// ----------------------------
	// (Clears pending interrupts!)
	// 
	//     BSY DRDY DF # DRQ Obsolete ERR
	//     |  / ___/  / /   /        /
	//     | | |  ___/ /   /        /
	//     | | | |  __/   /        /
	//     | | | | |  ___/        /
	//     | | | | | | |  _______/
	//     | | | | | | | |
	//     7 6 5 4 3 2 1 0
	//                         Mask
	// BSY (Busy)              80h
	// DRDY(Device ReaDY)      40h
	// DF  (Device Fault)      20h
	// #   (Command dependent) 10h
	// DRQ (Data Request)      08h
	// Obsolete
	// ERR (ERRor)             01h


	// Mask of STATUSALT with 1100_0000 (bits BSY and DRDY), compares it to 0100_0000
	// 		if r=0100_0000 (BSY=0 and DRDY=1) breaks
	//		if r=0000_0000 also breaks

  	while((((r = inb(STATUSALT)) & (IDE_BSY|IDE_DRDY)) != IDE_DRDY) && (r!=0)) 
		decodeStatusRegister(r);
		;
		X("\n");
		decodeStatusRegister(r);


	// Mask of STATUSALT with 0010_0001 (bits DF and ERR)
	//		if different from 0, an error has ocurred, return -1
	//		if r==0000_0000 an error has ocurred (device not busy but data still not ready)

	if(checkerr && ( (r == 0) || ((r & (IDE_DF|IDE_ERR)) != 0) ))
		return -1;
	return 0;
}

void ide_soft_reset()
{
	// Device Control Register: 0x3F6 (W)
	// ------------------------
	// Permet fer un SOFT RESET al device (tant master com slave). O activar/desactivar
	// la 'assertion' del signal INTRQ del device.
	// 
	//     HOB Reserved SRST nIEN 0
	//     |  /        /    /    /
	//     | |        /    /    /
	//     | |       |  __/    /
	//     | |       | |  ____/
	//     | |_____  | | |
	//     | | | | | | | |
	//     7 6 5 4 3 2 1 0
	//
	// HOB  (High Order Byte) (Should be 0)
	// Bits (6:3) are reserved
	// SRST (Sotware ReSeT)
	// nIEN (No Interrupt) Enable/Disable INTRQ assertion. 0 -> INTRQ shall be enabled, 1 -> Release INTRQ signal

	outb(DEVCTL, 0x4);		// do a "Software ReSeT"(SRST) on the control bus (0000 0100b)
	outb(DEVCTL, 0x0);		// reset the bus to normal operation

	inb(STATUSALT);			// it might take 4 tries for status bits to reset
	inb(STATUSALT);			// ie. do a 400ns delay as spec details.
	inb(STATUSALT);			// (Each call takes ~100ns)
	inb(STATUSALT);

	idewait(0);
}

/* Resets IDE Device and checks for master and slave */
int ideinit(void)
{
  int i;

  ide_soft_reset();

  // Device Register content [1]:
  // ----------------------------
  //  Obsolete # Obs. DEV #
  //  |  _____/ /    /   /
  //  | |  ____/    /   /
  //  | | |  ______/   /
  //  | | | |  _______/
  //  | | | | | | | |
  //  | | | | | | | |
  //  7 6 5 4 3 2 1 0
  //
  //                        			Mask
  //	Obsolete (these are set to 1)
  //	# (Command dependent)
  //	DEV (DEVice)            		10h
  //    0 selects Device 0; 1 selects Device 1


	// Check if disk 1 is present (select device and check for any answer)

	// Send 111X_0000 to the device register, being X=1 to select drive 1
	// Then read the status register for a while and wait for an answer from the device.
	// 	if the status register is written by the device the variable havedisk1 is set. 

	outb(DEV, 0xe0 | (1<<4));
	for(i=0; i<1000; i++)
	{
		if(inb(STATUS) != 0)
		{
			havedisk1 = 1;
			break;
		}
	}

	// Same as before but with X=0, to select drive 0, and the variable havedisk0 is set instead.
	// This will switch back to device 0 even if device 1 was detected.

	// Switch back to disk 0.
	outb(DEV, 0xe0 | (0<<4));
	for(i=0; i<1000; i++)
	{
		if(inb(STATUS) != 0)
		{
			havedisk0 = 1;
			break;
		}
	}

	printk("\n** IDE Driver ");
	printk(IDE_VERSION);
	printk(" Loaded\n");
	
	return (havedisk0 || havedisk1)? 0: -1;
}

/* Set number of 'sectors' to access starting at block number 'block' from device 'device' */
int idesetblock(unsigned char sectors, unsigned block, unsigned char device)
{
	// Parameter check, block should never be above 28 bits large, since we're using 28 bit LBA + device can only be 0 or 1
	if (block >= 1<<28) return -1;
	if (device > 1) return -1;

	// Write block and sector information to the corresponding registers
	outb(SECCOUNT, sectors);
	outb(LBAL, (block         & 0xFF));	// block[7:0]
	outb(LBAM, ((block >> 8)  & 0xFF));	// block[15:8]
	outb(LBAH, ((block >> 16) & 0xFF));	// block[23:16]
	// Last 4 bits of the 28 are written later, in the DEV register

	// Prepare the higher part of the DEV register to be written
	Byte bits = 0x0F; 	// bits=0000_1111	All bits set to 1 (we want LBA=1 (bit 6), bit 7 and bit 5 set to 1 for backward compatibility)
	bits <<=1;			// bits=0001_1110
	bits |= device;		// bits=0001_111X	Set the device bit (its value is 0 or 1)
	bits <<=4;			// bits=111X_0000	Last bits reserved for block
	
	// The higher half of the register will be written as 111X (X being 0 or 1 depending on the device)
	// The lower half will be formed by the 4 highest bits of the 'block' variable
	// 	that value is masked with 0x0F
	outb(DEV, (((block >> 24) & 0x0F) | bits)); // 111X block[27:24]
	return 0;
}

/*
    Protocol for writing:
        9.3
    (1) Write_parameters
        Update Sector Count, LABx, Device Registers
			Register        Value
			-------------------------------------------
			Sector Count    N   (a 0 value specifies that 256 sectors are to be transfered)
			LBA Low         LBA(7:0)
			LBA Mid         LBA(15:8)
			LBA High        LBA(23:16)
			Device          obs:LBA:obs:DEV:LBA(27:24)
				LBA shall be set to 1 to specify the address is an LBA
    (2) Write_command
        Update Command Register
        9.6
    (3) Check_Status
        Using Alternate Status Register and ignoring the result (400ns minimum)(STATUSALT)
        When BSY==0 and DRQ==1 go to (4)
    (4) Transfer_Data
        Write the device Data Register (DATA)
        While DRQ == 1
        [DRQ==0] Set nIEN = 0 in Device Control Register go to (5)
    (5) INTRQ_Wait
*/
int idewrite( char * b, unsigned bloc )
{
	if (b == NULL) return -1;

	// Wait bus idle (aka Device READY)
	idewait(0);

	X("\nReady to write...");
	// Prepare parameters 
	idesetblock(1, bloc, 0);

	// Disable interrupts, aka synchronous read nIEN=1 (0000 0010b)
	outb(DEVCTL, 0x2);

	// Insert command 
	outb(CMD, IDE_CMD_WRITE);
	X("\nWrite command sent...\n");

	// Notes: When you send a command byte and the RDY bit of the Status Registers is clear, you may have to wait (technically up to 30 seconds) 
	//		  for the drive to spin up, before DRQ sets. You may also need to ignore ERR and DF the first four times that you read the Status, if you are polling.
	//Wait DATA REQUEST
	int r;
	unsigned times=0;

	// Mask of STATUSALT with 1000_1000 (bits BSY and DRQ), compares it to 0000_1000
	//	Similarly to other cases, checks for BSY=0 and DRQ=1
  	while(((r = inb(STATUSALT)) & (IDE_BSY|IDE_DRQ)) != IDE_DRQ) 
	{
		// If the check has been done more than 4 times, mask STATUSALT with the Error bit, if 1 return error.
		decodeStatusRegister(r);
		if ((times > 4) && ((r&IDE_ERR) == IDE_ERR)) return -1;
		times++;
	}
	X("\n");
	decodeStatusRegister(r);

	X("\nWriting data...");
	//Write DATA
	outsl(DATA, b, BLOCK_SIZE/4);

	X("\nData Written.");
	//Wait for transfer finalization
	
#if 0
  	while(((r = inb(STATUSALT)) & (IDE_BSY|IDE_DRQ)) != 0)
		{X(".");}
	;
	X("\n");
	decodeStatusRegister(r);
#endif

	idewait(0);

	X("\nFinished\n");
	//Done
	return 0;
}

/* Read 'bloc' block (BLOCK_SIZE bytes) into buffer 'b' */
// Behavior is exactly the same as the write function
int ideread( char * b, unsigned bloc )
{
	if (b ==  NULL) return -1;

	X("\nWait device ready...");
	// Wait bus idle (aka Device READY)
	idewait(0);

	X("\nReady to read...");
	// Prepare parameters 
	idesetblock(1, bloc, 0);

	// Disable interrupts, aka synchronous read nIEN=1 (0000 0010b)
	outb(DEVCTL, 0x2);

	// Insert command 
	outb(CMD, IDE_CMD_READ);
	X("\nRead command sent...\n");

	// Notes: When you send a command byte and the RDY bit of the Status Registers is clear, you may have to wait (technically up to 30 seconds) for the drive to spin up, before DRQ sets. You may also need to ignore ERR and DF the first four times that you read the Status, if you are polling.

	X("Wait command answer...\n");
	//Wait DATA REQUEST
	int r;
	unsigned times=0;
  	while(((r = inb(STATUSALT)) & (IDE_BSY|IDE_DRQ)) != IDE_DRQ) 
	{
		decodeStatusRegister(r);
		if ((times >4) && ((r&IDE_ERR) == IDE_ERR)) return -1;
		times++;
	}
	X("\n");
	decodeStatusRegister(r);

	X("\nReading data...");
	//Write DATA
	insl(DATA, b, BLOCK_SIZE/4);

	X("\nData Read.");
	//Wait for transfer finalization
	
#if 0
  	while(((r = inb(STATUSALT)) & (IDE_BSY|IDE_DRQ)) != 0)
		{X(".");}
	;
	X("\n");
	decodeStatusRegister(r);
#endif

	idewait(0);

	X("\nFinished");
	//Done
	return 0;
}

/* tobinary - return an array of 9 chars with the binary codification for byte 'b'. 'tmp' pointer should have enough space for these 9 chars. */
char * tobinary(Byte b, char *tmp)
{
	int i;
	tmp[8]=0; //Mark the end
	for(i = 7; i>=0; i--) {
		tmp[i] = (b&1) ? '1' : '0';
		b>>=1;
	}
	return tmp;
}

#if 0
char * myitoa(int a, char *b)
{
  int i, i1;
  char c;

  if (a==0) { b[0]='0'; b[1]=0; return b;}

  i=0;
  while (a>0)
  {
    b[i]=(a%10)+'0';
    a=a/10;
    i++;
  }

  for (i1=0; i1<i/2; i1++)
  {
    c=b[i1];
    b[i1]=b[i-i1-1];
    b[i-i1-1]=c;
  }
  b[i]=0;
  return b;
}
#define itoa myitoa
#else
char * itoa(int a, char *b);
#endif

void decodeStatusRegister(Byte b)
{
	if (!ide_debug) return;
	char tmp[20];
	printk("Status Register: ");
	itoa((int)b, tmp);
	printk(tmp);
	printk(" - ");
	printk(tobinary(b, tmp));
	printk(" - ");
	printk( (b&IDE_BSY) ? "BUSY" : "busy" );
	printk("|");
	printk( (b&IDE_DRDY) ?"READY": "ready" );
	printk("|");
	printk( (b&IDE_DF) ? "FAULT" : "fault" );
	printk("|#|");
	printk( (b&IDE_DRQ) ? "DATA REQ" : "data_req" );
	printk("|-|-|");
	printk( (b&IDE_ERR) ? "ERROR": "error" );
	printk("\n");
}

/* ide_routine - ide interrupts Service Routine */
void ide_routine(void)
{
	X("\n=============== IDE interrupt ===============\n");
	//Byte b = inb(STATUSALT);
	Byte b = inb(STATUS);

	decodeStatusRegister(b);

}

int ide_check_disk()
{
#define BLOC 512
  char b[BLOC];
  void init_block(char *b, char c) {
	int i;
	for (i=0; i<BLOC; i++) {b[i] = c;}
  }
  int check_block(const char *b, char val) {
	int pos = -1;
	int i;for(i=0; i<BLOC; i++) {if (b[i] != val) { return i;} };
	return pos;
  }

  if (!(havedisk0 || havedisk1)) { printk("\nNo disk available\n");return -1; }
  unsigned i;
  int res;
  res = 0;

  printk("** Starting tests **\n");
  /**********/
  init_block(b, 0xff);
  for(i=0; res>=0; i++) { //Write until failure...
	res = idewrite(b, i);
  }
  itoa(i-1, b);
  printk("\n");
  printk(b);
  printk(" blocs written\n");
  proso_blocks = i-1;

  res = 0;
  for(i=0; res>=0; i++) { //Read until failure...
	res = ideread(b, i);
  }
  itoa(i-1, b);
  printk("\n");
  printk(b);
  printk(" blocs read\n\n");

  /**********/
  printk("Write block 1 with 512 '0xab' bytes \n");
  init_block(b, 0xab);
  idewrite(b, 1);

  printk("Write block 3 with 512 '0xca' bytes \n");
  init_block(b, 0xca);
  idewrite(b, 3);

  printk("Write block 5 with 512 '0xab' bytes \n");
  init_block(b, 0xab);
  idewrite(b, 5);

  init_block(b, 0xff);
  ideread(b, 5);
  printk("Read block 5 that contain 512 '0xab' bytes ");
  res = check_block(b, 0xab);
  printk( ((res>=0) ? "[KO]\n" : "[OK]\n") );

  init_block(b, 0xff);
  ideread(b, 3);
  printk("Read block 3 that contain 512 '0xca' bytes ");
  res = check_block(b, 0xca);
  printk( ((res>=0) ? "[KO]\n" : "[OK]\n") );

  init_block(b, 0xff);
  ideread(b, 1);
  printk("Read block 1 that contain 512 '0xab' bytes ");
  res = check_block(b, 0xab);
  printk( ((res>=0) ? "[KO]\n" : "[OK]\n") );

  /**********/
  printk("Write block 2 with 512 '0x22' bytes \n");
  init_block(b, 0x22);
  idewrite(b, 2);
  init_block(b, 0xff);
  ideread(b, 2);
  res = check_block(b, 0x22);
  printk("Read block 2 that contain 512 '0x22' bytes ");
  printk( ((res>=0) ? "[KO]\n" : "[OK]\n") );


  printk("** Finished tests **\n\n");
  return 0;
}
