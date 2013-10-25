#include <Poco/Logger.h>
#include <Poco/PatternFormatter.h>
#include <Poco/FormattingChannel.h>
#include <Poco/ConsoleChannel.h>
#include <Poco/Format.h>
#include <Poco/Notification.h>
#include <Poco/NotificationQueue.h>
#include <Poco/PriorityNotificationQueue.h>
#include <Poco/ThreadPool.h>
#include <Poco/Thread.h>
#include <Poco/Runnable.h>
#include <Poco/Random.h>
#include <Poco/AutoPtr.h>
#include <Poco/NotificationCenter.h>
#include <Poco/Observer.h>

#include <string>
#include <typeinfo>

#include "ScopedLogMessage.h"

const int kNumNotification = 3;
const int kPriorityHighest = 0;
const int kPriorityNormal = 5;
const long kSleepMaxTime = 200;
const long kSleepWaitTime = 500;

class BaseNotification: public Poco::Notification {
public:
	BaseNotification(int data) :
			m_data(data) {
	}

	int data() const {
		return m_data;
	}

private:
	int m_data;
};

template<int N>
class EachNotification: public BaseNotification {
public:
	EachNotification(int data) :
			BaseNotification(data) {
	}
};

template<class T>
class MyNotificationQueue: public T {
public:
	MyNotificationQueue() :
			m_pMsg(NULL) {
	}

	MyNotificationQueue(ScopedLogMessage& msg) :
			m_pMsg(&msg) {
	}

	void MyEnqueueNotification(Poco::Notification::Ptr pNotification,
			int /*priority*/) {
		m_pMsg->Message(
				Poco::format("Class %s is not supported!",
						std::string(typeid(T).name())));
	}

private:
	ScopedLogMessage* m_pMsg;
};

template<>
void MyNotificationQueue<Poco::NotificationQueue>::MyEnqueueNotification(
		Poco::Notification::Ptr pNotification, int /*priority*/) {
	enqueueNotification(pNotification);
}

template<>
void MyNotificationQueue<Poco::PriorityNotificationQueue>::MyEnqueueNotification(
		Poco::Notification::Ptr pNotification, int priority) {
	enqueueNotification(pNotification, priority);
}

template<class T, int N>
class Worker: public Poco::Runnable {
public:
	Worker(ScopedLogMessage& msg) :
			m_name(Poco::format("Worker<T,%d>", N)), m_msg(msg) {
		m_msg.Message(Poco::format("  %s created", m_name));
	}

	~Worker() {
		m_msg.Message(Poco::format("  %s deleted", m_name));
	}

	void run() {
		Poco::Random rnd;
		for (;;) {
			Poco::Notification::Ptr pNf(m_queue.waitDequeueNotification());
			if (pNf) {
				Poco::AutoPtr<EachNotification<N> > pWorkNf = pNf.cast<
						EachNotification<N> >();
				if (pWorkNf) {
					m_msg.Message(
							Poco::format("    %s got EachNotification<%d>(%d)",
									m_name, N, pWorkNf->data()));
					if (0 == pWorkNf->data()) {
						m_msg.Message(
								Poco::format("    %s exits run loop", m_name));
						break;
					}
					Poco::Thread::sleep(rnd.next(kSleepMaxTime));
				}
			} else
				break;
		}
		m_queue.wakeUpAll();
	}

	void handleNotification(Poco::Notification* pNf) {
		Poco::AutoPtr<EachNotification<N> > pWorkNf =
				dynamic_cast<EachNotification<N>*>(pNf);
		if (pWorkNf) {
			m_msg.Message(
					Poco::format(
							"    %s handleNotification got EachNotification<%d>(%d)",
							m_name, N, pWorkNf->data()));
			m_msg.Message(
					Poco::format("     -> enqueue EachNotification<%d>(%d)", N,
							pWorkNf->data()));
			m_queue.MyEnqueueNotification(pWorkNf, kPriorityNormal);
		} else {
			Poco::AutoPtr<BaseNotification> pNotification =
					dynamic_cast<BaseNotification*>(pNf);
			if (pNotification) {
				m_msg.Message(
						Poco::format(
								"    %s handleNotification got BaseNotification(%d)",
								m_name, pNotification->data()));
				if (0 == pNotification->data()) {
					m_msg.Message(
							Poco::format(
									"     -> enqueue EachNotification<%d>(%d)",
									N, pNotification->data()));
					m_queue.MyEnqueueNotification(new EachNotification<N>(0),
							kPriorityHighest);
				}
			}
		}
	}

private:
	std::string m_name;
	T m_queue;
	ScopedLogMessage& m_msg;
};

template<class T>
void TestNotificationCenter(ScopedLogMessage& msg, const std::string& title) {
	msg.Message(Poco::format("--- T = %s ---", title));

	Poco::NotificationCenter nc;

	Worker<MyNotificationQueue<T>, 1> worker1(msg);
	Worker<MyNotificationQueue<T>, 2> worker2(msg);
	Worker<MyNotificationQueue<T>, 3> worker3(msg);

	nc.addObserver(
			Poco::Observer<Worker<MyNotificationQueue<T>, 1>, Poco::Notification>(
					worker1,
					&Worker<MyNotificationQueue<T>, 1>::handleNotification));
	nc.addObserver(
			Poco::Observer<Worker<MyNotificationQueue<T>, 2>, Poco::Notification>(
					worker2,
					&Worker<MyNotificationQueue<T>, 2>::handleNotification));
	nc.addObserver(
			Poco::Observer<Worker<MyNotificationQueue<T>, 3>, Poco::Notification>(
					worker3,
					&Worker<MyNotificationQueue<T>, 3>::handleNotification));

	nc.postNotification(new EachNotification<3>(3));
	nc.postNotification(new EachNotification<2>(2));
	nc.postNotification(new EachNotification<1>(1));
	nc.postNotification(new BaseNotification(0));	// exit trigger for run loop

	Poco::ThreadPool::defaultPool().start(worker1);
	Poco::ThreadPool::defaultPool().start(worker2);
	Poco::ThreadPool::defaultPool().start(worker3);

	Poco::Thread::sleep(kSleepWaitTime);

	Poco::ThreadPool::defaultPool().joinAll();
}

void PrepareConsoleLogger(const std::string& name, int level =
		Poco::Message::PRIO_INFORMATION) {
	Poco::FormattingChannel* pFCConsole = new Poco::FormattingChannel(
			new Poco::PatternFormatter("%t"));
	pFCConsole->setChannel(new Poco::ConsoleChannel);
	pFCConsole->open();

	Poco::Logger::create(name, pFCConsole, level);
}

int main_notCenter(int /*argc*/, char** /*argv*/) {
	PrepareConsoleLogger(Poco::Logger::ROOT, Poco::Message::PRIO_INFORMATION);

	ScopedLogMessage msg("NotificationCenterTest ", "start", "end");

	TestNotificationCenter<Poco::NotificationQueue>(msg,
			"Poco::NotificationQueue");
	TestNotificationCenter<Poco::PriorityNotificationQueue>(msg,
			"Poco::PriorityNotificationQueue");

	return 0;
}

