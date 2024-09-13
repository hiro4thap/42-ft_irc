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

Channel	*Server::getChannelByName(const std::string &name)
{
	for (std::size_t i = 0; i < _channels.size(); i++)
	{
		if(_channels[i].getName() == name)
			return &_channels[i];
	}
	return NULL;
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
	*channel = NULL;
	return false;
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

void	Server::sendClient(std::string message, int toFd)
{
	std::string response = message + "\r\n";
	int serverSocket = _pfds[0].fd;
	if (toFd != serverSocket)
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
		if (tokens.size() == 1)
			inviteUser("", "", fromFd);
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
		if (tokens.size() == 2) //TODO: return modes which are on currently
			return ;
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

bool Server::userExists(const std::string &value)
{
	for (std::map<int, std::string>::iterator it = this->_users.begin(); it != this->_users.end(); it++)
	{
		if (value == it->second)
			return true;
	}
	return false;
}

int Server::getUserFd(const std::string &value)
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
	(void) nickname;
	return true;
}

void	Server::setNickname(const std::string &nickname, int fromFd)
{
	// 431 ERR_NONICKNAMEGIVEN
	if (nickname == "")
	{
		sendClient(":No nickname given", fromFd);
	}
	// 433 ERR_NICKNAMEINUSE
	else if (userExists(nickname))
	{
		sendClient(nickname + " :Nickname is already in use", fromFd);
	}
	// 432 ERR_ERRONEUSNICKNAME
	else if (checkValidName(nickname) == false)
	{
		sendClient(_users[fromFd] + " :Erroneus nickname", fromFd);
	}
	// 436 ERR_NICKCOLLISION not implemented
	// RESPONSE
	else if (_users.find(fromFd) == _users.end())
	{
		sendClient(":server 001 " + nickname, fromFd);
		_users[fromFd] = nickname;
	}
	else
	{
		sendClient(":" + _users[fromFd] + " NICK " + nickname, fromFd);
		sendAllClients(":" + _users[fromFd] + " NICK " + nickname, fromFd);
		_users[fromFd] = nickname;
	}
}

void	Server::joinChannel(const std::string &channel, int fd, const std::string &password)
{
	Channel *ch;
	// 461 ERR_NEEDMOREPARAMS
	if (channel == "")
	{
		std::string message = "JOIN :Not enough parameters";
		sendClient(message, fd);
	}
	// 476 ERR_BADCHANMASK
	else if (Channel::validChannelName(channel) == false)
	{
		// std::string message = channel + " :Bad Channel ";
		std::string message = channel + " :No such channel";
		sendClient(message, fd);
	}
	// 403 ERR_NOSUCHCHANNEL
	// else if (channelExists(channel, &ch) == false)
	// {
	// 	std::string message = channel + " :No such channel";
	// 	sendClient(message, fd);
	// }
	// 405 ERR_TOOMANYCHANNELS not supported
	// 475 ERR_BADCHANNELKEY
	else if (channelExists(channel, &ch) && ch->getHasPassword() && ch->checkPassword(password) == false)
	{
		std::string message = channel + " :Cannot join channel (+k)";
		sendClient(message, fd);
	}
	// 474 ERR_BANNEDFROMCHAN not supported
	// 471 ERR_CHANNELISFULL
	else if (ch && ch->getHasLimit() && ch->getUsers().size() >= ch->getLimit())
	{
		std::string message = channel + " :Cannot join channel (+l)";
		sendClient(message, fd);
	}
	// 473 ERR_INVITEONLYCHAN
	else if (ch && ch->getIsInviteOnly() && Channel::containsUser(ch->getInvitedUsers(), _users[fd]) == false)
	{
		std::string message = channel + " :Cannot join channel (+i)";
		sendClient(message, fd);
	}
	// 332 RPL_TOPIC
	// TODO: 333 RPL_TOPICWHOTIME
	// 353 RPL_NAMREPLY
	// 366 RPL_ENDOFNAMES
	else if (!ch) //create new one
	{
		Channel new_channel(channel);
		new_channel.addUser(_users[fd]);
		new_channel.addOperator(_users[fd]);
		_channels.push_back(new_channel);
		sendChannel(":" + _users[fd] + " JOIN " + channel, channel, fd);
		sendClient(":" + _users[fd] + " JOIN " + channel, fd);
		if (new_channel.getTopic().empty())
			sendClient(":server 331 " + _users[fd] + " " + channel + " :" + "No topic is set", fd);
		else
			sendClient(":server 332 " + _users[fd] + " " + channel + " :" + ch->getTopic(), fd);
		sendClient(":server 353 " + _users[fd] + " = " + channel + " :@" + _users[fd], fd);
		sendClient(":server 366 " + _users[fd] + " " + channel + " :End of NAMES list", fd);
	}
	else
	{
		if (Channel::containsUser(ch->getInvitedUsers(), _users[fd]))
			ch->removeInvitedUser(_users[fd]);
		ch->addUser(_users[fd]);
		sendChannel(":" + _users[fd] + " JOIN " + channel, channel, fd);
		sendClient(":" + _users[fd] + " JOIN " + channel, fd);
		if (ch->getTopic().empty())
			sendClient(":server 331 " + _users[fd] + " " + channel + " :" + "No topic is set", fd);
		else
			sendClient(":server 332 " + _users[fd] + " " + channel + " :" + ch->getTopic(), fd);
		sendClient(":server 353 " + _users[fd] + " = " + channel + " :" + getNameList(ch), fd);
		sendClient(":server 366 " + _users[fd] + " " + channel + " :End of NAMES list", fd);
	}
}

void	Server::setMode(const std::string &channel, const std::string mode, int fd, const std::string &parameters)
{
	(void) channel;
	(void) mode;
	(void) fd;
	(void) parameters;
	Channel *ch;

	// 403 ERR_NOSUCHCHANNEL
	if (channelExists(channel, &ch) == false)
	{
		std::string message = channel + " :No such channel";
		sendClient(message, fd);
	}

	// 442 ERR_NOTONCHANNEL
	else if (Channel::containsUser(ch->getUsers(), _users[_pfds->fd]) == false)
	{
		std::string message = channel + " :You're not on that channel";
		sendClient(message, fd);
	}

	// 482 ERR_CHANOPRIVSNEEDED
	else if (Channel::containsUser(ch->getOperators(), _users[_pfds->fd]) == false)
	{
		std::string message = channel + " :You're not channel operator";
		sendClient(message, fd);
	}

	// 324 RPL_CHANNELMODEIS
	else if (mode == "")
	{
		std::string message = channel;
		std::string mode_list = "";
		if (ch->getIsInviteOnly())
			mode_list += "i";
		if (ch->getHasRestrictTopic())
			mode_list += "t";
		if (ch->getHasPassword())
			mode_list += "k";
		if (ch->getHasLimit())
			mode_list += "l";
		if (mode_list.size() > 0)
			message += " +" + mode_list;
		sendClient(message, fd);
	}

	// 461 ERR_NEEDMOREPARAMS
	// 472 ERR_UNKNOWNMODE
	// 401 ERR_NOSUCHNICK
	// 467 ERR_KEYSET
	// 502 ERR_USERSDONTMATCH
	// 501 ERR_UMODEUNKNOWNFLAG
	
	
	// 367 RPL_BANLIST
	// 368 RPL_ENDOFBANLIST
}

void	Server::sendAllClients(std::string response, int fromFd)
{
	for (std::size_t i = 0; i < _users.size(); i++)
	{
		int fd = getUserFd(_users[i]);
		if (fd == fromFd)
			continue ;
		sendClient(response, fd);
	}
}

void	Server::sendToChannel(const std::string &channel, const std::string &message ,int fd)
{
	sendChannel(":" + _users[fd] + " PRIVMSG " + message, channel, fd);
}

void	Server::sendToUser(const std::string &user, const std::string &message, int fd)
{
	sendClient(":" + _users[fd] + " PRIVMSG " + message, getUserFd(user));
}

// KICK command
void	Server::kickUser(const std::string &channel, const std::string &user, int fd, const std::string &comment)
{
	Channel *ch;

	// 476 BADCHANMASK
	if (Channel::validChannelName(channel) == false)
	{
		std::string message = channel + " :Bad Channel ";
		sendClient(message, fd);
	}

	// 403 ERR_NOSUCHCHANNEL
	else if (channelExists(channel, &ch) == false)
	{
		std::string message = channel + " :No such channel";
		sendClient(message, fd);
	}

	// 442 ERR_NOTONCHANNEL
	else if (Channel::containsUser(ch->getUsers(), _users[_pfds->fd]) == false)
	{
		std::string message = channel + " :You're not on that channel";
		sendClient(message, fd);
	}

	// 482 ERR_CHANOPPRIVSNEED
	else if (Channel::containsUser(ch->getOperators(), _users[_pfds->fd]) == false)
	{
		std::string message = channel + " :You're not channel operator";
		sendClient(message, fd);
	}

	// 462 ERR_NEEDMOREPARAMS
	
	// 
	else
	{
		ch->removeUser(user);
		std::string message = "KICK" + channel + " " + user;
		if (comment.size() > 0)
			message += " " + comment;
		sendAllClients(message, fd);
	}
}

// INVITE command
void	Server::inviteUser(const std::string &channel, const std::string &user, int fd)
{
	Channel	*ch;
	// 336 RPL_INVITELIST
	// 337 RPL_ENDOFINVITELIST
	if (channel.empty() && user.empty())
	{
		std::string message = ":server 336 " + _users[fd] + getInvitedChannels(_users[fd]);
		sendClient(message, fd);
		message = ":server 337 " + _users[fd] + " :End of /INVITE list";
		sendClient(message, fd);
	}
	// 403 ERR_NOSUCHCHANNEL
	else if (channelExists(channel, &ch) == false)
	{
		std::string message = ":server 403 " + channel + " :No such channel";
		sendClient(message, fd);
	}
	// 461 ERR_NEEDMOREPARAMS
	// 442 ERR_NOTONCHANNEL
	else if (ch && Channel::containsUser(ch->getUsers(), _users[fd]) == false)
	{
		std::string message = ":server 442 " + _users[fd] + " " + channel + " :You're not on that channel";
		sendClient(message, fd);
	}
	// 482 ERR_CHANOPRIVSNEEDED
	else if (ch && ch->getIsInviteOnly() && Channel::containsUser(ch->getOperators(), _users[fd]) == false)
	{
		std::string message = ":server 482 " + channel + " :You're not channel operator";
		sendClient(message, fd);
	}
	// 341 RPL_INVITING
	else if (ch && Channel::containsUser(ch->getInvitedUsers(), user))
	{
		std::string message = ":server 341 " + _users[fd] + " " + user + " " + channel;
		sendClient(message, fd);
	}
	// 443 ERR_USERONCHAN
	else if (ch && Channel::containsUser(ch->getUsers(), user))
	{
		std::string message = ":server 443 " + channel + " :You're not channel operator";
		sendClient(message, fd);
	}
	// RESPONSE
	else
	{
		ch->addInvitedUser(user);
		std::string message = ":" + _users[fd] + " INVITE " + user + " " + channel;
		sendClient(message, fd);
		sendClient(message, getUserFd(user));
	}
}

// TOPIC command
void	Server::setTopic(const std::string &channel, int fd, const std::string topic)
{
	Channel *ch;
	// 461 ERR_NEEDMOREPARAMS
	// if (topic.empty())
	// {
	// 	std::string message = "JOIN :Not enough parameters";
	// 	sendClient(message, fd);
	// }
	// 403 ERR_NOSUCHCHANNEL
	if (channelExists(channel, &ch) == false)
	{
		std::string message = channel + " :No such channel";
		sendClient(message, fd);
	}
	// 442 ERR_NOTONCHANNEL
	else if (ch && Channel::containsUser(ch->getUsers(), _users[fd]) == false)
	{
		std::string message = channel + " :You're not on that channel";
		sendClient(message, fd);
	}

	// 331 RPL_NOTOPIC
	// 332 RPL_TOPIC
	// TODO: 333 RPL_TOPICWHOTIME
	else if (topic.empty())
	{
		if (ch->getTopic().empty())
			sendClient(":server 331 " + _users[fd] + " " + channel + " :" + "No topic is set", fd);
		else
			sendClient(":server 332 " + _users[fd] + " " + channel + " :" + ch->getTopic(), fd);
	}
	// 482 ERR_CHANOPRIVSNEEDED
	else if (ch && ch->getHasRestrictTopic() && Channel::containsUser(ch->getOperators(), _users[fd]) == false)
	{
		std::string message = channel + " :You're not channel operator";
		sendClient(message, fd);
	}
	else
	{
		ch->setTopic(topic.substr(1, topic.size() - 1));
		sendClient(":" + _users[fd] + " TOPIC " + channel + " " + ch->getTopic(), fd);
		sendChannel(":" + _users[fd] + " TOPIC " + channel + " " + topic, channel, fd);
	}
}

// PART command
void	Server::leaveChannel(const std::string &channel, int fd, const std::string &reason)
{
	(void) channel;
	(void) fd;
	(void) reason;
}

// QUIT command
void	Server::quitServer(int fd, const std::string &comment)
{
	(void) fd;
	(void) comment;
}

void	Server::sendChannel(std::string response, const std::string &channel, int fromFd)
{
	Channel *cnl = getChannelByName(channel);
	if (!cnl)
		return ;
	std::vector<std::string> users = cnl->getUsers();
	for (std::size_t i = 0; i < users.size(); i++)
	{
		int fd = getUserFd(users[i]);
		if (fd == fromFd)
			continue ;
		sendClient(response, fd);
	}
}

const std::string	Server::getNameList(const Channel *channel) const
{
	std::vector<std::string> operators = channel->getOperators();
	std::vector<std::string> users = channel->getUsers();

	std::stringstream ss;
	for (std::size_t i = 0; i < operators.size(); i++)
	{
		ss << "@" << users[i] << " ";
	}
	for (std::size_t i = 0; i < users.size(); i++)
	{
		if (std::find(operators.begin(), operators.end(), users[i]) == operators.end())
			ss << users[i] << " ";
	}
	return ss.str();
}

const std::string	Server::getInvitedChannels(const std::string &user) const
{
	std::string channels = "";
	for (std::size_t i = 0; i <_channels.size(); i++)
	{
		if (Channel::containsUser(_channels[i].getInvitedUsers(), user))
		{
			channels += " ";
			channels += _channels[i].getName();
		}
	}
	return channels;
}
