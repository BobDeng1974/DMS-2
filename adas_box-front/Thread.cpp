#include "Thread.h"

static void runThread(Thread* t, struct Data* data) {
	t->runWrapper(data);
}


Thread::Thread() {

}

Thread::~Thread() {

}


void Thread::create(struct Data* data) {
	mThd = std::thread(runThread, this, data);
}


void Thread::start() {
	mStart = true;
}


void Thread::join() {

}

void Thread::runWrapper(struct Data* data) {
	
	run(data);

}
