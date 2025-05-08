#pragma once
#include <iostream>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/asio.hpp>
#include <fstream>


/**
 * @namespace logger
 * @brief Contains logging utilities.
 */
namespace logger
{
    enum class LogLevel { Info, Success, Error, Warn };

    /**
     * @class Logger
     * @brief Provides timestamped logging functionality with log levels.
     */
    class Logger {
    public:
        /**
         * @brief Logs a message with a current timestamp and log level to standard output.
         *
         * @param message The message to log.
         * @param level The log level (default is Info).
         */
        void log(const std::string& message, LogLevel level = LogLevel::Info);
    };
}

/**
 * @namespace GlobalUID
 * @brief Contains utilities for generating globally unique identifiers (UUIDs).
 */
namespace GlobalUID
{
    /**
     * @class UIDGenerator
     * @brief Generates unique identifiers using UUIDs.
     */
    class UIDGenerator
    {
    public:
        /**
         * @brief Generates a new UUID string.
         *
         * @return std::string A unique UUID.
         */
        std::string generate_uid();
    };
}

using boost::asio::ip::tcp;

/**
 * @namespace Peers
 * @brief Contains structures and utilities related to peer information.
 */
namespace Peers {

    /**
     * @struct PeerInfo
     * @brief Holds information about a connected peer.
     *
     * Identified by its IP address and port. Also stores the last active time
     * and a list of shared files.
     */
    struct PeerInfo {
        std::string username;
        std::string ip_address;  /**< IP address of the peer. */
        unsigned short port;     /**< TCP port of the peer. */
        std::chrono::system_clock::time_point last_active; /**< Last activity timestamp. */
        std::vector<std::string> shared_files; /**< List of shared file names. */

        /**
         * @brief Default constructor.
         */
        PeerInfo() = default;

        /**
         * @brief Constructs a PeerInfo object.
         *
         * @param socket The socket associated with the peer.
         * @param ip IP address of the peer.
         * @param p Port number of the peer.
         * @param files List of shared files.
         */
        PeerInfo(std::string ip, unsigned short p,std::string username, const std::vector<std::string>& files);
    };

}


namespace FileHandling
{
    int makeBinaryChunks(const std::string& filename, std::string dirName, std::size_t chunkSize = 20);

    void reconstructFile(const std::string& outputFilename, int chunkCount);


}