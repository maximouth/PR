#include "logger.h"

/* Write loginfo into filename *
 * This function is thread safe */
void WriteLog(Loginfo l, char* filename){
	char *line, *cur;
	int fd, lineMaxSize;

	lineMaxSize = ADD_SIZE + TIM_SIZE + 2*PID_SIZE +LIN_SIZE + RET_SIZE + SIZ_SIZE;
	if((line=calloc(sizeof(char), lineMaxSize)) == NULL) {
		perror("malloc() line");
		exit(1);
	}
	cur = line;
	
	/* write caddr field into the line */
	strncpy(cur, l.caddr, ADD_SIZE);
	/* update writing position */
	cur = strchr(cur, (int)'\0');
	/* appends a space */
	strncpy(cur, " ",1);
	/* update writing position */
	cur += 1;
	strncpy(cur, l.time, TIM_SIZE);
	cur = strchr(cur, (int)'\0');
	strncpy(cur, " ",1);
	cur += 1;
	strncpy(cur, l.spid, PID_SIZE);
	cur = strchr(cur, (int)'\0');
	strncpy(cur, " ",1);
	cur += 1;
	strncpy(cur, l.thid, PID_SIZE);
	cur = strchr(cur, (int)'\0');
	strncpy(cur, " ",1);
	cur += 1;
	strncpy(cur, l.line, LIN_SIZE);
	cur = strchr(cur, (int)'\0');
	strncpy(cur, " ",1);
	cur += 1;
	strncpy(cur, l.sret, RET_SIZE);
	cur = strchr(cur, (int)'\0');
	strncpy(cur, " ",1);
	cur += 1;
	strncpy(cur, l.rsize, SIZ_SIZE);
	cur = strchr(cur, (int)'\0');
	strncpy(cur, "\n\0", 2);

	/* Lock to avoid thread problems */
	if (pthread_mutex_lock (&mutex_logger) < 0) {
		perror("lock mutex_logger");
		exit(1);
	}
	/* open log file */
	if( (fd=open(LOGFILENAME, O_WRONLY|O_CREAT|O_APPEND, 0666)) < 0){
		perror("open() logs");
		exit(1);
	}
	/* write log into log file */
	if (write(fd, line, lineMaxSize) < 0){
		perror("write() logs");
		exit(1);
	}
	/* close log file */
	if(close(fd)<0){
		perror("close() logs");
		exit(1);
	}
	/* unlock mutex */
	if (pthread_mutex_unlock(&mutex_logger) <0) {
		perror("unlock mutex_logger");
		exit(1);
	}

	free(line);
	line = NULL;
	cur = NULL;
}

/* Set client address field in Loginfo *
 * This function is thread safe */
void SetLogAddr(Loginfo l, const struct in_addr *csin) {
	/* thread unsafe function, calls need to be protected */
	if(pthread_mutex_lock(&mutex_loginfo) < 0) {
		perror("lock mutex_loginfo");
		exit(1);
	}
	/* fill caddr field in Loginfo struct */
	strncpy(l.caddr, inet_ntoa(*csin), ADD_SIZE);
	/* unlock mutex */
	if(pthread_mutex_unlock(&mutex_loginfo) < 0) {
		perror("unlock mutex_loginfo");
		exit(1);
	}
}

/* Set time field in Loginfo *
 * This function is thread safe */
void SetLogTime(Loginfo l) {
	time_t t = time(NULL);
	/* thread unsafe function, call needs to be protected */
	if(pthread_mutex_lock(&mutex_loginfo) < 0) {
		perror("lock mutex_loginfo");
		exit(1);
	}
	/* fill time field in Loginfo struct */
	strncpy(l.time, ctime(&t));
	/* unlock mutex */
	if(pthread_mutex_unlock(&mutex_loginfo) < 0) {
		perror("unlock mutex_loginfo");
		exit(1);
	}
}

/* Set server PID field in Loginfo *
 * This function is thread safe */
void SetLogPid(Loginfo l) {
	snprintf(l.spid, PID_SIZE, "%d\0", (int) getpid());
}

/* Set thread id field in Loginfo *
 * This function is thread safe */
void SetLogTid(Loginfo l) {
	snprintf(l.thid, PID_SIZE, "%d\0", (int) pthread_self());
}

/* Set line field in Loginfo *
 * This function is thread safe */
void SetLogLine(Loginfo l, const char* line) {
	strncpy(l.line, line, LIN_SIZE);
}

/* Set return code field in Loginfo *
 * This function is thread safe */
void SetLogSret(Loginfo l, const unsigned int r) {
	snprintf(l.sret, RET_SIZE, "%u\0", r);
}

/* Set request size field in Loginfo *
 * This function is thread safe */
void SetLogRsize(Loginfo l, const unsigned int s) {
	if ( s < ONEKILO) /* < 1ko */
		snprintf(l.rsize, SIZ_SIZE, "%u o\0", s);
	else if (s < ONEMEGA) /* < 1Mo */
		snprintf(l.rsize, SIZ_SIZE, "%4.2f Mo\0", (float)s/ONEKILO);
	else
		snprintf(l.rsize, SIZ_SIZE, "%4.2f Go\0", (float)s/ONEMEGA);
}