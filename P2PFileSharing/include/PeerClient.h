#pragma once
#include <boost/asio.hpp>
#include <iostream>
#include <string>

using boost::asio::ip::tcp;

class PeerClient
{
	boost::asio::io_context clientContext;
public:
	void connect();
};