
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
using boost::asio::awaitable;
using boost::asio::co_spawn;
using boost::asio::detached;
using boost::asio::use_awaitable;
namespace this_coro = boost::asio::this_coro;





class PeerServer
{
    boost::asio::io_context serverContext;
    logger::Logger ServerLogger;
    GlobalUID::UIDGenerator _uid_generator;
    std::unordered_map<std::string, Peers::PeerInfo> _peers;


public:
    PeerServer();
    void StartServer();
    
    awaitable<void> echo(tcp::socket socket);
    awaitable<void> listener();

};