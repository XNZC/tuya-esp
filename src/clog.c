#include "clog.h"
#include <stdio.h>
#include <unistd.h>
#include <syslog.h>

#define NAME "tuya-esp"

void create_log(char* msg, char* err) {
    openlog(NAME, LOG_CONS | LOG_NDELAY, LOG_LOCAL0);
    
    if (err == NULL) {
        syslog(LOG_INFO, "%s\n", msg);
    }
    else {
        syslog(LOG_ERR, "%s\n %s\n", msg, err);
    }
    
    closelog();
}