/*
 * fat32.c
 */

#include <fat32.h>
#include <ide.h>
#include <mm.h>

// File system data structures
BPB biosParameterBlock;
EBR extendedBootRecord;
FSInfo fsInfoStruct;

DirectoryEntry rootDirectory;

DWord * FAT;

// Other global variables
DWord clustersFAT;


// Start up the file system
void initializeFAT()
{
	getFAT32attributes();
	initializeRootDir();
}


// Get the different attributes from the corresponding disk sections
// Only works for FAT32, since other FAT versions have different data organization
void getFAT32attributes()
{
	Byte * bootRecPointer;	// Pointer to the start of the boot record
	Byte * fsinfoPointer;	// Pointer to the start of the FSInfo structure

	ideread(bootRecPointer, 0);
	ideread(fsinfoPointer, 1);

	biosParameterBlock.bytesPerSector 	 = *(Word*)(bootRecPointer + 0x0B);
	biosParameterBlock.sectorsPerCluster = *(bootRecPointer + 0x0D);
	biosParameterBlock.reservedSectors 	 = *(Word*)(bootRecPointer + 0x0E);
	biosParameterBlock.FATs 			 = *(bootRecPointer + 0x10);
	biosParameterBlock.rootDirEntries 	 = *(Word*)(bootRecPointer + 0x11);
	biosParameterBlock.numSectors 		 = *(Word*)(bootRecPointer + 0x13);
	biosParameterBlock.mediaDescriptor 	 = *(bootRecPointer + 0x15);
	biosParameterBlock.sectorsPerTrack 	 = *(Word*)(bootRecPointer + 0x16);
	biosParameterBlock.numHeads 		 = *(Word*)(bootRecPointer + 0x18);
	biosParameterBlock.hiddenSectors 	 = *(DWord*)(bootRecPointer + 0x1A);
	biosParameterBlock.numSectorsLSC 	 = *(DWord*)(bootRecPointer + 0x20);

	extendedBootRecord.sectorsPerFAT 	 = *(DWord*)(bootRecPointer + 0x24);
	extendedBootRecord.flags 			 = *(Word*)(bootRecPointer + 0x28);
	extendedBootRecord.FATversion 		 = *(Word*)(bootRecPointer + 0x2A);
	extendedBootRecord.rootClusterNum 	 = *(DWord*)(bootRecPointer + 0x2C);
	extendedBootRecord.FSInfoSectorNum 	 = *(Word*)(bootRecPointer + 0x30);
	extendedBootRecord.bkBootSectorNum 	 = *(Word*)(bootRecPointer + 0x32);
	extendedBootRecord.driveNum 		 = *(bootRecPointer + 0x40);
	extendedBootRecord.signature 		 = *(bootRecPointer + 0x42);

	for (int i=0; i<8; i++) extendedBootRecord.systemId[i] = *(bootRecPointer + 0x52 + i);

	fsInfoStruct.leadSignature 		= *(DWord*)(fsinfoPointer + 0x00);
	fsInfoStruct.otherSignature 	= *(DWord*)(fsinfoPointer + 0x1E4);
	fsInfoStruct.lastKnownCluster 	= *(DWord*)(fsinfoPointer + 0x1E8);
	fsInfoStruct.whereClusters 		= *(DWord*)(fsinfoPointer + 0x1EC);
	fsInfoStruct.trailSignature 	= *(DWord*)(fsinfoPointer + 0x1FC);
}


// Initialize useful fields of the root directory
void initializeRootDir()
{
	rootDirectory.filename[0] = '/';
	rootDirectory.attributes = 0x10; // Is a directory
	rootDirectory.first_cluster_hi = (extendedBootRecord.rootClusterNum >> 16);
	rootDirectory.first_cluster_lo = (extendedBootRecord.rootClusterNum);
	for (int i=1; i<11; i++) rootDirectory.filename[i] = ' ';
}


void func()
{
	// Check for actual number of sectors in the FAT
	if (biosParameterBlock.numSectors == 0)
		 clustersFAT = biosParameterBlock.numSectors * biosParameterBlock.sectorsPerCluster;
	else clustersFAT = biosParameterBlock.numSectorsLSC * biosParameterBlock.sectorsPerCluster;


	// 32 bit DWord will only support FAT_size of up to ~3900MB drives (less than 4GB)
	DWord FAT_size = 4*clustersFAT; // size in bytes, 32 bits per entry
	DWord numFrames = FAT_size/0x1000;

	int frames[numFrames]; // needed memory frames to allocate the FAT
	// 20 pages for 128MB drive

	for (int i=0; i<numFrames; i++)
	{
		frames[i] = alloc_frame();
		// add error check later
		// set_ss_pag(?,?,?)
	}
	

	DWord currentFATsector;

	// Read the FAT from disk to memory, sector by sector
	for (int i=0; i<extendedBootRecord.sectorsPerFAT; i++)
	{
		// The FAT region starts right after the reserved sectors, which include 
		//  the boot sector, the FSInfo and sometimes other sectors
		currentFATsector = biosParameterBlock.reservedSectors + i;
		ideread(FAT+i*512, currentFATsector);
	}
	
	

}

// Takes a path and cuts up to the first '/', example:
//  /path/to/file -> path/to/file
//   path/to/file -> to/file
char * trimPath(char * path)
{
	size_t pathSize = sizeof(path);
	int i = 0;
	int newPathStart = 0;

	do { // Sets the position where the path will be copied after trimming the first dir
		i++; 
		newPathStart++;
	}
	while(path[i] != '/');

	char newPath[pathSize-newPathStart];

	// Copy the remainder of the path
	for (i; i<pathSize; i++) newPath[i-newPathStart] = path[i];
	return newPath;
}

// ADD ERROR CHECKS FOR BOTH
// ADD ERROR CHECKS FOR BOTH
// ADD ERROR CHECKS FOR BOTH

char * extractFilename(char * path)
{
	size_t pathSize = sizeof(path);
	int i = 0;
	int newPathStart = 0;


	char filename[8];

	if (path[0] == '/')
	{
		i++;
		newPathStart++;
	}

	// Copy the first filename and fill the rest with blank spaces
	for (i; i<pathSize || path[i]!='/'; i++) filename[i-newPathStart] = path[i];
	for (i; i<pathSize; i++) filename[i] = ' ';
}


int searchFile(char * path, DWord cluster)
{
	// call recursive function
}

// Returns first cluster of the file
int recursiveSearch(char * path, DWord cluster)
{
	Byte sector[512];

	char * filename = extractFilename(path);
	char * nextPath = trimPath(path);

	// Each cluster has multiple consecutive sectors, so we can read them consecutively
	for (int sectorNum=0; sectorNum<biosParameterBlock.sectorsPerCluster; sectorNum++)
	{
		// Subtracting 2 to the cluster number is necessary since the first 2 clusters
		//  in a FAT table are reserved, not doing so would waste 2 clusters in the disk
		unsigned disk_sector = (cluster-2)*biosParameterBlock.sectorsPerCluster + sectorNum;
		ideread(sector, disk_sector);

		// Read a directory entry by entry (each one is 32 bytes long)
		for (int i=0; i<512; i+=32)
		{
			// Read the first byte of the directory entry, which contains either
			//  the file name or special codes, such as the following:
			switch (sector[i])
			{
				case 0x00: // entry is empty and there are no further entries, return
					return -1;
					break;

				case 0x2E: // . entry, self directory, skip
					break;

				case 0xE5: // entry has been previously deleted and/or is available, skip
					break;

				default: // used entry with a filename
					if (sector[i+0x0B] == 0x80); // entry is reserved
					if (sector[i+0x0B] == 0x10)  // entry is a subdirectory
					{
						///////// THIS SEARCHES ANY SUBDIR, IT SHOULD FOLLOW THE PATH ///////
						///////// FIX ////////////////

						if (compareFilename(filename, &sector[i+0x0B]))
						{
							// Build the subdir first cluster number from the two halves
							DWord subDirCluster = ((DWord)(sector[i+0x14]))<<16 | sector[i+0x1A];
							return recursiveSearch(path, subDirCluster);
						}

						
					}
					// parse filename and compare, return file start cluster if found
			}
		}
	}
	// FAT chain is over, there are no more entries in the directory
	//  0x0FFFFFF8-0x0FFFFFFF, 0x0FFFFFF7 would be a bad sector
	if (FAT[cluster] > 0x0FFFFFF7) return -1; 
	else {
		// Follow cluster chain
		return recursiveSearch(path, FAT[cluster]);
	}
}

// write would need a previous open, then use a specific FD for it to write to disk


// writes should update both the memory and the disk FATs

// create dir should set first entry first byte to 0x00




// Si una entrada de la FAT vale 0x0FFFFFF7 es un bad sector
// Si vale 0x0FFFFFF8-0x0FFFFFFF es que no hay nada más en la cadena


// Esqueleto de lectura multi-sector
// Se supone un cluster de 4KB, es decir, 8 sectores de 512B

// for (int i=0; i<biosParameterBlock.sectorsPerCluster; i++)
// {
//	   ideread(b+(512*i), sector + i);
// }
