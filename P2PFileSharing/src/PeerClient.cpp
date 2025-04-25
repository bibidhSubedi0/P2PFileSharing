
#include "../include/PeerClient.h"

void PeerClient::connect()
{
    ClientLogger.log("Attepting to connect to the server!\n");
    try
    {
        

        tcp::resolver resolver(clientContext);
        auto endpoints = resolver.resolve("127.0.0.1", "55555");

        tcp::socket socket(clientContext);

        // Here is when port assignemnt happens and the 3 way handhsake for TCP occours
        // socket can be manually binded to a port as well, but no point in doing that here
        boost::asio::connect(socket, endpoints);
        ClientLogger.log("Connected to server.\n");

        for(;;){
            std::string msg;
            std::cin >> msg;


            // Send message to server
            boost::asio::write(socket, boost::asio::buffer(msg));

            // Read echoed message back
            char reply[1024];
            size_t reply_length = socket.read_some(boost::asio::buffer(reply));
            ClientLogger.log("Message Recived from server!");
            std::string str(reply, reply_length);
            ClientLogger.log(str);
        }
        
    }
    catch (std::exception& e)
    {
        std::cerr << "Client Exception: " << e.what() << "\n";
    }
}
