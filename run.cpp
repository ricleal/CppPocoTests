/*
 * run.cpp
 *
 *  Created on: Oct 4, 2012
 *      Author: leal
 */

#include <iostream>

// see also: http://code.google.com/p/pocotest/

int main_1(int argc, char** argv);
int main_2(int argc, char** argv);
int main_3(int argc, char** argv);
int main_notCenter(int argc, char** argv);
int main_factory(int argc, char** argv);

int main(int argc, char** argv)
{

	std::cout << "--------------------------------------" << std::endl;
	//main_1( argc, argv);
	std::cout << "--------------------------------------" << std::endl;
//	main_2( argc, argv);
	std::cout << "--------------------------------------" << std::endl;
	//main_3( argc, argv);
	std::cout << "--------------------------------------" << std::endl;
	//main_notCenter(argc, argv);
	std::cout << "--------------------------------------" << std::endl;
	main_factory(argc, argv);


}

