#pragma once
#include "common.h"

// Define the possible log levels
#define LOG_DEBUG   0
#define LOG_INFO    1
#define LOG_WARNING 2
#define LOG_ERROR   3
#define LOG_FATAL   4

void set_log_lvl(int);
void toggle_logs(int);
void log(int,char *,int,char *,...);

// Logging macros
#define LOG(severity,format,...) log(severity,__FILE__,__LINE__,format,##__VA_ARGS__);
#define LOG_SHORT(severity,format,...) log(severity,NULL,NULL,format,##__VA_ARGS__);