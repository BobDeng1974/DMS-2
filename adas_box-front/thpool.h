#ifndef _THPOOL_
#define _THPOOL_

typedef struct thpool_* threadpool;


// initialize thread pool
threadpool thpool_init(int num_threads);


// add work to the job queue
int thpool_add_work(threadpool, void (*function_p)(void*), void* arg_p);


// wait for all queued jobs to finish
void thpool_wait(threadpool);


// pauses all threads immediately
void thpool_pause(threadpool);


// unpauses all threads if they are paused
void thpool_resume(threadpool);


// destroy the threadpool
void thpool_destroy(threadpool);


// show currently working threads
int thpool_num_threads_working(threadpool);


#endif
