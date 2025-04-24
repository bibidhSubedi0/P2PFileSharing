#include "../include/PeerServer.h"


awaitable<void> PeerServer::listener()
{
    // Gets the executor associcated with the current coroutine
    auto executor = co_await this_coro::executor; 
    tcp::acceptor acceptor(executor, { tcp::v4(), 55555 });
    for (;;)
    {
        std::cout << "Waitng for a new client\n";
        tcp::socket socket = co_await acceptor.async_accept(use_awaitable);
        std::cout << "Client connected\n";
        co_spawn(executor, echo(std::move(socket)), detached);
    }
}


awaitable<void> PeerServer::echo(tcp::socket socket)
{
    std::cout << "Started the echo thread\n";
    try
    {
        for (;;)
        {
            char data[1024] = { 0 };
            std::size_t n = co_await socket.async_read_some(boost::asio::buffer(data), use_awaitable);
            co_await async_write(socket, boost::asio::buffer(data, n), use_awaitable);

            std::string strx(data,n);
            std::cout << strx << std::endl;
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
    // when when signal is recived, the called back runs i.e. io_context.stop()
    boost::asio::signal_set signals(serverContext, SIGINT, SIGTERM);
    signals.async_wait([&](auto, auto) { serverContext.stop(); });


    co_spawn(serverContext, listener(), detached);
    serverContext.run();

}


