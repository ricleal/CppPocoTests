#include "Poco/NotificationCenter.h"
#include "Poco/Notification.h"
#include "Poco/Observer.h"
#include "Poco/NObserver.h"
#include "Poco/AutoPtr.h"

#include <iostream>
#include <string>


using Poco::NotificationCenter;
using Poco::Notification;
using Poco::Observer;
using Poco::NObserver;
using Poco::AutoPtr;

using namespace std;


/*
 Compile as:

 g++ poco.cpp -o poco -lPocoFoundation

 [ferrazpc 148] POCO > ./poco
 handleBase: 16BaseNotification
 handleBase: 15SubNotification
 handleSub: 15SubNotification

 */
class BaseNotification: public Notification {
public:
	BaseNotification(string _data) :
			data(_data) {
	}

	string getData() const {
		return data;
	}

private:
	string data;
};

class SubNotification: public BaseNotification {
public:
	SubNotification(string data) : BaseNotification(data) {}
};

class Target {
public:
	void handleBase(BaseNotification* pNf) {
		std::cout << "handleBase: " << pNf->name() << std::endl;
		std::cout << "Data: "<< pNf->getData() << std::endl;

		pNf->release();		 // we got ownership, so we must release
	}
	void handleSub(const AutoPtr<SubNotification>& pNf) {
		std::cout << "handleSub: " << pNf->name() << std::endl;
		std::cout << "Data: "<< pNf->getData() << std::endl;
	}
};

int main_1(int argc, char** argv) {

	NotificationCenter nc;
	Target target;

	// Observer works with plain pointers to Notification objects.
	nc.addObserver(
			Observer<Target, BaseNotification>(target, &Target::handleBase));
	// NObserver works with AutoPtr<Notification>.
	// handleBase and handleSub both got this
	nc.addObserver(
			NObserver<Target, SubNotification>(target, &Target::handleSub));

	std::cout << "* postNotification: BaseNotification" << std::endl;
	nc.postNotification(new BaseNotification("Base Class Notification"));

	// Targets subscribed for a particular notification class also receive
	// notifications that are subclasses of that class.
	std::cout << "* postNotification: SubNotification" << std::endl;
	nc.postNotification(new SubNotification("Sub Class Notification"));

	nc.removeObserver(
			Observer<Target, BaseNotification>(target, &Target::handleBase));
	nc.removeObserver(
			NObserver<Target, SubNotification>(target, &Target::handleSub));

	return 0;
}
