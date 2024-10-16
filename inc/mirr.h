#include "global.h"

struct inodenames;
struct inodestruct;
struct nodetree;
struct leafnode;
/*struct nodetree* createtree(struct nodetree *,char*,int);
struct nodetree* insert(struct nodetree *,char*,char *,int );
int searchtree(struct nodetree *,char*,int );
void delete(struct nodetree *,char* );*/



struct inodenames{          //nothing(?)
	char name[50];
	struct inodenames * next;
};

typedef struct inodestruct{  //inodearray
	int inod;
	char datemodified[25];
	int filesize;
	int pointingnames;
	struct inodestruct* target;
}inodeTreeNode;

struct nodetree{
	char *name;
	int inod;          //pointer to inodearray
	struct nodetree *next;  //every next is every file in directory
	//struct leafnode* child;
};

struct leafnode{             /*multiple childs*/
	struct node* leaf;
	struct leafnode* next;  //next directory inside same directory
};

struct node {
    struct nodetree *data;        //first data is the "father" directory
    struct leafnode* nextfolder;
};

struct node* maketree(char* ,struct node*,struct inodestruct **,int,int);
int printTreeNodeData(struct node*);
struct node* append(struct node* , struct node *) ;
struct node* createNode(struct node *,char*, int );
//struct node* insertLeft(struct node *,char*, int  );
struct leafnode* insertFolder(struct node *,char*, int  );
struct node* insertRoot(struct node *,char* , int );
struct node* synch(struct node*,struct inodestruct **,struct node* ,struct inodestruct **,struct node*,struct node*);
struct node* deletefile(struct node *,char* , int );
struct nodetree * inodesearch(struct node*,struct inodestruct **,int);
void deletefolder(struct leafnode *,struct inodestruct **);
int addWatchToWholeTree(struct node* treeNode, int fd);
int searchForLinkedFiles(struct node* treeNode, struct inodestruct **inodearray, int inod);
struct node*  foldersearch(struct node* treeNode,char* name);

