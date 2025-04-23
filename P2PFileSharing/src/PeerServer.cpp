#include "../include/PeerServer.h"


awaitable<void> PeerServer::listener()
{
    auto executor = co_await this_coro::executor; // basically the context of this corutine
    tcp::acceptor acceptor(executor, { tcp::v4(), 55555 });
    for (;;)
    {
        std::cout << "Waitng for a new client\n";
        tcp::socket socket = co_await acceptor.async_accept(use_awaitable);
        co_spawn(executor, echo(std::move(socket)), detached);
    }
}



awaitable<void> PeerServer::echo(tcp::socket socket)
{
    try
    {
        char data[1024];
        for (;;)
        {
            std::size_t n = co_await socket.async_read_some(boost::asio::buffer(data), use_awaitable);
            co_await async_write(socket, boost::asio::buffer(data, n), use_awaitable);
        }
    }
    catch (std::exception& e)
    {
        std::printf("echo Exception: %s\n", e.what());
    }
}

