#ifndef GLOBAL__H
#define GLOBAL__H

    #include <stdio.h>
    #include <stdlib.h>
    #include <string.h>
    #include <unistd.h>
    #include <sys/ipc.h>
    #include <sys/shm.h>
    #include <sys/sem.h>
    #include <semaphore.h>
    #include <time.h>
    #include <sys/stat.h>
    #include <fcntl.h>
    #include <sys/types.h>
    #include <sys/wait.h>
    #include <dirent.h>
    #include <sys/inotify.h>

    void errCatch(char* errmsg);
    //helper function prototypes
    char * target_type(struct inotify_event *event) ;
    char * target_name(struct inotify_event *event);
    void fail(const char *message) ;

#endif
