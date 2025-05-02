
#include "../include/PeerServer.h"
#include "../include/PeerClient.h"


#include<iostream>


int main()
{
    bool role = false;
    std::cin >> role;


    try
    {

        // For server role
        if (!role) {
            PeerServer ps;
            ps.StartServer();
        }

        // For Client role
        else {
            
            std::string username;
            std::cin.ignore();
            std::getline(std::cin, username);
            PeerClient pc(username);
            //pc.connect(username);
        }
    }

    catch (std::exception& e)
    {
        std::printf("Exception: %s\\n", e.what());
    }


}
    