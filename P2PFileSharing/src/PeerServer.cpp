
#include "../include/PeerServer.h"



awaitable<void> PeerServer::listener()
{
    // Gets the executor associcated with the current coroutine
    auto executor = co_await this_coro::executor; 
    tcp::acceptor acceptor(executor, { tcp::v4(), 55555 });
    for (;;)
    {
        tcp::socket socket = co_await acceptor.async_accept(use_awaitable);
        
        // Generate a GUID for the new peer
        std::string id = _uid_generator.generate_uid();
        
        // Generate the information for this peer
        Peers::PeerInfo _info(socket.remote_endpoint().address().to_string(),socket.remote_endpoint().port(),std::vector<std::string>{});

        // Add this peer to the list
        _peers[id] = _info;

        ServerLogger.log("New client joind the network : " + id);

        // Echo will later be replaced by some corutine to handel each client 
        co_spawn(executor, echo(std::move(socket)), detached);
    }
}


awaitable<void> PeerServer::echo(tcp::socket socket)
{
    try
    {
        for (;;)
        {
            char data[1024] = { 0 };
            std::size_t n = co_await socket.async_read_some(boost::asio::buffer(data), use_awaitable);
            co_await async_write(socket, boost::asio::buffer(data, n), use_awaitable);

            std::string strx(data,n);
            ServerLogger.log(strx);
        }
    }
    catch (std::exception& e)
    {
        std::printf("echo Exception: %s\n", e.what());
    }
}

PeerServer::PeerServer() 
{
    ;
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


