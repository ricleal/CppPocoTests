#include <Poco/Logger.h>
#include <Poco/PatternFormatter.h>
#include <Poco/FormattingChannel.h>
#include <Poco/ConsoleChannel.h>
#include <Poco/DynamicFactory.h>
#include <Poco/TypeList.h>
#include <Poco/SharedPtr.h>

#include <boost/shared_ptr.hpp>

#include <string>
#include <iostream>

enum EShapeType {
	eTriangle = 0, eRectangle, eOval, eNumShapes
};

const std::string kShapeNames[] = { "Triangle", "Rectangle", "Oval" };

class Shape {
public:
	Shape() :
			m_pMsg() {
	}

	virtual ~Shape() {
	}

	void setMessage(const std::string& msg) {
		m_pMsg = msg;
	}

	virtual void draw(void) = 0;

protected:
	void draw(const std::string& str) {
		if (m_pMsg.length() > 0) {
			std::cout << m_pMsg << " -> " << str << std::endl;
		}
	}

private:
	std::string m_pMsg;
};

class TriangleShape: public Shape {
public:
	void draw(void) {
		Shape::draw(std::string("Triangle"));
	}
};

class RectangleShape: public Shape {
public:
	void draw(void) {
		Shape::draw(std::string("Rectangle"));
	}
};

class OvalShape: public Shape {
public:
	void draw(void) {
		Shape::draw(std::string("Oval"));
	}
};

typedef Poco::TypeListType<TriangleShape, RectangleShape, OvalShape>::HeadType ShapeTypeList;

template<EShapeType N>
void RegisterClass(Poco::DynamicFactory<Shape>& shapeFactory) {
	typedef typename Poco::TypeGetter<N, ShapeTypeList>::HeadType ShapeType;
	shapeFactory.registerClass<ShapeType>(kShapeNames[N]);
	RegisterClass<static_cast<EShapeType>(N + 1)>(shapeFactory); // recursive call
}

template<>
void RegisterClass<eOval>(Poco::DynamicFactory<Shape>& shapeFactory) {
	typedef Poco::TypeGetter<eOval, ShapeTypeList>::HeadType ShapeType;
	shapeFactory.registerClass<ShapeType>(kShapeNames[eOval]);
}

int main_factory(int /*argc*/, char** /*argv*/) {

	std::cout << "DynamicFactoryTest: start" << std::endl;

	Poco::DynamicFactory<Shape> shapeFactory;

	RegisterClass<eTriangle>(shapeFactory); // recursive register

	boost::shared_ptr<Shape> shapes[eNumShapes];
	for (std::size_t i = 0; i < eNumShapes; ++i) {
		shapes[i] = shapeFactory.createInstance(kShapeNames[i]);
		shapes[i]->setMessage("Shape");
	}

	for (std::size_t i = 0; i < eNumShapes; ++i) {
		shapes[i]->draw();
	}

	std::cout << "DynamicFactoryTest: end" << std::endl;

	return 0;
}
