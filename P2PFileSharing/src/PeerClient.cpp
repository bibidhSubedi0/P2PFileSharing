
#include "../include/PeerClient.h"
#include <boost/asio/ip/address.hpp>
#include <boost/asio/ip/address_v4.hpp>
#include <sstream>
#include<queue>

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

        // For now i am connected to the server
        boost::asio::connect(socket, endpoints);
        sendMessageToServer(socket, username);


        // Ip and port assigned to this peer by the OS
        auto local_ep = socket.local_endpoint();
        std::string local_ip = local_ep.address().to_string();
        unsigned short local_port = local_ep.port();

        ClientLogger.log("Connect to the server with: " + local_ip + ":" + std::to_string(local_port));

        // Handel both connection to server and peers
        handelConnections(socket,local_ip,local_port);

    }
    catch (std::exception& e)
    {
        std::cerr << "Client Exception: " << e.what() << "\n";
    }

    ClientLogger.log("\n\n\nSession Ended");
}

void PeerClient::handelConnections(tcp::socket& socket, const std::string& local_ip, unsigned short local_port) {

    // To listen to peers
    std::thread accept_thread(&PeerClient::listenForPeers, this, local_ip, local_port);
    accept_thread.detach();


    // To handel the connection with server for queries
    std::thread input_thread(&PeerClient::CommWithServer, this, std::ref(socket));

    
    clientContext.run();
    input_thread.join();
}


void PeerClient::listenForPeers(const std::string& local_ip, unsigned short local_port) {

    try {
        boost::asio::ip::tcp::acceptor acceptor(clientContext,
            boost::asio::ip::tcp::endpoint(boost::asio::ip::make_address(local_ip), local_port));

        ClientLogger.log("Listening for peer connections on " + local_ip + ":" + std::to_string(local_port));

        while (true) {
            // Accept an incoming connection from another client
            boost::asio::ip::tcp::socket peer_socket(clientContext);
            acceptor.accept(peer_socket);

            // Handle the connection with a coroutine
            ClientLogger.log("Accepted connection from: " + peer_socket.remote_endpoint().address().to_string() + ":" + std::to_string(peer_socket.remote_endpoint().port()));
            boost::asio::co_spawn(clientContext, CommWithPeers(peer_socket), boost::asio::detached);
            clientContext.run();
        }
    }
    catch (std::exception& e) {
        std::cerr << "Peer acceptor exception: " << e.what() << "\n";
    }
}

boost::asio::awaitable<void> PeerClient::CommWithPeers(boost::asio::ip::tcp::socket &peer_socket) {
    try {
        ClientLogger.log("Waiting for meesage from : " + peer_socket.remote_endpoint().address().to_string() + ":" + std::to_string(peer_socket.remote_endpoint().port()));
        for (;;) {
            char data[1024];
            size_t length = co_await peer_socket.async_read_some(boost::asio::buffer(data), boost::asio::use_awaitable);

            std::string message(data, length);
            ClientLogger.log("Received message from peer: " + message);

            // Handle peer message here (e.g., echo it back or process the data)
            co_await peer_socket.async_write_some(boost::asio::buffer("Message received"), boost::asio::use_awaitable);
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Peer communication failed: " << e.what() << std::endl;
    }
}




void PeerClient::CommWithServer(tcp::socket& socket){
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
            sendMessageToServer(socket, "echo|" + msg);

            // No continue cuz we are waiting for the reply
            // Good desgin? Fuck no! Does it work? yep
        }
        else {
            ClientLogger.log("Invalid Command!");
            continue;
        }


        // Receive server reply
        char reply[1024];
        size_t reply_length = socket.read_some(boost::asio::buffer(reply));
        std::string str(reply, reply_length);
        ClientLogger.log("Message Received from server!");
        ClientLogger.log(str);
    }
    //inputThread.join(); // Wait for input thread to finish before exiting

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

    std::istringstream stream(str);

    std::string ip;

    std::getline(stream, ip, ':');
    std::string port;
    std::getline(stream, port, ' ');
    
    // Now we have ip and port, make the connection to the client directly

    
    tcp::resolver resolver(clientContext);
    auto endpoints = resolver.resolve(ip, port);
    tcp::socket privateSocket(clientContext);

    ClientLogger.log("Attempting connectinon to : "+ip+":" + port);
    boost::asio::connect(privateSocket, endpoints);
    boost::asio::write(privateSocket, boost::asio::buffer("Lolol, sup bro"));
    ClientLogger.log("Connection Successful");
    

    //ClientLogger.log("Connected to server.\n");

    


}