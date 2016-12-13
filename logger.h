#ifndef LOGGER_H
#define LOGGER_H

#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <time.h>
#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

/* Path to logfile */
#define LOGFILENAME "/tmp/http3000038.log"

/* String length of loginfo parameters *
 * some values are purposely over-estimated *
 * to avoid problems */
#define ADD_SIZE 30 /* address max size */
#define TIM_SIZE 60 /* time max size */
#define PID_SIZE 10  /* PID/ThreadID max size */
#define LIN_SIZE 150 /* first line of request max size */
#define RET_SIZE 4  /* return code max size */
#define SIZ_SIZE 20 /* request size max size */

/* Some constants for formatted output */
#define ONEKILO (int)1024 /* size of a ko */
#define ONEMEGA (int)1024*1024 /*size of a Mo */
#define ONEGIGA (int)1024*1024*1024 /*size of a Go */

/* Mutex to protect logfile access */
static pthread_mutex_t mutex_logger = PTHREAD_MUTEX_INITIALIZER;

/* 2nd mutex to fill struct, as some fct arent threadsafe */
static pthread_mutex_t mutex_loginfo = PTHREAD_MUTEX_INITIALIZER;

/* Structure containing all information needed in logs */
typedef struct {
	char caddr[ADD_SIZE]; /* client information */
	char time[TIM_SIZE]; /* time of request */
	char spid[PID_SIZE]; /* server pid */
	char thid[PID_SIZE]; /* thread id */
	char line[LIN_SIZE]; /* first line of request */
	char sret[RET_SIZE]; /* 3digits return code */
	char rsize[SIZ_SIZE]; /* request size */
} Loginfo;

/* Write an entry in logfile *
 * This function is thread safe */
void WriteLog(Loginfo l, char* filename);

/* Functions to set Loginfo struct fields. *
 * Those functions are thread safe */
void SetLogAddr(Loginfo l, const struct in_addr *csin);
void SetLogTime(Loginfo l);
void SetLogPid(Loginfo l);
void SetLogTid(Loginfo l);
void SetLogLine(Loginfo l, const char* line);
void SetLogSret(Loginfo l, const unsigned int r);
void SetLogRsize(Loginfo l, const unsigned int s);

#endif
