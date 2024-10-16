#include "../inc/global.h"

void errCatch(char* errmsg){
	printf("Error: %s\n", errmsg);
}


void fail(const char *message) {
	perror(message);
	exit(1);
}
char * target_type(struct inotify_event *event) {
	if( event->len == 0 )
		return "";
	else
		return event->mask & IN_ISDIR ? "directory" : "file";
}

char * target_name(struct inotify_event *event) {
	return event->len > 0 ? event->name : NULL;
}
