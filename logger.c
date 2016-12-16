#include "logger.h"

/* Mutex to protect logfile access */
static pthread_mutex_t mutex_logger = PTHREAD_MUTEX_INITIALIZER;

/* 2nd mutex to fill struct, as some fct arent threadsafe */
static pthread_mutex_t mutex_loginfo = PTHREAD_MUTEX_INITIALIZER;

/* Write loginfo into filename *
 * This function is thread safe */
void WriteLog(Loginfo *l, char* filename){
	char *line, *cur;
	int fd, lineMaxSize;

	if(filename==NULL)
	  filename = LOGFILENAME;

	lineMaxSize = ADD_SIZE + TIM_SIZE + 2*PID_SIZE +LIN_SIZE + RET_SIZE + SIZ_SIZE + 6;
	if((line=calloc(sizeof(char), lineMaxSize)) == NULL) {
		perror("calloc() line");
		exit(1);
	}

	/* Copy all information contained in loginfo struct
	 * to output it in logfile */
	strncat(line, l->caddr, ADD_SIZE);
	strcat(line, " ");
	strncat(line, l->time, TIM_SIZE);
	/* Remove '\n' char from
	 * default formatted time string */
	cur = strchr(line, (int)'\n');
	if (cur != NULL) strcpy(cur, " ");
	strncat(line, l->spid, PID_SIZE);
	strcat(line, " ");
	strncat(line, l->thid, PID_SIZE);
	strcat(line, " ");
	strncat(line, l->line, LIN_SIZE);
	strcat(line, " ");
	strncat(line, l->sret, LIN_SIZE);
	strcat(line, " ");
	strncat(line, l->rsize, SIZ_SIZE);
	strcat(line, "\n");
	printf("line : %s", line);

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
	if (write(fd, line, strlen(line)) < 0){
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
}

/* Set client address field in Loginfo *
 * This function is thread safe */
void SetLogAddr(Loginfo *l, const struct in_addr *csin) {
	/* thread unsafe function, calls need to be protected */
	if(pthread_mutex_lock(&mutex_loginfo) < 0) {
		perror("lock mutex_loginfo");
		exit(1);
	}
	/* fill caddr field in Loginfo struct */
#ifdef DEBUG
	printf("Setting address in Loginfo struct ...");
#endif
	strncpy(l->caddr, inet_ntoa(*csin), ADD_SIZE);
#ifdef DEBUG
	printf(" DONE :\nAddress : %s\n", l->caddr);
	fflush(stdout);
#endif
	/* unlock mutex */
	if(pthread_mutex_unlock(&mutex_loginfo) < 0) {
		perror("unlock mutex_loginfo");
		exit(1);
	}
	return;
}

/* Set time field in Loginfo *
 * This function is thread safe */
void SetLogTime(Loginfo *l) {
	time_t t = time(NULL);
	/* thread unsafe function, call needs to be protected */
	if(pthread_mutex_lock(&mutex_loginfo) < 0) {
		perror("lock mutex_loginfo");
		exit(1);
	}
	/* fill time field in Loginfo struct */
#ifdef DEBUG
	printf("Setting time in Loginfo struct ...");
#endif
	strncpy(l->time, ctime(&t), TIM_SIZE);
#ifdef DEBUG
	printf(" DONE :\nTime : %s\n", l->time);
	fflush(stdout);
#endif
	/* unlock mutex */
	if(pthread_mutex_unlock(&mutex_loginfo) < 0) {
		perror("unlock mutex_loginfo");
		exit(1);
	}
}

/* Set server PID field in Loginfo *
 * This function is thread safe */
void SetLogPid(Loginfo *l) {
#ifdef DEBUG
	printf("Setting PID in Loginfo struct ...");
#endif
	snprintf(l->spid, PID_SIZE, "%d", (int) getpid());
#ifdef DEBUG
	printf(" DONE :\nPID : %s\n", l->spid);
	fflush(stdout);
#endif
}

/* Set thread id field in Loginfo *
 * This function is thread safe */
void SetLogTid(Loginfo *l) {
#ifdef DEBUG
	printf("Setting ThID in Loginfo struct ...");
#endif
 	snprintf(l->thid, PID_SIZE, "%u", (unsigned int) pthread_self());
#ifdef DEBUG
	printf(" DONE :\nThID : %s\n", l->thid);
	fflush(stdout);
#endif
}

/* Set line field in Loginfo *
 * This function is thread safe */
void SetLogLine(Loginfo *l, const char* line) {
#ifdef DEBUG
	printf("Setting line in Loginfo struct ...");
#endif
	strncpy(l->line, line, LIN_SIZE);
#ifdef DEBUG
	printf(" DONE :\nLine : %s\n", l->line);
	fflush(stdout);
#endif
}

/* Set return code field in Loginfo *
 * This function is thread safe */
void SetLogSret(Loginfo *l, const unsigned int r) {
#ifdef DEBUG
	printf("Setting sret in Loginfo struct ...");
#endif
	snprintf(l->sret, RET_SIZE, "%u", r);
#ifdef DEBUG
	printf(" DONE :\nLine : %s\n", l->sret);
	fflush(stdout);
#endif
}

/* Set request size field in Loginfo *
 * This function is thread safe */
void SetLogRsize(Loginfo *l, const unsigned int s) {
#ifdef DEBUG
	printf("Setting rsize in Loginfo struct ...");
#endif
	if ( s < ONEKILO) /* < 1ko */
		snprintf(l->rsize, SIZ_SIZE, "%u o", s);
	else if (s < ONEMEGA) /* < 1Mo */
		snprintf(l->rsize, SIZ_SIZE, "%4.2f ko", (float)s/ONEKILO);
	else if (s < ONEGIGA) /* < 1Go */
		snprintf(l->rsize, SIZ_SIZE, "%4.2f Mo", (float)s/ONEMEGA);
	else /* > 1Go */
		snprintf(l->rsize, SIZ_SIZE, "%4.2f Go", (float)s/ONEGIGA);
#ifdef DEBUG
	printf(" DONE :\nRSize : %s\n", l->rsize);
	fflush(stdout);
#endif
}
