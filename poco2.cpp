#include "Poco/Notification.h"
#include "Poco/NotificationQueue.h"
#include "Poco/ThreadPool.h"
#include "Poco/Runnable.h"
#include "Poco/AutoPtr.h"
#include <Poco/Mutex.h>

#include <iostream>

using Poco::Notification;
using Poco::NotificationQueue;
using Poco::ThreadPool;
using Poco::Runnable;
using Poco::AutoPtr;



/*
 Compile as:

 g++ poco2.cpp -o poco2 -lPocoFoundation


 */

class WorkNotification: public Notification {
public:
	WorkNotification(int data) :
			_data(data) {
	}
	int data() const {
		return _data;
	}
private:
	int _data;
};

Poco::Mutex m_mutex;

class Worker: public Runnable {
public:
	Worker(NotificationQueue& queue) :
			_queue(queue) {
	}
	void run() {
		AutoPtr<Notification> pNf(_queue.waitDequeueNotification());
		while (pNf) {
			WorkNotification* pWorkNf =
					dynamic_cast<WorkNotification*>(pNf.get());
			if (pWorkNf) {
				// do some work
				m_mutex.lock();
				std::cout << "Worker: " << pWorkNf->data() << std::endl;
				std::cout << std::flush;
				m_mutex.unlock();
			}
			pNf = _queue.waitDequeueNotification();
		}
	}
private:
	NotificationQueue& _queue;
};

int main_2(int argc, char** argv) {
	NotificationQueue queue;
	Worker worker1(queue);		 // create worker threads
	Worker worker2(queue);
	// start workers
	ThreadPool::defaultPool().start(worker1);
	ThreadPool::defaultPool().start(worker2);
	// create some work
	for (int i = 0; i < 100; ++i) {
		queue.enqueueNotification(new WorkNotification(i));
	}
	while (!queue.empty())		 // wait until all work is done
		Poco::Thread::sleep(100);
	queue.wakeUpAll();			 // tell workers they're done
	ThreadPool::defaultPool().joinAll();
	return 0;
}
