#include "common.h"
#include "logging.h"
#include "../communication/serial/serial.h"
#include "../communication/protocol.h"
#include <stdarg.h>
#include <stdlib.h>

// Display names for the log levels
char *log_lvl_names[5] = {"DBG","INF","WRN","ERR","DED"};

int curr_log_lvl = LOG_INFO;
int do_logging = true;

void set_log_lvl(int log_level) {
    curr_log_lvl = log_level;
    return;
}

void toggle_logs(int enabled) {
    do_logging = enabled;
}

int count_tokens(char *format) {
    int argc = 0;

    for(int i=0;i<strlen(format)-1;i++) {
        if(format[i] == '%' && format[i+1] != '%') {
            argc++;
        }
    }

    return argc;
}

// Same format as printf
void log(int log_level,char *file_name,int line_no,char *msg_format,...) {
    // Check if we have to log at all
    if(log_level < curr_log_lvl || !do_logging) {
        return;
    }

    // process the vararg list
    va_list args;
    va_start(args,count_tokens(msg_format));

    // Construct the log message
    int msg_size = vsnprintf(nullptr,0,msg_format,args);
    char *msg_buffer = (char *) memalloc((msg_size+1)*sizeof(char));
    vsprintf(msg_buffer,msg_format,args);

    va_end(args);

    // Construct the log line
    int log_size;
    char *log_buffer;

    if(file_name != 0) {
        char *log_pattern = "[%s] (%s:%d) %s";
        log_size = snprintf(nullptr,0,log_pattern,log_lvl_names[log_level],file_name,line_no,msg_buffer);
        log_buffer = (char *) memalloc((log_size+1)*sizeof(char));
        sprintf(log_buffer,log_pattern,log_lvl_names[log_level],file_name,line_no,msg_buffer);
    } else {
        char *log_pattern = "[%s] %s";
        log_size = snprintf(nullptr,0,log_pattern,log_lvl_names[log_level],msg_buffer);
        log_buffer = (char *) memalloc((log_size+1)*sizeof(char));
        sprintf(log_buffer,log_pattern,log_lvl_names[log_level],msg_buffer);
    }

    if(SP.is_available()) {
        // Do NOT log if we don't have an available serial port or infinite recursion will occur
        pack((byte *) log_buffer,strlen(log_buffer)*sizeof(char),DType::LOG,Cmd::WRITE_LOG,&SP);
    }

    free(msg_buffer);
    free(log_buffer);
    return;
}