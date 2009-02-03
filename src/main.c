#include <ctype.h>
#include "header.h"
#include "util.h"
#include "main.h"
/*
 * Three essential physical area in terms of sector
 * 1.Reserved Area( starts from 0, size=BootEntry.BPB_RsvdSecCnt )
 * 2.FAT Area(starts after Area 1, size=BootEntry.BPB_NumFATs * be.BPB_FATsz32)
 * 3.Data Area(stars after Area 2, size=all, equals to Cluster 2)
 */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */ 

/* print usage */
void printUsage(){
	printf("Usage: ./a3 [arguments]\n");
	printf("  -m [1|2|3]   The milestone: should be 1, 2, or 3. Always Required;\n");
	printf("  -d [name]    The device file name. Always Required;\n");
	printf("  -t [name]    The target file name. Required with \"-m 2\" and \"-m 3\";\n");
	printf("  -i [name]    The input filename. Required with \"-m 3\";\n");
	printf("  -w [t|a]     The file writing mode. Reuqired with \"-m 2\":\n");
	printf("                        -w t: truncation mode;\n");
	printf("                        -w a: appendage mode;\n");
	exit(1);
}

/* update FSINFO from the buffer */
void updateFSINFO(){
	ULONG currpos = lseek(fd,0,SEEK_CUR);
	lseek(fd, be.BPB_FSInfo * bps, SEEK_SET);
	if( write(fd,(unsigned char *)&fsinfo,sizeof(FSInfo))==-1)
		report_exit("update fsinfo error!\n");
	lseek(fd,currpos,SEEK_SET);
}

/* update DirEntry in address addr */
int updateDirEntry(ULONG addr,DirEntry * entry){
	/* keep the old position */
	ULONG original = lseek(fd,0,SEEK_CUR);
	int result = lseek(fd,addr,SEEK_SET);
	if( result == -1 || write(fd,entry,sizeof(DirEntry)) == -1 )
		report_exit("update dir entry error!\n");
	/* recover the old position */
	lseek(fd,original,SEEK_SET);
	return 1;
}

/* Print the information in Milestone 1 */
void printFsInfo() {
	ULONG numFatEntry = be.BPB_FATSz32 * bps / 4;
	printf("Number of FATs = %d\n",be.BPB_NumFATs);	
	printf("Size of a FAT(in sectors) =  %lu\n",be.BPB_FATSz32);
	printf("Number of FAT entry = %lu\n",numFatEntry);	
	printf("Number of bytes per sector = %d\n",be.BPB_BytsPerSec);
	printf("Number of sectors per cluster = %d\n",be.BPB_SecPerClus);
	printf("Number of reserved sectors = %d\n",be.BPB_RsvdSecCnt);
	printf("FAT (1st) starts from(sector) = %ld\n", fat_secNum);
	printf("Cluster 2 starts from(sector) = %ld\n",data_secNum);
	printf("Root Dir  starts from(sector) = %ld\n",rootDir_secNum);
}

/* 
 * given a DirEntry cwd and a basename
 * search in cwd to find a DirEntry B with filename=`basename'
 * return 1 if success, value_addr points to the first byte of B 
 * return 0 if failed, value_addr is 0, 
 */
int locateFileInDir(char *basename, DirEntry * cwd,DirEntry * value, 
		ULONG * value_addr){
	char name[9],ext[4],*dot;
	int n = dps,i,j;
	ULONG cwdCluster;
	ULONG base_sector;
	DirEntry * de;
	*value_addr=0;
	/* name and ext padded with spaces */
	memset(name,' ',sizeof(name));name[8]=0;
	memset(ext,' ',sizeof(ext));ext[3]=0;
	
	/* special case:basename is "." or ".." in root directory */
	cwdCluster = getDirClusterNum(cwd);
	if(cwdCluster == rootDir_clusterNum){
		if( strcmp(basename,".")==0 || strcmp(basename,"..")==0){
			setDirClusHiLo(value,rootDir_clusterNum);
			value->DIR_Attr = ATTR_DIR;
			return 1;
		}
	}

	/* split the basename into name and ext */
	if(strcmp(basename,"..") == 0 || strcmp(basename,".") == 0 ||
			(dot=strstr(basename,"."))==NULL)
		strncpy(name,basename,strlen(basename));
	else
		splitNameExt(name,ext,basename);

	int success = 0;
	/* read each clusters in cluster chain */
	do{
		base_sector = data_secNum + spc*(cwdCluster-2);
#ifdef DEBUG
		printf("Cluster : %lu\n",cwdCluster);
		printf("Next    : %lu\n",FAT[cwdCluster]);
#endif 
		/* read sectors in each cluster */
		//for(i=0;i<spc && !success;i++){
			//read_sectors(base_sector+i,buffer,spc);
			read_sectors(base_sector,buffer,spc);
			/* read DirEntry in each sector */
			for(j=0;j<n && !success;j++){
				de = (DirEntry * )(buffer+j*sizeof(DirEntry));
				unsigned char temp = ((unsigned char*)de)[0];
				/* 0x00 or 0xe5 allocated */ 
				if( temp == 0) continue; 
						//|| temp == 0xe5 ) continue;
#ifdef DEBUG
				printf("SRC: [%s].[%s]\n",name,ext);
				printf("DST: [%.8s].[%.3s]\n",de->DIR_Name,de->DIR_Ext);
#endif
				/* search for file 'name.ext',case insensitive */
				if( strncasecmp(name,(const char *)de->DIR_Name,8)==0 &&
						strncasecmp(ext,(const char *)de->DIR_Ext,3)==0){
					success = 1;
					memcpy(value,de,sizeof(DirEntry));
					/* special case : name is '..', point to root directory */
					ULONG cluster = getDirClusterNum(value);
					if( cluster == 0 && strcmp(name,"..      ")==0){
						setDirClusHiLo(value, rootDir_clusterNum);
						value->DIR_Attr = ATTR_DIR;
					}
#ifdef DEBUG
					printf("Found : [%.8s].[%.3s]\n",de->DIR_Name,de->DIR_Ext);
#endif
					//*value_addr = (j*sizeof(DirEntry) + (base_sector+i)*bps);
					*value_addr = (j*sizeof(DirEntry) + base_sector*bps);
					break;
				}
			}
		//}
		cwdCluster=FAT[cwdCluster];
	}while( cwdCluster < END_MARK && ! success);

#ifdef DEBUG
	if( success == 0 ) printf("Not Found [%s].[%s]\n",name,ext);
#endif
	return success;
}

/* given a filename(absolute path), search it in the file system
 * return 1 if found and copy its DirEntry to de
 * else return 0 */
int locateFileInFS(char * filename,DirEntry * de){
	/* special case : filename="/", root directory, simply copies
	 * 'create' an entry for it by set the start cluster of root dir to it.
	 * Also set the ATTR filed as directory */
	if( strcmp(filename,"/") == 0){
		setDirClusHiLo(de, rootDir_clusterNum);
		de->DIR_Attr = ATTR_DIR;
		return 1;
	}

	int success = 0,i=0;
	ULONG addr;
	char buf[MAXBUF];
	char * saveptr,*token,*input=buf;
	strncpy(buf,filename,MAXBUF);
	DirEntry * parent = (DirEntry*) malloc(sizeof(DirEntry));
	DirEntry * next = (DirEntry*) malloc(sizeof(DirEntry));
	/* initialize first searching directory point to root dir */
	setDirClusHiLo( parent, rootDir_clusterNum);
	parent->DIR_Attr = ATTR_DIR;
	do{
		token = strtok_r(input,"/",&saveptr);
		if(token == NULL ) break;
		input = NULL;
#ifdef DEBUG
		printf("Finding: %s, ",token);
		printf("Depth : %d\n",++i);
#endif
		/* First starts from root directory */
		success = locateFileInDir(token,parent,next,&addr);
		memcpy(parent,next,sizeof(DirEntry));
	}while(success);
	if(success) memcpy(de,next,sizeof(DirEntry));
	free(parent); free(next);
	return success;
}

/* traverse the given buffer to find if there is available DirEntry 
 * return 1 if it needs another traversal
 * return 0 if found available one and set stopat to the position in the buffer
 */
int traverse_availableDirEntry(unsigned char * buf,int size,ULONG * stopat){
	DirEntry *de;
	int i,n=size/sizeof(DirEntry);
	for(i=0;i<n;i++){
		de = (DirEntry * )(buf+i*sizeof(DirEntry));
		unsigned char temp = ((unsigned char*)de)[0];
		if( temp == 0 || temp == 0xe5 ){
			*stopat=i*sizeof(DirEntry);
			return 0;
		}
	}

	// return 1 means it should continue traverse
	*stopat=size+1;
	//printf("No available DirEntry in this dir,return 0x%08x\n",*stopat);
	return 1;
}

/* output raw buffer 
 * return 1 means whole buffer traversed
 */
int traverse_outputBuffer(unsigned char * buf,int size,ULONG * stopat){
	/* 16 bytes per row, with ASCII content(0x20-0x7E) */
	int i=0;
	while(size--) printf("%c",buf[i++]);
	return 1;
}

/* print out the filename in buffer, which is a DirEntry */
int filecnt;
int traverse_printDirName(unsigned char * buf,int size,ULONG * stopat){
	static int counter=0;
	DirEntry *de;

	int i,n = size / sizeof(DirEntry);
	for(i=0;i<n;i++){
		de = (DirEntry*) (buf+i*sizeof(DirEntry));
		unsigned char temp;temp = ((unsigned char*)de)[0];
#ifndef DEBUG
		/* not allocated or long file name will not be displayed */
		if( temp == 0 || temp == 0xe5 ||
				(de->DIR_Attr & ATTR_LOG)) 
			;
		else{
			char filename[9],ext[4];
			filename[8]=ext[3]=0;
			int namelen = strtrimcpy(filename,de->DIR_Name,8);
			int extlen = strtrimcpy(ext,de->DIR_Ext,3);
			char fullname[11];
			int len = sprintf(fullname,"%s",filename);
			if( extlen > 0 )
				len += sprintf(fullname+len,".%s",ext);
			if(de->DIR_Attr & ATTR_DIR) sprintf(fullname+len,"/");
			ULONG startCluster = getDirClusterNum(de);
			printf("%-16s%-16d%-16u\n",fullname,de->DIR_FileSize,startCluster);
			filecnt++;
		}
#else
		/* debug output */
		printf("%4d: [%.8s].[%.3s]\n",++counter,de->DIR_Name,de->DIR_Ext);
#endif
	}

	return 1;
}

/*
 * traverse all cluster belonged to the DirEntry de,
 * pass the cluster read to func for processing
 * set a value `value_addr' to indicate the stop address
 */
int _traverse(DirEntry * de,int (*func)(unsigned char *,int,ULONG *),
		ULONG * value_addr) {
	int result=0;
	ULONG base_sector,dir_cluster;
	ULONG nRemain = de->DIR_FileSize;
	dir_cluster = getDirClusterNum(de);

	if( nRemain == 0 ){
		/* for a Directory DirEntry.size= 0 
		 * set to INF to keep the traverse going*/
		if( de->DIR_Attr & ATTR_DIR ) nRemain = INF;
		/* some normal file may have size=0,no allocated cluster for it */
		else return 1;
	}

	/* traverse the FAT chain */
	ULONG stopat,len=0;
	do{
		base_sector = data_secNum + spc*(dir_cluster-2);
		/* read the whole cluster */
		len = read_sectors(base_sector,buffer,spc);
#ifdef DEBUG
		printf("Cluster : %lu\n",dir_cluster);
		printf("Sector : %lu\n",base_sector);
		printf("Read:   %lu\nRemain: %lu\n",len,nRemain);
#endif
		/* pass the buffer content to func for processing 
		 * NOTE: result=1 means keep traversing
		 * result=0 means stop traversing, and stopat point to 
		 * the stop position in buffer
		 * */
		if(nRemain<len) result = func(buffer,nRemain,&stopat);
		else result = func(buffer,len,&stopat);
		if( result == 0 ){
			*value_addr = (cluster2sector(dir_cluster)*bps + stopat);
			break;
		}

		nRemain -= len;
		dir_cluster=FAT[dir_cluster];
	}while( dir_cluster < END_MARK );
	return result;
}

/* a wrapper function for _traverse 
 * locate the DirEntry of `filename' 
 */
int traverse(char * filename,int (*func)(unsigned char *,int,ULONG *),
		ULONG * value_addr) {
#ifdef DEBUG
	printf("** traverse starts **\n");
#endif
	int result=0;
	DirEntry de;
	/* can not find the DirEntry */
	if( locateFileInFS(filename,&de) == 0 ) {
		printf("%s: No such file or directory.\n",filename);
		return result;
	}
	result = _traverse(&de,func,value_addr);
#ifdef DEBUG
	printf("** traverse ends **\n");
#endif
	return result;
}

/* output content of a file */
void outputFile(char * filename){
#ifdef DEBUG
	printf("** Output starts **\n");
#endif
	ULONG stopat;
	traverse(filename,traverse_outputBuffer,&stopat);
#ifdef DEBUG
	printf("\n** Output ends **\n");
#endif
}

/* List the files and dirs in dirname */
int filecnt=0;
void listDirectory(char * dirname){
#ifdef DEBUG
	printf("** List Directory **\n");
#endif
	ULONG stopat;
	filecnt=0;
	printf("FILENAME        FILE SIZE       STARTING CLUSTER #\n");
	printf("--------------------------------------------------\n");
	traverse(dirname,traverse_printDirName,&stopat);
	printf("\nTotal number of entries = %d\n",filecnt);
#ifdef DEBUG
	printf("** List Ends **\n");
#endif
}

/* update an FAT in file system according to FAT buffer */
int updateFat(int which){
	/* since we store the FAT by entries, every entry is 4 Bytes */
	return write_sectors(fat_addr + which*be.BPB_FATSz32,
			(unsigned char *)FAT, be.BPB_FATSz32);
}

/* update ALL FAT(copy content of FAT buffer to all FATs in the filesystem) */
int updateAllFat(){
	int i;
	for(i=0;i<be.BPB_NumFATs;i++) if(updateFat(i) == 0) return 0;
	return 1;
}

/* unallocate a FAT train in FAT buffer: set all value in chain to 0 */
int unallocateFatChain(ULONG startCluster){
	if(startCluster < 2) return 0;
	ULONG q,p=startCluster;
	while( p<END_MARK ){
		q = p;
		p = FAT[p];
		FAT[q]=0;
	}
	return 1;
}

/* allocate a cluster(from FAT table)
 * use next_available algorithm
 * return the allocated cluster number
 * if not enough space, exit program
 * NOTE: sector per cluster should be takein into account
 */
ULONG allocateCluster(){
	/* it seems that it would not update Free_Count */
	ULONG value = fsinfo.FSI_Nxt_Free;
	ULONG i=value;
	/* search for a free cluster
	while(FAT[i]!=0){
		i+=1;
	}
	*/
	/* now i is the cluster to return */
	// ULONG ret = i;
	ULONG numFatEntry = be.BPB_FATSz32 * bps / 4;
	/* clusters are from 0 to numFatEntry, in which 0,1 not used */
	do{/* search for a next free cluster */
		/* 2008.12.19 : allocate cluster from current position */
		i++;
		if(i>numFatEntry) i = 2;

		/* circle,failed */
		if(i==value) {
			report_exit("Not enough space!\n");
			break;
		}

		if(FAT[i] == 0) {
			fsinfo.FSI_Nxt_Free = i;
			break;
		}
	}while(1);
#ifdef DEBUG
	printf("**Allocate : %lu**\n",i);
#endif
	updateFSINFO();
//	return ret;
	return i;
}

/* allocate a DirEntry in the directory `dirEntry' 
 * its address in filesystem is returned in addr
 */
int allocateDirEntry(DirEntry *dirEntry,ULONG * value_addr){
	int result = _traverse(dirEntry,traverse_availableDirEntry,value_addr);
	/* result == 1 means all clusters of dirEntry were searched 
	 * which means we can not found a available entry 
	 * allocate a new cluster to get one
	 */
	if( result == 1 ){
#ifdef DEBUG
		printf("No available Dir Entry in current cluster !\n");
#endif
		/* search the last cluster in the FAT chain
		 * allocate a new cluster and append it to the last cluster
		 */
		ULONG startCluster = getDirClusterNum(dirEntry);
		ULONG last = getLastCluster(startCluster);
		ULONG newCluster = allocateCluster();
		if( newCluster == 0 )
			report_exit("Not enough space!\n");

		FAT[last] = newCluster;
		FAT[newCluster] = EOD;
		updateAllFat();
		/* allocate the first dir entry in the cluster */
		*value_addr = (cluster2sector( newCluster ) * bps);
	}
#ifdef DEBUG
	printf("Allocated dirEntry address: 0x%08x\n",(int)*value_addr);
#endif
	return 1;
}

/* Write content to a new file */
int writeFile(char* filename,const char * inputname,char mode){
#ifdef DEBUG
	printf("** write file **\n");
#endif
	/* get the parent directory name and basename */
	char dirname[80]="/",basename[80];
	if( getDirBaseName(dirname,basename,filename) == 0 )
		report_exit("Invalid filename\n");
	int inputfd = open(inputname,O_RDONLY);
#ifdef DEBUG
	printf("dirname: [%s]\n",dirname);
	printf("basename:[%s]\n",basename);
#endif
	/* Check if its containing directory exist */
	DirEntry dirEntry,baseEntry;
	if( locateFileInFS(dirname,&dirEntry) == 0) return 0;

	/* the address of the baseEntry */
	ULONG addr;
	/* write pointer */
	ULONG writeAddr=0;
	/* bytes per cluster */
	ULONG bCluster = bps*spc;

	/* check if the file exists */
	if( locateFileInDir(basename,&dirEntry,&baseEntry,&addr) == 0 ){
#ifdef DEBUG
		printf("allocate new DirEntry\n");
#endif
		/* allocate a new entry in its containg directory 
		 * or find a new one in another cluster */
		allocateDirEntry(&dirEntry,&addr);
		/* initialize the DirEntry */
		memset(&baseEntry,0,sizeof(DirEntry));
		baseEntry.DIR_FileSize = 0;
		setDirClusHiLo(&baseEntry,0);
		setDirEntryFileName(&baseEntry,basename); 
		baseEntry.DIR_Attr = ATTR_ARC;
		updateDirEntry(addr,&baseEntry);
	}
	else{/* otherwise use the existing entry in 'baseEntry'*/
		if( mode == 't' ){
			/* write mode:unallocate the FAT chain */
			ULONG c_num = getDirClusterNum(&baseEntry);
#ifdef DEBUG
			printf("Unalocate FAT chain,start cluster :%lu\n",c_num);
#endif
			baseEntry.DIR_FileSize = 0;
			setDirClusHiLo(&baseEntry,0);
			updateDirEntry(addr,&baseEntry);
			unallocateFatChain(c_num);
			/* 2008.12.19 : modified according to Dr.WONG 
			 * update FSINFO.nxt_free to current after unallocate*/
			//fsinfo.FSI_Nxt_Free = c_num;
			//updateFSINFO();
		}
		else{
			/* append mode: continue with the FAT chain */
			ULONG fileSize=baseEntry.DIR_FileSize;
			ULONG startCluster=getDirClusterNum(&baseEntry);
			ULONG last = getLastCluster(startCluster);
			ULONG totalCluster = getClusterChainSize(startCluster);
			ULONG left=totalCluster*spc*bps-fileSize;
			ULONG bytesInLast = bCluster - left;
			writeAddr = getClusterOffset(last) + bytesInLast;
		}
	}
	/* update the 1st FAT in fs */
	updateFat(0);

	/* read sth. from stdin,every time a cluster */
	int len,result = 1;
	ULONG startCluster = getDirClusterNum(&baseEntry);
	ULONG fileSize = baseEntry.DIR_FileSize;
	ULONG totalCluster = getClusterChainSize(startCluster);
	ULONG left=totalCluster*spc*bps-fileSize;
	ULONG p=getLastCluster(getDirClusterNum(&baseEntry)),q=0;
#ifdef DEBUG
	printf("fileSize=%lu,totalCluster=%lu,startCluster=%lu,left=%lu\n",
			fileSize,totalCluster,startCluster,left);
#endif
	lseek(fd, writeAddr,  SEEK_SET);
	/* every time read a cluster */
	while( (len=read(inputfd,(void*)buffer,bCluster))!=0){
		fileSize+=len;
		buffer[len]=0;
#ifdef DEBUG
		printf("len=%d buffer:%s\n",len,buffer);
#endif
		if( left >= len ){
#ifdef DEBUG
			printf("Before=%lu, After=%lu\n",left,left-len);
#endif
			write(fd,buffer,len);
			left-=len;
		}
		else{
			/* not enought space, allocate new cluster */
#ifdef DEBUG
			printf("Before=%lu,full!\n",left);
#endif
			/* write the enought part to current cluster */
			write(fd,buffer,left);
			q=p;
			p=allocateCluster();
			/* write the remaining part to new cluster */
			writeAddr = getClusterOffset(p);
			lseek(fd, writeAddr, SEEK_SET);
			write(fd,buffer+left,len-left);
#ifdef DEBUG
			printf("Write %lu to new cluster,left=%lu\n",len-left,bCluster-(len-left));
#endif
			left = bCluster-(len-left);

			/* update the starting cluster if it is first allocated */
			if(q==0) {
				setDirClusHiLo(&baseEntry,p);
#ifdef DEBUG
				printf("File start cluster changed to %lu\n",p);
#endif
			}
			else 
				FAT[q]=p;
			FAT[p]=EOD;

			/* update the first FAT in fs,update all later */
			updateFat(0);
		}
		/* update size field in DirEntry */
		baseEntry.DIR_FileSize = fileSize;
		updateDirEntry(addr,&baseEntry);
	}
	updateAllFat();

#ifdef DEBUG
	printf("** write file end **\n");
#endif
	return result;
}

/* given a absolute path, recover the given file */
int recoverFile(char * filename){
	char dirname[80],basename[20],deletename[20],name[9],ext[4];
	if( getDirBaseName(dirname,basename,filename) == 0 )
		report_exit("Invalid filename.\n");
	memset(name,' ',sizeof(name));name[8]=0;
	memset(ext,' ',sizeof(ext));ext[3]=0;
	splitNameExt(name,ext,basename);
	strcpy(deletename,basename);
	deletename[0]=0xe5;

	/* locate its containg directory */
	DirEntry dirEntry,baseEntry;
	ULONG value_addr;
	if( locateFileInFS(dirname,&dirEntry) == 0) return 0;

	/* search the deleted DirEntry */
	int result = locateFileInDir(deletename,&dirEntry,&baseEntry,&value_addr);

	/* recover the first character */
	baseEntry.DIR_Name[0]=toupper(basename[0]);

	/* update dir entry */
	updateDirEntry(value_addr,&baseEntry);
	
	/* update FAT chain */
	ULONG startCluster = getDirClusterNum(&baseEntry);
	FAT[startCluster] = EOD;
	updateAllFat();

	return result;
}

/* Do some initialize stuff */
void init(char * deviceName){
	/* open FAT32 filesystem */
	if( (fd = open(deviceName,O_RDWR)) == -1 ) 
		report_exit("Open device error!");

	/* read the DOS Boot Record(DBR)=512 Bytes */
	int count; 	
	if( (count=read(fd,buffer,512)) != 512 )
		report_exit("read Boot Sector error!");

	/* check file system signature (0xAA55) */
	if( buffer[510] != 0x55 && buffer[511] != 0xAA )
		report_exit("Not a valid filesystem!");

	/* copy the Boot Sector */
	memcpy(&be,buffer,sizeof(struct BootEntry)); 
	bps = be.BPB_BytsPerSec;
	spc = be.BPB_SecPerClus;
	dps=spc*bps/sizeof(DirEntry);

	/* copy the FSINFO */
	read_sectors(be.BPB_FSInfo,buffer,1);
	memcpy(&fsinfo,buffer,sizeof(struct FSInfo));

	/* calculate the FAT starting address */
	fat_secNum = fat_addr = be.BPB_RsvdSecCnt;

	/* calculate the starting sector of data area, locates just after FAT */
	data_secNum = fat_secNum +  be.BPB_NumFATs * be.BPB_FATSz32;

	/* calculate the cluster and sector of root directory */
	rootDir_clusterNum = be.BPB_RootClus;
	rootDir_secNum = data_secNum + (be.BPB_RootClus-2) * spc;

	/* allocate a temporary FAT for reading and writing */
	FAT = (ULONG*)malloc(be.BPB_FATSz32 * bps);

	/* read contents in FAT to temporary FAT buffer */
	read_sectors(fat_addr,(unsigned char *)FAT, be.BPB_FATSz32);
}

int removeFile(char * filename){
	char dirname[80]="/",basename[80];
	if( getDirBaseName(dirname,basename,filename) == 0 )
		report_exit("Invalid filename\n");

	DirEntry dirEntry,baseEntry;
	if( locateFileInFS(dirname,&dirEntry) == 0) return 0;

	/* the address of the baseEntry */
	ULONG addr;
	/* write pointer */
	/*ULONG writeAddr=0;*/
	/* bytes per cluster */
	/*ULONG bCluster = bps*spc;*/

	/* locate the file*/
	if( locateFileInDir(basename,&dirEntry,&baseEntry,&addr) == 0 ){/* file not exist */
		printf("File not exist!");
	}
	else{
		ULONG c_num = getDirClusterNum(&baseEntry);
		if( c_num != 0 ){
			unallocateFatChain(c_num);
			fsinfo.FSI_Nxt_Free = c_num;
		}
		baseEntry.DIR_Name[0] = 0xe5;
		updateDirEntry(addr,&baseEntry);
		updateFSINFO();
	}
	return 1;
}

/* Program Entry */
int main(int argc,char * argv[]) {
	if( argc < 2 )
		printUsage();
	int opt;
	char job=0,wmode=0;
	char deviceName[MAX_NAME]={};
	char targetName[MAX_NAME]={};
	char inputName[MAX_NAME]={};
	int setDevice=0,setMilestone=0,setTarget=0,setInput=0;
	while( (opt = getopt(argc,argv,"pm:d:t:i:w:")) != -1 )
	{
		switch(opt)
		{
			case 'm':/* milestone [1-3]*/
				job = optarg[0];
				setMilestone=1;
				break;
			case 'p':/* list FAT32 information */	
				break;
			case 'd':/* specify device file name */
				strncpy(deviceName,optarg,MAX_NAME);
				setDevice=1;
				break;
			case 't':/* specify target file name */
				strncpy(targetName,optarg,MAX_NAME);
				setTarget=1;
				break;
			case 'i':
				strncpy(inputName,optarg,MAX_NAME);
				setInput=1;
				break;
			case 'w':
				wmode=optarg[0];
				break;
			case ':':/* Need argument */
				report_exit("Option needs an argument");
				break;
		}
	}
	if( setMilestone == 0 )
		report_exit("Please specify milestone");
	if( setDevice == 0 )
		report_exit("Please specify device name");
	init(deviceName);
	switch(job){
		case '1':/* list root directory */
			printFsInfo();
			listDirectory("/");
			break;
		case '2':
			if( setTarget == 0 )
				report_exit("Please specify target file");
			outputFile(targetName);
			break;
		case '3':
			if( setInput == 0 )
				report_exit("Please specify input file");
			switch(wmode){
				case 't':
					if(!writeFile(targetName,inputName,'t')) printf("write failed.\n");
					break;
				case 'a':
					if(!writeFile(targetName,inputName,'a')) printf("append failed.\n");
					break;
				default:
					report_exit("Writing mode should be 't' or 'a'");
					break;
			}
			break;
		case '4':/* extra function: remove a file */
			if( setTarget == 0 )
				report_exit("Please specify target file");
			removeFile(targetName);
			break;
		default:
			report_exit("Milestone should be 1,2 or 3");
			break;
	}
	close(fd);
	return 0;
}
/* End of Program */
