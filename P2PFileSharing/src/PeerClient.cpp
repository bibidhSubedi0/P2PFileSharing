
#include "../include/PeerClient.h"
#include <boost/asio/ip/address.hpp>
#include <boost/asio/ip/address_v4.hpp>
#include <sstream>
#include<queue>
#include <format>
#include <filesystem>
#include <utils.h>


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

    connectToAll(queryForPeers());

    clientContext.run();
    input_thread.join();

}

void PeerClient::mainLoop()
{
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
            ClientLogger.log("Connection to the server broken");
            break;
        }
        if (cm == "list") {
            std::string list = queryForPeers();
            ClientLogger.log("Peer List:\n" + list);
            continue;
        }

        if (cm == "upload") {
            readFilesFromPc(msg);
            continue;
            
        }
        if ((cm == "send")){
            size_t pos = msg.find(' ');
            msg = msg.substr(pos + 1);
            pos = msg.find(' ');
            msg = msg.substr(pos + 1);
            sendMessageToPeers(commands[1], "TEXTMSG:"+msg);
            continue;
        }


        else {
            ClientLogger.log("Invalid Command!");
            continue;
        }


    }
}

void PeerClient::readFilesFromPc(std::string path) {
    size_t pos = path.find(' ');
    path = path.substr(pos + 1);
    std::replace(path.begin(), path.end(), '\\', '/');

    ClientLogger.log("Reading from " + path);

    // Extract filename
    std::filesystem::path fsPath(path);
    std::string filename = fsPath.filename().string();

    size_t chunkSize = 1024;

    std::vector<std::string> peer_names;
    {
        std::lock_guard<std::mutex> lock(_peerMutex);
        for (const auto& [key, socket] : _connectedPeers) {
            size_t at_pos = key.find('@');
            std::string peer_name = key.substr(0, at_pos);
            peer_names.push_back(peer_name);
        }
    }

    std::ifstream file(path, std::ios::binary);
    if (!file) {
        ClientLogger.log("Cannot open file: " + path);
        return;
    }

    // Send filename metadata first
    for (const auto& peer_name : peer_names) {
        sendMessageToPeers(peer_name, "CMD:__file_Start__");
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
        sendMessageToPeers(peer_name, "FILENAME:" + filename);
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }

    int index = 0;
    while (!file.eof()) {
        std::string buffer(chunkSize, '\0');
        file.read(&buffer[0], chunkSize);
        std::streamsize bytesRead = file.gcount();

        if (bytesRead == 0) break;

        std::string chunkName = "chunk_" + std::to_string(index++);

        for (const auto& peer_name : peer_names) {
            sendMessageToPeers(peer_name, "CMD:__file_Packet__");
            std::this_thread::sleep_for(std::chrono::milliseconds(2));
            sendMessageToPeers(peer_name, "META:" + chunkName);
            std::this_thread::sleep_for(std::chrono::milliseconds(2));
            sendMessageToPeers(peer_name, "DATA:" + buffer.substr(0, bytesRead));
            std::this_thread::sleep_for(std::chrono::milliseconds(2));
        }
    }

    for (const auto& peer_name : peer_names) {
        sendMessageToPeers(peer_name, "CMD:__file_Complete__");
    }

    ClientLogger.log("File transfer complete! Sent " + std::to_string(index) + " chunks");
}



void PeerClient::sendMessageToPeers(const std::string& cm, const std::string& msg) 
{
    
    const std::string& target = cm;
    std::string content = msg;

    std::shared_ptr<tcp::socket> peerSocket;
    {
        std::lock_guard<std::mutex> lock(_peerMutex);
        for (const auto& [key, socket] : _connectedPeers) {
            if (key.rfind(target 
                + "@", 0) == 0) {  // peer name match
                peerSocket = socket;
                break;
            }
        }
    }

    if (peerSocket && peerSocket->is_open()) {
        boost::asio::write(*peerSocket, boost::asio::buffer(content));
    }
    else {
        ClientLogger.log("Peer not connected: " + target);
    }

}


boost::asio::awaitable<void> PeerClient::listenForPeers() {

    boost::asio::ip::tcp::endpoint endpoint(boost::asio::ip::tcp::v4(), 0);
    boost::asio::ip::tcp::acceptor acceptor(clientContext, endpoint);
    boost::asio::ip::tcp::endpoint bound_endpoint = acceptor.local_endpoint();
    
    // Get the actual LAN IP
    boost::asio::ip::tcp::resolver resolver(clientContext);
    auto results = resolver.resolve(boost::asio::ip::host_name(), "");

    // Find the first non-loopback address
    std::string local_ip = "127.0.0.1";  // fallback
    for (const auto& entry : results) {
        boost::asio::ip::tcp::endpoint ep = entry.endpoint();
        if (ep.address().is_v4() && !ep.address().is_loopback()) {
            local_ip = ep.address().to_string();
            break;
        }
    }
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
            

            boost::asio::streambuf buffer;
            co_await async_read_until(peer_socket, buffer, "\n", boost::asio::use_awaitable);
            std::istream is(&buffer);
            std::string target_username;
            std::getline(is, target_username);

            ClientLogger.log("Accepted connection from: " + target_username);//+"\t" +
                /*peer_socket.remote_endpoint().address().to_string() + ":" +
                std::to_string(peer_socket.remote_endpoint().port()));*/




            // Handle peer communication asynchronously
            boost::asio::co_spawn(clientContext, CommWithPeers(std::move(peer_socket),target_username), boost::asio::detached);
        }
    }
    catch (std::exception& e) {
        std::cerr << "Peer acceptor exception: " << e.what() << "\n";
    }
}


std::string PeerClient::queryForPeers()
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
    disconnectFrom(socket);
    return response;
}


void PeerClient::connectToAll(std::string list)
{
    std::stringstream stream(list);
    std::string name;
    std::vector < std::string> p_l;
    while (std::getline(stream, name)) {
        
        if(name!=_username){
            std::cout << "Line: " << name << std::endl;
            p_l.push_back(name);
        }
    }

    for (auto p : p_l) {
        requestConnection(p);
    }
}


void PeerClient::requestConnection(std::string connect_to)
{

    std::vector<std::pair<std::string, std::string>> all_nodes;

    auto socket = connectTo("127.0.0.1", "55555", clientContext);
    sendMessageToServer(socket, _username + "\n");
    sendMessageToServer(socket, "conn_request|" + connect_to + "\n");


    char reply[1024];
    size_t reply_length = socket.read_some(boost::asio::buffer(reply));
    std::string str(reply, reply_length);

    std::istringstream stream(str);
    std::string ip;
    std::getline(stream, ip, ':');
    std::string port;
    std::getline(stream, port, ' ');

    // Now we have ip and port, make the connection to the client directly
    disconnectFrom(socket);


    // We are connecting with new socket
    tcp::resolver resolver(clientContext);
    auto endpoints = resolver.resolve(ip, port);
    tcp::socket privateSocket(clientContext);

    ClientLogger.log("Attempting connectinon to  "+connect_to+ " at " + ip + ":" + port);
    try
    {
        boost::asio::connect(privateSocket, endpoints);
        // Send username as well!
        boost::asio::write(privateSocket, boost::asio::buffer(_username+"\n"));
        ClientLogger.log("Connection Successful");\
    }
    catch (const std::exception& e) {
        std::cerr << "Peer connection failed : " << e.what() << std::endl;
    }

    boost::asio::co_spawn(clientContext,
        PeerClient::CommWithPeers(std::move(privateSocket), connect_to),
        boost::asio::detached);
    


}


boost::asio::awaitable<void> PeerClient::CommWithPeers(boost::asio::ip::tcp::socket peer_socket, std::string username) {
    try {
        auto remote_ep = peer_socket.remote_endpoint();
        std::string key = username + "@" + remote_ep.address().to_string() + ":" + std::to_string(remote_ep.port());

        {
            std::lock_guard<std::mutex> lock(_peerMutex);
            _connectedPeers[key] = std::make_shared<tcp::socket>(std::move(peer_socket));
        }

        ClientLogger.log("Connected to peer: " + key);

        // Create user's download folder
        std::string userFolder = "downloads_" + _username + "/" + username;
        std::filesystem::create_directories(userFolder);

        std::string buffer;
        char data[2048];

        std::string currentChunkName;
        std::string currentFilename;
        bool receivingFile = false;

        while (true) {
            std::size_t length = co_await _connectedPeers[key]->async_read_some(
                boost::asio::buffer(data), boost::asio::use_awaitable);
            buffer.append(data, length);

            while (true) {
                // Check for text messages
                std::size_t msg_pos = buffer.find("TEXTMSG:");
                if (msg_pos != std::string::npos) {
                    std::string textMsg = buffer.substr(msg_pos + 8);
                    ClientLogger.log("Received text from " + username + ": " + textMsg);
                    buffer.clear();
                    break;
                }

                // Find headers
                std::size_t cmd_pos = buffer.find("CMD:");
                std::size_t meta_pos = buffer.find("META:");
                std::size_t data_pos = buffer.find("DATA:");

                std::vector<std::pair<std::size_t, std::string>> headers;
                if (cmd_pos != std::string::npos) headers.emplace_back(cmd_pos, "CMD:");
                if (meta_pos != std::string::npos) headers.emplace_back(meta_pos, "META:");
                if (data_pos != std::string::npos) headers.emplace_back(data_pos, "DATA:");

                if (headers.empty()) break;

                std::sort(headers.begin(), headers.end());
                auto [start, type] = headers.front();

                std::size_t msg_length;
                if (headers.size() > 1) {
                    msg_length = headers[1].first - start;
                }
                else {
                    msg_length = buffer.length() - start;
                }

                std::string message = buffer.substr(start, msg_length);
                buffer.erase(0, start + msg_length);

                // Process messages
                if (type == "CMD:") {
                    std::string cmd = message.substr(4);

                    if (cmd == "__file_Packet__") {
                        receivingFile = true;
                        ClientLogger.log("Starting file reception from " + username);
                    }
                    else if (cmd == "__file_Complete__") {
                        if (receivingFile) {
                            // Reconstruct file
                            std::string transferKey = username + "_transfer";
                            FileTransfer& transfer = _activeTransfers[transferKey];

                            std::string outputPath = userFolder + "/received_file.bin";
                            std::ofstream outFile(outputPath, std::ios::binary);

                            if (outFile) {
                                // Sort and write chunks in order
                                for (int i = 0; i < transfer.totalChunks; ++i) {
                                    std::string chunkKey = "chunk_" + std::to_string(i);
                                    if (transfer.chunks.count(chunkKey)) {
                                        outFile.write(transfer.chunks[chunkKey].data(),
                                            transfer.chunks[chunkKey].size());
                                    }
                                }
                                outFile.close();
                                ClientLogger.log("File saved to: " + outputPath);
                            }

                            // Clean up
                            _activeTransfers.erase(transferKey);
                            receivingFile = false;
                        }
                        break;
                    }
                }
                else if (type == "META:") {
                    currentChunkName = message.substr(5);
                    ClientLogger.log("Receiving chunk: " + currentChunkName);
                }
                else if (type == "DATA:") {
                    std::string chunkData = message.substr(5);

                    if (receivingFile && !currentChunkName.empty()) {
                        std::string transferKey = username + "_transfer";
                        std::lock_guard<std::mutex> lock(_transferMutex);

                        _activeTransfers[transferKey].chunks[currentChunkName] = chunkData;

                        // Extract chunk index to track total chunks
                        size_t underscorePos = currentChunkName.find_last_of('_');
                        if (underscorePos != std::string::npos) {
                            int chunkIdx = std::stoi(currentChunkName.substr(underscorePos + 1));
                            if (chunkIdx >= _activeTransfers[transferKey].totalChunks) {
                                _activeTransfers[transferKey].totalChunks = chunkIdx + 1;
                            }
                        }

                        currentChunkName.clear();
                    }
                }

                if (buffer.empty()) break;
            }
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Peer communication failed: " << e.what() << std::endl;
    }
}

