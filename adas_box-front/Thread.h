#ifndef THREAD_H_
#define THREAD_H_

#include <thread>
#include <mutex>
#include "data.h"
// #include "MvncNetwork.h"

class Thread {

public:
	Thread();
	virtual ~Thread();

	void create(struct Data* data);

	void start();

	void join();

	void runWrapper(struct Data* data);

	virtual int run(struct Data* data) = 0;

protected:

 	std::thread mThd;
	bool mStart;
	// MvncNetwork mNetwork;
};




#endif
