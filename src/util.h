#ifndef __UTIL_H__
#define __UTIL_H__
/* util.c */
int strtrimcpy(char *dest, const char *src, int n);
void report_exit(char *msg);
ULONG read_sectors(ULONG sector_number, unsigned char *buffer, ULONG num_sectors);
ULONG write_sectors(ULONG sector_number, unsigned char *buffer, ULONG num_sectors);
ULONG write_cluster(ULONG cluster_number, unsigned char *buffer, ULONG size);
ULONG sector2cluster(ULONG sector_number);
ULONG cluster2sector(ULONG cluster_number);
void splitNameExt(char *name, char *ext, const char *basename);
ULONG getDirClusterNum(struct DirEntry *entry);
void setDirClusHiLo(struct DirEntry *entry, ULONG num);
int setDirEntryFileName(struct DirEntry *entry, char *basename);
ULONG getClusterOffset(ULONG cluster);
ULONG getLastCluster(ULONG startCluster);
ULONG getClusterChainSize(ULONG startNum);
int getDirBaseName(char *dirname, char *basename, char *filename);
#endif 
