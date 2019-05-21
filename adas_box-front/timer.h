#ifndef TIMER_H_
#define TIMER_H_

#ifdef __cplusplus
extern "C" {
#endif

/**
 * allocate a timer
 * @return return a timer instance on success, otherwise returning 0 means there is no timer available
 */
void* timerAlloc();

// free a timer returned by TimerAlloc
void timerFree(void* timer);

// start timer
void timerStart(void* timer);

// end timer and return duration in ms
int timerEnd(void* timer);

#ifdef __cplusplus
}
#endif

#endif
