#ifndef __HEADER_H__
#define __HEADER_H__

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <limits.h>
//#define DEVICE_NAME "/dev/ram0"
//#define DEVICE_NAME "./filesystem"
#define MAX_NAME 1024

#define MAXBUF 1000000
#define INF ULONG_MAX
#define ULONG unsigned long 

#define ATTR_RO	 0x01
#define ATTR_HID 0x02
#define ATTR_SYS 0x04
#define ATTR_LBL 0x08
#define ATTR_DIR 0x10
#define ATTR_ARC 0x20
#define ATTR_LOG 0x0f
/* value greater or equal than this value marks the end */
#define END_MARK 0x0FFFFFF8
/* use this value to indicate end of FAT chain */
#define EOD      0x0FFFFFFF

struct BootEntry be;
struct FSInfo fsinfo;
int fd;			// device file descriptor
int bps;		// bytes per sector
int spc;		// sectors per cluster
ULONG fat_addr,fat_secNum,data_secNum,rootDir_secNum,rootDir_clusterNum;
ULONG *FAT; 
int dps; /* dir entry per sector */
unsigned char buffer[MAXBUF];

/*****************************************************************************/

/* Boot Record Entry,size=512B */
#pragma pack(push,1)	/* BYTE align in memory (no padding) */
typedef struct BootEntry{
	unsigned char BS_jmpBoot[3];	/* Assembly instruction to jump to boot code */
	unsigned char BS_OEMName[8]; 	/* OEM Name in ASCII */
	unsigned short BPB_BytsPerSec; 	/* Bytes per sector. Allowed values include 512,1024, 2048, and 4096 */
	unsigned char BPB_SecPerClus; 	/* Sectors per cluster (data unit). Allowed values are powers of 2, but the cluster size must be 32KB  or smaller */
	unsigned short BPB_RsvdSecCnt;	/* Size in sectors of the reserved area */
	unsigned char BPB_NumFATs; 	/* Number of FATs */
	unsigned short BPB_RootEntCnt;	/* Maximum number of files in the root directory for  FAT12 and FAT16. This is 0 for FAT32 */
	unsigned short BPB_TotSec16; 	/* 16-bit value of number of sectors in file system */
	unsigned char BPB_Media; 	/* Media type */
	unsigned short BPB_FATSz16;	/* 16-bit size in sectors of each FAT for FAT12 and  FAT16. For FAT32, this field is 0 */
	unsigned short BPB_SecPerTrk; 	/* Sectors per track of storage device */
	unsigned short BPB_NumHeads;	/* Number of heads in storage device */
	ULONG BPB_HiddSec;	/* Number of sectors before the start of partition */
	ULONG BPB_TotSec32;	/* 32-bit value of number of sectors in file system.  Either this value or the 16-bit value above must be  0 */
	ULONG BPB_FATSz32;	/* 32-bit size in sectors of one FAT */
	unsigned short BPB_ExtFlags;	/* A flag for FAT */
	unsigned short BPB_FSVer;	/* The major and minor version number */
	ULONG BPB_RootClus;	/* Cluster where the root directory can be found */
	unsigned short BPB_FSInfo;	/* Sector where FSINFO structure can be found */
	unsigned short BPB_BkBootSec;	/* Sector where backup copy of boot sector is located */
	unsigned char BPB_Reserved[12];	/* Reserved */
	unsigned char BS_DrvNum;	/* BIOS INT13h drive number */
	unsigned char BS_Reserved1;	/* Not used */
	unsigned char BS_BootSig;	/* Extended boot signature to identify if the next three values are valid */
	ULONG BS_VolID;		/* Volume serial number */	
	unsigned char BS_VolLab[11];	/* Volume label in ASCII. User defines when creating the file system */
	unsigned char BS_FilSysType[8];	/* File system type label in ASCII */ 
}BootEntry;
#pragma pack(pop)			/* BYTE align in memory (no padding)*/

/* DirEntry,size=32B*/
#pragma pack(push,1)			/* BYTE align in memory (no padding)*/
typedef struct DirEntry {
	unsigned char DIR_Name[8];
	unsigned char DIR_Ext[3];
	unsigned char DIR_Attr;
	unsigned char DIR_NTRes;
	unsigned char DIR_CrtTimeTenth;
	unsigned short DIR_CrtTime;
	unsigned short DIR_CrtDate;
	unsigned short DIR_LstAccDate;
	unsigned short DIR_FstClusHI;
	unsigned short DIR_WrtTime;
	unsigned short DIR_WrtDate;
	unsigned short DIR_FstClusLO;
	ULONG DIR_FileSize;
}DirEntry;
#pragma pack(pop)			/* BYTE align in memory (no padding) */

/* FSINFO,size=512B */
#pragma pack(push,1)			// BYTE align in memory (no padding)
typedef struct FSInfo {       
	ULONG FSI_LeadSig;		/* Signature (0x41615252) */
	unsigned char FSI_Reserved1[480];	/* Not used */
	ULONG FSI_StrucSig;		/* Signature (0x61417272) */
	ULONG FSI_Free_Count;		/* Number of free clusters */
	ULONG FSI_Nxt_Free;		/* Next free cluster (set to the last allocated cluster) */
	unsigned char FSI_Reserved2[12];	/* Not used */
	ULONG FSI_TrailSig;		/* Signature (0xAA550000) */
}FSInfo;
#pragma pack(pop)			// BYTE align in memory (no padding)

#endif
