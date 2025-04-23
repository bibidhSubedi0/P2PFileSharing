#pragma once


#include <boost/asio.hpp>
#include <boost/asio/experimental/co_spawn.hpp> // Include the correct header for co_spawn


#include <boost/asio/detached.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/signal_set.hpp>
#include <boost/asio/write.hpp>
#include <cstdio>*/

#include <boost/asio/co_spawn.hpp>
#include <iostream>

using boost::asio::ip::tcp;
using boost::asio::awaitable;
using boost::asio::co_spawn;
using boost::asio::detached;
using boost::asio::use_awaitable;
namespace this_coro = boost::asio::this_coro;




class PeerServer
{
public:
    awaitable<void> echo(tcp::socket socket);
    awaitable<void> listener();

};