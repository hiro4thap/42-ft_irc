#include "ft_irc.hpp"
#define PORT 8080
// C++ program to show the example of server application in
// socket programming
#include <cstring>
#include <iostream>
#include <netinet/in.h>

void	printPollfd(struct pollfd fd[], int len)
{
	for (int i = 0; i < len; i++)
	{
		std::cout << "index: " << i << std::endl;
		std::cout << "fd: " << fd[i].fd << std::endl;
		std::cout << "events: " << fd[i].events << std::endl;
		std::cout << "revents: " << fd[i].revents << std::endl;
	}
}

int main()
{
    // creating socket
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
	// convert socket non-blocking
	fcntl(serverSocket, F_SETFL, O_NONBLOCK);

    // specifying the address
    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(8080);
    serverAddress.sin_addr.s_addr = INADDR_ANY;

    // binding socket.
    if (bind(serverSocket, reinterpret_cast<struct sockaddr *>(&serverAddress), sizeof(serverAddress)) == -1)
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

	struct pollfd	pfds[5];
	pfds[0].fd = serverSocket;
	pfds[0].events = POLLIN;
	int	fd_count = 1;

	while (true)
	{
		int	events = poll(pfds, fd_count, 20000);
		if (events == 0)
		{
			std::cout << "Poll timed out" << std::endl;
			break ;
		}
		for (int i = 0; i < fd_count; i++)
		{
			if (pfds[i].revents != POLLIN)
				continue ;
			// accepting connection request
			if (pfds[i].fd == serverSocket)
			{
				if (fd_count == 5)
				{
					std::cout << "already 4 clients connected" << std::endl;
					break ;
				}
				std::cout << "Client " << fd_count << " is accepted" << std::endl;
				int clientSocket = accept(serverSocket, nullptr, nullptr);
				if (clientSocket == -1)
				{
					perror("accept");
					break ;
				}
				pfds[fd_count].fd = clientSocket;
				pfds[fd_count].events = POLLIN;
				fd_count++;
			}
			// recieving data
			else
			{
				char buffer[1024] = { 0 };
				if (recv(pfds[i].fd, buffer, sizeof(buffer), 0) == -1)
				{
					perror("recv");
					return 1;
				}
				std::cout << "Message from client " << i << " :" << buffer << std::endl;
				if (!strncmp(buffer, "close", 5))
				{
					close(serverSocket);
					return 0;
				}
			}
		}
	}
    // closing the socket.
    close(serverSocket);

    return 0;
}


