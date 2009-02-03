#ifndef __MAIN_H__
#define __MAIN_H__
/* main.c */
void printUsage(void);
void updateFSINFO(void);
int updateDirEntry(ULONG addr, DirEntry *entry);
void printFsInfo(void);
int locateFileInDir(char *basename, DirEntry *cwd, DirEntry *value, ULONG *value_addr);
int locateFileInFS(char *filename, DirEntry *de);
int traverse_availableDirEntry(unsigned char *buf, int size, ULONG *stopat);
int traverse_outputBuffer(unsigned char *buf, int size, ULONG *stopat);
int traverse_printDirName(unsigned char *buf, int size, ULONG *stopat);
int _traverse(DirEntry *de, int (*func)(unsigned char *, int, ULONG *), ULONG *value_addr);
int traverse(char *filename, int (*func)(unsigned char *, int, ULONG *), ULONG *value_addr);
void outputFile(char *filename);
void listDirectory(char *dirname);
int updateFat(int which);
int updateAllFat(void);
int unallocateFatChain(ULONG startCluster);
ULONG allocateCluster(void);
int allocateDirEntry(DirEntry *dirEntry, ULONG *value_addr);
int writeFile(char *filename, const char *inputname, char mode);
int recoverFile(char *filename);
void init(char *deviceName);
int removeFile(char *filename);
int main(int argc, char *argv[]);
#endif 
