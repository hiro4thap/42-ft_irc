#include "../inc/Server.hpp"

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

void	Server::sendClient(std::string message, int toFd, int fromFd)
{
	std::string response = message + "\r\n";
	int serverSocket = _pfds[0].fd;
	if (toFd != serverSocket && toFd != fromFd)
	{
		if (send(toFd, response.c_str() , response.size(), 0) == -1)
			perror("send");
	}
}

void	Server::processCommand(std::string command, int fromFd)
{
	std::vector<std::string> tokens = tokenise(command.substr(0, command.size() - 2));
	// NICK command
	// void			setNickname(const std::string &nickname, int fd);
	if (tokens[0] == "NICK")
	{
		setNickname(tokens[1], fromFd);
	}

	// JOIN command
	// void			joinChannel(const std::string &channel, int fd, const std::string &password = "");
	else if (tokens[0] == "JOIN")
	{
		if (tokens.size() > 2)
			joinChannel(tokens[1], fromFd, tokens[2]);
		else
			joinChannel(tokens[1], fromFd);
	}

	// PRIVMSG command
	// void			sendToChannel(const std::string &channel, const std::string &message ,int fd);
	// void			sendToUser(const std::string &user, const std::string &message, int fd);
	else if (tokens[0] == "PRIVMSG")
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
	else if (tokens[0] == "KICK")
	{
		if (tokens.size() > 2)
			kickUser(tokens[1], tokens[2], fromFd, command.substr(5 + tokens[1].size() + tokens[2].size() + 2, std::string::npos));
		else
			kickUser(tokens[1], tokens[2], fromFd);
	}

	// INVITE command
	// void			inviteUser(const std::string &channel, const std::string &user, int fd);
	else if (tokens[0] == "INVITE")
	{
		inviteUser(tokens[1], tokens[2], fromFd);
	}

	// TOPIC command
	// void			setTopic(const std::string &channel, int fd, const std::string topic = "");
	else if (tokens[0] == "TOPIC")
	{
		if (tokens.size() > 2)
			setTopic(tokens[1], fromFd, tokens[2]);
		else
			setTopic(tokens[1], fromFd);
	}	

	// MODE command
	// void			setMode(const std::string &channel, const char mode, int fd, const std::string &limit, const std::string &user);
	else if (tokens[0] == "MODE")
	{
		setMode(tokens[1], tokens[2], fromFd, command.substr(tokens[0].size() + tokens[1].size() + tokens[2].size() + 3, std::string::npos));
	}

	// PART command
	// void			leaveChannel(const std::string &channel, int fd, const std::string &reason);
	else if (tokens[0] == "PART")
	{
		if (tokens.size() > 2)
			leaveChannel(tokens[1], fromFd, command.substr(tokens[0].size() + tokens[1].size() + 2, std::string::npos));
		else
			leaveChannel(tokens[1], fromFd);
	}

	// QUIT command
	// void			quitServer(int fd, const std::string &comment = "");
	else if (tokens[0] == "QUIT")
	{
		if (tokens.size() > 1)
			quitServer(fromFd, tokens[1]);
		else
			quitServer(fromFd);
	}

}

bool Server::valueExits(const std::string &value)
{
	for (std::map<int, std::string>::iterator it = this->_users.begin(); it != this->_users.end(); it++)
	{
		if (value == it->second)
			return true;
	}
	return false;
}

int Server::findKey(const std::string &value)
{
	for (std::map<int, std::string>::iterator it = this->_users.begin(); it != this->_users.end(); it++)
	{
		if (value == it->second)
			return it->first;
	}
	return -1;
}

static bool checkValidName(const std::string &nickname)
{
	return true;
}

void	Server::setNickname(const std::string &nickname, int fromFd)
{
	// 431 ERR_NONICKNAMEGIVEN
	if (nickname == "")
	{
		sendClient(":No nickname given", fromFd, _pfds->fd);
	}
	// 433 ERR_NICKNAMEINUSE
	else if (valueExits(nickname))
	{
		sendClient(nickname + " :Nickname is already in use", fromFd, _pfds->fd);
	}
	// 432 ERR_ERRONEUSNICKNAME
	else if (checkValidName(nickname) == false)
	{
		sendClient(_users[fromFd] + " :Erroneus nickname", fromFd, _pfds->fd);
	}
	// 436 ERR_NICKCOLLISION not implemented
	// RESPONSE
	else
	{
		sendClient(":" + _users[fromFd] + " NICK " + nickname, fromFd, _pfds->fd);
	}
}

void	Server::joinChannel(const std::string &channel, int fd, const std::string &password)
{
	Channel *ch;
	// 461 ERR_NEEDMOREPARAMS
	if (channel == "")
	{
		std::string message = "JOIN :Not enough parameters";
		sendClient(message, fd, _pfds->fd);
	}
	// 476 ERR_BADCHANMASK
	else if (Channel::validChannelName(channel) == false)
	{
		// std::string message = channel + " :Bad Channel ";
		std::string message = channel + " :No such channel";
		sendClient(message, fd, _pfds->fd);
	}
	// 403 ERR_NOSUCHCHANNEL
	// else if (channelExists(channel, &ch) == false)
	// {
	// 	std::string message = channel + " :No such channel";
	// 	sendClient(message, fd, _pfds->fd);
	// }
	// 405 ERR_TOOMANYCHANNELS not supported
	// 475 ERR_BADCHANNELKEY
	else if (ch->getHasPassword() && ch->checkPassword(password) == false)
	{
		std::string message = channel + " :Cannot join channel (+k)";
		sendClient(message, fd, _pfds->fd);
	}
	// 474 ERR_BANNEDFROMCHAN not supported
	// 471 ERR_CHANNELISFULL
	else if (ch->getHasLimit() && ch->getUsers().size() >= ch->getLimit())
	{
		std::string message = channel + " :Cannot join channel (+l)";
		sendClient(message, fd, _pfds->fd);
	}
	// 473 ERR_INVITEONLYCHAN
	else if (ch->getIsInviteOnly() && Channel::containsUser(ch->getInvitedUsers(), _users[fd]) == false)
	{
		std::string message = channel + " :Cannot join channel (+i)";
		sendClient(message, fd, _pfds->fd);
	}
	else
	{
		
		std::string message = ":" + _users[fd] + " JOIN " + channel;
		sendClient(message, fd, _pfds->fd);

		// 332 RPL_TOPIC
		// 333 RPL_TOPICWHOTIME
		// 353 RPL_NAMREPLY
		// 366 RPL_ENDOFNAMES

	}
}

void	Server::setMode(const std::string &channel, const std::string mode, int fd, const std::string &parameters)
{

}

bool	Server::channelExists(const std::string &channel_str, Channel **channel)
{
	for (std::vector<Channel>::iterator it = this->_channels.begin(); it != this->_channels.end(); it++)
	{
		if (it->getName() == channel_str)
		{
			if (channel != 0)
			{
				*channel = &(*it);
			}
			return true;
		}
	}
	return false;
}

const std::vector<std::string>::iterator Server::findIn(std::string str, std::vector<std::string> vec)
{
	return (std::find(vec.begin(), vec.end(), str));
}
