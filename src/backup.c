#include "../inc/mirr.h"
#include "../inc/global.h"

struct node* synch(struct node* root,struct inodestruct **inodearray,struct node* backuproot,struct inodestruct **inodearray2,struct node* rootreal,struct node* backuprootreal){
	struct leafnode* leaf1,*leaf2,*leaf21;   //synchronize trees
	int status,i,j;                          //make backuproot
	struct stat buf;
	char filename[50];
	char filename2[50];
	//size_t sizeof1=sizeof(inodearray)/sizeof(inodearray[0]);
	//size_t sizeof2=sizeof(*inodearray2)/sizeof(inodearray2[0]);
	struct nodetree *temp=(struct nodetree*)malloc(sizeof(struct nodetree));
	if(root!=NULL){
		temp = root->data->next;
	}
	struct nodetree *temp2=NULL;
	struct nodetree *temp21=NULL;
	if(backuproot!=NULL){
		temp2 = backuproot->data->next;
		temp21 = backuproot->data;
	}
	struct nodetree *temp3;
	while(temp!=NULL || temp2!=NULL){  //until i searched every file and directory from every tree
		if(temp==NULL && temp2!=NULL){  //if file in backup exist that doesn't in sample
			status=remove(temp2->name);
			deletefile(backuproot,temp2->name ,temp2->inod );   //delete it
			inodearray2[temp2->inod]=NULL;
			temp2=temp21;
		}
		else{
			if(temp!=NULL && temp2==NULL){       //if file from sample exists but not on backup
				if(inodearray[temp->inod]->target!=NULL){  //if it is linked
					temp3=inodesearch(backuprootreal,inodearray2,inodearray[temp->inod]->target->inod);
					strcpy(filename,backuprootreal->data->name);//transform path for backupsample //filename=../backupsample
					strcpy(filename2,temp->name);             
					for(i=0;i<strlen(rootreal->data->name);i++){
						filename2[i]=' ';        //delete ../sample from path
					}
					strcat(filename,filename2);  //connect the two names
					for(i=0;i<50;i++){
						if(filename[i]==' '){
							for(j=i;j<50;j++){
								if(filename[j]!=' '){
									filename[i]=filename[j];    //replace it with ../backupsample
									filename[j]=' ';
									break;
								}
							}
						}
					}
					for(i=0;i<50;i++){
						if(filename[i]==' '){
							filename[i]='\0';     //delete spaces
						}
					}
					insertRoot(backuproot,filename,temp3->inod);  //into tree
					link(filename,temp3->name);             //create it
					inodearray[temp3->inod]->pointingnames++;
				}
				else{
					strcpy(filename,backuprootreal->data->name);   //create path name
					strcpy(filename2,temp->name);
					for(i=0;i<strlen(rootreal->data->name);i++){
						filename2[i]=' ';
					}
					strcat(filename,filename2);
					for(i=0;i<50;i++){
						if(filename[i]==' '){
							for(j=i;j<50;j++){
								if(filename[j]!=' '){
									filename[i]=filename[j];
									filename[j]=' ';
									break;
								}
							}
						}
					}
					for(i=0;i<50;i++){
						if(filename[i]==' '){
							filename[i]='\0';
						}
					}
					open(filename, O_RDWR | O_CREAT ,0666);  //create file
					stat(filename,&buf);
					for(i=0;i<100;i++){//printf("here%s!\n",inodearray[i]->datemodified);
						if(inodearray2[i]==NULL){                                                       //search for null inodestruct
							inodearray2[i]=(struct inodestruct*)malloc(sizeof(struct inodestruct ));    //create nodestruct
							inodearray2[i]->inod=buf.st_ino;
							strcpy(inodearray2[i]->datemodified,inodearray[temp->inod]->datemodified);
							inodearray2[i]->filesize=inodearray[temp->inod]->filesize;
							inodearray2[i]->pointingnames=0;
							inodearray[temp->inod]->target=inodearray2[i];
							insertRoot(backuproot,filename,i);                     //create tree leaf
							break;
						}
					}
				}
			}
			else if(strcmp(temp->name,temp2->name)==0){  //if they have the same name
				if(strcmp(inodearray[temp->inod]->datemodified,inodearray2[temp2->inod]->datemodified)!=0 || inodearray[temp->inod]->filesize!=inodearray2[temp2->inod]->filesize){      //but different information
					status=remove(temp2->name);    //delete it 
					deletefile(backuproot,temp2->name ,temp2->inod );
					temp2=temp21;
					if(inodearray[temp->inod]->target!=NULL){       //and create new one
						temp3=inodesearch(backuprootreal,inodearray2,inodearray[temp->inod]->target->inod);
						strcpy(filename,backuprootreal->data->name);
						strcpy(filename2,temp->name);
						for(i=0;i<strlen(rootreal->data->name);i++){
							filename2[i]=' ';
						}
						strcat(filename,filename2);
						for(i=0;i<50;i++){
							if(filename[i]==' '){
								for(j=i;j<50;j++){
									if(filename[j]!=' '){
										filename[i]=filename[j];
										filename[j]=' ';
										break;
									}
								}
							}
						}
						for(i=0;i<50;i++){
							if(filename[i]==' '){
								filename[i]='\0';
							}
						}
						insertRoot(backuproot,temp->name,temp3->inod);
						link(filename,temp3->name);
						inodearray[temp3->inod]->pointingnames++;
						temp2=temp2->next;
					}
					else{
						strcpy(filename,backuprootreal->data->name);
						strcpy(filename2,temp->name);
						for(i=0;i<strlen(rootreal->data->name);i++){
							filename2[i]=' ';
						}
						strcat(filename,filename2);
						for(i=0;i<50;i++){
							if(filename[i]==' '){
								for(j=i;j<50;j++){
									if(filename[j]!=' '){
										filename[i]=filename[j];
										filename[j]=' ';
										break;
									}
								}
							}
						}
						for(i=0;i<50;i++){
							if(filename[i]==' '){
								filename[i]='\0';
							}
						}
						open(filename, O_RDWR | O_CREAT ,0666);
						stat(filename,&buf);
						for(i=0;i<100;i++){
							if(inodearray2[i]==NULL){                                                       //search for null inodestruct
								inodearray2[i]=(struct inodestruct*)malloc(sizeof(struct inodestruct ));
								inodearray2[i]->inod=buf.st_ino;
								strcpy(inodearray2[i]->datemodified,inodearray[temp->inod]->datemodified);
								inodearray2[i]->filesize=inodearray[temp->inod]->filesize;
								inodearray2[i]->pointingnames=0;
								inodearray[temp->inod]->target=inodearray2[i];
								insertRoot(backuproot,filename,i);
								break;
							}
						}
						temp2=temp2->next;
					}

				}
				else if(strcmp(inodearray[temp->inod]->datemodified,inodearray2[temp2->inod]->datemodified)==0 || inodearray[temp->inod]->filesize==inodearray2[temp2->inod]->filesize){   //if they have the same information 
					inodearray[temp->inod]->target=inodearray2[temp2->inod];  //just connect them
				}
			}
			else if(strcmp(temp->name,temp2->name)!=0){  //else if they have different names
					status=remove(temp2->name);
					deletefile(backuproot,temp2->name ,temp2->inod );   //delete the backup
					temp2=temp21;
					if(inodearray[temp->inod]->target!=NULL){         //and create a new
						temp3=inodesearch(backuprootreal,inodearray2,inodearray[temp->inod]->target->inod);
						strcpy(filename,backuprootreal->data->name);
						//memmove(filename2,temp->name+strlen(rootreal->data->name),strlen(temp->name)-strlen(rootreal->data->name));
						strcpy(filename2,temp->name);
						for(i=0;i<strlen(rootreal->data->name);i++){
							filename2[i]=' ';
						}
						strcat(filename,filename2);
						for(i=0;i<50;i++){
							if(filename[i]==' '){
								for(j=i;j<50;j++){
								if(filename[j]!=' '){
										filename[i]=filename[j];
										filename[j]=' ';
										break;
									}
								}
							}
						}
						for(i=0;i<50;i++){
							if(filename[i]==' '){
								filename[i]='\0';
							}
						}
						insertRoot(backuproot,filename,temp3->inod);
						link(filename,temp3->name);
						inodearray[temp3->inod]->pointingnames++;
						temp2=temp2->next;
					}
					else{
						strcpy(filename,backuprootreal->data->name);
						strcpy(filename2,temp->name);
						for(i=0;i<strlen(rootreal->data->name);i++){
							filename2[i]=' ';
						}
						strcat(filename,filename2);
						for(i=0;i<50;i++){
							if(filename[i]==' '){
								for(j=i;j<50;j++){
								if(filename[j]!=' '){
										filename[i]=filename[j];
										filename[j]=' ';
										break;
									}
								}
							}
						}
						for(i=0;i<50;i++){
							if(filename[i]==' '){
								filename[i]='\0';
							}
						}
						open(filename, O_RDWR | O_CREAT ,0666);
						stat(filename,&buf);
						for(i=0;i<100;i++){
							if(inodearray2[i]==NULL){                                                       //search for null inodestruct
								inodearray2[i]=(struct inodestruct*)malloc(sizeof(struct inodestruct ));
								inodearray2[i]->inod=buf.st_ino;
								strcpy(inodearray2[i]->datemodified,inodearray[temp->inod]->datemodified);
								inodearray2[i]->filesize=inodearray[temp->inod]->filesize;
								inodearray2[i]->pointingnames=0;
								inodearray[temp->inod]->target=inodearray2[i];
								insertRoot(backuproot,filename,i);
								break;
							}
						}
						temp2=temp2->next;
					}		
			}
		}
		if(temp!=NULL){      //until everything = NULL
			temp = temp->next;
		}
		if(temp2!=NULL){
			temp2 = temp2->next;
			temp21 = temp21->next;  //keep previous node
		}
	}
	leaf1=NULL;
	leaf2=NULL;
	leaf1=malloc(sizeof(struct leafnode));
	leaf2=malloc(sizeof(struct leafnode));

	leaf1=root->nextfolder;
	leaf2=backuproot->nextfolder;
	leaf21=leaf2;
	while(leaf1!=NULL || leaf2!=NULL){
		if(leaf1!=NULL && leaf2!=NULL){
			if(strcmp(leaf1->leaf->data->name,leaf2->leaf->data->name)!=0){ //if folders have different names
				deletefolder(leaf2,inodearray2);
				backuproot->nextfolder=NULL;   //delete everything
				leaf2=backuproot->nextfolder;
			}
		}                                    //and create new
		if(leaf1!=NULL && leaf2==NULL){     //if backup folder doesn't exist
			strcpy(filename,backuprootreal->data->name);  //create backup path name for folder the same as for files
			strcpy(filename2,leaf1->leaf->data->name);
				
			
			for(i=0;i<strlen(rootreal->data->name);i++){
				filename2[i]=' ';
			}
			strcat(filename,filename2);
			for(i=0;i<50;i++){
				if(filename[i]==' '){
					for(j=i;j<50;j++){
					if(filename[j]!=' '){
							filename[i]=filename[j];
							filename[j]=' ';
							break;
						}
					}
				}
			}
			for(i=0;i<50;i++){
				if(filename[i]==' '){
					filename[i]='\0';
				}
			}
			mkdir(filename,0777);        //make directory
			stat(filename,&buf);
			for(i=0;i<100;i++){
				if(inodearray2[i]==NULL){                                                       //search for null inodestruct
					inodearray2[i]=(struct inodestruct*)malloc(sizeof(struct inodestruct ));
					inodearray2[i]->inod=buf.st_ino;
					strcpy(inodearray2[i]->datemodified,ctime(&buf.st_atime));
					inodearray2[i]->filesize=inodearray[leaf1->leaf->data->inod]->filesize;
					inodearray2[i]->pointingnames=0;
					inodearray[leaf1->leaf->data->inod]->target=inodearray2[i];
					struct leafnode* killme=backuproot->nextfolder;
					if(backuproot->nextfolder!=NULL) {
						while(killme->next!=NULL){
							killme=killme->next;
						}
						killme->next=insertFolder(backuproot,filename,i);
						leaf2=killme->next;
					}
					else{
						backuproot->nextfolder=insertFolder(backuproot,filename,i);
						leaf2=backuproot->nextfolder;
					}	
							break;
				}
			}
			
			leaf2->leaf=synch(leaf1->leaf,inodearray,leaf2->leaf,inodearray2,rootreal,backuprootreal);  //for next folders
		}
		else if(leaf1==NULL && leaf2!=NULL){//if it exists in backup but not in sample
			deletefolder(leaf2,inodearray2);   //delete everything from that and further
			leaf21->next=NULL;
		}
		if(leaf1!=NULL)
			leaf1=leaf1->next;	
		if(leaf2!=NULL){
			leaf21=leaf2;
			leaf2=leaf2->next;
		}
	}
	return backuproot;
}
