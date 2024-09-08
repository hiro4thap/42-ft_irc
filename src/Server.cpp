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
				for (unsigned int j = 0; j < _size; j++)
				{
					if (_pfds[j].fd != serverSocket && _pfds[j].fd != _pfds[i].fd)
					{
						if (send(_pfds[j].fd, buffer, sizeof(buffer), 0) == -1)
							perror("send");
					}
				}
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

Channel	&Server::getChannelByName(const std::string &name)
{
	for (std::size_t i = 0; i < _channels.size(); i++)
	{
		if(_channels[i].getName() == name)
			return _channels[i];
	}
	return *(_channels.end());
}

int	Server::getFdByName(const std::string &name) const
{
	for (std::map<int, std::string>::const_iterator it = _users.cbegin(); it != _users.cend(); it++)
	{
		if (it->second == name)
			return it->first;
	}
	return -1;
}

void	Server::setNickname(const std::string &nickname, int fd)
{
	//TODO: check if nickname is unique
	if (_users.find(fd) == _users.end()) // new name
	{
		sendClient(":server 001 " + nickname, fd);
	}
	else
	{
		sendClient(":" + _users[fd] + " NICK " + nickname, fd);
	}
	_users[fd] = nickname;
}

void	Server::joinChannel(const std::string &channel, int fd, const std::string &password)
{
	Channel cnl = getChannelByName(channel);
	if (!password.empty() && cnl.getHasPassword())
	{
		if (!cnl.checkPassword(password))
			return ; // fail to join because of wrong password
	}
	if (cnl.getName().empty()) //create new one
	{
		Channel new_channel(channel);
		new_channel.addUser(_users[fd]);
		new_channel.addOperator(_users[fd]);
		_channels.push_back(new_channel);
		sendChannel(":" + _users[fd] + " JOIN " + channel, channel, fd);
		sendClient(":" + _users[fd] + " JOIN " + channel, fd);
		sendClient(":server 332 " + _users[fd] + " " + channel + " :" + new_channel.getTopic(), fd);
		sendClient(":server 353 " + _users[fd] + " = " + channel + " :members", fd); //TODO:get list of members
		sendClient(":server 366 " + _users[fd] + " " + channel + " :End of NAMES list", fd);
	}
	else
	{
		cnl.addUser(_users[fd]);
		sendChannel(":" + _users[fd] + " JOIN " + channel, channel, fd);
		sendClient(":" + _users[fd] + " JOIN " + channel, fd);
		sendClient(":server 332 " + _users[fd] + " " + channel + " :" + cnl.getTopic(), fd);
		sendClient(":server 353 " + _users[fd] + " = " + channel + " :members", fd); //TODO:get list of members
		sendClient(":server 366 " + _users[fd] + " " + channel + " :End of NAMES list", fd);
	}
}

//void	Server::sendToChannel(const std::string &channel, const std::string &message ,int fd);
//void	Server::sendToUser(const std::string &user, const std::string &message, int fd);
//void	Server::kickUser(const std::string &user, int fd, const std::string &comment = "");
//void	Server::inviteUser(const std::string &channel, const std::string &user, int fd);
//void	Server::setTopic(const std::string &channel, int fd, const std::string topic = "");
//void	Server::setMode(const std::string &channel, const char mode, int fd, const std::string &limit, const std::string &user);
//void	Server::leaveChannel(const std::string &channel, int fd);
//void	Server::quitServer(int fd, const std::string &comment = "");

void	Server::sendClient(std::string response, int toFd)
{
	int serverSocket = _pfds[0].fd;
	if (toFd != serverSocket)
	{
		if (send(toFd, response.c_str(), response.size(), 0) == -1)
			perror("send");
	}
}

void	Server::sendChannel(std::string response, const std::string &channel, int fromFd)
{
	(void)response;
	(void)fromFd;
	Channel cnl = getChannelByName(channel);
}
