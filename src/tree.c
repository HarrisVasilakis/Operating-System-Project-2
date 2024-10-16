
#include "../inc/mirr.h"
#include "../inc/global.h"


int printTreeNodeData(struct node* treeNode){
	struct leafnode* whatevs;//=(struct leafnode*)malloc(sizeof(struct leafnode));
	if(treeNode!=NULL){
		if(treeNode->data!=NULL){
			struct nodetree *temp = treeNode->data;
			//printf("New node\n");
			while(temp!=NULL){
				printf("Tree node with inod pointer %d has name: %s !!\n" ,
				    temp->inod,
				    temp->name
				);
				temp = temp->next;      //next file
			}
		}
	}
	whatevs=(struct leafnode*)malloc(sizeof(struct leafnode));
	whatevs=treeNode->nextfolder;
	while(whatevs!=NULL){
		printf("New folder\n");
		printTreeNodeData(whatevs->leaf);
		whatevs=whatevs->next;          //next folder
	}
}

int addWatchToWholeTree(struct node* treeNode, int fd){
	struct leafnode* whatevs;//=(struct leafnode*)malloc(sizeof(struct leafnode));
	if(treeNode!=NULL){
		if(treeNode->data!=NULL){
			struct nodetree *temp = treeNode->data;
			while(temp!=NULL){
				int retVal = inotify_add_watch(fd, treeNode->data->name, IN_ALL_EVENTS);
				if(retVal == -1)
					printf("Failed to add watch %s \n", treeNode->data->name);
				else
					printf("watching %s \n", treeNode->data->name);

				temp = temp->next;      //next file
			}
		}
	}
}

int searchForLinkedFiles(struct node* treeNode, struct inodestruct **inodearray, int inod){
	struct leafnode* whatevs;
	if(treeNode!=NULL){
		if(treeNode->data!=NULL){
			struct nodetree *temp = treeNode->data;
			while(temp!=NULL){
				if(inodearray[temp->inod]->inod == inod){
						unlink(temp->name);
				}
				temp = temp->next;      //next file
			}
		}
	}
}


struct node* append(struct node* head_ref, struct node *toAppend)
{
    struct node *last;
	struct nodetree *temp,*temp2;
    last = head_ref;
    //This new node is going to be the last node, so next is NULL
    toAppend->data->next = NULL;
    //If the Linked List is empty, then make the new node as head
    if (head_ref == NULL) {
        head_ref = toAppend;
        return head_ref;
    }
    if(last==NULL){
        printf("last null\n");
    }
    //Else traverse till the last node
	temp=last->data;
    while (temp->next != NULL && strcmp(temp->next->name,toAppend->data->name)<0){
        temp = temp->next;
    }
    //Change the next of last node
	if(temp->next==NULL){
		temp->next = toAppend->data;
		return head_ref;
	}
	else{                                 //put node inside tree in alphabetical order

		temp2=temp->next;
		temp->next = toAppend->data;
		temp->next->next=temp2;
		return head_ref;
	}
}

struct node* createNode(struct node *root,char* name, int inod){
    struct node* newNode = malloc(sizeof(struct node));
    newNode->data = malloc(sizeof(struct nodetree));
    newNode->data->inod = inod;
	newNode->data->name=(char*)malloc(sizeof(name));
    strcpy(newNode->data->name, name);
	newNode->nextfolder=NULL;

    root=append(root, newNode);
    return root;
}


struct leafnode* insertFolder(struct node *root,char* name, int inod){
	struct leafnode* temp=root->nextfolder;
	while(temp!=NULL){
		temp=temp->next;
	}
	temp=(struct leafnode*)malloc(sizeof(struct leafnode));
	temp->next=NULL;
	temp->leaf=NULL;
    temp->leaf = createNode(temp->leaf,name, inod);
    return temp;
}

struct node* insertRoot(struct node *root,char* name, int inod){  //insert file into root
    root = createNode(root,name, inod);
    return root;
}

struct node* maketree(char* a,struct node* root,struct inodestruct **inodearray,int i,int maxnumoffiles){
	pid_t childpid;
	FILE* f;
	int status,j;
	struct stat buf;
	char x[50],y[50],z[50],z2[50];          //make tree from file sample(phgh)
	struct inodestruct **inodearray2;
	struct leafnode* temp;
        char systemString[100];
	char sampleFolderName[50];
        char backupFolderName[50];
	strcpy(z,a);
	strcpy(z2,z);
	childpid=fork();
	if(childpid==0){
		execl("writefile","writefile",z,"texfil.txt",NULL);  //write in texfil.txt every file and directory name
	}
	stat(a,&buf);
	wait(&status);
	f=fopen("texfil.txt", "r");
	while(fgets(x, 25, f)!=NULL){                 //read every name of file
		i++;
		if(i>=maxnumoffiles){
			printf("out of memory %d < %d\n",maxnumoffiles,i);
			maxnumoffiles+=100;
			inodearray=realloc(inodearray,maxnumoffiles*sizeof(struct inodestruct *));
		}
		sprintf(y, "%s/%s" ,a, x);       //make full path name
		strtok(y,"\n");					/*remove \n*/
		int fd;
		fd=open(y,O_RDONLY,0644);
		fstat(fd,&buf);
		if ((buf.st_mode & S_IFMT) == S_IFREG){
			//printf("this is a file.\n");
			for(j=0;j<i;j++){
				if(inodearray[j]->inod==buf.st_ino){  //check if it is linked with another file
					break;
				}
			}
			if(inodearray[j]==NULL){           //if not  // if j<=i
				inodearray[i]=(struct inodestruct*)malloc(sizeof(struct inodestruct ));
				inodearray[i]->inod=buf.st_ino;
				strcpy(inodearray[i]->datemodified,ctime(&buf.st_atime));
				inodearray[i]->filesize=buf.st_size;
				inodearray[i]->pointingnames=0;
				inodearray[i]->target=NULL;   //fix inodearray
				insertRoot(root,y,i);         //insert it in tree
			}
			else{  //if yes
				inodearray[j]->pointingnames++;
				insertRoot(root,y,j);
			}
			//printf("inserted %d\n",i);printTreeNodeData(root);
		}
		else if((buf.st_mode & S_IFMT) == S_IFDIR){
			//printf("this is a directory.\n");
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
				killme->next=insertFolder(root,y,i);
			}
			else{
				root->nextfolder=insertFolder(root,y,i);
			}	
		}
		close(fd);
	}
	fclose(f);
	temp=root->nextfolder;
	while(temp!=NULL){
		status=remove("texfil.txt");  //delete texfil if i don't need it
		temp->leaf=maketree(temp->leaf->data->name,temp->leaf,inodearray,i,maxnumoffiles);  //maketree for next directory
		temp=temp->next;
	}
	return root;
}

struct node* deletefile(struct node *root,char* name, int inod){
	struct nodetree *temp=root->data;
	struct nodetree *temp2;
	while(temp->next!=NULL && (strcmp(temp->next->name,name)!=0 || temp->next->inod!=inod) ){//find file to delete
		temp=temp->next;
	}
	if(temp->next!=NULL){  //delete one and connect the others
		temp2=temp->next;
		temp->next=temp->next->next;
		temp2->next=NULL;
		free(temp2);
	}
	return root;
}

void deletefolder(struct leafnode *treeNode,struct inodestruct **inodearray){
	struct leafnode* whatevs,*temp;        //delete from this folder and further
	int i=1;
	temp=treeNode;
	while(treeNode->next!=NULL){
		i=0;
		while(temp->next!=NULL){
			i++;
			whatevs=temp;
			temp=temp->next;  //go to last folder
		}
		deletefolder(temp,inodearray);
		temp=treeNode->next;
		if(i==0){
			treeNode->next=NULL;
		}
		else{
			whatevs->next=NULL;
		}
	}
	if(treeNode!=NULL){
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
	free(treeNode);
}

struct nodetree * inodesearch(struct node* treeNode,struct inodestruct **inodearray,int ino){   //find file with inode==ino
	struct leafnode* whatevs;
	if(treeNode!=NULL){
		if(treeNode->data!=NULL){
			struct nodetree *temp = treeNode->data;
			while(temp!=NULL){
				if(inodearray[temp->inod]->inod==ino){
					return temp;
				}
				temp = temp->next;
			}
		}
	}
	whatevs=(struct leafnode*)malloc(sizeof(struct leafnode));
	whatevs=treeNode->nextfolder;
	while(whatevs!=NULL){
		inodesearch(whatevs->leaf,inodearray,ino);
		whatevs=whatevs->next;
	}
}
