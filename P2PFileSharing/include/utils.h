#pragma once
#include<iostream>

class test {
public:
	void testFunc() {
		std::cout << "Test function called!" << std::endl;
	}

	bool is_valid_ip(const::std::string& ip);

};