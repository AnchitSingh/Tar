#include <stdio.h>
#include<string.h>
#include <dirent.h>
#include<stdlib.h>
#include<unistd.h>
#include<signal.h>
#include<fcntl.h>
#include<assert.h>
#include<math.h>
#include <sys/stat.h>
#include <sys/types.h>

#define FILE_NAME_SIZE 100

char *strrev(char *str){
    char c, *front, *back;

    if(!str || !*str)
        return str;
    for(front=str,back=str+strlen(str)-1;front < back;front++,back--){
        c=*front;*front=*back;*back=c;
    }
    return str;
}

void getfiles(char files[][FILE_NAME_SIZE], char *dir,int *fcount){
	DIR *folder;
	struct dirent *entry;
	folder = opendir(dir);
    while((entry=readdir(folder))){
		if(!(strcmp(entry->d_name,".")==0 || strcmp(entry->d_name,"..")==0)){
			strcpy(files[(*fcount)],entry->d_name);
			(*fcount)++;
		}
    }
    closedir(folder);
	return;
}

void joinStrings(char a[],char b[],char c[]){
	int l1 = 0,l2 = 0;
	while (a[l1] != '\0')
		++l1;
	for (int j = 0; a[j] != '\0'; ++j) {
		c[j] = a[j];
	}
	int k;
	for (k = 0; b[k] != '\0'; ++k) {
		c[k+l1] = b[k];
	}
	c[k+l1]='\0';
	return;
}


off_t getFileSize(char fpath[]){
	int con = open(fpath,O_RDONLY);
    off_t size = lseek(con, 0L, SEEK_END);
    lseek(con,0,SEEK_SET);
	return size;
}

void getMetaData(char *buff,off_t size,char* meta,int *metaPos){
    for(int i =0;i!=size;i++){
        if(buff[i]=='E' && buff[i+1] == 'O' && buff[i+2] == 'M' && buff[i+3]=='\n'){
            *metaPos+=i+4;
            break;
        }
        meta[i] = buff[i];
    }
	return;
}

char* copyString(char s[]){
    int i;
    char* s2;
    s2 = (char*)malloc(1000);
    for (i = 0; s[i] != '\0'; i++) {
		s2[i] = s[i];
    }
    return (char*)s2;
}

void getTarFileAndDir(char *fullPath,char *file, char *dir){
	fullPath = strrev(fullPath);
	int k=0;
	int breakFile = 0;
	for(int i =0;i<strlen(fullPath);i++){
		if(fullPath[i]=='/'){
			breakFile=1;
		}
			
		if(breakFile==0){
			file[i] = fullPath[i];
			k++;
		}else{
			dir[i-k]=fullPath[i];
		}
	}
	file[k]='\0';
	file = strrev(file);
	dir = strrev(dir);
	return;
}


void createDumpDirectory(char *tarFile,char *dirPath,char *dirName){
	tarFile = strrev(tarFile);
	int k=0;
	int breakFile = 0;
	for(int i =0;i<strlen(tarFile);i++){
		if(breakFile==1){
			dirName[i-k] = tarFile[i];
		}else{
			k++;
		}
		if(tarFile[i]=='.'){
			breakFile=1;
		}
	}
	dirName = strrev(dirName);
	char target[1000];
	joinStrings(dirName,"Dump",dirName);
	joinStrings(dirPath,dirName,target);
	mkdir(target,0777);
	return;
}


void createIDumpDirectory(char *dirPath){
	char target[1000];
	joinStrings(dirPath,"IndividualDump",target);
	mkdir(target,0777);
	return;
}

int getFileCount(char *meta){
	int lineCount = 0;
	for(int i=0;i!=strlen(meta);i++){
		if(meta[i]=='\n'){
			lineCount++;
		}
	}
	return lineCount;
}

void getFileNameAndSize(char* meta,char *filename,off_t *fileSize,int *metaPos){
	char line[1000];
	int lb=0;
	int i=0;
	int charCoverd = 0;
	for(i=0;i!=strlen(meta);i++){
		if(meta[i]=='\n'){
			lb++;
			if(lb==*(metaPos)){
				break;
			}
		}
		if(lb == (*(metaPos)-1)){
			line[i-charCoverd]=meta[i];
		}else{
			charCoverd++;
		}
	}
	line[i-charCoverd]='\0';
	*(metaPos)+=1;
	char *p = strtok(line," -> ");
    int varCount=0;
	char bad_filename[1000];
	strcpy(bad_filename,p);
	int bfc=0,l;
	for(l=0;l<strlen(bad_filename);l++){
		if(bad_filename[l]=='\n'){
			bfc+=1;
		}else{
			filename[l-bfc]=bad_filename[l];
		}
	}
	filename[l-bfc]='\0';
	char fs[1000];
    while( p != NULL ) {
        strcpy(fs, p);
        varCount++;
        p = strtok(NULL," -> ");
    }
	*fileSize=atoi(fs);
	return;
}


int main(int argc, char *argv[]){
	char c[] = "-c";
	char d[] = "-d";
	char e[] = "-e";
	char l[] = "-l";
	if(strcmp(argv[1],c)==0){
		char filename[FILE_NAME_SIZE],filepath[1000];
		char files[100][FILE_NAME_SIZE];
		int fcount = 0,tfd;
		strcpy(filename,argv[3]);
		getfiles(files,argv[2],&fcount);
		char dirPath[1000];
		joinStrings(argv[2],"/",dirPath);
		joinStrings(dirPath,filename,filepath);
		tfd = open(filepath, O_WRONLY | O_TRUNC | O_CREAT, 0644);
		for(int i=0;i<fcount;i++){
			char fpath[1000];
			joinStrings(dirPath,files[i],fpath);
			off_t fsize = getFileSize(fpath);
			int nDigits = floor(log10(abs(fsize))) + 1;
			char fsize_string[nDigits+10];
			sprintf(fsize_string, "%ld",fsize);
			write(tfd,files[i],strlen(files[i]));
			write(tfd," -> ",4);
			write(tfd,fsize_string,nDigits);
			write(tfd,"\n",1);
		}
		write(tfd,"EOM\n",4);
		for(int i=0;i<fcount;i++){
			char fpath[1000];
			int fd;
			joinStrings(dirPath,files[i],fpath);
			off_t fsize = getFileSize(fpath);
			char *content = (char*)malloc(fsize);
			fd = open(fpath,O_RDONLY);
			read(fd, content, fsize);
			write(tfd,content,fsize);
		}
	}else if(strcmp(d,argv[1])==0){
		char dirName[1000];
		char dirPath[1000];
		char tarFile[1000];
		getTarFileAndDir(argv[2],tarFile,dirPath);
		createDumpDirectory(tarFile,dirPath,dirName);
		argv[2]=strrev(argv[2]);
		int tfd = open(argv[2],O_RDONLY);
		off_t size = getFileSize(argv[2]);
		char* buff = (char*)malloc(size);
		char* meta = (char*)malloc(size);
		read(tfd, buff, size);
		int metaPos = 0;
		getMetaData(buff,size,meta,&metaPos);
		lseek(tfd,0,SEEK_SET);
		int countFiles = getFileCount(meta);
		int metaFilePos = 1;
		lseek(tfd,metaPos,SEEK_CUR);
		while(countFiles){
			char filename[FILE_NAME_SIZE];
			off_t filesize;
			getFileNameAndSize(meta,filename,&filesize,&metaFilePos);
			char *f1 = (char*)malloc(filesize+100);
			char destFilePath[1000],dummy[1000];
			joinStrings(dirPath,dirName,dummy);
			joinStrings(dummy,"/",dummy);
			joinStrings(dummy,filename,destFilePath);
			int dummy1 = open(destFilePath,O_WRONLY|O_CREAT,0644);
			read(tfd,f1,filesize);
			write(dummy1,f1,filesize);
			countFiles--;
		}
	}else if(strcmp(e,argv[1])==0){
		char dirName[]="IndividualDump";
		char dirPath[1000];
		char tarFile[1000];
		getTarFileAndDir(argv[2],tarFile,dirPath);
		createIDumpDirectory(dirPath);
		argv[2]=strrev(argv[2]);
		int tfd = open(argv[2],O_RDONLY);
		off_t size = getFileSize(argv[2]);
		char* buff = (char*)malloc(size);
		char* meta = (char*)malloc(size);
		read(tfd, buff, size);
		int metaPos = 0;
		getMetaData(buff,size,meta,&metaPos);
		lseek(tfd,0,SEEK_SET);
		int countFiles = getFileCount(meta);
		int metaFilePos = 1;
		lseek(tfd,metaPos,SEEK_CUR);
		while(countFiles){
			char filename[FILE_NAME_SIZE];
			off_t filesize;
			getFileNameAndSize(meta,filename,&filesize,&metaFilePos);
			if(strcmp(filename,argv[3])==0){
				char *f1 = (char*)malloc(filesize+100);
				char destFilePath[1000],dummy[1000];
				joinStrings(dirPath,dirName,dummy);
				joinStrings(dummy,"/",dummy);
				joinStrings(dummy,filename,destFilePath);
				int dummy1 = open(destFilePath,O_WRONLY|O_CREAT,0644);
				read(tfd,f1,filesize);
				write(dummy1,f1,filesize);
				return 0;
			}
			lseek(tfd,filesize,SEEK_CUR);
			countFiles--;
		}
		printf("No such file is present in tar file.");
	}else if(strcmp(l,argv[1])==0){
		char filName[1000];
		char dirPath[1000];
		char tarFile[1000];
		getTarFileAndDir(argv[2],tarFile,dirPath);
		joinStrings(dirPath,"tarStructure",filName);
		argv[2]=strrev(argv[2]);
		int tfd = open(argv[2],O_RDONLY);
		off_t size = getFileSize(argv[2]);
		char* buff = (char*)malloc(size);
		char* meta = (char*)malloc(size);
		read(tfd, buff, size);
		int metaPos = 0;
		getMetaData(buff,size,meta,&metaPos);
		lseek(tfd,0,SEEK_SET);
		int countFiles = getFileCount(meta);
		char countFiles_string[1000];
		sprintf(countFiles_string,"%d",countFiles);
		int metaFilePos = 1;
		lseek(tfd,metaPos,SEEK_CUR);
		off_t tarFileSize = getFileSize(argv[2]);
		char tarFileSize_string[1000];
		int dummy1 = open(filName,O_WRONLY | O_TRUNC | O_CREAT,0644);
		// read(meta,f1,filesize);
		sprintf(tarFileSize_string,"%ld",tarFileSize);
		write(dummy1,tarFileSize_string,strlen(tarFileSize_string));
		write(dummy1,"\n",1);
		write(dummy1,countFiles_string,strlen(countFiles_string));
		write(dummy1,"\n",1);
		while(countFiles){
			char filename[FILE_NAME_SIZE];
			off_t filesize;
			getFileNameAndSize(meta,filename,&filesize,&metaFilePos);
			char filesize_string[1000];
			sprintf(filesize_string,"%ld",filesize);
			write(dummy1,filename,strlen(filename));
			write(dummy1," ",1);
			write(dummy1,filesize_string,strlen(filesize_string));
			write(dummy1,"\n",1);
			countFiles--;
		}
	}else{
		printf("Failed to complete extraction operation");
		return -1;
	}
	return 0;
}

