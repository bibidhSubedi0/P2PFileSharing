#pragma once
#include <boost/asio.hpp>
#include <iostream>
#include <string>

using boost::asio::ip::tcp;

class PeerClient
{
public:
	void connect();
};