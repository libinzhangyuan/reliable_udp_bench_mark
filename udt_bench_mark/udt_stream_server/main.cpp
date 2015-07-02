#include <arpa/inet.h>
#include <udt.h>
#include <iostream>
#include <cstring>

#include <sys/socket.h>
#include <signal.h>

#include "udt_server.h"

int main(int argc, char* argv[])
{
    try
    {
        // Check command line arguments.
        if (argc != 3)
        {
            std::cerr << "Usage: server_udt <address> <port>\n";
            std::cerr << "  For IPv4, try:\n";
            std::cerr << "    battle_server 0.0.0.0 80\n";
            std::cerr << "  For IPv6, try:\n";
            std::cerr << "    battle_server 0::0 80\n";
            return 1;
        }
    }
    catch (std::exception& e)
    {
        std::cerr << "exception: " << e.what() << "\n";
    }

    std::cout << "run battle_server_udt on " << argv[1] << ':' << argv[2] << std::endl;

	//ignore SIGPIPE
	sigset_t ps;
	sigemptyset(&ps);
	sigaddset(&ps, SIGPIPE);
	pthread_sigmask(SIG_BLOCK, &ps, NULL);

    auto server = std::make_shared<UDTServer>();
    server->Run(std::atoi(argv[2]));
    return 1;
}
