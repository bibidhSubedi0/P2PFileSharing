
#include "../include/PeerServer.h"



awaitable<void> PeerServer::listener()  
{  
   // Gets the executor associated with the current coroutine  
   auto executor = co_await this_coro::executor;  
   tcp::acceptor acceptor(executor, { tcp::v4(), 55555 });  
   for (;;)  
   {  
       tcp::socket socket = co_await acceptor.async_accept(use_awaitable);  
         
       // Get the username first
       char username[1024] = { 0 };
       std::size_t n = co_await socket.async_read_some(boost::asio::buffer(username), use_awaitable);

       // Generate a GUID for the new peer  
       std::string id = _uid_generator.generate_uid();  
         
       
       // Generate the information for this peer  
       Peers::PeerInfo _info(socket.remote_endpoint().address().to_string(), socket.remote_endpoint().port(), std::string(username), std::vector<std::string>{});
         
       // Add this peer to the list  
       _peers[id] = std::move(_info);  
         
       ServerLogger.log("New client joined the network:\n"
           "Id: " + id + "\n" +
           "Username: "+username +"\n"+
           "IP Address: " + socket.remote_endpoint().address().to_string() + "\n" +
           "Port: " + std::to_string(socket.remote_endpoint().port()) + "\n");
         
       // Echo will later be replaced by some coroutine to handle each client  
        co_spawn(executor, PeerConn(std::move(socket),id), detached);  
   }  
}
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

            ServerLogger.log(
                "Received message from client " + _peers[uid].username + "\t" +
                ip + ":" + std::to_string(port) + "\n" +
                "Message: " + received_message + "\n"
            );

            // Handle "list" command
            if (received_message == "list")
            {
                std::string peer_list;
                for (const auto& [peer_id, peer_info] : _peers)
                {
                    peer_list += peer_info.username + "\n";
                }
                peer_list += "END\n"; // End marker for client

                co_await async_write(socket, boost::asio::buffer(peer_list), use_awaitable);
            }

            // Deafult
            else
            {
                // Echo back normal message
                co_await async_write(socket, boost::asio::buffer(read_buffer, n), use_awaitable);
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


