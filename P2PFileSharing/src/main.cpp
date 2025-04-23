#include"../include/utils.h"
#include <iostream>
#include <boost/asio.hpp>

int main(int argc, char** argv) {

	/**
	* @brief Main function of the P2P File Sharing application.
	* 
	* Just testing 1st the test function in utils.h
	*/
	test t;
	t.testFunc();
	std::cout << "Hello, World!" << std::endl;
	try {
		boost::asio::io_context io;
		boost::asio::steady_timer timer(io, std::chrono::seconds(1));
		timer.wait();
		std::cout << "Boost.Asio is working! Timer completed." << std::endl;
	}
	catch (std::exception& e) {
		std::cerr << "Error: " << e.what() << std::endl;
		return 1;
	}
	/*::testing::InitGoogleTest(&argc, argv);
	RUN_ALL_TESTS();*/
	

	return 0;
}