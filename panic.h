#ifndef __H_PANIC_
#define __H_PANIC_

/**
 * stop execution, tries to get stacktrace on debug
 */
void panic(char *msg);

/**
 * @see panic, supports printf-style printing
 */
void panicf(char *msg, ...);

/**
 * stop execution, prints the last error value
 */
void perrpanic(char *msg);

/** 
 * if onpanic is set, panic will call that instead of usual panic logic
 */
extern void (*onpanic)(char *msg);

#endif
