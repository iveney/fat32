#include "util.h"

/* copy char src to dest, right trimming the space */
int strtrimcpy(char * dest,const char * src,int n){
	int len=0;
	while(src[len] != ' ' && len<n && (dest[len] = src[len++] ));
	dest[len]=0;
	return len;
}

/* A utility function: report error msg and exit program */
void report_exit(char * msg) {
	printf("%s\n",msg);	
	exit(1);
}

/* go to the sector indicated by "sector_number", 
 * then read n contiguous sectors (n = num_sectors), 
 * and write the result into "buffer" 
 * Remember to clear the buffer first */
ULONG read_sectors(ULONG sector_number, unsigned char *buffer, ULONG num_sectors) {
	ULONG dest, len;     /* used for error checking only */
	ULONG currpos = lseek(fd,0,SEEK_CUR);
	dest = lseek(fd, sector_number*bps, SEEK_SET);
	if(dest != sector_number*bps) {
		printf("Error in reading sector: \ndest=%lu sector_number*bps=%lu\n",dest,sector_number*bps);
	}
	if(bps*num_sectors > MAXBUF) {
		printf("Buffer size too small\n");
	}
	len = read(fd, buffer, bps*num_sectors);
	if(len != bps*num_sectors) {
		printf("Error in reading sector: len=%lu bps*num_sectors=%lu\n",len,bps*num_sectors);
	}
	lseek(fd,currpos,SEEK_SET);
	return len;
}

/* write contiguous "num_sectors" sectors to a given sector number,
 * the contents are stored in buffer
 */
ULONG write_sectors(ULONG sector_number, unsigned char *buffer, ULONG num_sectors) {
	ULONG currpos = lseek(fd,0,SEEK_CUR);
	ULONG dest, len;
	dest = lseek(fd, sector_number*bps, SEEK_SET);
	if (dest != sector_number*bps) {
		printf("Error in writing sector: \ndest=%lu sector_number*bps=%lu\n",dest,sector_number*bps);
	}
	if(bps*num_sectors > MAXBUF) {
		printf("Buffer size too small\n");
	}
	len = write(fd, buffer, bps*num_sectors);
	if(len != bps*num_sectors) {
		printf("Error in reading sector: len=%lu bps*num_sectors=%lu\n",len,bps*num_sectors);
	}
	lseek(fd,currpos,SEEK_SET);
	return len;
}

/* write into an existing cluster, size should not exceed cluster size */
ULONG write_cluster(ULONG cluster_number,unsigned char *buffer,ULONG size){
	ULONG dest,len;
	ULONG pos=((cluster_number-2)*spc+data_secNum)*bps;
	ULONG currpos = lseek(fd,0,SEEK_CUR);
	dest = lseek(fd,pos,SEEK_SET);
	if(dest != pos){
		printf("Error in lseek\n");
	}
	len = write(fd,buffer,size);
	lseek(fd,currpos,SEEK_SET);
	return len;
}

/* translate sector number to cluster number */
ULONG sector2cluster(ULONG sector_number){
	return ((sector_number - data_secNum) / spc + 2);
}

/* translate cluster number to sector number */
ULONG cluster2sector(ULONG cluster_number){
	return (cluster_number-2)*spc+data_secNum;
}

/* split basename="what.txt" to name and ext */
void splitNameExt(char * name, char * ext,const char * basename){
	char * dot=strstr(basename,".");
	strncpy(name,basename,dot-basename);
	strncpy(ext,dot+1,3);
}

/* translate cluster number from a DirEntry */
ULONG getDirClusterNum(DirEntry * entry){
	return ((entry->DIR_FstClusHI<<16) + entry->DIR_FstClusLO);
}

/* translate cluster number to Hi Lo and write to a DirEntry */
void setDirClusHiLo(DirEntry * entry,ULONG num){
	entry->DIR_FstClusLO = num & 0x0000FFFF;
	entry->DIR_FstClusHI = num >> 16;
}

/* set the file name of entry to basename */
int setDirEntryFileName(DirEntry * entry,char * basename){
	memset(entry->DIR_Name,' ',8);
	memset(entry->DIR_Ext,' ',3);
	splitNameExt((char*)entry->DIR_Name,(char*)entry->DIR_Ext,basename);
	return 1;
}

/* translate the cluster number to file offset in the filesystem */
ULONG getClusterOffset(ULONG cluster){
	return cluster2sector(cluster)*bps;
}

/* get the last cluster from a cluster chain 
 * return 0 if the input value is smaller than 2
 */
ULONG getLastCluster(ULONG startCluster){
	if( startCluster < 2 ) return 0;
	ULONG last = 0,current=startCluster;
	while(current < END_MARK){
		last =current ;
		current = FAT[current];
	}
	return last;
}

/* count how many clusters it has in a FAT chain 
 * return 0 if input is 0 or 1
 */
ULONG getClusterChainSize(ULONG startNum){
	if( startNum < 2 ) return 0;
	ULONG count = 1;
	while(FAT[startNum++] < END_MARK) ++count;
	return count;
}

/* split a given filename to dirname and basename */
int getDirBaseName(char * dirname,char * basename,char * filename){
	dirname[0]='/';dirname[1]=0;
	basename[0]=0;
	int len_file = strlen(filename);
	char * idx = rindex(filename,'/');
	if(idx==NULL){/* suppose to use relative path */
		strncpy(basename,filename,len_file);
		return 1;
	}
	int len_dir = idx-filename;
	int len_base = len_file-len_dir-1;
	strncpy(dirname,filename,len_dir+1);
	strncpy(basename,idx+1,len_base);
	basename[len_base]=0;
	dirname[len_dir+1]=0;
	return 1;
}

