#include "../include/PeerServer.h"
#include "../include/PeerClient.h"


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
            PeerClient pc;
            pc.connect();
        }
    }

    catch (std::exception& e)
    {
        std::printf("Exception: %s\\n", e.what());
    }


}
