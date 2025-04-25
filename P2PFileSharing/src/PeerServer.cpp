
#include "../include/PeerServer.h"



awaitable<void> PeerServer::listener()  
{  
   // Gets the executor associated with the current coroutine  
   auto executor = co_await this_coro::executor;  
   tcp::acceptor acceptor(executor, { tcp::v4(), 55555 });  
   for (;;)  
   {  
       tcp::socket socket = co_await acceptor.async_accept(use_awaitable);  
         
       // Generate a GUID for the new peer  
       std::string id = _uid_generator.generate_uid();  
         
       // Generate the information for this peer  
       Peers::PeerInfo _info(socket.remote_endpoint().address().to_string(), socket.remote_endpoint().port(), std::vector<std::string>{});  
         
       // Add this peer to the list  
       _peers[id] = std::move(_info);  
         
       ServerLogger.log("New client joined the network:\n"
           "Id: " + id + "\n" +
           "IP Address: " + socket.remote_endpoint().address().to_string() + "\n" +
           "Port: " + std::to_string(socket.remote_endpoint().port()) + "\n");
         
       // Echo will later be replaced by some coroutine to handle each client  
        co_spawn(executor, PeerConn(std::move(socket)), detached);  
   }  
}


awaitable<void> PeerServer::PeerConn(tcp::socket socket)
{
    try
    {
        for (;;)
        {
            char data[1024] = { 0 };
            std::size_t n = co_await socket.async_read_some(boost::asio::buffer(data), use_awaitable);
            co_await async_write(socket, boost::asio::buffer(data, n), use_awaitable);

            // Convert received data to a string
            std::string strx(data,n);

            // Get IP address and port from the socket
            std::string ip = socket.remote_endpoint().address().to_string();
            unsigned short port = socket.remote_endpoint().port();

            // Log the received message from the client
            ServerLogger.log("Received message from client "+ ip + ":" +std::to_string(port) + "\n" +
                 "Message: " + strx + "\n");

        }
    }
    catch (std::exception& e)
    {
        std::printf("echo Exception: %s\n", e.what());
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


