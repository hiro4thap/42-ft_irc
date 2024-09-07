#include "Server.hpp"

Server::Server()
{
}

Server::~Server()
{
	delete[] _pfds;
}

Server::Server(unsigned int port, std::string passwrod):
	_port(port), _password(passwrod), _capacity(5), _size(0)
{
	_pfds = new struct pollfd[_capacity];
}


int	Server::getSocketFd()
{
    // creating socket
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);

	// convert socket non-blocking
	fcntl(serverSocket, F_SETFL, O_NONBLOCK);

    // specifying the address
    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(_port);
    serverAddress.sin_addr.s_addr = INADDR_ANY;

    // binding socket.
    if (bind(serverSocket, reinterpret_cast<struct sockaddr *>(&serverAddress), sizeof(serverAddress)) == -1)
	{
		perror("bind");
		return -1;
	}

    // listening to the assigned socket
    if (listen(serverSocket, 5) == -1)
	{
		perror("listen");
		return -1;
	}

	_pfds[0].fd = serverSocket;
	_pfds[0].events = POLLIN;
	_size = 1;

	return serverSocket;
}

void	Server::launch(int serverSocket)
{
	while (true)
	{
		poll(_pfds, _size, -1);
		for (unsigned int i = 0; i < _size; i++)
		{
			if (_pfds[i].revents != POLLIN)
				continue ;
			// accepting connection request
			if (_pfds[i].fd == serverSocket)
			{
				std::cout << "Client " << _size << " is accepted" << std::endl;
				int clientSocket = accept(serverSocket, nullptr, nullptr);
				if (clientSocket == -1)
				{
					perror("accept");
					break ;
				}
				addToPfds(clientSocket);
			}
			// recieving data
			else
			{
				char buffer[1024] = { 0 };
				if (recv(_pfds[i].fd, buffer, sizeof(buffer), 0) == -1)
				{
					perror("recv");
					return;
				}
				std::cout << "Message from client " << i << " :" << buffer << std::endl;
				processCommand(buffer, _pfds[i].fd);
				// for (unsigned int j = 0; j < _size; j++)
				// {
				// 	if (_pfds[j].fd != serverSocket && _pfds[j].fd != _pfds[i].fd)
				// 	{
				// 		if (send(_pfds[j].fd, buffer, sizeof(buffer), 0) == -1)
				// 			perror("send");
				// 	}
				// }
			}
		}
	}
    // closing the socket.
    close(serverSocket);
}

void	Server::addToPfds(int fd)
{
	if (_size == _capacity)
	{
		_capacity *= 2;
		struct pollfd	*new_pfds = new struct pollfd[_capacity];
		for (unsigned int i = 0; i < _size; i++)
			new_pfds[i] = _pfds[i];
		delete[] _pfds;
		_pfds = new_pfds;
	}
	_pfds[_size].fd = fd;
	_pfds[_size].events = POLLIN;
	_size++;
}

void	Server::delFromPfds(int index)
{
	_pfds[index] = _pfds[_size - 1];
	_size--;
}

bool	Server::checkPassword(const std::string &password) const
{
	return (password == _password);
}

static std::vector<std::string> tokenise(std::string input)
{
	std::vector<std::string> tokens;

	std::stringstream ss(input);

	std::string token;
	while (getline(ss, token, ' '))
	{
		if (!token.empty())
		{
			tokens.push_back(token);
		}
	}
	return tokens;
}

void	Server::sendClient(std::string response, int toFd, int fromFd)
{
	int serverSocket = _pfds[0].fd;
	if (toFd != serverSocket && toFd != fromFd)
	{
		if (send(toFd, response.c_str(), response.size(), 0) == -1)
			perror("send");
	}

}

void	Server::processCommand(std::string command, int fromFd)
{
	std::vector<std::string> tokens = tokenise(command);
	// NICK command
	// void			setNickname(const std::string &nickname, int fd);
	if (tokens[0] == "NICK")
	{
		setNickname(tokens[1], fromFd);
	}

	// JOIN command
	// void			joinChannel(const std::string &channel, int fd, const std::string &password = "");
	if (tokens[0] == "JOIN")
	{
		if (tokens.size() > 2)
			joinChannel(tokens[1], fromFd, tokens[2]);
		else
			joinChannel(tokens[1], fromFd);
	}

	// PRIVMSG command
	// void			sendToChannel(const std::string &channel, const std::string &message ,int fd);
	// void			sendToUser(const std::string &user, const std::string &message, int fd);
	if (tokens[0] == "PRIVMSG")
	{
		if (tokens[1].at(0) == '#' || tokens[1].at(0) == '&')
		{
			sendToChannel(tokens[1], command.substr(5 + tokens[1].size() + 1, std::string::npos), fromFd);
		}
		else
		{
			sendToUser(tokens[1], command.substr(5 + tokens[1].size() + 1, std::string::npos), fromFd);
		}
	}

	// KICK command
	// void			kickUser(const std::string &user, int fd, const std::string &comment = "");&
	

	// INVITE command
	// void			inviteUser(const std::string &channel, const std::string &user, int fd);


	// TOPIC command
	// void			setTopic(const std::string &channel, int fd, const std::string topic = "");


	// MODE command
	// void			setMode(const std::string &channel, const char mode, int fd, const std::string &limit, const std::string &user);
	
	
	// PART command
	// void			leaveChannel(const std::string &channel, int fd);
	
	
	// QUIT command
	// void			quitServer(int fd, const std::string &comment = "");


}