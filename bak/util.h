/* util.c */
#ifndef __UTIL_H__
#define __UTIL_H__
#include "header.h"
int strtrimcpy(char *dest, const char *src, int n);
void report_exit(char *msg);
unsigned long read_sectors(unsigned long sector_number, unsigned char *buffer, unsigned long num_sectors);
unsigned long write_sectors(unsigned long sector_number, unsigned char *buffer, unsigned long num_sectors);
unsigned long write_cluster(unsigned long cluster_number, unsigned char *buffer, unsigned long size);
unsigned long sector2cluster(unsigned long sector_number);
unsigned long cluster2sector(unsigned long cluster_number);
void splitNameExt(char *name, char *ext, const char *basename);
unsigned long getDirClusterNum(DirEntry *entry);
void setDirClusHiLo(DirEntry *entry, unsigned long num);
int setDirEntryFileName(DirEntry *entry, char *basename);
unsigned long getClusterOffset(unsigned long cluster);
unsigned long getLastCluster(unsigned long startCluster);
unsigned long getClusterChainSize(unsigned long startNum);
int getDirBaseName(char *dirname, char *basename, char *filename);
#endif
