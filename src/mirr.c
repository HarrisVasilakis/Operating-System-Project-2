#include "../inc/global.h"
#include "../inc/mirr.h"

//The fixed size of the event buffer:
#define EVENT_SIZE  ( sizeof (struct inotify_event) )
//The size of the read buffer: estimate 1024 events with 16 bytes per name over and above the fixed size above
#define EVENT_BUF_LEN     ( 1024 * ( EVENT_SIZE + 16 ) )

int main(int argc, char const *argv[])
{
	pid_t childpid;
	FILE* f;
	int status,i,maxnumoffiles,j;
	struct stat buf;
	char x[25],y[25],z[50],z2[50];
	//2 arrays to hold inode info
	struct inodestruct **inodearray;
	struct inodestruct **inodearray2;
    
	    char systemString[100];
	char sampleFolderName[50];
        char backupFolderName[50];
	struct node *root,*backuproot;

	maxnumoffiles=100;

	//z is the source folder and z2 is the backup folder
	strcpy(z,"../");
	strcpy(z2,"../");
	strcat(z,argv[1]);
	strcat(z2,argv[2]);
	char *tempmkdir = (char*)malloc(100*sizeof(char));
	strcpy(tempmkdir, "mkdir ");
	strcat(tempmkdir, z2);
	printf("%s\n",tempmkdir);
	//creating backup folder, in case it doesn't exist
	system(tempmkdir);
	status=remove("texfil.txt");
	stat(z,&buf);
	inodearray=(struct inodestruct**)malloc(maxnumoffiles*sizeof(struct inodestruct *));
	
	inodearray[0]=(struct inodestruct*)malloc(sizeof(struct inodestruct ));
	inodearray[0]->inod=buf.st_ino;
	strcpy(inodearray[0]->datemodified,ctime(&buf.st_atime));
	inodearray[0]->filesize=buf.st_size;
	inodearray[0]->pointingnames=0;
	root = createNode(NULL,z, 0);
	//Creating tree for source directory
	root=maketree(z,root,inodearray,0,maxnumoffiles);
	printTreeNodeData(root);
	status=remove("texfil.txt");
	stat(z2,&buf);
	inodearray2=(struct inodestruct**)malloc(maxnumoffiles*sizeof(struct inodestruct *));
	for(j=0;j<100;j++){
		inodearray2[j]=NULL;
	}
	inodearray2[0]=(struct inodestruct*)malloc(sizeof(struct inodestruct ));
	inodearray2[0]->inod=buf.st_ino;
	strcpy(inodearray2[0]->datemodified,ctime(&buf.st_atime));
	inodearray2[0]->filesize=buf.st_size;
	inodearray2[0]->pointingnames=0;
	backuproot = createNode(NULL,z2, 0);

	//Creating tree for backup directory
	backuproot=maketree(z2,backuproot,inodearray2,0,maxnumoffiles);
	printTreeNodeData(backuproot);
	backuproot = synch(root,inodearray,backuproot,inodearray2,root,backuproot);
	printTreeNodeData(backuproot);

	//Inotify starts here
	int fd = inotify_init();
	//addWatchToWholeTree(root,fd);
	int retVal=inotify_add_watch(fd,z,IN_ALL_EVENTS);
	if(retVal==-1)
		printf("Failed to add watch %s \n",z);
	else
		printf("watching %s \n",z);
	//length of events and read buffer
	int length, read_ptr, read_offset; 

	//the buffer to use for reading the events
	char buffer[EVENT_BUF_LEN];	
	char *fileName = (char *)malloc(100*sizeof(char));

	read_offset = 0; //remaining number of bytes from previous read
	int modified = 0;
	
	//rootFolder is the backup directory relative to the project home and startRootFolder is the source directory
	char *rootFolder = (char*)malloc(100*sizeof(char));
	char *startRootFolder = (char*)malloc(100*sizeof(char));
	
	//Variables used to create system commands for creating, deleting, etc...
	char *newLocation = (char*)malloc(100*sizeof(char));
	char *delete = (char*)malloc(100*sizeof(char));
	char *dateModified = (char*)malloc(100*sizeof(char));
	char *dateModifiedCommand = (char*)malloc(100*sizeof(char));
	char *mkdirCommand = (char*)malloc(100*sizeof(char));
	char *mvCommand = (char*)malloc(100*sizeof(char));

	//Cookie is the variable which holds the previous event cookie number (used in move in/out)
	uint32_t movedOutCookie = 0;
	strcat(z,"/");
	strcat(z2,"/");
	while (1) {
		
		/* read next series of events */
		length = read(fd, buffer + read_offset, sizeof(buffer) - read_offset);

		if (length < 0)
			fail("read");
		length += read_offset; // if there was an offset, add it to the number of bytes to process
		read_ptr = 0;
		
		// process each event
		// make sure at least the fixed part of the event in included in the buffer
		while (read_ptr + EVENT_SIZE <= length ) {
			
			//point event to beginning of fixed part of next inotify_event structure
			struct inotify_event *event = (struct inotify_event *) &buffer[ read_ptr ];

			// if however the dynamic part exceeds the buffer,
			// that means that we cannot fully read all event data
			if( read_ptr + EVENT_SIZE + event->len > length )
				break;
			//event is fully received, process

			fileName = target_name(event);
			strcpy(rootFolder, z2);
			strcpy(startRootFolder,z);
			strcat(rootFolder, fileName);
			strcat(startRootFolder, fileName);

			//If current event is not IN_MOVED_TO and last event was IN_MOVE_FROM
			if(event->cookie == 0 && movedOutCookie != 0){	
				//Unlink the moved file as it is out from the monitored directory		
				stat(rootFolder, &buf);
				struct nodetree *tempTree = inodesearch(backuproot, inodearray2, buf.st_ino);
				if(inodearray2[tempTree->inod]->pointingnames!=0){
					//Function used to unlink files
					searchForLinkedFiles(backuproot, inodearray2, buf.st_ino);
				}
			}
			else if(event->cookie != movedOutCookie)	//else a different command is input, so make the cookie 0
				movedOutCookie = 0;

			if (event->mask & IN_ACCESS)
				printf("access\n");
			else if (event->mask & IN_ATTRIB){
				printf("attrib\n");
				stat(startRootFolder, &buf);
				//If the trigger for the current event is a file, then touch the backup to update the date modified
				if ((buf.st_mode & S_IFMT) == S_IFREG){
					struct nodetree *tempOriginal = inodesearch(root, inodearray, buf.st_ino);
					//Update both trees with the new date modified
					sprintf (dateModified, "%s", inodearray[tempOriginal->inod]->datemodified);
					
					stat(rootFolder, &buf);		
					struct nodetree *tempTree = inodesearch(backuproot, inodearray2, buf.st_ino);
					//Update tree
					sprintf (inodearray2[tempTree->inod]->datemodified, "%s" ,dateModified);
					//Do the change through the touch command
					strcpy(dateModifiedCommand,"touch ");
					strcat(dateModifiedCommand, rootFolder);
					system(dateModifiedCommand);
				}		
			}
			else if (event->mask & IN_CLOSE_WRITE){
				printf("close write\n");
				int len = strlen(fileName);
				const char *last_four = &fileName[len-4];				
				//if file is not a swp file copy it to the backup folder
				if(strcmp(last_four,".swp")){
					stat(rootFolder, &buf);

					strcpy(newLocation, "cp ");
					strcat(newLocation, z);
					strcat(newLocation, fileName);
					strcat(newLocation, " ");
					strcat(newLocation, rootFolder);
					system(newLocation);

					struct nodetree *tempTree;
					tempTree = inodesearch(backuproot, inodearray2, buf.st_ino);
					stat(rootFolder, &buf);
					strcpy(inodearray2[tempTree->inod]->datemodified,ctime(&buf.st_atime));
					modified = 0;
				}
			}
			else if (event->mask & IN_CLOSE_NOWRITE ){printf("in close no write\n");}
			else if (event->mask & IN_OPEN  ){printf("in open\n");}
			else if (event->mask & IN_CREATE){
				printf("in create\n");
				int len = strlen(fileName);
				const char *last_four = &fileName[len-4];
				printf("LAST FOUR %s\n", last_four);
				//if file is not a swp file, allocate space for the new file and add it to the source tree
				if(strcmp(last_four,".swp")){
					stat(startRootFolder, &buf);
					if ((buf.st_mode & S_IFMT) == S_IFREG){
						//File
						struct node* tempNow;
						struct nodetree *tempOriginal = inodesearch(root, inodearray, buf.st_ino);
						if(tempOriginal==NULL){
							for (int i = 0; i < 100; i++) {
								if(inodearray[i]==NULL){
									inodearray[i]=(struct inodestruct*)malloc(sizeof(struct inodestruct ));    //create nodestruct
									inodearray[i]->inod=buf.st_ino;
									strcpy(inodearray[i]->datemodified,ctime(&buf.st_atime));
									inodearray[i]->filesize=buf.st_size;
									inodearray[i]->pointingnames=0;
									tempNow=insertRoot(root,startRootFolder,i);                    //create tree leaf
									break;
								}
							}
						}
						else{
							inodearray[tempOriginal->inod]->pointingnames++;
							insertRoot(root,startRootFolder,tempOriginal->inod);
						}
						//do the same for backup, allocate space for the new file and add it to the source tree
						//but also open the new file in the backup directory
						struct nodetree *temp = inodesearch(backuproot, inodearray2, buf.st_ino);
						if(temp==NULL){
							open(rootFolder, O_CREAT | O_RDWR ,0666);
							stat(rootFolder, &buf);
							for (int i = 0; i < 100; i++) {
								if(inodearray2[i]==NULL){
									inodearray2[i]=(struct inodestruct*)malloc(sizeof(struct inodestruct ));    //create nodestruct
									inodearray2[i]->inod=buf.st_ino;
									strcpy(inodearray2[i]->datemodified,ctime(&buf.st_atime));
									inodearray2[i]->filesize=buf.st_size;
									inodearray2[i]->pointingnames=0; 
									inodearray[tempNow->data->inod]->target=inodearray2[i];
									insertRoot(backuproot, rootFolder, i);                    //create tree leaf
									break;
								}
							}
						}
						else{
							inodearray2[temp->inod]->pointingnames++;
							insertRoot(backuproot,rootFolder,temp->inod);
							link(rootFolder,temp->name);
						}
					}
					else{
					//same process but for directories
					for(i=0; i<100; i++){
						if(inodearray[i]==NULL){
						inodearray[i]=(struct inodestruct*)malloc(sizeof(struct inodestruct ));
						inodearray[i]->inod=buf.st_ino;
						strcpy(inodearray[i]->datemodified,ctime(&buf.st_atime));
						inodearray[i]->filesize=buf.st_size;
						inodearray[i]->pointingnames=0;
						inodearray[i]->target=NULL;
						struct leafnode* killme=root->nextfolder;
						if(root->nextfolder!=NULL) {
							while(killme->next!=NULL){
							killme=killme->next;
							}
							killme->next=insertFolder(root,startRootFolder,i);
						}
						else{
							root->nextfolder=insertFolder(root,startRootFolder,i);
						}  
						break;
						}
					}

					for(i=0; i<100; i++){
						if(inodearray2[i]==NULL){
						inodearray2[i]=(struct inodestruct*)malloc(sizeof(struct inodestruct ));
						inodearray2[i]->inod=buf.st_ino;
						strcpy(inodearray2[i]->datemodified,ctime(&buf.st_atime));
						inodearray2[i]->filesize=buf.st_size;
						inodearray2[i]->pointingnames=0;
						inodearray2[i]->target=NULL;
						struct leafnode* killme=backuproot->nextfolder;
						if(backuproot->nextfolder!=NULL) {
							while(killme->next!=NULL){
							killme=killme->next;
							}
							killme->next=insertFolder(backuproot,rootFolder,i);
						}
						else{
							backuproot->nextfolder=insertFolder(backuproot,rootFolder,i);
						}	
						break;
						}
					}
					//create the directory through the system command
					strcpy(mkdirCommand,"mkdir ");
					strcat(mkdirCommand, rootFolder);
					system(mkdirCommand);
				}
			}
				}
				
			else if (event->mask & IN_DELETE){
				//if file is not a swp file, unlink it from every other file in the trees and remove it from the directories
				printf("in delete\n");
				int len = strlen(fileName);
				const char *last_four = &fileName[len-4];
				printf("LAST FOUR %s\n", last_four);
				if(strcmp(last_four,".swp")){
					stat(startRootFolder, &buf);
					if ((buf.st_mode & S_IFMT) == S_IFREG){					
						stat(rootFolder, &buf);
						struct nodetree *tempTree = inodesearch(backuproot, inodearray2, buf.st_ino);
						if(inodearray2[tempTree->inod]->pointingnames!=0){
								searchForLinkedFiles(backuproot, inodearray2, buf.st_ino);
						}

						//This should work... maybe
						inodearray2[tempTree->inod] = NULL;
						deletefile(backuproot, rootFolder, buf.st_ino);
						strcpy(delete, "rm ");
						strcat(delete, rootFolder);
						system(delete);
					}
				}
			}
			else if (event->mask & IN_DELETE_SELF){
        		struct node* temporary;//=(struct node*)malloc(sizeof(struct node*));
				temporary=foldersearch(root, startRootFolder);
				struct leafnode* treeNode=temporary->nextfolder;
				struct leafnode* treeNodeprev=NULL;
				while(strcmp(startRootFolder,treeNode->leaf->data->name)!=0){					
				  	treeNode=treeNode->next;
				  	treeNodeprev=treeNode;
				}
				if(treeNode!=NULL){
					printf("watch target deleted1\n");
				  if(treeNode->leaf!=NULL){
				    if(treeNode->leaf->nextfolder!=NULL){
				      deletefolder(treeNode->leaf->nextfolder,inodearray);  //delete from last folder
				      treeNode->leaf->nextfolder=NULL;
				    }
				    struct nodetree *temp2 ,*temp21;
				    while(treeNode->leaf!=NULL){
				      i=0;
				      temp2 = treeNode->leaf->data;
				      while(temp2->next!=NULL){
					i++;
					temp21=temp2;
					temp2=temp2->next;   //find last file in folder
				      }
				      inodearray[temp2->inod]=NULL;
				      free(temp2);
				      if(i==0){
					treeNode->leaf=NULL;
				      }
				      else{
					temp21->next=NULL;
				      }
				    }
				    free(treeNode->leaf);
				  }
				}
				if(treeNodeprev==NULL){
				  temporary->nextfolder=temporary->nextfolder->next;
				}
				else{
				  treeNodeprev->next=treeNodeprev->next->next;
				}
				inotify_rm_watch(event->wd,fd);
				free(treeNode); 
		
				temporary=foldersearch(backuproot, rootFolder);
				treeNode=temporary->nextfolder;
				treeNodeprev=NULL;
				while(strcmp(rootFolder,treeNode->leaf->data->name)!=0){
				  treeNode=treeNode->next;
				  treeNodeprev=treeNode;
				}
				if(treeNode!=NULL){
					if(treeNode->leaf!=NULL){
						if(treeNode->leaf->nextfolder!=NULL){
						deletefolder(treeNode->leaf->nextfolder,inodearray2);  //delete from last folder
						treeNode->leaf->nextfolder=NULL;
						}
						struct nodetree *temp2 ,*temp21;
						while(treeNode->leaf!=NULL){
						i=0;
						temp2 = treeNode->leaf->data;
						while(temp2->next!=NULL){
						i++;
						temp21=temp2;
						temp2=temp2->next;   //find last file in folder
						}
						inodearray[temp2->inod]=NULL;
							if(i==0){
						rmdir(temp2->name);
						}
						else{
						remove(temp2->name);   //delete it
						}
						free(temp2);
						if(i==0){
						treeNode->leaf=NULL;
						}
						else{
						temp21->next=NULL;
						}
						}
						free(treeNode->leaf);
					}
				}
				if(treeNodeprev==NULL){
				  temporary->nextfolder=temporary->nextfolder->next;
				}
				else{
				  treeNodeprev->next=treeNodeprev->next->next;
				}
				free(treeNode);
      		}
			else if (event->mask & IN_MODIFY){
				//toggle modify value
				printf("modify\n");
				modified = 1;
			}
			else if (event->mask & IN_MOVE_SELF){printf("move self\n");}
			else if (event->mask & IN_MOVED_FROM){
				//make the movedOutCookie variable the event cookie
				printf("moved from\n");
				movedOutCookie = event->cookie;
			}
			else if (event->mask & IN_MOVED_TO){
				printf("moved to\n");	
				//if the last cookie is the same as the new cookie (and non zero) that means file is renamed
				if(event->cookie == movedOutCookie){
					stat(rootFolder, &buf);

					strcpy(mvCommand, "cp ");
					strcat(mvCommand, z);
					strcat(mvCommand, fileName);
					strcat(mvCommand, " ");
					strcat(mvCommand, rootFolder);
					printf("\n Command %s \n ", mvCommand);
					//"cp ../sample/aa ../backupsample/aa"
					system(mvCommand);

					struct nodetree *tempTree;
					tempTree = inodesearch(backuproot, inodearray2, buf.st_ino);

					stat(rootFolder, &buf);

					strcpy(inodearray2[tempTree->inod]->datemodified,ctime(&buf.st_atime));

					modified = 0;
				}
				movedOutCookie = 0;
				printf("moved into\n");
			}
			else
				printf("unknown event\n");
			
			printf("WD:%i %s %s COOKIE=%u\n", event->wd, target_type(event), target_name(event), event->cookie);
			//advance read_ptr to the beginning of the next event
			read_ptr += EVENT_SIZE + event->len;
		}
		//check to see if a partial event remains at the end
		if( read_ptr < length ) {
			//copy the remaining bytes from the end of the buffer to the beginning of it
			memcpy(buffer, buffer + read_ptr, length - read_ptr);
			//and signal the next read to begin immediatelly after them
			read_offset = length - read_ptr;
		} else
			read_offset = 0;

	}
	// typically, for each wd, need to: inotify_rm_watch(fd, wd);

	close(fd);
	status=remove("texfil.txt");
  return 0;
}


struct node*  foldersearch(struct node* treeNode,char* name){   //find file with inode==ino
	struct leafnode* whatevs;
	struct node* temp=NULL;
	whatevs=(struct leafnode*)malloc(sizeof(struct leafnode));
	whatevs=treeNode->nextfolder;
	while(whatevs!=NULL){
		if(strcmp(whatevs->leaf->data->name,name)==0){
			return treeNode;
		}
		temp=foldersearch(whatevs->leaf,name);
		if(temp!=NULL){
			return temp;
		}
		whatevs=whatevs->next;
	}
	return NULL;
}
