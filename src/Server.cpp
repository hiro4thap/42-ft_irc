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
					break ;
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

void	Server::delFromPfds(int fromFd)
{
	for (std::size_t i = 0; i < _size; i++)
	{
		if (_pfds[i].fd != fromFd)
			continue ;
		close(_pfds[i].fd);
		_pfds[i] = _pfds[_size - 1];
		_size--;
	}	
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

// static std::vector<std::string> tokenise(std::string input)
// {
// 	std::vector<std::string> tokens;

// 	std::stringstream ss(input);

// 	std::string token;
// 	while (getline(ss, token, ' '))
// 	{
// 		if (!token.empty())
// 		{
// 			tokens.push_back(token);
// 		}
// 	}
// 	return tokens;
// }

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
	// if (command.size() >= 3 && command.substr(0,3) == "CAP")
	// 	return ;
	Command cmd;
	cmd.command = "";
	cmd.err_response = 0;
	cmd.threw_error = false;
	cmd.message_set = false;
	cmd.message = "";

	std::map<std::string, void(Server::*)(const Command&, int)> commands;
	commands["JOIN"] = &Server::joinChannel;
	commands["NICK"] = &Server::setNickname;
	commands["PRIVMSG"] = &Server::sendMessage;
	commands["KICK"] = &Server::kickUser;
	commands["INVITE"] = &Server::inviteUser;
	commands["TOPIC"] = &Server::processTopic;
	commands["MODE"] = &Server::processMode;
	commands["PART"] = &Server::leaveChannel;
	commands["QUIT"] = &Server::quitServer;

	Parser parser;

	parser.message(command, cmd);

	std::map<std::string, void(Server::*)(const Command&, int)>::iterator command_function = commands.find(cmd.command);
	if (command_function != commands.end())
	{
		(this->*(command_function->second))(cmd, fromFd);
	}

	// // QUIT command
	// // void			quitServer(int fd, const std::string &comment = "");
	// else if (tokens[0] == "QUIT")
	// {
	// 	if (tokens.size() > 1)
	// 		quitServer(fromFd, tokens[1]);
	// 	else
	// 		quitServer(fromFd);
	// 	delFromPfds(fromFd);
	// }

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

void	Server::setNickname(const Command &cmd, int fromFd)
{
	// 431 ERR_NONICKNAMEGIVEN
	if (cmd.users.size() < 1)
	{
		sendClient(":No nickname given", fromFd);
		return ;
	}
	std::string nickname = cmd.users[0];
	// 433 ERR_NICKNAMEINUSE
	if (userExists(nickname))
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

void	Server::joinChannel(const Command &cmd, int fromFd)
{
	Channel *ch;
	// 461 ERR_NEEDMOREPARAMS
	if (cmd.channels.size() == 0)
	{
		std::string message = "JOIN :Not enough parameters";
		sendClient(message, fromFd);
		return ;
	}
	// 476 ERR_BADCHANMASK
	std::string channel_name = "";
	if (cmd.channels.size() > 0)
		channel_name = cmd.channels[0];
	if (Channel::validChannelName(channel_name) == false)
	{
		// std::string message = channel + " :Bad Channel ";
		std::string message = channel_name + " :No such channel";
		sendClient(message, fromFd);
	}
	// 403 ERR_NOSUCHCHANNEL
	// else if (channelExists(channel, &ch) == false)
	// {
	// 	std::string message = channel + " :No such channel";
	// 	sendClient(message, fd);
	// }
	// 405 ERR_TOOMANYCHANNELS not supported
	// 475 ERR_BADCHANNELKEY
	else if (channelExists(channel_name, &ch) && ch->getHasPassword() && ch->checkPassword(cmd.keys[0]) == false)
	{
		std::string message = channel_name + " :Cannot join channel (+k)";
		sendClient(message, fromFd);
	}
	// 474 ERR_BANNEDFROMCHAN not supported
	// 471 ERR_CHANNELISFULL
	else if (ch && ch->getHasLimit() && ch->getUsers().size() >= ch->getLimit())
	{
		std::string message = channel_name + " :Cannot join channel (+l)";
		sendClient(message, fromFd);
	}
	// 473 ERR_INVITEONLYCHAN
	else if (ch && ch->getIsInviteOnly() && Channel::containsUser(ch->getInvitedUsers(), _users[fromFd]) == false)
	{
		std::string message = channel_name + " :Cannot join channel (+i)";
		sendClient(message, fromFd);
	}
	// 332 RPL_TOPIC
	// 333 RPL_TOPICWHOTIME
	// 353 RPL_NAMREPLY
	// 366 RPL_ENDOFNAMES
	else if (!ch) //create new one
	{
		Channel new_channel(channel_name);
		new_channel.addUser(_users[fromFd]);
		new_channel.addOperator(_users[fromFd]);
		_channels.push_back(new_channel);
		sendChannel(":" + _users[fromFd] + " JOIN " + channel_name, channel_name, fromFd);
		sendClient(":" + _users[fromFd] + " JOIN " + channel_name, fromFd);
		if (new_channel.getTopicSetAt().empty())
			sendClient(":server 331 " + _users[fromFd] + " " + channel_name + " :" + "No topic is set", fromFd);
		else
		{
			sendClient(":server 332 " + _users[fromFd] + " " + channel_name + " :" + new_channel.getTopic(), fromFd);
			sendClient(":server 333 " + _users[fromFd] + " " + channel_name + " " + new_channel.getTopicSetBy() + " " + new_channel.getTopicSetAt(), fromFd);
		}
		sendClient(":server 353 " + _users[fromFd] + " = " + channel_name + " :@" + _users[fromFd], fromFd);
		sendClient(":server 366 " + _users[fromFd] + " " + channel_name + " :End of NAMES list", fromFd);
	}
	else
	{
		if (Channel::containsUser(ch->getInvitedUsers(), _users[fromFd]))
			ch->removeInvitedUser(_users[fromFd]);
		ch->addUser(_users[fromFd]);
		sendChannel(":" + _users[fromFd] + " JOIN " + channel_name, channel_name, fromFd);
		sendClient(":" + _users[fromFd] + " JOIN " + channel_name, fromFd);
		if (ch->getTopicSetAt().empty())
			sendClient(":server 331 " + _users[fromFd] + " " + channel_name + " :" + "No topic is set", fromFd);
		else
		{
			sendClient(":server 332 " + _users[fromFd] + " " + channel_name + " :" + ch->getTopic(), fromFd);
			sendClient(":server 333 " + _users[fromFd] + " " + channel_name + " " + ch->getTopicSetBy() + " " + ch->getTopicSetAt(), fromFd);
		}
		sendClient(":server 353 " + _users[fromFd] + " = " + channel_name + " :" + getNameList(ch), fromFd);
		sendClient(":server 366 " + _users[fromFd] + " " + channel_name + " :End of NAMES list", fromFd);
	}
}

void	Server::processMode(const Command &cmd, int fromFd)
{
	Channel *ch;
	std::string channel_name = "";
	if (cmd.channels.size() > 0)
		channel_name = cmd.channels[0];

	// 403 ERR_NOSUCHCHANNEL
	if (channelExists(channel_name, &ch) == false)
	{
		std::string message = channel_name + " :No such channel";
		sendClient(message, fromFd);
	}

	// 442 ERR_NOTONCHANNEL
	else if (Channel::containsUser(ch->getUsers(), _users[_pfds->fd]) == false)
	{
		std::string message = channel_name + " :You're not on that channel";
		sendClient(message, fromFd);
	}

	// 482 ERR_CHANOPRIVSNEEDED
	else if (Channel::containsUser(ch->getOperators(), _users[_pfds->fd]) == false)
	{
		std::string message = channel_name + " :You're not channel operator";
		sendClient(message, fromFd);
	}

	// 324 RPL_CHANNELMODEIS
	else if (cmd.mode_operations.size() == 0)
	{
		std::string message = channel_name;
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
		sendClient(message, fromFd);
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
	for (std::size_t i = 0; i < _size; i++)
	{
		int fd = _pfds[i].fd;
		if (fd == fromFd)
			continue ;
		sendClient(response, fd);
	}
}

void	Server::sendMessage(const Command &cmd, int fromFd)
{
	if (cmd.users.size() > 0)
		sendClient(":" + _users[fromFd] + " PRIVMSG " + cmd.message, getUserFd(cmd.users[0]));
	else
		sendChannel(":" + _users[fromFd] + " PRIVMSG " + cmd.message, cmd.channels[0], fromFd);
}

/* void	Server::sendToChannel(const std::string &channel, const std::string &message ,int fd)
{
	sendChannel(":" + _users[fd] + " PRIVMSG " + message, channel, fd);
}

void	Server::sendToUser(const std::string &user, const std::string &message, int fd)
{
	sendClient(":" + _users[fd] + " PRIVMSG " + message, getUserFd(user));
} */

// KICK command
void	Server::kickUser(const Command &cmd, int fromFd)
{
	Channel *ch;
	std::string channel_name = "";
	if (cmd.channels.size() > 0)
		channel_name = cmd.channels[0];

	// 476 BADCHANMASK
	if (Channel::validChannelName(channel_name) == false)
	{
		std::string message = channel_name + " :Bad Channel ";
		sendClient(message, fromFd);
	}

	// 403 ERR_NOSUCHCHANNEL
	else if (channelExists(channel_name, &ch) == false)
	{
		std::string message = channel_name + " :No such channel";
		sendClient(message, fromFd);
	}

	// 442 ERR_NOTONCHANNEL
	else if (Channel::containsUser(ch->getUsers(), _users[fromFd]) == false)
	{
		std::string message = channel_name + " :You're not on that channel";
		sendClient(message, fromFd);
	}

	// 482 ERR_CHANOPPRIVSNEED
	else if (Channel::containsUser(ch->getOperators(), _users[fromFd]) == false)
	{
		std::string message = channel_name + " :You're not channel operator";
		sendClient(message, fromFd);
	}

	// 462 ERR_NEEDMOREPARAMS
	
	// 
	else
	{
		std::string message = ":" + _users[fromFd] + " KICK " + channel_name + " " + cmd.users[0];
		if (cmd.message_set)
			message += " " + cmd.message;
		sendClient(message, fromFd);
		sendChannel(message, channel_name, fromFd);
		ch->removeUser(cmd.users[0]);
	}
}

// INVITE command
void	Server::inviteUser(const Command &cmd, int fromFd)
{
	Channel	*ch;
	std::string channel_name = "";
	if (cmd.channels.size() > 0)
		channel_name = cmd.channels[0];
	// 336 RPL_INVITELIST
	// 337 RPL_ENDOFINVITELIST
	if (cmd.channels.empty() && cmd.users.empty())
	{
		std::string channel_list = getInvitedChannels(_users[fromFd]);
		if (!channel_list.empty())
		{
			std::string	message = ":server 336 " + _users[fromFd] + " :" + channel_list;
			sendClient(message, fromFd);
		}
		std::string	message = ":server 337 " + _users[fromFd] + " :End of /INVITE list";
		sendClient(message, fromFd);
	}
	// 403 ERR_NOSUCHCHANNEL
	else if (channelExists(channel_name, &ch) == false)
	{
		std::string message = ":server 403 " + channel_name + " :No such channel";
		sendClient(message, fromFd);
	}
	// 461 ERR_NEEDMOREPARAMS
	// 442 ERR_NOTONCHANNEL
	else if (ch && Channel::containsUser(ch->getUsers(), _users[fromFd]) == false)
	{
		std::string message = ":server 442 " + _users[fromFd] + " " + channel_name + " :You're not on that channel";
		sendClient(message, fromFd);
	}
	// 482 ERR_CHANOPRIVSNEEDED
	else if (ch && ch->getIsInviteOnly() && Channel::containsUser(ch->getOperators(), _users[fromFd]) == false)
	{
		std::string message = ":server 482 " + channel_name + " :You're not channel operator";
		sendClient(message, fromFd);
	}
	// 341 RPL_INVITING
	else if (ch && Channel::containsUser(ch->getInvitedUsers(), cmd.users[0]))
	{
		std::string message = ":server 341 " + _users[fromFd] + " " + cmd.users[0] + " " + channel_name; //TODO: needs to store who invited a user
		sendClient(message, fromFd);
	}
	// 443 ERR_USERONCHAN
	else if (ch && Channel::containsUser(ch->getUsers(), cmd.users[0]))
	{
		std::string message = ":server 443 " + channel_name + " :You're not channel operator";
		sendClient(message, fromFd);
	}
	// RESPONSE
	else
	{
		ch->addInvitedUser(cmd.users[0]);
		std::string	message = ":" + _users[fromFd] + " INVITE " + cmd.users[0] + " :" + channel_name;
		sendClient(message, getUserFd(cmd.users[0]));
	}
}

// TOPIC command
void	Server::processTopic(const Command &cmd, int fromFd)
{
	Channel *ch;
	std::string channel_name = "";
	if (cmd.channels.size() > 0)
		channel_name = cmd.channels[0];
	// 461 ERR_NEEDMOREPARAMS
	// if (topic.empty())
	// {
	// 	std::string message = "JOIN :Not enough parameters";
	// 	sendClient(message, fd);
	// }
	// 403 ERR_NOSUCHCHANNEL
	if (channelExists(channel_name, &ch) == false)
	{
		std::string message = channel_name + " :No such channel";
		sendClient(message, fromFd);
	}
	// 442 ERR_NOTONCHANNEL
	else if (ch && Channel::containsUser(ch->getUsers(), _users[fromFd]) == false)
	{
		std::string message = channel_name + " :You're not on that channel";
		sendClient(message, fromFd);
	}

	// 331 RPL_NOTOPIC
	// 332 RPL_TOPIC
	// 333 RPL_TOPICWHOTIME
	else if (cmd.message_set == false)
	{
		if (ch->getTopicSetAt().empty())
			sendClient(":server 331 " + _users[fromFd] + " " + channel_name + " :" + "No topic is set", fromFd);
		else
		{
			sendClient(":server 332 " + _users[fromFd] + " " + channel_name + " :" + ch->getTopic(), fromFd);
			sendClient(":server 333 " + _users[fromFd] + " " + channel_name + " " + ch->getTopicSetBy() + " " + ch->getTopicSetAt(), fromFd);
		}
	}
	// 482 ERR_CHANOPRIVSNEEDED
	else if (ch && ch->getHasRestrictTopic() && Channel::containsUser(ch->getOperators(), _users[fromFd]) == false)
	{
		std::string message = channel_name + " :You're not channel operator";
		sendClient(message, fromFd);
	}
	else
	{
		ch->setTopic(cmd.message, _users[fromFd]);
		sendClient(":" + _users[fromFd] + " TOPIC " + channel_name + " " + cmd.message, fromFd);
		sendChannel(":" + _users[fromFd] + " TOPIC " + channel_name + " " + cmd.message, channel_name, fromFd);
	}
}

// PART command
void	Server::leaveChannel(const Command &cmd, int fromFd)
{
	Channel	*ch;
	std::string channel_name = "";
	if (cmd.channels.size() > 0)
		channel_name = cmd.channels[0];
	// 461 ERR_NEEDMOREPARAMS
	// 403 ERR_NOSUCHCHANNEL
	if (channelExists(channel_name, &ch) == false)
	{
		std::string message = ":server 403 " + channel_name + " :No such channel";
		sendClient(message, fromFd);
	}
	// 442 ERR_NOTONCHANNEL
	else if (ch && Channel::containsUser(ch->getUsers(), _users[fromFd]) == false)
	{
		std::string message = ":server 442 " + channel_name + " :You're not on that channel";
		sendClient(message, fromFd);
	}
	// RESPONSE
	else
	{
		ch->removeUser(_users[fromFd]);
		std::string message = ":" + _users[fromFd] + " PART " + channel_name + " " + cmd.message;
		sendClient(message, fromFd);
		sendChannel(message, channel_name, fromFd);
	}
}

// QUIT command
void	Server::quitServer(const Command &cmd, int fromFd)
{
	std::string	message = "ERROR :closing connection [Quit " + cmd.message + "]";
	sendClient(message, fromFd);
	message = ":" + _users[fromFd] + " QUIT :Quit " + cmd.message;
	sendAllClients(message, fromFd);
	for (std::size_t i = 0; i < _channels.size(); i++)
	{
		if (Channel::containsUser(_channels[i].getUsers(), _users[fromFd]))
			_channels[i].removeUser(_users[fromFd]);		
		if (Channel::containsUser(_channels[i].getOperators(), _users[fromFd]))
			_channels[i].removeOperator(_users[fromFd]);		
		if (Channel::containsUser(_channels[i].getInvitedUsers(), _users[fromFd]))
			_channels[i].removeInvitedUser(_users[fromFd]);		
	}
	if (_users.find(fromFd) != _users.end())
		_users.erase(fromFd);
	delFromPfds(fromFd);
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
		if (!Channel::containsUser(_channels[i].getInvitedUsers(), user))
			continue ;
		if (channels.size() != 0)
			channels += " ";
		channels += _channels[i].getName();
	}
	return channels;
}
