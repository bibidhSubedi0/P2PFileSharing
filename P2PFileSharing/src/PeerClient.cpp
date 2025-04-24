#include "../include/PeerClient.h"

void PeerClient::connect()
{
    std::cout << "Attepting to connect to the server!\n";
    try
    {
        

        tcp::resolver resolver(clientContext);
        auto endpoints = resolver.resolve("127.0.0.1", "55555");

        tcp::socket socket(clientContext);
        boost::asio::connect(socket, endpoints);
        std::cout << "Connected to server.\n";

        for(;;){
            std::string msg;
            std::cin >> msg;


            // Send message to server
            boost::asio::write(socket, boost::asio::buffer(msg));

            // Read echoed message back
            char reply[1024];
            size_t reply_length = socket.read_some(boost::asio::buffer(reply));
            std::cout << "Echoed from server: ";
            std::cout.write(reply, reply_length);
            std::cout << "\n";
        }
        
    }
    catch (std::exception& e)
    {
        std::cerr << "Client Exception: " << e.what() << "\n";
    }
}
