// This file provide dummy function and definition to bypass log. 

#ifndef __SYSLOG_H__
#define __SYSLOG_H__

/* void openlog(const char *ident, int option, int facility); */
#define openlog( i, o, f )

/* void syslog(int priority, const char *format, ...); */
#define syslog( p, f, ... )

/* void closelog(void); */
#define closelog()


/* log facility (IDs don't match linux's ones) */
#define LOG_DAEMON	(1<<1)
#define LOG_PID		(1<<2)
#define LOG_ERR		(1<<3)
#define LOG_DEBUG	(1<<4)
#define LOG_INFO	(1<<6)
#define LOG_WARNING	(1<<7)
#define LOG_CRIT	(1<<8)


#endif // __SYSLOG_H__

