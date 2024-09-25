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
	for (std::vector<Channel>::iterator it = _channels.begin(); it != _channels.end(); it++)
	{
		if(it->getName() == name)
			return &(*it);
	}
	return NULL;
}

bool	Server::removeChannelFromServer(Channel &channel)
{
	for (std::vector<Channel>::iterator it = _channels.begin(); it != _channels.end(); it++)
	{
		if(it->getName() == channel.getName())
		{
			_channels.erase(it);
			return true;
		}
	}
	return false;
}

bool	Server::channelExists(const std::string &channel_str)
{
	for (std::vector<Channel>::iterator it = this->_channels.begin(); it != this->_channels.end(); it++)
	{
		if (it->getName() == channel_str)
			return true;
	}
	return false;
}

bool	Server::removeUserFromChannel(const std::string &user, Channel &channel)
{
	bool removedUser = false;
	if (Channel::containsUser(channel.getUsers(), user))
	{
		removedUser = true;
		channel.removeUser(user);
	}
	if (Channel::containsUser(channel.getOperators(), user))
		channel.removeOperator(user);
	return removedUser;
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

// Need a way of handling which channel/user throws an error when multiple are possible.
void Server::sendError(enum Replies err_code, const Command &cmd, int requesting_client_fd, std::string extra_prefix)
{
	(void) cmd;
	std::map<enum Replies, std::string> err_msg;
	err_msg[ERR_NOSUCHNICK]			= "No such nickname";
	err_msg[ERR_NOSUCHCHANNEL]		= "No such channel";
	err_msg[ERR_CANNOTSENDTOCHAN]	= "Cannot send to channel";
	err_msg[ERR_TOOMANYCHANNELS]	= "You have joined too many channels";
	err_msg[ERR_NORECIPIENT]		= "No recipient given";
	err_msg[ERR_NOTEXTTOSEND]		= "No text to send";
	err_msg[ERR_INPUTTOOLONG]		= "Input line was too long";
	err_msg[ERR_UNKNOWNCOMMAND]		= "Unknown command";
	err_msg[ERR_NONICKNAMEGIVEN]	= "No nickname given";
	err_msg[ERR_ERRONEUSNICKNAME]	= "Erroneus nickname";
	err_msg[ERR_NICKNAMEINUSE]		= "Nickname is already in use";
	err_msg[ERR_USERNOTINCHANNEL]	= "They aren't on that channel";
	err_msg[ERR_NOTONCHANNEL]		= "You're not on that channel";
	err_msg[ERR_USERONCHANNEL]		= "is already on channel";
	err_msg[ERR_NOTREGISTERED]		= "You have not yet registered";
	err_msg[ERR_NEEDMOREPARAMS]		= "Not enough parameters";
	err_msg[ERR_ALREADYREGISTRED]	= "You may not reregister";
	err_msg[ERR_PASSWDMISMATCH]		= "Password incorrect";
	err_msg[ERR_CHANNELISFULL]		= "Cannot join channel (+l)";
	err_msg[ERR_UNKNOWNMODE]		= "is unkown mode char to me";
	err_msg[ERR_INVITEONLYCHAN]		= "Cannot join channel (+i)";
	err_msg[ERR_BADCHANNELKEY]		= "Cannot join channel (+k)";
	err_msg[ERR_BADCHANMASK]		= "Bad Channel Mask";
	err_msg[ERR_NOPRIVILEGES]		= "Permission Denied- You're not an IRC operator";
	err_msg[ERR_CHANOPRIVSNEEDED]	= "You're not channel operator";
	err_msg[ERR_UMODEUNKNOWNFLAG]	= "Unknown MODE flag";
	err_msg[ERR_INVALIDKEY]			= "Key is not well-formed";

	std::string message = ":Server " + Log::str(err_code) + " " + _users[requesting_client_fd];
	
	if (extra_prefix.size() > 0)
		message += " " + extra_prefix;
	message += " :" + err_msg[err_code];
	sendClient(message, requesting_client_fd);
}


// 474 ERR_BANNEDFROMCHAN not supported
// 476 ERR_BADCHANMASK not supported
// 405 ERR_TOOMANYCHANNELS not supported
void	Server::joinChannel(const Command &cmd, int fromFd)
{
	if (cmd.channels.size() == 0)
		return sendError(ERR_NEEDMOREPARAMS, cmd, fromFd);

	std::string channel_name = "";
	for (std::size_t i = 0; i < cmd.channels.size(); i++)
	{
		channel_name = cmd.channels[i];

		if (Channel::validChannelName(channel_name) == false)
			return sendError(ERR_NOSUCHCHANNEL, cmd, fromFd, channel_name);

		if (!channelExists(channel_name))
		{
			Channel new_channel(channel_name);
			new_channel.addOperator(_users[fromFd]);
			_channels.push_back(new_channel);
		}
		Channel *ch = getChannelByName(channel_name);

		if (ch->getHasPassword() && ch->checkPassword(cmd.keys[i]) == false)
			return sendError(ERR_BADCHANNELKEY, cmd, fromFd, channel_name);
		if (ch->getHasLimit() && ch->getUsers().size() >= ch->getLimit())
			return sendError(ERR_CHANNELISFULL, cmd, fromFd, channel_name);
		if (ch->getIsInviteOnly() && Channel::containsUser(ch->getInvitedUsers(), _users[fromFd]) == false)
			return sendError(ERR_INVITEONLYCHAN, cmd, fromFd, channel_name);
		// 332 RPL_TOPIC
		// 333 RPL_TOPICWHOTIME
		// 353 RPL_NAMREPLY
		// 366 RPL_ENDOFNAMES
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
	if (cmd.channels.empty())
		return sendError(ERR_NEEDMOREPARAMS, cmd, fromFd);
	
	std::string channel_name = cmd.channels[0];

	if (channelExists(channel_name) == false)
		return sendError(ERR_NOSUCHCHANNEL, cmd, fromFd, channel_name);
	
	Channel *ch = getChannelByName(channel_name);
	if (Channel::containsUser(ch->getUsers(), _users[fromFd]) == false)
		return sendError(ERR_NOTONCHANNEL, cmd, fromFd, channel_name);

	// 324 RPL_CHANNELMODEIS
	if (cmd.mode_operations.size() == 0)
	{
		std::string message = ":server 324 " + _users[fromFd] + " " + channel_name;
		std::string	limit_arg = "";
		std::string	password_arg = "";
		std::string mode_list = "";
		if (ch->getIsInviteOnly())
			mode_list += "i";
		if (ch->getHasPassword())
			mode_list += "k";
		if (ch->getHasLimit())
		{
			mode_list += "l";
			limit_arg = " " + std::to_string(ch->getLimit());
		}
		if (ch->getHasRestrictTopic())
		{
			mode_list += "t";
			password_arg = " " + ch->getPassword();
		}
		if (mode_list.size() > 0)
			message += " +" + mode_list + limit_arg + password_arg;
		sendClient(message, fromFd);
		return ;
	}

	if (ch && Channel::containsUser(ch->getOperators(), _users[fromFd]) == false)
		return sendError(ERR_CHANOPRIVSNEEDED, cmd, fromFd, channel_name);

	// SET MODES
	else
	{
		std::size_t index = 0;
		std::string	processed_operations;
		std::string	processed_parameters;
		for (std::vector<std::string>::const_iterator it = cmd.mode_operations.cbegin(); it < cmd.mode_operations.cend(); it++)
		{
			if (*it == "+i")
			{
				ch->setIsInviteOnly(true);
				processed_operations += *it;
			}
			else if (*it == "-i")
			{
				ch->setIsInviteOnly(false);
				processed_operations += *it;
			}
			else if (*it == "+k")
			{
				if (cmd.mode_parameters.size() <= index)
					continue ;
				ch->setHasPassword(true);
				ch->setPassword(cmd.mode_parameters[index]);
				processed_operations += *it;
				processed_parameters += cmd.mode_parameters[index];
				index++;
			}
			else if (*it == "-k")
			{
				if (cmd.mode_parameters.size() <= index || !checkPassword(cmd.mode_parameters[index]))
				{
					index++;
					continue ;
				}
				ch->setHasPassword(true);
				ch->setPassword(cmd.mode_parameters[index]);
				processed_operations += *it;
				processed_parameters += cmd.mode_parameters[index];
				index++;
			}
			else if (*it == "+l")
			{
				if (cmd.mode_parameters.size() <= index)
					continue ;
				std::stringstream ss(cmd.mode_parameters[index]);
				std::size_t limit;
				ss >> limit;
				ch->setHasLimit(true);
				ch->setLimit(limit);
				processed_operations += *it;
				processed_parameters += cmd.mode_parameters[index];
				index++;
			}
			else if (*it == "-l")
			{
				ch->setHasLimit(false);
				processed_operations += *it;
			}
			else if (*it == "+o" || *it == "-o")
			{
				if (!userExists(cmd.mode_parameters[index]))
				{
					sendError(ERR_NOSUCHNICK, cmd, fromFd, cmd.mode_parameters[index]);
					index++;
					continue ;
				}
				if (!Channel::containsUser(ch->getUsers(), cmd.mode_parameters[index])
					|| Channel::containsUser(ch->getOperators(), cmd.mode_parameters[index]))
				{
					index++;
					continue ;
				}
				if (*it == "+o")
					ch->addOperator(cmd.mode_parameters[index]);
				else
					ch->removeOperator(cmd.mode_parameters[index]);
				processed_operations += *it;
				processed_parameters += cmd.mode_parameters[index];
				index++;
			}
			else if (*it == "+t")
			{
				ch->setHasRestrictTopic(true);
				processed_operations += *it;
			}
			else if (*it == "-t")
			{
				ch->setHasRestrictTopic(false);
				processed_operations += *it;
			}
			else
			{
				sendError(ERR_UNKNOWNMODE, cmd, fromFd, (*it).substr(1,1));
			}
		}
		std::string	message = ":" + _users[fromFd] + " MODE " + channel_name + " " + processed_operations + " " + processed_parameters;
		sendClient(message, fromFd);
		sendChannel(message, channel_name, fromFd);
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

// PRIVMSG command
void	Server::sendMessage(const Command &cmd, int fromFd)
{
	if (cmd.users.empty() && cmd.channels.empty())
		return sendError(ERR_NORECIPIENT, cmd, fromFd);
	if (cmd.message_set == false)
		return sendError(ERR_NOTEXTTOSEND, cmd, fromFd);

	for (std::size_t i = 0; i < cmd.channels.size(); i++)
	{
		if (channelExists(cmd.channels[i]))
			sendChannel(":" + _users[fromFd] + " PRIVMSG " + cmd.message, cmd.channels[0], fromFd);
		else
			sendError(ERR_NOSUCHNICK, cmd, fromFd, cmd.channels[i]); // Docs suggest this, but ERR_NOSUCHCHANNEL seems more appropriate
	}
	for (std::size_t i = 0; i < cmd.users.size(); i++)
	{
		if (userExists(cmd.users[i]))
			sendClient(":" + _users[fromFd] + " PRIVMSG " + cmd.message, getUserFd(cmd.users[0]));
		else
			sendError(ERR_NOSUCHNICK, cmd, fromFd, cmd.users[i]);
	}
}

// KICK command
void	Server::kickUser(const Command &cmd, int fromFd)
{
	
	if (cmd.channels.size() == 0 || cmd.users.size() == 0)
		return sendError(ERR_NEEDMOREPARAMS, cmd, fromFd);

	std::string channel_name = cmd.channels[0];
	if (Channel::validChannelName(channel_name) == false)
		return sendError(ERR_BADCHANMASK, cmd, fromFd, channel_name);
	if (!channelExists(channel_name))
		return sendError(ERR_NOSUCHCHANNEL, cmd, fromFd, channel_name);
	Channel *ch = getChannelByName(channel_name);

	if (Channel::containsUser(ch->getUsers(), _users[fromFd]) == false)
		return sendError(ERR_NOTONCHANNEL, cmd, fromFd, channel_name);
	if (Channel::containsUser(ch->getOperators(), _users[fromFd]) == false)
		return sendError(ERR_CHANOPRIVSNEEDED, cmd, fromFd, channel_name);

	for (std::size_t i = 0; i < cmd.users.size(); i++)
	{
		if (Channel::containsUser(ch->getUsers(), cmd.users[i]) == false)
		{
			sendError(ERR_USERNOTINCHANNEL, cmd, fromFd, cmd.users[i] + " " + channel_name);
			continue ;
		}
		std::string message = ":" + _users[fromFd] + " KICK " + channel_name + " " + cmd.users[i];
		if (cmd.message_set)
			message += " :" + cmd.message;
		else
			message += " :Kicked from " + channel_name + " by " + _users[fromFd];
		sendClient(message, fromFd);
		sendChannel(message, channel_name, fromFd);
		removeUserFromChannel(cmd.users[i], *ch);
	}
	if (ch->getUsers().empty())
		removeChannelFromServer(*ch);
}

// INVITE command
void	Server::inviteUser(const Command &cmd, int fromFd)
{
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
		return ;
	}

	if (cmd.channels.empty())
		return sendError(ERR_NEEDMOREPARAMS, cmd, fromFd);

	std::string channel_name = cmd.channels[0];

	if (!channelExists(channel_name))
		return sendError(ERR_NOSUCHCHANNEL, cmd, fromFd, channel_name);
	Channel *ch = getChannelByName(channel_name);

	if (Channel::containsUser(ch->getUsers(), _users[fromFd]) == false)
		return sendError(ERR_NOTONCHANNEL, cmd, fromFd, channel_name);
	if (ch->getIsInviteOnly() && Channel::containsUser(ch->getOperators(), _users[fromFd]) == false)
		return sendError(ERR_CHANOPRIVSNEEDED, cmd, fromFd, channel_name);
	
	// 341 RPL_INVITING
	if (ch && Channel::containsUser(ch->getInvitedUsers(), cmd.users[0]))
	{
		std::string message = ":server 341 " + _users[fromFd] + " " + cmd.users[0] + " " + channel_name; //TODO: needs to store who invited a user
		sendClient(message, fromFd);
		return ;
	}

	// 443 ERR_USERONCHAN
	if (ch && Channel::containsUser(ch->getUsers(), cmd.users[0]))
		return sendError(ERR_USERONCHANNEL, cmd, fromFd, channel_name);

	// RESPONSE
	ch->addInvitedUser(cmd.users[0]);
	std::string	message = ":" + _users[fromFd] + " INVITE " + cmd.users[0] + " :" + channel_name;
	sendClient(message, getUserFd(cmd.users[0]));

}

// TOPIC command
void	Server::processTopic(const Command &cmd, int fromFd)
{
	if (cmd.channels.empty())
		return sendError(ERR_NEEDMOREPARAMS, cmd, fromFd);
	
	std::string channel_name = cmd.channels[0];

	if (channelExists(channel_name) == false)
		return sendError(ERR_NOSUCHCHANNEL, cmd, fromFd, channel_name);
	
	Channel *ch = getChannelByName(channel_name);

	if (ch && Channel::containsUser(ch->getUsers(), _users[fromFd]) == false)
		return sendError(ERR_NOTONCHANNEL, cmd, fromFd, channel_name);

	// 331 RPL_NOTOPIC
	// 332 RPL_TOPIC
	// 333 RPL_TOPICWHOTIME
	if (cmd.message_set == false)
	{
		if (ch->getTopicSetAt().empty())
			sendClient(":server 331 " + _users[fromFd] + " " + channel_name + " :" + "No topic is set", fromFd);
		else
		{
			sendClient(":server 332 " + _users[fromFd] + " " + channel_name + " :" + ch->getTopic(), fromFd);
			sendClient(":server 333 " + _users[fromFd] + " " + channel_name + " " + ch->getTopicSetBy() + " " + ch->getTopicSetAt(), fromFd);
		}
		return ;
	}

	if (ch->getHasRestrictTopic() && Channel::containsUser(ch->getOperators(), _users[fromFd]) == false)
		return sendError(ERR_CHANOPRIVSNEEDED, cmd, fromFd, channel_name);
	
	ch->setTopic(cmd.message, _users[fromFd]);
	sendClient(":" + _users[fromFd] + " TOPIC " + channel_name + " " + cmd.message, fromFd);
	sendChannel(":" + _users[fromFd] + " TOPIC " + channel_name + " " + cmd.message, channel_name, fromFd);
	
}

// PART command
void	Server::leaveChannel(const Command &cmd, int fromFd)
{
	if (cmd.channels.empty())
		return sendError(ERR_NEEDMOREPARAMS, cmd, fromFd);

	for (std::size_t i = 0; i < cmd.channels.size(); i++)
	{
		std::string channel_name = cmd.channels[i];

		if (channelExists(channel_name) == false)
			return sendError(ERR_NOSUCHCHANNEL, cmd, fromFd, channel_name);

		Channel	*ch = getChannelByName(channel_name);

		if (Channel::containsUser(ch->getUsers(), _users[fromFd]) == false)
			return sendError(ERR_NOTONCHANNEL, cmd, fromFd, channel_name);

		// RESPONSE
		removeUserFromChannel(_users[fromFd], *ch);
		std::string message = ":" + _users[fromFd] + " PART " + channel_name + " " + cmd.message;
		sendClient(message, fromFd);
		sendChannel(message, channel_name, fromFd);
		if (ch->getUsers().empty())
			removeChannelFromServer(*ch);
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
	Channel *ch = getChannelByName(channel);
	if (!ch)
		return ;
	std::vector<std::string> users = ch->getUsers();
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
