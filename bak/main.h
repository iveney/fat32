/* main.c */
#ifndef __MAIN_H__
#define __MAIN_H__
#include "header.h"
void updateFSINFO(void);
int updateDirEntry(unsigned long addr, DirEntry *entry);
void printFsInfo(void);
int locateFileInDir(char *basename, DirEntry *cwd, DirEntry *value, unsigned long *value_addr);
int locateFileInFS(char *filename, DirEntry *de);
int traverse_availableDirEntry(unsigned char *buf, int size, unsigned long *stopat);
int traverse_outputBuffer(unsigned char *buf, int size, unsigned long *stopat);
int traverse_printDirName(unsigned char *buf, int size, unsigned long *stopat);
int _traverse(DirEntry *de, int (*func)(unsigned char *, int, unsigned long *), unsigned long *value_addr);
int traverse(char *filename, int (*func)(unsigned char *, int, unsigned long *), unsigned long *value_addr);
void outputFile(char *filename);
void listDirectory(char *dirname);
int updateFat(int which);
int updateAllFat(void);
int unallocateFatChain(unsigned long startCluster);
unsigned long allocateCluster(void);
int allocateDirEntry(DirEntry *dirEntry, unsigned long *value_addr);
int writeFile(char* filename,const char * inputname,char mode);
int recoverFile(char *filename);
void init(char * deviceName);
int main(int argc, char *argv[]);
#endif
