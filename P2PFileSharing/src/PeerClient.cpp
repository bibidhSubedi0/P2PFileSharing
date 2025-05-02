
#include "../include/PeerClient.h"
#include <boost/asio/ip/address.hpp>
#include <boost/asio/ip/address_v4.hpp>
#include <sstream>
#include<queue>
/*



*/
void PeerClient::sendMessageToServer(tcp::socket& socket, std::string message)
{
    boost::asio::write(socket, boost::asio::buffer(message));
}
void disconnectFrom(tcp::socket& socket) {
    if (socket.is_open()) {
        boost::system::error_code ec;
        socket.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);
        socket.close(ec);
        // Optional: Handle error code if needed
    }
}



tcp::socket connectTo(const std::string& ip, const std::string& port, boost::asio::io_context& context) {

    tcp::resolver resolver(context);
    auto endpoints = resolver.resolve(ip, port);

    tcp::socket socket(context);
    boost::asio::connect(socket, endpoints);

    return socket;
}

PeerClient::PeerClient(std::string username)
{
    _username = username;

    // Register to the server
    auto socket = connectTo("127.0.0.1", "55555", clientContext);


    sendMessageToServer(socket, username+"\n");
    disconnectFrom(socket);


    // Ip and port assigned to this peer by the OS
    /*auto local_ep = socket.local_endpoint();
    std::string local_ip = local_ep.address().to_string();
    unsigned short local_port = local_ep.port();*/


    boost::asio::co_spawn(clientContext, listenForPeers(), boost::asio::detached);
    std::thread input_thread(&PeerClient::mainLoop, this);
    //std::thread input_thread([this]() { this->mainLoop(); });



    clientContext.run();
    input_thread.join();

}

void PeerClient::mainLoop()
{
    ClientLogger.log("Started the io thread");
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
            queryForPeers();
            continue;
        }
        if (cm == "connect" && commands.size() > 1) {
            //requestConnection(commands[1]);
            continue;
        }

        if (cm == "echo" && commands.size() > 1)
        {
            size_t spacePos = msg.find(' ');
            if (spacePos != std::string::npos) {
                msg.erase(0, spacePos + 1);
            }


            continue;
            //sendMessageToServer(socket, "echo|" + msg);

            // No continue cuz we are waiting for the reply
            // Good desgin? Fuck no! Does it work? yep
        }
        else {
            ClientLogger.log("Invalid Command!");
            continue;
        }


    }
}

boost::asio::awaitable<void> PeerClient::listenForPeers() {

    boost::asio::ip::tcp::endpoint endpoint(boost::asio::ip::tcp::v4(), 0);
    boost::asio::ip::tcp::acceptor acceptor(clientContext, endpoint);
    boost::asio::ip::tcp::endpoint bound_endpoint = acceptor.local_endpoint();
    std::string local_ip = bound_endpoint.address().to_string();
    int local_port = bound_endpoint.port();

    ClientLogger.log("Listening for peer connections on " + local_ip + ":" + std::to_string(local_port));
    
    auto temp_socket = connectTo("127.0.0.1", "55555", clientContext);
    sendMessageToServer(temp_socket, _username + "\n");
    sendMessageToServer(temp_socket, "peer_endpoint|");
    sendMessageToServer(temp_socket, local_ip+"|");
    sendMessageToServer(temp_socket, std::to_string(local_port) + "|\n");
    disconnectFrom(temp_socket);



    try {
        while (true) {
            // Async accept (non-blocking)
            boost::asio::ip::tcp::socket peer_socket = co_await acceptor.async_accept(boost::asio::use_awaitable);

            // Log accepted connection
            ClientLogger.log("Accepted connection from: " +
                peer_socket.remote_endpoint().address().to_string() + ":" +
                std::to_string(peer_socket.remote_endpoint().port()));

            // Handle peer communication asynchronously
            //boost::asio::co_spawn(clientContext, CommWithPeers(std::move(peer_socket)), boost::asio::detached);
        }
    }
    catch (std::exception& e) {
        std::cerr << "Peer acceptor exception: " << e.what() << "\n";
    }
}

void PeerClient::queryForPeers()
{
    auto socket = connectTo("127.0.0.1", "55555", clientContext);
    sendMessageToServer(socket, _username + "\n");
    sendMessageToServer(socket, "list|\n");
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
    disconnectFrom(socket);
}


/*

void PeerClient::handelConnections(tcp::socket& socket, const std::string& local_ip, unsigned short local_port) {

    // To listen to peers
    std::thread accept_thread(&PeerClient::listenForPeers, this, local_ip, local_port);
    accept_thread.detach();


    // To handel the connection with server for queries
    std::thread input_thread(&PeerClient::CommWithServer, this, std::ref(socket));

    
    clientContext.run();
    input_thread.join();
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
            //queryForPeers(socket);
            continue;
        }
        if (cm == "connect" && commands.size() > 1) {
            requestConnection(commands[1]);
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


void PeerClient::requestConnection(std::string connect_to)
{
    auto socket = connectTo("127.0.0.1", "55555", clientContext);
    sendMessageToServer(socket, _username + "\n");
    sendMessageToServer(socket, "conn_request|"+connect_to+"\n");

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

    
    // We are connecting with new socket
    tcp::resolver resolver(clientContext);
    auto endpoints = resolver.resolve(ip, port);
    tcp::socket privateSocket(clientContext);

    ClientLogger.log("Attempting connectinon to : "+ip+":" + port);
    boost::asio::connect(privateSocket, endpoints);
    boost::asio::write(privateSocket, boost::asio::buffer("Lolol, sup bro"));
    ClientLogger.log("Connection Successful");
    


}
*/