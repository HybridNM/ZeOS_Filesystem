/*
 * fat32.c
 */

#include <fat32.h>
#include <ide.h>


// File system data structures
BPB biosParameterBlock;
EBR extendedBootRecord;
FSInfo fsInfoStruct;

DirectoryEntry rootDirectory;

// File Allocation Table, of fixed size and cluster size (4096B cluster)
// Drive is 5120000 bytes long (~4.9MB): 5120000/4096=1250
// But the FAT takes up 2 whole clusters, so we will snap to the closest whole cluster
//  number, which is 2048
#define FAT_SIZE 2048
DWord FAT[FAT_SIZE];


// Get the different attributes from the corresponding disk sections
// Only works for FAT32, since other FAT versions have different data organization
void getFAT32attributes() 
{
	char bootRecord[512];
	char fsInfo[512];

	// Read the first two sectors, which contain most FS information
	ideread(bootRecord, 0);
	ideread(fsInfo, 1);

	char * bootRecPointer = &(bootRecord[0]);	// Pointer to the start of the boot record
	char * fsinfoPointer = &(fsInfo[0]);		// Pointer to the start of the FSInfo structure

	biosParameterBlock.bytesPerSector 	 = *(Word*)(bootRecPointer  + 0x0B);
	biosParameterBlock.sectorsPerCluster = *(bootRecPointer + 0x0D);
	biosParameterBlock.reservedSectors 	 = *(Word*)(bootRecPointer  + 0x0E);
	biosParameterBlock.FATs 			 = *(bootRecPointer + 0x10);
	biosParameterBlock.rootDirEntries 	 = *(Word*)(bootRecPointer  + 0x11);
	biosParameterBlock.numSectors 		 = *(Word*)(bootRecPointer  + 0x13);
	biosParameterBlock.mediaDescriptor 	 = *(bootRecPointer + 0x15);
	biosParameterBlock.sectorsPerTrack 	 = *(Word*)(bootRecPointer  + 0x16);
	biosParameterBlock.numHeads 		 = *(Word*)(bootRecPointer  + 0x18);
	biosParameterBlock.hiddenSectors 	 = *(DWord*)(bootRecPointer + 0x1A);
	biosParameterBlock.numSectorsLSC 	 = *(DWord*)(bootRecPointer + 0x20);

	extendedBootRecord.sectorsPerFAT 	 = *(DWord*)(bootRecPointer + 0x24);
	extendedBootRecord.flags 			 = *(Word*)(bootRecPointer  + 0x28);
	extendedBootRecord.FATversion 		 = *(Word*)(bootRecPointer  + 0x2A);
	extendedBootRecord.rootClusterNum 	 = *(DWord*)(bootRecPointer + 0x2C);
	extendedBootRecord.FSInfoSectorNum 	 = *(Word*)(bootRecPointer  + 0x30);
	extendedBootRecord.bkBootSectorNum 	 = *(Word*)(bootRecPointer  + 0x32);
	extendedBootRecord.driveNum 		 = *(bootRecPointer + 0x40);
	extendedBootRecord.signature 		 = *(bootRecPointer + 0x42);

	for (int i=0; i<8; i++) extendedBootRecord.systemId[i] = *(bootRecPointer + 0x52 + i);

	fsInfoStruct.leadSignature 		= *(DWord*)(fsinfoPointer + 0x00);
	fsInfoStruct.otherSignature 	= *(DWord*)(fsinfoPointer + 0x1E4);
	fsInfoStruct.lastKnownCluster 	= *(DWord*)(fsinfoPointer + 0x1E8);
	fsInfoStruct.whereClusters 		= *(DWord*)(fsinfoPointer + 0x1EC);
	fsInfoStruct.trailSignature 	= *(DWord*)(fsinfoPointer + 0x1FC);

	// The file system has size restrictions due to its simple implementation
	if (biosParameterBlock.bytesPerSector != 512 && biosParameterBlock.sectorsPerCluster != 8)
	{
		printk("Unsupported number of bytes per sector and/or sectors per cluster.\n");
	}

	// TEMP
	char buf[50];
	itoa2(biosParameterBlock.bytesPerSector, buf);
	printk("Bytes per sector: ");
	printk(buf);
	printk("\n");

	itoa2(biosParameterBlock.sectorsPerCluster, buf);
	printk("Sectors per cluster: ");
	printk(buf);
	printk("\n");

	itoa2(biosParameterBlock.reservedSectors, buf);
	printk("Reserved sectors: ");
	printk(buf);
	printk("\n");

	itoa2(extendedBootRecord.sectorsPerFAT, buf);
	printk("Sectors per FAT: ");
	printk(buf);
	printk("\n");

	itoa2(extendedBootRecord.rootClusterNum, buf);
	printk("Root dir cluster: ");
	printk(buf);
	printk("\n\n");
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


// Start up the file system
void initializeFAT()
{
	getFAT32attributes();
	initializeRootDir();
	readFATfromDisk();
}


void printFAT()
{
	char buf[60];
	for (int i=0; i<64; i++)
	{
		itoa2(i, buf);
		printk("FAT entry number ");
		printk(buf);
		printk(": ");
		itoa2(FAT[i], buf);
		printk(buf);
		printk("\n");
	}
}

// cluster is the entry where changes have been made
void copyFATtoDisk(DWord cluster)
{
	DWord startingFATsector = biosParameterBlock.reservedSectors;
	DWord sector_offset = (cluster * 4) / 512;
	DWord FATsectorStart = cluster - (cluster%128);

	char FATsector[512];
	
	for (int i=FATsectorStart; i<(FATsectorStart+128); i++)
	{
		FATsector[i*4+0-FATsectorStart] = (char)(FAT[i]);
		FATsector[i*4+1-FATsectorStart] = (char)(FAT[i]>>8);
		FATsector[i*4+2-FATsectorStart] = (char)(FAT[i]>>16);
		FATsector[i*4+3-FATsectorStart] = (char)(FAT[i]>>24);
	}
	idewrite(FATsector, startingFATsector+sector_offset);
}

void readFATfromDisk()
{
	DWord startingFATsector = biosParameterBlock.reservedSectors;
	DWord FATvalue;
	char FATsector[512];

	// Read all FAT sectors
	for (int i=0; i<extendedBootRecord.sectorsPerFAT; i++)
	{
		ideread(FATsector, startingFATsector + i);
		//printk("sector read\n");
		for (int j=0; j<128; j+=4)
		{
			DWord byte0 = ((Byte)(FATsector[j+0]));
			DWord byte1 = ((Byte)(FATsector[j+1]))<<8;
			DWord byte2 = ((Byte)(FATsector[j+2]))<<16;
			DWord byte3 = ((Byte)(FATsector[j+3]))<<24;

			FATvalue = byte3 | byte2 | byte1 | byte0;
			FAT[(i*128)+(j/4)] = FATvalue;
		}
	}

	printk("FAT Written to memory\n\n");
}


// Takes a path and extracts the entry number 'searchPos', assumes 11 characters per entry
//  /path/to/file, searchPos=0 -> "/          "
//  /path/to/file, searchPos=1 -> "path       "
//  /path/to/file, searchPos=2 -> "to         "
int extractFilename(char * path, char * filename, int searchPos)
{
	int i;
	int path_it = 0;
	int slashes = 0;
	
	// Initialize to whitespaces
	for (i=0; i<11; i++) filename[i] = ' ';

	// path_it will be set to the first char that must be read after the loop
	while (slashes < searchPos)
	{
		if (path[path_it] == '\0') return -1;
		
		if (path[path_it] == '/') slashes++;
		path_it++;
	}

	// Reset iterator
	i = 0;
	
	// Copy the corresponding path to filename until:
	//  a) Another '/' is detected (next directory/file has been reached)
	//  b) The file being copied is the last one and the end of string has been reached
	//  c) All 11 chars have been copied (redundant with both of the above, but better safe than sorry)
	while (path[path_it] != '/' && path[path_it] != '\0' && i<11)
	{
		filename[i] = path[path_it];
		path_it++;
		i++;
	}
	return 0;
}


// Returns 1 if the 11 bytes of path1 and path2 are equal
int compareFilename(char * path1, char * path2)
{
	for (int i=0; i<11; i++)
	{
		if (path1[i] != path2[i]) return 0;
	}

	return 1;	
}


// Reads a directory entry into a struct given a sector and entry
int getDirectoryEntry(char * sector, DirectoryEntry * entry, int offset) // TO DO: probably only reads a single byte
{
	// In case the offset is not a multiple of 32, returns the first entry
	if (offset % 32 != 0) offset = 0;

	// Read all useful fields into the entry
	for (int i=0; i<11; i++)
	{
		entry->filename[i] = sector[i + offset];
	}
	entry->attributes = sector[0x0B + offset];
	entry->first_cluster_hi = sector[0x14 + offset];
	entry->first_cluster_lo = sector[0x1A + offset];
	entry->size = sector[0x1C + offset];

	return 0;
}


int setDirectoryEntry(char * sector, DirectoryEntry * entry, int offset)
{
	for (int i=0; i<11; i++)
	{
		sector[i+offset] = entry->filename[i];
	}
	sector[0x0B + offset] = entry->attributes;
	// First cluster high
	sector[0x14 + offset] = (Byte)(entry->first_cluster_hi);
	sector[0x15 + offset] = (Byte)(entry->first_cluster_hi>>8);
	// First cluster low
	sector[0x1A + offset] = (Byte)(entry->first_cluster_lo);
	sector[0x1B + offset] = (Byte)(entry->first_cluster_lo>>8);
	// File size
	sector[0x1C + offset] = (Byte)(entry->size);
	sector[0x1D + offset] = (Byte)(entry->size>>8);
	sector[0x1E + offset] = (Byte)(entry->size>>16);
	sector[0x1F + offset] = (Byte)(entry->size>>24);
	
	return 0;
}


// Returns the disk sector given a FAT table cluster and optionally, an offset
unsigned getDiskSector(DWord cluster, int offset)
{
	return (cluster * biosParameterBlock.sectorsPerCluster + biosParameterBlock.reservedSectors + offset);
}


int searchFile(char * path)
{
	// Call recursive function with the root cluster (searchPos must be at least 1)
	return recursiveSearch(path, extendedBootRecord.rootClusterNum, 1);
}


// Returns first cluster of the file
//  'path' is a Unix-style path to the file to be accessed, formatted to have 11 character long files
//  'cluster' is the first cluster of the directory to be scanned
DWord recursiveSearch(char * path, DWord cluster, int searchPos)
{
	char sector[biosParameterBlock.bytesPerSector];

	// filename is the current file/directory from the path being checked
	char filename[11];
	int ret = extractFilename(path, filename, searchPos);

	// extractFilename will return a -1 if the searchPos parameter is larger than
	//  the number of files/directories in the path, this is used to search for directories
	if (ret == -1) return cluster;

	// Each cluster has multiple consecutive sectors, so we can read them consecutively
	for (int sectorNum=0; sectorNum<biosParameterBlock.sectorsPerCluster; sectorNum++)
	{
		// Get the disk sector
		unsigned disk_sector = getDiskSector(cluster, sectorNum);
		ideread(sector, disk_sector);
		

		// Read a directory entry by entry (each one is 32 bytes long)
		for (int i=0; i<biosParameterBlock.bytesPerSector; i+=32)
		{
			DirectoryEntry entry;
			getDirectoryEntry(sector, &entry, i);

			// Read the first byte of the directory entry, which contains either
			//  the file name or special codes, such as the following:
			switch (entry.filename[0])
			{
				case 0x00: // entry is empty and there are no further entries, return
					return 0xFFFFFFFF;
					break;

				case 0x2E: // . or .. entry, self/prior directory, skip
					break;

				case 0xE5: // entry has been previously deleted and/or is available, skip
					break;

				default: // used entry with a filename
					if (entry.attributes == 0x0F) break; // entry is a vfat entry
					if (entry.attributes == 0x10)  // entry is a subdirectory
					{
						// Check if the subdirectory is part of the path, if it is, search
						//  recursively in the cluster taken from its entry, otherwise skip
						if (compareFilename(filename, (char *)entry.filename))
						{
							// Build the subdir first cluster number from the two halves
							DWord subDirCluster = ((DWord)(entry.first_cluster_hi))<<16 | entry.first_cluster_lo;
							return recursiveSearch(path, subDirCluster, searchPos+1);
						}
					}
					// Entry is not a subdirectory (thereforse it's a file), if the
					//  names match, then the file has been found, return the first
					//  cluster built by the two halves
					else if (compareFilename(filename, (char *)entry.filename))
					{
						DWord fileCluster = ((DWord)(entry.first_cluster_hi))<<16 | entry.first_cluster_lo;
						return fileCluster;
					}
			}
		}
	}
	// FAT chain is over, there are no more entries in the directory (also checks range)
	//  0x0FFFFFF8-0x0FFFFFFF, 0x0FFFFFF7 would be a bad sector
	if (cluster > FAT_SIZE || FAT[cluster] > 0x0FFFFFF7) return 0xFFFFFFFF; 
	else {
		// Otherwise follow cluster chain
		return recursiveSearch(path, FAT[cluster], searchPos);
	}
}


DWord allocateCluster(DWord startingCluster) // startingCluster = 0 is a special value
{
	DWord cluster = startingCluster;
	DWord i = 2;

	// newFile=1 implies there is no existing cluster chain, so a fresh one has to be created
	char newFile;
	if (startingCluster == 0) newFile = 1;
	
	if (newFile == 0)
	{
		// Search for the last cluster in the chain
		while (FAT[cluster] < 0x0FFFFFF7) cluster = FAT[cluster];
	}
	
	// Maybe the filesystem has a hint as to where to start the search for free clusters
	if (fsInfoStruct.whereClusters != 0xFFFFFFFF)
	{
		i = fsInfoStruct.whereClusters;
	}
	
	// Search for the first available cluster (0 and 1 are reserved)
	for (; i<FAT_SIZE; i++)
	{
		// Update the FAT with the new values when found
		// 0x?0000000 means the cluster is not in use and available
		if (FAT[i] == 0x00000000) 
		{
			if (newFile == 0) FAT[cluster] = i; // Only for existing FAT chains
			FAT[i] = 0x0FFFFFFF; // End of chain
			fsInfoStruct.whereClusters = i+1;
			return i;
		}
	}
	// No available clusters
	return 0x0FFFFFFF;
}


// Change parameters to OFT_entry
int writeFile(DWord startingCluster, char * buffer, int size, int startByte) // TO DO: Size update
{
	DWord cluster = startingCluster;

	// Variable initialization

	int bytesPerCluster = biosParameterBlock.bytesPerSector*biosParameterBlock.sectorsPerCluster;
	// Number of the cluster within the cluster chain, first cluster is 0
	int clusterChainNum = startByte / bytesPerCluster;
	// Same but with sectors within the cluster
	int clusterSectorNum = (startByte % bytesPerCluster) / biosParameterBlock.bytesPerSector;
	// Initial offset within the starting sector
	int offset = startByte % biosParameterBlock.bytesPerSector;

	// Gets the cluster and sector where the read begins
	while (clusterChainNum > 0)
	{
		cluster = FAT[cluster];
		clusterChainNum--;
	}

	// Get the disk sector
	unsigned disk_sector = getDiskSector(cluster, clusterSectorNum);


	// Actual writing

	char localBuffer[biosParameterBlock.bytesPerSector];
	int remainingBytes = size;
	int buffer_it = offset;
	int sector_it = offset;

	// Read the starting sector
	ideread(localBuffer, disk_sector);

	// Reads a sector from disk, writes over it, and once the sector has been gone
	//  through or there are no remaining bytes to write, the sector is once again
	//  written to the disk
	while (remainingBytes > 0)
	{
		// End of sector has been reached
		if (sector_it == biosParameterBlock.bytesPerSector)
		{
			// Write the modified sector to disk
			idewrite(localBuffer, disk_sector);

			// There are remaining sectors in the cluster
			if (clusterSectorNum < biosParameterBlock.sectorsPerCluster)
			{
				disk_sector++;
				clusterSectorNum++;
				ideread(localBuffer, disk_sector);
			}
			// Last sector in the cluster reached
			else
			{
				// Follow the FAT chain and read the first sector of the new cluster
				cluster = FAT[cluster];

				// End of last cluster in the file reached, must allocate a new cluster
				if (cluster > 0x0FFFFFF7)
				{
					cluster = allocateCluster(startingCluster);
				}

				disk_sector = getDiskSector(cluster, 0);
				ideread(localBuffer, disk_sector);
			}
			sector_it = 0;
		}

		localBuffer[sector_it] = buffer[buffer_it-offset];

		buffer_it++;
		sector_it++;
		remainingBytes--;
	}

	// Write the remaining bytes to the disk
	idewrite(localBuffer, disk_sector);

	return 0;
}


int readFile(DWord startingCluster, char * buffer, int size, int startByte)
{
	DWord cluster = startingCluster;

	// Variable initialization

	int bytesPerCluster = biosParameterBlock.bytesPerSector*biosParameterBlock.sectorsPerCluster;
	// Number of the cluster within the cluster chain, first cluster is 0
	int clusterChainNum = startByte / bytesPerCluster;
	// Same but with sectors within the cluster
	int clusterSectorNum = (startByte % bytesPerCluster) / biosParameterBlock.bytesPerSector;
	// Initial offset within the starting sector
	int offset = startByte % biosParameterBlock.bytesPerSector;

	// Gets the cluster and sector where the read begins
	while (clusterChainNum > 0)
	{
		cluster = FAT[cluster];
		clusterChainNum--;
	}

	// Get the disk sector
	unsigned disk_sector = getDiskSector(cluster, clusterSectorNum);


	// Actual reading

	char localBuffer[biosParameterBlock.bytesPerSector];
	int remainingBytes = size;
	int buffer_it = offset;
	int sector_it = offset;

	// Read the starting sector
	ideread(localBuffer, disk_sector);

	// Copies the content of the sector until the end is reached, then copies
	//  a new sector into 'localBuffer' once the end of the first one is reached
	while (remainingBytes > 0)
	{
		// End of sector has been reached
		if (sector_it == biosParameterBlock.bytesPerSector)
		{
			// There are remaining sectors in the cluster
			if (clusterSectorNum < biosParameterBlock.sectorsPerCluster)
			{
				disk_sector++;
				clusterSectorNum++;
				ideread(localBuffer, disk_sector);
			}
			// Last sector in the cluster reached
			else
			{
				// Follow the FAT chain and read the first sector of the new cluster
				cluster = FAT[cluster];
				if (cluster > 0x0FFFFFF7) break; // Chain is over

				clusterSectorNum = 0;
				disk_sector = getDiskSector(cluster, 0);
				ideread(localBuffer, disk_sector);
			}
			sector_it = 0;
		}

		buffer[buffer_it-offset] = localBuffer[sector_it];

		buffer_it++;
		sector_it++;
		remainingBytes--;
	}

	return 0;
}


// TEMP
void itoa2(int a, char *b)
{
  int i, i1;
  char c;
  
  if (a==0) { b[0]='0'; b[1]=0; return ;}
  
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
}

int createFile(char * path, char * filename, Byte attributes) // filename must be 11 bytes long
{
	DWord dirCluster = searchFile(path); 
	DWord newCluster = 0;
	char sector[512];

	// TEMP
	char clu[20];
	itoa2(dirCluster, clu);
	printk("createFile func, dirCluster: ");
	printk(clu);
	printk("\n");


	// Go through the clusters in the directory
	while (dirCluster < 0x0FFFFFF7)
	{
		// Go through the sectors of each cluster
		for (int sectorNum=0; sectorNum<biosParameterBlock.sectorsPerCluster; sectorNum++)
		{
			// Get the directory disk sector
			unsigned disk_sector = getDiskSector(dirCluster, sectorNum);
			ideread(sector, disk_sector);

			// Read a directory entry by entry (each one is 32 bytes long)
			for (int i=0; i<biosParameterBlock.bytesPerSector; i+=32)
			{
				DirectoryEntry entry;
				getDirectoryEntry(sector, &entry, i);

				// End of directory, and where the new dir. entry will reside
				if (entry.filename[0] == 0x00)
				{
					// Last entry in the sector, must use the next one for the end of dir. entry
					if (i == (biosParameterBlock.bytesPerSector-32))
					{
						// Allocate a new cluster to contain the new file
						newCluster = allocateCluster(0);
						if (newCluster == 0x0FFFFFFF) return -1; // No available clusters
						FAT[newCluster] = 0x0FFFFFFF;
						copyFATtoDisk(newCluster);

						// Fill the contents of the directory entry
						for (int byte=0; byte<11; byte++)
						{
							// Additional check to make sure the name is 11 bytes long
							//  and filled with spaces to the end (0x20 is a blank space)
							if (filename[byte] < 0x20) entry.filename[byte] = 0x20;
							else entry.filename[byte] = filename[byte];
						}
						entry.first_cluster_lo = (Word)(newCluster);
						entry.first_cluster_hi = (Word)(newCluster>>16);
						entry.size = 0;
						entry.attributes = attributes; // attributes = 0x10 means the file is a directory

						// Write the contents of the entry to the corresponding place in the sector
						setDirectoryEntry(sector, &entry, i);
						idewrite(sector, disk_sector);

						// Allocate a new cluster and write the end of directory entry
						//  in the first sector of said cluster in case we're on the last sector
						if (sectorNum == (biosParameterBlock.sectorsPerCluster-1))
						{
							dirCluster = allocateCluster(dirCluster);
							disk_sector = getDiskSector(dirCluster, 0);
						}
						else disk_sector++; // Otherwise just go to the next sector

						// Next sector should be empty, but it's safer to just read it anyway
						// Set the first entry on the next cluster as end of directory
						ideread(sector, disk_sector);
						sector[0] = 0x00;
						idewrite(sector, disk_sector);

						return newCluster;
					}
					// Not in an else since it will always return before reaching here
					// Allocate a new cluster to contain the new file
					newCluster = allocateCluster(0);

					itoa2(newCluster, clu);
					printk("New cluster: ");
					printk(clu);
					printk("\n");

					if (newCluster == 0x0FFFFFFF) return -1; // No available clusters
					FAT[newCluster] = 0x0FFFFFFF;
					copyFATtoDisk(newCluster);

					// Fill the contents of the directory entry
					for (int byte=0; byte<11; byte++)
					{
						// Additional check to make sure the name is 11 bytes long
						//  and filled with spaces to the end (0x20 is a blank space)
						if (filename[byte] < 0x20) entry.filename[byte] = 0x20;
						else entry.filename[byte] = filename[byte];
					}
					entry.first_cluster_lo = (Word)(newCluster);
					entry.first_cluster_hi = (Word)(newCluster>>16);
					entry.size = 0;
					entry.attributes = attributes; // attributes = 0x10 means the file is a directory

					// Write the contents of the entry to the corresponding place in the sector
					setDirectoryEntry(sector, &entry, i);
					sector[i+32] = 0x00;

					// Set next entry as end of directory and write the changes
					idewrite(sector, disk_sector);

					return newCluster;
				}
			}
		}
		// Jump to next cluster in the directory
		dirCluster = FAT[dirCluster];
	}
	return -1;
}


void deleteFATchain(DWord cluster)
{
	// Go through the FAT entries
	if (FAT[cluster] < 0x0FFFFFF6)
	{
		deleteFATchain(FAT[cluster]);
	}
	// Flag entry as empty and available
	FAT[cluster] = 0;
}


int deleteFile(char * path, char * filename) 
{
	// TO DO: check for dirs, only delete if first entry = 0x00
	// Discarded for now, too much check needed (entries can be deleted and have valid files later)

	DWord dirCluster = searchFile(path);
	char sector[512];

	// Go through the clusters in the directory
	while (dirCluster < 0x0FFFFFF7)
	{
		// Go through the sectors of each cluster
		for (int sectorNum=0; sectorNum<biosParameterBlock.sectorsPerCluster; sectorNum++)
		{
			// Get the directory disk sector
			unsigned disk_sector = getDiskSector(dirCluster, sectorNum);
			ideread(sector, disk_sector);

			// Read a directory entry by entry (each one is 32 bytes long)
			for (int i=0; i<biosParameterBlock.bytesPerSector; i+=32)
			{
				DirectoryEntry entry;
				getDirectoryEntry(sector, &entry, i);

				// End reached without finding the file
				if (entry.filename[0] == 0x00) return -2; // 2 is ENOENT (No such file or directory)

				// File found
				if (compareFilename(filename, (char *)entry.filename))
				{
					// Remove the FAT chain 
					DWord fileCluster = ((DWord)(entry.first_cluster_hi))<<16 | entry.first_cluster_lo;
					deleteFATchain(fileCluster);

					// Flag as deleted and clean the rest of the directory entry
					sector[i] = 0xE5;
					for (int file_it=i+1; file_it<i+32; file_it++) sector[file_it] = 0x00;
					
					// Write changes to the disk
					idewrite(sector, disk_sector);
				}
			}
		}
		// Jump to next cluster in the directory
		dirCluster = FAT[dirCluster];
	}
	return 0;
}


void FStest(int testNum)
{
	// Starting sirectory hierarchy and content:
	//
	// / ____ DIR1 ____ FILE0 (file 0 here)
	//     |         \_ FILE1 (file 1 here)
	//     |
	//     \_ DIR2 ____ FILE2 (file 2 here)

	#define READ_TEST 1
	#define WRITE_TEST 2
	#define CREATE_TEST 3
	#define DELETE_TEST 4

	// Variables for the read and write test
	char file0r[30] = "/DIR1/FILE0";
	char file1r[30] = "/DIR1/FILE1";
	char file2r[30] = "/DIR2/FILE2";
	char fileContent[50];
	DWord cluster0;
	DWord cluster1;
	DWord cluster2;

	// Variables for the write test
	char writeContent0[50] = "Overwritten FILE0";
	char writeContent1[50] = "Overwritten FILE1";
	char writeContent2[50] = "Overwritten FILE2";

	// Variables for the create and delete test
	char dir1c[30] = "/DIR1";
	char dir2c[30] = "/DIR2";
	char file3c[11] = "FILE3      ";
	char file4c[11] = "FILE4      ";
	char file5c[11] = "DIR3       ";
	char dir3c[30] = "/DIR1/DIR3";
	char file6c[11] = "FILE6      ";

	// Variables for the delete test:
	char file1d[11] = "FILE1      ";
	char file2d[11] = "FILE2      ";

	switch (testNum)
	{
	case READ_TEST:

		cluster0 = searchFile(file0r);
		cluster1 = searchFile(file1r);
		cluster2 = searchFile(file2r);

		readFile(cluster0, fileContent, 50, 0);
		printk("file0 content: ");
		printk(fileContent);
		printk("\n");

		readFile(cluster1, fileContent, 50, 0);
		printk("file1 content: ");
		printk(fileContent);
		printk("\n");

		readFile(cluster0, fileContent, 50, 5);
		printk("file2 content, starting position=5: ");
		printk(fileContent);
		printk("\n");

		break;

	case WRITE_TEST:
		
		cluster0 = searchFile(file0r);
		cluster1 = searchFile(file1r);
		cluster2 = searchFile(file2r);

		writeFile(cluster0, writeContent0, 50, 0);
		writeFile(cluster1, writeContent1, 50, 0);
		writeFile(cluster2, writeContent2, 50, 5);
		printk("Files written\n");

		break;

	case CREATE_TEST:
		
		createFile(dir1c, file3c, 0x20);
		createFile(dir1c, file4c, 0x20);
		createFile(dir1c, file5c, 0x10);
		createFile(dir3c, file6c, 0x20);
		printk("Files created\n");

		break;

	case DELETE_TEST:
		
		deleteFile(dir1c, file1d);
		deleteFile(dir2c, file2d);
		printk("Files deleted\n");

		break;
	
	default:
		break;
	}

	copyFATtoDisk(0);
}

#if 0 // not used
// Gets the value of some important variables and then copies the FAT to memory
void FATtoMemory()
{
	// VARIABLE INIZIALISATION

	// Check for actual number of sectors in the FAT
	if (biosParameterBlock.numSectors == 0)
		 clustersFAT = biosParameterBlock.numSectors * biosParameterBlock.sectorsPerCluster;
	else clustersFAT = biosParameterBlock.numSectorsLSC * biosParameterBlock.sectorsPerCluster;

	// 32 bit DWord will only support FAT_size of up to ~3900MB drives (less than 4GB)
	DWord FAT_size = 4*clustersFAT; // size in bytes, 32 bits per entry
	DWord numFrames = FAT_size/0x1000;


	// MEMORY ALLOCATION

	int frames[numFrames]; // needed memory frames to allocate the FAT
	// 20 pages for 128MB drive

	for (int i=0; i<numFrames; i++)
	{
		frames[i] = alloc_frame();
		// add error check later

		// !!!!!!!!!!!!!!!!!!!!!!!!!!!!
		// paginas reservadas, ahora toca buscar la primera direccion 
		// y asignarsela al puntero FAT
		// !!!!!!!!!!!!!!!!!!!!!!!!!!!!
	}
	

	// COPYING FAT FROM DISK

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
	int i = 0;
	int newPathStart = 0;
	int pathLength = strLen(path);

	do { // Sets the position where the path will be copied after trimming the first dir
		i++; 
		newPathStart++;
	}
	while(path[i] != '/');

	char newPath[pathLength-newPathStart];

	// Copy the remainder of the path
	for (i; i<pathLength; i++) newPath[i-newPathStart] = path[i];
	return &newPath;
}

#endif