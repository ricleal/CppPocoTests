#include "Poco/BasicEvent.h"
#include "Poco/Delegate.h"
#include <iostream>
using Poco::BasicEvent;
using Poco::Delegate;

/*
 Compile as:

 g++ poco3.cpp -o poco3 -lPocoFoundation

 [ferrazpc 150] POCO > ./poco3
 onEvent: 42


 */

class Source {
public:
	BasicEvent<int> theEvent;
	void fireEvent(int n) {
		theEvent(this, n);
		// theEvent.notify(this, n); // alternative syntax
	}
};

class Target {
public:
	void onEvent(const void* pSender, int& arg) {
		std::cout << "onEvent: " << arg << std::endl;
	}
};
int main_3(int argc, char** argv) {
	Source source;
	Target target;
	source.theEvent += Poco::delegate(&target, &Target::onEvent);
	source.fireEvent(42);
	source.theEvent -= Poco::delegate(&target, &Target::onEvent);
	return 0;
}
