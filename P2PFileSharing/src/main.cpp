
#include "../include/PeerServer.h"


int main()
{



    PeerServer ps;
    try
    {
        boost::asio::io_context io_context(1);
        boost::asio::signal_set signals(io_context, SIGINT, SIGTERM);
        signals.async_wait([&](auto, auto) { io_context.stop(); });

        co_spawn(io_context, ps.listener(), detached);

        io_context.run();
    }
    catch (std::exception& e)
    {
        std::printf("Exception: %s\\n", e.what());
    }

}