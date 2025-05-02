
#include "../include/PeerServer.h"
#include <sstream>




/*



awaitable<void> PeerServer::PeerConn(tcp::socket socket, const std::string uid)
{
    try
    {
        std::string ip = socket.remote_endpoint().address().to_string();
        unsigned short port = socket.remote_endpoint().port();

        for (;;)
        {
            // Read incoming commands
            char read_buffer[1024] = { 0 };
            std::size_t n = co_await socket.async_read_some(boost::asio::buffer(read_buffer), use_awaitable);

            std::string received_message(read_buffer, n);


            {
                std::scoped_lock lock(_peers_mutex);
                ServerLogger.log(
                    "Received message from client " + _peers[uid].username + "\t" +
                    ip + ":" + std::to_string(port) + "\n" +
                    "Message: " + received_message + "\n"
                );
            }



            std::istringstream stream(received_message);
            std::string command;
            std::getline(stream, command, '|');  // Read up to the first '|'
            
            // Handle "list" command
            if (command == "list")
            {
                std::string peer_list;
                {
                    std::scoped_lock lock(_peers_mutex);
                    for (const auto& [peer_id, peer_info] : _peers)
                    {
                        peer_list += peer_info.username + "\n";
                    }
                }
                peer_list += "END\n"; // End marker for client

                co_await async_write(socket, boost::asio::buffer(peer_list), use_awaitable);
            }
            else if (command == "echo")
            {
                // Echo back normal message
                size_t spacePos = received_message.find('|');
                if (spacePos != std::string::npos) {
                    received_message.erase(0, spacePos + 1);
                }
                co_await async_write(socket, boost::asio::buffer(received_message, received_message.length()), use_awaitable);
            }

            else if (command == "conn_request") {
                std::string connectTo;
                std::getline(stream, connectTo, '|');  // Read the target (after '|')

                ServerLogger.log("Received conn_request to connect to: " + connectTo);
                // Handle the connection request here...
                std::string requested_info;
                {
                    std::scoped_lock lock(_peers_mutex);
                    for (const auto& [peer_id, peer_info] : _peers)
                    {
                        if (peer_info.username == connectTo) {
                            requested_info = peer_info.ip_address + ":" + std::to_string(peer_info.port);
                            break;
                        }
                    }
                }
                co_await async_write(socket, boost::asio::buffer(requested_info, requested_info.length()), use_awaitable);
            }
            else {
                std::cout << "Unknown command: " << command << std::endl;
            }


        }
    }
    catch (boost::system::system_error& e)
    {
        try
        {
            if (e.code() == boost::asio::error::eof || e.code() == boost::asio::error::connection_reset)
            {
                ServerLogger.log(
                    socket.remote_endpoint().address().to_string() + "+" +
                    std::to_string(socket.remote_endpoint().port()) +
                    "  Client disconnected."
                );
            }
            else
            {
                ServerLogger.log(std::string("PeerConn Error: ") + e.what());
            }
        }
        catch (const std::exception&)
        {
            ServerLogger.log("PeerConn Error: could not retrieve remote endpoint (socket already closed?)");
        }
    }
}


*/

PeerServer::PeerServer() 
{
    
}
 
void PeerServer::StartServer()
{
    // ctrl + c sends the sigint and stops the program
    // when signal is received, the callback runs i.e. io_context.stop()
    boost::asio::signal_set signals(serverContext, SIGINT, SIGTERM);
    signals.async_wait([&](auto, auto) { serverContext.stop(); });

    ServerLogger.log("Server Started!");


    co_spawn(serverContext, listener(), detached);


    serverContext.run();
}




awaitable<void> PeerServer::listener()  
{  
    auto executor = co_await this_coro::executor;  
    tcp::acceptor acceptor(executor, { tcp::v4(), 55555 });  
    
    for (;;)  
    {  
        tcp::socket socket = co_await acceptor.async_accept(use_awaitable);  

        // Use streambuf to read until newline
        boost::asio::streambuf buffer;
        std::size_t n = co_await async_read_until(socket, buffer, "\n", use_awaitable);

        std::istream is(&buffer);
        std::string username;
        std::getline(is, username);

        bool username_exists = false;
        {
            std::scoped_lock lock(_peers_mutex);
            for (const auto& pr : _peers) {
                if (pr.second.username == username) {
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



        printStuff();
        if (username_exists) {

            ServerLogger.log("Continued");
            // Replace the credentials of that peer



            std::istream is(&buffer);
            std::string command;
            std::getline(is, command, '|');

            ServerLogger.log(command);
            if (command == "list")
            {
                
                std::string peer_list;
                { 
                    {
                        std::scoped_lock lock(_peers_mutex);
                        for (const auto& [peer_id, peer_info] : _peers)
                        {
                            peer_list += peer_info.username + "\n";
                        }
                    }
                }
                peer_list += "END\n"; // End marker for client

                co_await async_write(socket, boost::asio::buffer(peer_list), use_awaitable);
            }
            else if (command == "peer_endpoint") {
                // Now update the endpoint's position
                std::string ip;
                std::string port;
                std::getline(is, ip, '|');
                std::getline(is, port, '|');

                _info.ip_address = ip;
                _info.port = std::stoi(port);

                {
                    std::scoped_lock lock(_peers_mutex);
                    for (auto& pr : _peers) {
                        if (pr.second.username == username)
                            pr.second = _info;
                    }
                }
                ServerLogger.log("Updated the info of " + username+ " :"+std::to_string(_info.port));

            }
            else if (command == "conn_request") {
                std::string connectTo;
                std::getline(is, connectTo);  // Read the target (after '|')

                ServerLogger.log("Received conn_request from"+username+" to connect to : " + connectTo);
                // Handle the connection request here...
                std::string requested_info;
                {
                    std::scoped_lock lock(_peers_mutex);
                    for (const auto& [peer_id, peer_info] : _peers)
                    {
                        if (peer_info.username == connectTo) {
                            requested_info = peer_info.ip_address + ":" + std::to_string(peer_info.port);
                            break;
                        }
                    }
                }
                co_await async_write(socket, boost::asio::buffer(requested_info, requested_info.length()), use_awaitable);
            }

            else {
                std::cout << "Unknown command: " << command << std::endl;
            }

            continue; // Simply discard this socket and move on
        }


        {
            std::scoped_lock lock(_peers_mutex); 
            _peers[id] = std::move(_info);  
        }

        ServerLogger.log("New client joined the network:\n"
            "Id: " + id + "\n" +
            "Username: " + username + "\n" +
            "IP Address: " + socket.remote_endpoint().address().to_string() + "\n" +
            "Port: " + std::to_string(socket.remote_endpoint().port()) + "\n");
            
    }  

    
    
    co_return;
}


void PeerServer::printStuff()
{
    // std::unordered_map<std::string, Peers::PeerInfo> _peers;

    for (auto user : _peers)
    {
        ServerLogger.log(user.second.username + " " + user.second.ip_address + " " + std::to_string(user.second.port));
    }
}