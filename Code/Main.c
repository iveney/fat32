#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <math.h>

#define MAX 1000000	//max. number of bytes for buffer
#define EOC 0xffffff8	//end-of-file marker for FAT32

int fid;
int bps;		//bytes per sector
int spc;		//sectors per cluster
unsigned long *FAT;

struct BootEntry 
{ 
	unsigned char BS_jmpBoot[3] __attribute__((packed)); 
	unsigned char BS_OEMName[8] __attribute__((packed)); 
	unsigned short BPB_BytsPerSec __attribute__((packed)); 
	unsigned char BPB_SecPerClus __attribute__((packed)); 
	unsigned short BPB_RsvdSecCnt __attribute__((packed)); 
	unsigned char BPB_NumFATs __attribute__((packed)); 
	unsigned short BPB_RootEntCnt __attribute__((packed)); 
	unsigned short BPB_TotSec16 __attribute__((packed)); 
	unsigned char BPB_Media __attribute__((packed)); 
	unsigned short BPB_FATSz16 __attribute__((packed)); 
	unsigned short BPB_SecPerTrk __attribute__((packed)); 
	unsigned short BPB_NumHeads __attribute__((packed)); 
	unsigned long BPB_HiddSec __attribute__((packed)); 
	unsigned long BPB_TotSec32 __attribute__((packed)); 
	unsigned long BPB_FATSz32 __attribute__((packed));
	unsigned short BPB_ExtFlags __attribute__((packed));
	unsigned short BPB_FSVer __attribute__((packed));
	unsigned long BPB_RootClus __attribute__((packed));
	unsigned short BPB_FSInfo __attribute__((packed));
	unsigned short BPB_BkBootSec __attribute__((packed));
	unsigned char BPB_Reserved[12] __attribute__((packed));
	unsigned char BS_DrvNum __attribute__((packed));
	unsigned char BS_Reserved1 __attribute__((packed));
	unsigned char BS_BootSig __attribute__((packed));
	unsigned long BS_VolID __attribute__((packed));
	unsigned char BS_VolLab[11] __attribute__((packed));
	unsigned char BS_FilSysType[8] __attribute__((packed));
};

struct DirEntry
{
	unsigned char DIR_Name[11] __attribute__((packed));
	unsigned char DIR_Attr __attribute__((packed));
	unsigned char DIR_NTRes __attribute__((packed));
	unsigned char DIR_CrtTimeTenth __attribute__((packed));
	unsigned short DIR_CrtTime __attribute__((packed));
	unsigned short DIR_CrtDate __attribute__((packed));
	unsigned short DIR_LstAccDate __attribute__((packed));
	unsigned short DIR_FstClusHI __attribute__((packed));
	unsigned short DIR_WrtTime __attribute__((packed));
	unsigned short DIR_WrtDate __attribute__((packed));
	unsigned short DIR_FstClusLO __attribute__((packed));
	unsigned long DIR_FileSize __attribute__((packed));
};

/* go to the sector indicated by "sector_number", 
 * then read n contiguous sectors (n = num_sectors), 
 * and write the result into "buffer" */
/* Remember to clear the buffer first */
int read_sectors(int sector_number, unsigned char *buffer, int num_sectors)
{
	int dest, len;     /* used for error checking only */

	dest = lseek(fid, sector_number*bps, SEEK_SET);
	if(dest != sector_number*bps)
	{
		printf("Error in reading sector\n");
	}
	if(bps*num_sectors > MAX)
	{
		printf("Buffer size too small");
	}
	len = read(fid, buffer, bps*num_sectors);
	if(len != bps*num_sectors)
	{
		printf("Error in reading sector\n");
	}
	return len;
}

int write_sectors(int sector_number, unsigned char *buffer, int num_sectors)
{
	int dest, len;

	dest = lseek(fid, sector_number*bps, SEEK_SET);
	if (dest != sector_number*bps)
	{
		printf("Error in writing sector\n");
	}
	if(bps*num_sectors > MAX)
	{
		printf("Buffer size too small");
	}
	len = write(fid, buffer, bps*num_sectors);
	if(len != bps*num_sectors)
	{
		printf("Error in writing sector\n");
	}
	return len;
}

int main()
{
	struct BootEntry bootEntry;
	struct DirEntry dirEntry;
	unsigned char *buffer;
	int data_SecNum;
	int rootDir_SecNum;
	int FAT_SecNum[2];	/* ASSUME that there are exactly 2 FATs */
	int i;

	buffer = (unsigned char *)malloc(sizeof(unsigned char) * MAX);

	fid = open("/dev/ram0", O_RDWR);

	/* read the boot sector from sector 0 */
	int count;
	if( (count=read(fid, buffer, 512)) != 512)
	{
		printf("Error in reading the root sector!!%d\n",count);
		exit(0);
	}
	memcpy(&bootEntry, buffer, sizeof(struct BootEntry));

	bps = bootEntry.BPB_BytsPerSec;
	spc = bootEntry.BPB_SecPerClus;
	printf("Name = %s\n", bootEntry.BS_OEMName);
	printf("Bytes per Sector = %d\n", bps);
	printf("Sector per Cluster = %d\n", spc);
	printf("Reserved Sector Count = %d\n", bootEntry.BPB_RsvdSecCnt);
	printf("Number of FATs = %d\n", bootEntry.BPB_NumFATs);
	printf("(32)Size of FAT = %ld\n", bootEntry.BPB_FATSz32);

	/* calculate the starting sectors of the 2 FATs, which are located just after the boot sector */
	FAT_SecNum[0] = bootEntry.BPB_RsvdSecCnt;
	FAT_SecNum[1] = bootEntry.BPB_RsvdSecCnt + bootEntry.BPB_FATSz32;

	/* allocate a temporary FAT for reading and writing */
	FAT = (unsigned long*)malloc(sizeof(unsigned long) * bootEntry.BPB_FATSz32 * bps / 4);

	/* calculate the starting sector of data area, which is located just after FAT */
	data_SecNum = bootEntry.BPB_RsvdSecCnt + 2 * bootEntry.BPB_FATSz32;

	/* read the root directory */
	memset(buffer, 0, MAX);
	rootDir_SecNum = data_SecNum + (bootEntry.BPB_RootClus-2) * spc;
	read_sectors(rootDir_SecNum, buffer, spc);

	printf("%d\n",rootDir_SecNum);

	/*** try to find the file which is created and deleted in a just formatted disk, so we just search the first 10 entries ***/
	for(i = 0; i < 10; i++)
	{
		memcpy(&dirEntry, buffer + i*sizeof(struct DirEntry), sizeof(struct DirEntry));
		printf("%d: %s (first char = %x)\n", i, dirEntry.DIR_Name, dirEntry.DIR_Name[0]);
		/*** the file name is fixed as "a.txt" ***/
		if(!strncmp((char *)dirEntry.DIR_Name+1, "B      TXT", 10))
		{
			printf("The file 'ab.txt' is found\n");
			printf("First Cluster = %d\n", dirEntry.DIR_FstClusLO);
			printf("File size = %ld\n", dirEntry.DIR_FileSize);
//			read_sectors(data_SecNum + (dirEntry.startCluster - 2) * spc);

			/* read the FAT on disk into the temp FAT */
			read_sectors(FAT_SecNum[0], (unsigned char *)FAT, bootEntry.BPB_FATSz32);

			/* see whether it is deleted already */
			printf("0x%lx, %ld\n", (unsigned long)FAT[dirEntry.DIR_FstClusLO], (unsigned long)FAT[dirEntry.DIR_FstClusLO]);

			if(FAT[dirEntry.DIR_FstClusLO] == 0)
			{
				/* update the temp FAT */
				printf("This file is deleted. Try to recover the file ...\n");
				FAT[dirEntry.DIR_FstClusLO] = 0xfffffff;

				/* write the temp FAT into the 2 FATs on disk */
				write_sectors(FAT_SecNum[0], (unsigned char *)FAT, bootEntry.BPB_FATSz32);
				write_sectors(FAT_SecNum[1], (unsigned char *)FAT, bootEntry.BPB_FATSz32);

				/* update the temp directory entry */
				dirEntry.DIR_Name[0] = 'A';

				/* write the temp directory entry into the directory entry on the disk */
				memcpy(buffer + i*sizeof(struct DirEntry), &dirEntry, sizeof(struct DirEntry));

				/* write the previous directory entry (i don't know why ... ) */
			//	memcpy(&dirEntry, buffer + (i-1)*sizeof(struct DirEntry), sizeof(struct DirEntry));
			//	dirEntry.DIR_Name[0] = 'A';
			//	memcpy(buffer + (i-1)*sizeof(struct DirEntry), &dirEntry, sizeof(struct DirEntry));

				write_sectors(rootDir_SecNum, buffer, spc);
			}

	//		break;
		}
	}

	/* close and free */
	close(fid);
	free(buffer);

	return 0;
}
