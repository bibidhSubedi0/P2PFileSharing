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

/**
 * @class PeerServer
 * @brief Manages peer-to-peer server operations using asynchronous networking.
 *
 * This class listens for incoming TCP connections, assigns each peer a unique ID,
 * maintains a registry of connected peers, and handles communication using coroutines.
 */
class PeerServer
{
    boost::asio::io_context serverContext;  /**< IO context for the server. */
    logger::Logger ServerLogger;            /**< Logger instance for logging events. */
    GlobalUID::UIDGenerator _uid_generator; /**< Utility to generate unique IDs for peers. */
    std::unordered_map<std::string, Peers::PeerInfo> _peers; /**< Map of peer ID to peer information. */
    std::mutex _peers_mutex;

public:
    /**
     * @brief Constructs a new PeerServer object.
     */
    PeerServer();

    /**
     * @brief Starts the peer server.
     *
     * Sets up signal handling and begins the asynchronous listener coroutine.
     */
    void StartServer();

    /**
     * @brief Handles communication with a connected peer.
     *
     * This coroutine reads data from a peer and echoes it back for now.
     * It also logs messages received from the client.
     *
     * @param socket A TCP socket representing the connected peer.
     * @return awaitable<void>
     */
    awaitable<void> PeerConn(tcp::socket socket,const std::string uid);

    /**
     * @brief Listens for incoming peer connections on a specified port.
     *
     * For every new peer, generates a unique ID, stores its information,
     * and spawns a coroutine to handle communication.
     *
     * @return awaitable<void>
     */
    awaitable<void> listener();
};
