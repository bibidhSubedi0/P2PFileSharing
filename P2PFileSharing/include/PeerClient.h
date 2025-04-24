#pragma once
// pch.h
#pragma once

#include <boost/asio.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/signal_set.hpp>
#include <boost/asio/write.hpp>
#include <iostream>
#include <unordered_map>
#include <vector>
#include <string>
#include <csignal>

#include "utils.h"

using boost::asio::ip::tcp;

class PeerClient
{
	boost::asio::io_context clientContext;
	logger::Logger ClientLogger;
public:
	void connect();
};