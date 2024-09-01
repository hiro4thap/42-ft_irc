#include "ft_irc.hpp"
#define PORT 8080
// C++ program to show the example of server application in
// socket programming
#include <cstring>
#include <iostream>
#include <netinet/in.h>

int main()
{
    // creating socket
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);

    // specifying the address
    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(8080);
    serverAddress.sin_addr.s_addr = INADDR_ANY;
	struct sockaddr	*pServerAddress = reinterpret_cast<struct sockaddr *>(&serverAddress);

    // binding socket.
    if (bind(serverSocket, pServerAddress, sizeof(serverAddress)) == -1)
	{
		perror("bind");
		return 1;
	}

    // listening to the assigned socket
    if (listen(serverSocket, 5) == -1)
	{
		perror("listen");
		return 1;
	}

    // accepting connection request
    int clientSocket = accept(serverSocket, nullptr, nullptr);

    // recieving data
    char buffer[1024] = { 0 };
	while (true)
	{
		memset(buffer, 0, 1024);
		recv(clientSocket, buffer, sizeof(buffer), 0);
		std::cout << "Message from client: " << buffer << std::endl;
		if (!strncmp(buffer, "EOF", 4))
			break ;
	}

    // closing the socket.
    close(serverSocket);

    return 0;
}


