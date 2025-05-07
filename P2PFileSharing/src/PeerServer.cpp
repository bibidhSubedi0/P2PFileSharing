#include "../include/PeerServer.h"
#include <sstream>

// Constructor
PeerServer::PeerServer() {}

// Start server
void PeerServer::StartServer()
{
    boost::asio::signal_set signals(serverContext, SIGINT, SIGTERM);
    signals.async_wait([&](auto, auto) { serverContext.stop(); });

    ServerLogger.log("Server Started!", logger::LogLevel::Success);

    co_spawn(serverContext, listener(), detached);
    serverContext.run();
}

// Listener
awaitable<void> PeerServer::listener()
{
    auto executor = co_await this_coro::executor;
    tcp::acceptor acceptor(executor, { tcp::v4(), 55555 });

    for (;;)
    {
        tcp::socket socket = co_await acceptor.async_accept(use_awaitable);

        // Read initial username
        boost::asio::streambuf buffer;
        co_await async_read_until(socket, buffer, "\n", use_awaitable);
        std::istream is(&buffer);
        std::string username;
        std::getline(is, username);

        bool username_exists = false;
        {
            std::scoped_lock lock(_peers_mutex);
            for (const auto& pr : _peers)
            {
                if (pr.second.username == username)
                {
                    username_exists = true;
                    break;
                }
            }
        }

        std::string id = _uid_generator.generate_uid();
        Peers::PeerInfo _info(
            socket.remote_endpoint().address().to_string(),
            socket.remote_endpoint().port(),
            username,
            std::vector<std::string>{}
        );

        

        if (username_exists)
        {

            std::string command;
            std::getline(is, command, '|');
            ServerLogger.log(username + ": " + command, logger::LogLevel::Info);

            if (command == "list")
            {
                std::string peer_list;
                {
                    std::scoped_lock lock(_peers_mutex);
                    for (const auto& [peer_id, peer_info] : _peers)
                        peer_list += peer_info.username + "\n";
                }
                peer_list += "END\n";
                co_await async_write(socket, boost::asio::buffer(peer_list), use_awaitable);
            }
            else if (command == "peer_endpoint")
            {
                std::string ip, port;
                std::getline(is, ip, '|');
                std::getline(is, port, '|');

                _info.ip_address = ip;
                _info.port = std::stoi(port);

                {
                    std::scoped_lock lock(_peers_mutex);
                    for (auto& pr : _peers)
                    {
                        if (pr.second.username == username)
                            pr.second = _info;
                    }
                }
                printStuff();

            }
            else if (command == "conn_request")
            {
                std::string connectTo;
                std::getline(is, connectTo);

                ServerLogger.log(username + " z " + connectTo, logger::LogLevel::Info);

                std::string requested_info;
                {
                    std::scoped_lock lock(_peers_mutex);
                    for (const auto& [peer_id, peer_info] : _peers)
                    {
                        if (peer_info.username == connectTo)
                        {
                            requested_info = peer_info.ip_address + ":" + std::to_string(peer_info.port);
                            break;
                        }
                    }
                }

                co_await async_write(socket, boost::asio::buffer(requested_info), use_awaitable);
            }
            else
            {
                ServerLogger.log("Unknown command: " + command, logger::LogLevel::Warn);
            }
            
            continue; // Done handling this old user
        }

        {
            std::scoped_lock lock(_peers_mutex);
            _peers[id] = std::move(_info);
        }

        ServerLogger.log(
            "New peer joined:\n"
            "\tId: " + id + "\n"
            "\tUsername: " + username + "\n"
            "\tIP: " + socket.remote_endpoint().address().to_string() + "\n"
            "\tPort: " + std::to_string(socket.remote_endpoint().port()), logger::LogLevel::Success);
    }

    co_return;
}

// Print all peers
void PeerServer::printStuff()
{
    std::string userList =
        "\n\t+----------------------------------+\n"
        "\t|        User Information          |\n"
        "\t+----------------------------------+\n";

    // Collect information for each user in the list
    for (const auto& user : _peers)
    {
        userList +=
            "\t| Username     : " + user.second.username + "\n" +
            "\t| IP Address   : " + user.second.ip_address + "\n" +
            "\t| Port         : " + std::to_string(user.second.port) + "\n" +
            "\t+----------------------------------+\n";
    }

    // Log the entire user list with a clean ASCII box
    ServerLogger.log(userList, logger::LogLevel::Info);
}
