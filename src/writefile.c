#include "../inc/global.h"

int main(int argc,char* argv[]){
	int fd;
	if((fd = open(argv[2], O_RDWR | O_CREAT ,0666))==-1){ 
 		 perror("open");
 		 return 1;
	}
	dup2(fd,STDOUT_FILENO); 
	dup2(fd,STDERR_FILENO); 
	close(fd);                       //write ls into texfil.txt

	execl("/bin/ls","ls",argv[1],NULL);
}
