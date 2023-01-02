/*
 * fat32.h - Definición del sistema de ficheros
 */

#ifndef __FAT32_H__
#define __FAT32_H__

#include <types.h>


// Standard 8.3 format for FAT directories, doesn't support Long name files,
//  so maximum file name length is 8 characters, plus 3 char extension.

// This data struct must be exactly 32 bytes long, and is stored in this 
//  exact format in directory clusters, one after another.

typedef struct
{
    Byte  filename[11];		// 8 first bytes are file name, last 3 are for extension
    Byte  attributes;    	// (*)
    Byte  WNT;           	// Reserved for Windows NT, unused
    Byte  creation_secs; 	// Creation time: tenths of seconds (0-199) (to add precision)
    Word  creation_time;	// Creation time: hours[15-11], minutes[10-5], seconds*2[4-0]
    Word  creation_date;	// Creation time: year[15-9], month[8-5], day[4-0]
    Word  last_access;		// Last date of access, same format as creation_date
    Word  first_cluster_hi;	// High 16 bits of this entry's first cluster number
    Word  last_mod_time;	// Last modification time, same format as creation_time
    Word  last_mod_date;	// Last modification time, same format as creation_date
    Word  first_cluster_lo;	// Low 16 bits of this entry's first cluster number
    DWord size;          	// Size in bytes
} DirectoryEntry;

/* Attribute Byte bits: (*)
//
//  7 6 5 4 3 2 1 0
//  | | | | | | | |
//  | | | | | | | \ 
//  | | | | | | \  READ_ONLY
//  | | | | | \  HIDDEN
//  | | | | \  SYSTEM
//  | | | \  VOLUME_ID
//  | | \  DIRECTORY
//  | \  ARCHIVE
//  \  not currently used
//   not currently used
//
//  If bits 3-0 are all set, it's a Long Name File */


// BPB (Bios Parameter Block), is locatead near the start of the boot sector
//  and contains basic information about the connected drive.

typedef struct
{
	Word  bytesPerSector;		// bytes per sector (little endian)
	Byte  sectorsPerCluster;	// sectors per cluster
	Word  reservedSectors; 		// reserved sectors
	Byte  FATs;					// number of FATs
	Word  rootDirEntries;		// number of root dir. entries (ignore)
	Word  numSectors;			// total sectors (ignore if =0)
	Byte  mediaDescriptor;		// media descriptor type (ignore)
	Word  sectorsPerTrack;		// sectors per track (ignore)
	Word  numHeads;				// number of heads (ignore)
	DWord hiddenSectors;		// number of hidden sectors
	DWord numSectorsLSC;		// large sector count (used if more than 65535 sectors)
} BPB;


// Extended boot record (FAT32 exclusive), contains additional information
//  about the drive. Differs from other FAT versions.

typedef struct
{
	DWord sectorsPerFAT;	// size of the FAT in sectors
	Word  flags;			// self explanatory
	Word  FATversion;		// high byte is the major and low is the minor
	DWord rootClusterNum;	// cluster number of the root directory
	Word  FSInfoSectorNum;	// sector number of the FSInfo structure
	Word  bkBootSectorNum;	// sector number of the backup boot sector
	Byte  driveNum;			// drive number (ignore)
	Byte  signature;		// must be 0x28 or 0x29
	Byte  systemId[8];		// system identifier string, always "FAT32" (not trustworthy)
} EBR;


// FSInfo Structure, a FAT32 only sector with little information.

typedef struct 
{
	DWord leadSignature;	// must be 0x41615252
	DWord otherSignature;	// must be 0x61417272
	DWord lastKnownCluster;	// last known free cluster, if =0xFFFFFFFF it should be calculated
	DWord whereClusters;	// tells the FS where to start looking for available clusters, no hint if =0xFFFFFFFF
	DWord trailSignature;	// must be 0xAA550000
} FSInfo;


// Functions

void getFAT32attributes();
void initializeRootDir();
void initializeFAT();

void extractFilename(char * path, Byte * filename, int searchPos);
int compareFilename(char * path1, char * path2);
void deleteFATchain(DWord cluster);
int setDirectoryEntry(Byte * sector, DirectoryEntry * entry, int offset);
DirectoryEntry getDirectoryEntry(char * sector, int offset);

DWord recursiveSearch(char * path, DWord cluster, int searchPos);
DWord allocateCluster(DWord startingCluster);
int searchFile(char * path);

int readFile(DWord startingCluster, char * buffer, int size, int startByte);
int writeFile(DWord startingCluster, char * buffer, int size, int startByte);

int createFile(char * path, char * filename, Byte attributes);
int deleteFile(char * path, char * filename);

#endif