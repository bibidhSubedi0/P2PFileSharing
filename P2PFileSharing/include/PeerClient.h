#pragma once

#include <boost/asio.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/signal_set.hpp>
#include <boost/asio/write.hpp>
#include <boost/asio/awaitable.hpp>
#include <iostream>
#include <unordered_map>
#include <vector>
#include <string>
#include <csignal>

#include "utils.h"

using boost::asio::ip::tcp;

/**
 * @class PeerClient
 * @brief Handles the client-side connection to a peer-to-peer server.
 *
 * The PeerClient is responsible for connecting to the server, sending messages,
 * and receiving echoed responses. It uses synchronous Boost.Asio operations.
 */
class PeerClient
{
	boost::asio::io_context clientContext; /**< IO context used by the client. */
	logger::Logger ClientLogger;           /**< Logger instance for logging client events. */
	std::string _username;
	//tcp::socket socket;

public:
	/**
	 * @brief Connects the client to the server and manages message exchange.
	 *
	 * Resolves the server endpoint, establishes a TCP connection, and enters a loop
	 * to send user input to the server and print echoed responses.
	 */
	PeerClient(std::string);

	void connect(std::string username);

	void mainLoop();

	void queryForPeers();

	void sendMessageToServer(tcp::socket&, std::string);

	void requestConnection(std::string); // Only usename is enough? IDK

	//boost::asio::awaitable<void> CommWithServer(tcp::socket&);
	void CommWithServer(tcp::socket&);

	boost::asio::awaitable<void> CommWithPeers(boost::asio::ip::tcp::socket &peer_socket);

	void handelConnections(tcp::socket& socket, const std::string& local_ip, unsigned short local_port);
	
	boost::asio::awaitable<void> listenForPeers();

	void processCommand(const std::string& msg);

	void start();

};
