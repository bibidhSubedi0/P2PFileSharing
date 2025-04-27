
#include "../include/PeerClient.h"
#include <sstream>

void PeerClient::sendMessageToServer(tcp::socket& socket,std::string message)
{
    boost::asio::write(socket, boost::asio::buffer(message));
}



void PeerClient::connect(std::string username)
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
        sendMessageToServer(socket, username);

        ClientLogger.log("Connected to server.\n");

        for (;;) {
            std::string msg;
            std::getline(std::cin, msg); 

            if (msg.empty()) continue;  

            std::istringstream stream(msg);
            std::vector<std::string> commands;
            std::string word;

            while (stream >> word) {
                commands.push_back(word);
            }
            if (commands.empty()) continue;

            const std::string& cm = commands[0];
            if (cm == "exit") {
                break;
            }
            if (cm == "list") {
                queryForPeers(socket);
                continue; 
            }
            if (cm == "connect" && commands.size() > 1) {
                requestConnection(socket, commands[1]);
                continue;
            }

            if (cm == "echo" && commands.size() > 1)
            {
                size_t spacePos = msg.find(' ');
                if (spacePos != std::string::npos) {
                    msg.erase(0, spacePos + 1);
                }
                sendMessageToServer(socket, "echo|"+msg);
            }
            

            // Receive server reply
            char reply[1024];
            size_t reply_length = socket.read_some(boost::asio::buffer(reply));
            std::string str(reply, reply_length);
            ClientLogger.log("Message Received from server!");
            ClientLogger.log(str);
        }


        
    }
    catch (std::exception& e)
    {
        std::cerr << "Client Exception: " << e.what() << "\n";
    }

    ClientLogger.log("\n\n\nSession Ended");
}



void PeerClient::queryForPeers(tcp::socket& socket)
{
    sendMessageToServer(socket, "list|");
    // Read until "END\n" is received
    std::string response;
    char buffer[1024];
    for (;;)
    {
        size_t n = socket.read_some(boost::asio::buffer(buffer));
        std::string chunk(buffer, n);
        response += chunk;

        // Check for termination signal
        if (response.find("END\n") != std::string::npos)
            break;
    }

    // Strip END and print the list
    size_t endPos = response.find("END\n");
    if (endPos != std::string::npos) {
        response = response.substr(0, endPos);
    }

    ClientLogger.log("Peer List:\n" + response + "\n");
}



void PeerClient::requestConnection(tcp::socket& socket, std::string connect_to)
{
    sendMessageToServer(socket, "conn_request|"+connect_to);

    char reply[1024];
    size_t reply_length = socket.read_some(boost::asio::buffer(reply));
    std::string str(reply, reply_length);
    ClientLogger.log("Message Received from server!");
    ClientLogger.log(str);

}