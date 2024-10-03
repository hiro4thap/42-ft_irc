#include "../inc/Server.hpp"

Server::~Server()
{
}

Server::Server(unsigned int port, std::string password):
	_port(port), _servername("localhost"),_ipv4_address("127.0.0.1"),
	_version("0.9.0"), _password(password), _motd_set(false)
{
	if (_password.empty())
		_requires_authentication = false;
	else
		_requires_authentication = true;
	_server_created = time(0);
}


int	Server::getSocketFd()
{
	Log::out("Starting IRC Server... ", COLOR_MAGENTA);
    // creating socket
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);

	int	optval = 1;
	if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)))
	{
		perror("setsockopt");
		return -1;
	}
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

	_pfds.resize(1);
	_pfds[0].fd = serverSocket;
	_pfds[0].events = POLLIN;

	Log::nl("Done!", COLOR_MAGENTA);
	return serverSocket;
}

void	Server::launch(int serverSocket)
{
	Log::nl("Listening on " + _ipv4_address + "/" + Log::str(_port) + "...", COLOR_MAGENTA);
	
	// Load bot
	_users[_bot.getFd()] = new User(_bot.getFd());
	_bot.setupBot(_users[_bot.getFd()]);
	
	processCommand("JOIN " + _bot.getChannel() + "\r\n", _bot.getFd());

	while (true)
	{
		poll(_pfds.data(), _pfds.size(), -1);
		for (unsigned int i = 0; i < _pfds.size(); i++)
		{
			if (_pfds[i].revents != POLLIN)
				continue ;
			// accepting connection request
			if (_pfds[i].fd == serverSocket)
			{
				Log::nl("Client " + Log::str(_pfds.size()) + " is accepted", COLOR_YELLOW);
				// std::cout << "Client " << _pfds.size() << " is accepted" << std::endl;
				int clientSocket = accept(serverSocket, NULL, NULL);
				if (clientSocket == -1)
				{
					perror("accept");
					delFromPfds(_pfds[i].fd);
					continue ;
				}
				addToPfds(clientSocket);
				_users[clientSocket] = new User(clientSocket);
			}
			// recieving data
			else
			{
				char buffer[1024] = { 0 };
				if (recv(_pfds[i].fd, buffer, sizeof(buffer), 0) == -1)
				{
					perror("recv");
					delFromPfds(_pfds[i].fd);
					continue ;
				}
				std::string client_name = "";
				if (i < _pfds.size() && _users.find(_pfds[i].fd) != _users.end())
					client_name = _users[_pfds[i].fd]->getNickname();
				Log::out("[Client " + Log::str(i) + ": \"" + client_name + "\"] ", COLOR_YELLOW);
				Log::nl(buffer);
				// std::cout << "Message from client " << i << " :" << buffer << std::endl;
				processCommand(buffer, _pfds[i].fd);
			}
		}
	}
	// closing the socket.
	close(serverSocket);
}

void	Server::addToPfds(int fd)
{
	struct pollfd	pfd;
	pfd.fd = fd;
	pfd.events = POLLIN;
	_pfds.push_back(pfd);
}

void	Server::delFromPfds(int fromFd)
{
	for (std::size_t i = 0; i < _channels.size(); i++)
	{
		if (Channel::containsUser(_channels[i].getUsers(), _users[fromFd]->getNickname()))
			_channels[i].removeUser(_users[fromFd]->getNickname());		
		if (Channel::containsUser(_channels[i].getOperators(), _users[fromFd]->getNickname()))
			_channels[i].removeOperator(_users[fromFd]->getNickname());		
		if (Channel::containsUser(_channels[i].getInvitedUsers(), _users[fromFd]->getNickname()))
			_channels[i].removeInvitedUser(_users[fromFd]->getNickname());		
		if (_channels[i].getUsers().size() == 0)
			removeChannelFromServer(_channels[i].getName());
	}
	if (_users.find(fromFd) != _users.end())
	{
		delete _users[fromFd];
		_users.erase(fromFd);
	}
		
	if (_remaining_command.find(fromFd) != _remaining_command.end())
		_remaining_command.erase(fromFd);
	for (std::size_t i = 0; i < _pfds.size(); i++)
	{
		if (_pfds[i].fd != fromFd)
			continue ;
		close(_pfds[i].fd);
	}	
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

const std::string		Server::getTimeCreated() const
{
	return ctime(&_server_created);
}

bool	Server::hasPassed(int fd)
{
	return _users[fd]->isAuthenticated();
	// std::vector<int>::iterator	it = std::find(_passed_fds.begin(), _passed_fds.end(), fd);
	// return (it != _passed_fds.end());
}

bool	Server::removeChannelFromServer(const std::string &channel_name)
{
	Channel *ch = getChannelByName(channel_name);
	if (!ch)
		return false;
	_channels.erase(std::remove(_channels.begin(), _channels.end(), *ch), _channels.end());
	return true;
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
	if (toFd == _bot.getFd())
	{
		Log::out("[Server -> \"" + _users[toFd]->getNickname() + "\"] ", COLOR_CYAN);
		Log::nl(response);
		Command cmd = _bot.proccessMessage(message);
		if (cmd.threw_error.at(0))
			return ;
		cmd.message += "\r\n";

		sendMessage(cmd, 2);
	}
	else if (toFd != serverSocket)
	{
		if (send(toFd, response.c_str() , response.size(), 0) == -1)
			perror("send");
		Log::out("[Server -> \"" + _users[toFd]->getNickname() + "\"] ", COLOR_CYAN);
		Log::nl(response);
	}
}

void	Server::processCommand(std::string command, int fromFd)
{
	User *user = _users[fromFd];
	std::size_t pos_end = 0;
	std::string	token;
	_remaining_command[fromFd] += command;
    while ((pos_end = _remaining_command[fromFd].find("\r\n")) != std::string::npos)
	{
		std::map<std::string, void(Server::*)(const Command&, int)> commands;
		if (this->_requires_authentication && user->isAuthenticated() == false)
		{
			commands["PASS"]	= &Server::checkPassword;
		}
		else if (user->getRegistrationState() < REGISTERED)
		{
			commands["NICK"]	= &Server::setNickname;
			commands["USER"]	= &Server::setUser;
			commands["PASS"]	= &Server::checkPassword;
		}
		else
		{
			commands["JOIN"]	= &Server::joinChannel;
			commands["NICK"]	= &Server::setNickname;
			commands["PRIVMSG"]	= &Server::sendMessage;
			commands["NOTICE"]	= &Server::sendNotice;
			commands["KICK"]	= &Server::kickUser;
			commands["INVITE"]	= &Server::inviteUser;
			commands["TOPIC"]	= &Server::processTopic;
			commands["MODE"]	= &Server::processMode;
			commands["PART"]	= &Server::leaveChannel;
			commands["QUIT"]	= &Server::quitServer;
			commands["PASS"]	= &Server::checkPassword;
		}

		Command cmd;
		cmd.command = "";
		cmd.message_set = false;
		cmd.message = "";

		pos_end += 2;
        token = _remaining_command[fromFd].substr(0, pos_end);
		_remaining_command[fromFd].erase(0, pos_end);
		
		Parser parser;
		parser.message(token, cmd);

		std::map<std::string, void(Server::*)(const Command&, int)>::iterator command_function = commands.find(cmd.command);
		if (command_function != commands.end())
		{
			(this->*(command_function->second))(cmd, fromFd);
		}
		else if (user->getRegistrationState() < REGISTERED)
		{
			sendError(ERR_NOTREGISTERED, fromFd);
		}
		if (user->getRegistrationState() == REGISTERED && user->isRegistered() == false)
		{
			user->setRegistered();
			std::size_t registered_users = getCurrentRegisteredUsers();
			sendReply(RPL_WELCOME, user->getFd(), "", "Welcome to the 42 Internet Relay Chat Network " + user->getNickname());
			sendReply(RPL_YOURHOST, user->getFd(),"", "Your host is " + _servername + "[" + _ipv4_address + "/" + Log::str(_port) + "], running version " + _version);
			sendReply(RPL_CREATED, user->getFd(), "", "This server was created " + this->getTimeCreated());
			// sendReply(RPL_MYINFO, user->getFd(), _servername + " " + _version, "o iklot ko				l", true);
			sendReply(RPL_ISUPPORT, user->getFd(), "NICKLEN=9 CHANMODES=,k,l,it");
			sendReply(RPL_LUSERCLIENT, user->getFd(), "", "There are " + Log::str(registered_users) + " users and 0 invisible on 1 servers");
			sendReply(RPL_LUSERCHANNELS, user->getFd(), Log::str(_channels.size()));
			sendReply(RPL_LUSERME, user->getFd(), "", "I have " + Log::str(registered_users) + " and 1 servers");
			if (_motd_set)
			{
				sendReply(RPL_MOTDSTART, user->getFd());
				for (std::size_t i = 0; i < _motd.size(); i++)
				{
					sendReply(RPL_MOTD, user->getFd());
				}
				sendReply(RPL_ENDOFMOTD, user->getFd());
			}
			else
				sendError(ERR_NOMOTD, user->getFd());
		}
    }
}

bool Server::userExists(const std::string &value)
{
	for (std::map<int, User*>::iterator it = this->_users.begin(); it != this->_users.end(); it++)
	{
		if (value == it->second->getNickname())
			return true;
	}
	return false;
}

int Server::getUserFd(const std::string &value)
{
	for (std::map<int, User*>::iterator it = this->_users.begin(); it != this->_users.end(); it++)
	{
		if (value == it->second->getNickname())
			return it->first;
	}
	return -1;
}

// NICK command
void	Server::setNickname(const Command &cmd, int fromFd)
{
	if (cmd.users.size() < 1)
		return sendError(ERR_NONICKNAMEGIVEN, fromFd);
	std::string nickname = cmd.users[0];
	if (cmd.threw_error[0] && cmd.err_response[0] == ERR_ERRONEUSNICKNAME)
		return sendError(ERR_ERRONEUSNICKNAME, fromFd, nickname);
	if (userExists(nickname))
		return sendError(ERR_NICKNAMEINUSE, fromFd, nickname);
	// 436 ERR_NICKCOLLISION not implemented
	// RESPONSE
	User *user = _users[fromFd];
	if (user->isRegistered() == false 
		&& user->getRegistrationState() != PROVIDED_NICK 
		&& user->getRegistrationState() != REGISTERED)
	{
		user->setNickname(nickname);
		user->setRegistrationState(PROVIDED_NICK);
		// sendReply(RPL_WELCOME, fromFd, "", ":Welcome " + nickname, true);
		// sendClient(":server 001 " + nickname, fromFd);
	}
	else
	{
		for (std::size_t i = 0; i < _channels.size(); i++)
		{
			_channels[i].replaceUser(_users[fromFd]->getNickname(), nickname);
			_channels[i].replaceOperator(_users[fromFd]->getNickname(), nickname);
			_channels[i].replaceInvitedUser(_users[fromFd]->getNickname(), nickname);
		}
		sendAllClients(
			sendReply("NICK", fromFd, nickname), 
			fromFd
		);
		_users[fromFd]->setNickname(nickname);
	}
}

// USER command
// Needs error handling, and proper integration into setup process.
void	Server::setUser(const Command &cmd, int fromFd)
{
	User* user = _users[fromFd];

	if (cmd.users.empty())
		return sendError(ERR_NEEDMOREPARAMS, fromFd);
	if (user->getRegistrationState() == PROVIDED_USER 
		|| user->getRegistrationState() == REGISTERED)
		return sendError(ERR_ALREADYREGISTRED, fromFd);
	user->setUsername(cmd.users[0]);
	user->setRealName(cmd.message);
	user->setRegistrationState(PROVIDED_USER);
}

/// @brief Sends error code to client. Any additional information after nick required in message must be passed via caller. 
/// @note Sends error in the format: "<server> <err_code> <client> [<extra_prefix]: <message>"
/// @param err_code The appropriate enum error code.
/// @param requesting_client_fd Client tp reply to regarding an error in their command.
/// @param extra_prefix [Optional] Extra specifier to add after nick but before the error message.
/// @todo Need a way of handling which channel/user throws an error when multiple are possible.
void Server::sendError(enum Replies err_code, int requesting_client_fd, std::string extra_prefix)
{
	std::map<enum Replies, std::string> err_msg;
	err_msg[ERR_NOSUCHNICK]			= "No such nick/channel";
	err_msg[ERR_NOSUCHCHANNEL]		= "No such channel";
	err_msg[ERR_CANNOTSENDTOCHAN]	= "Cannot send to channel";
	err_msg[ERR_TOOMANYCHANNELS]	= "You have joined too many channels";
	err_msg[ERR_NORECIPIENT]		= "No recipient given";
	err_msg[ERR_NOTEXTTOSEND]		= "No text to send";
	err_msg[ERR_INPUTTOOLONG]		= "Input line was too long";
	err_msg[ERR_UNKNOWNCOMMAND]		= "Unknown command";
	err_msg[ERR_NOMOTD]				= "MOTD has not been set";
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

	std::string message = ":" + _servername + " " + Log::str(err_code) + " " + _users[requesting_client_fd]->getNickname();
	
	if (extra_prefix.size() > 0)
		message += " " + extra_prefix;
	message += " :" + err_msg[err_code];
	sendClient(message, requesting_client_fd);
}

/// @brief Sends numeric replies to the requesting client. 
/// @warning Not all numerics have static replies. Non-static replies must be provided by @p msg_override . 
/// @param rpl_code Enum value of reply numerics.
/// @param requesting_client_fd The client requesting a reply.
/// @param extra_prefix Additional specifiers after nick but before colon.
/// @param msg_override Message string if non-static reply message (see #rpl_msg).
/// @param no_colon [Optional: Default = false] Disables display of colon in message if true.
void Server::sendReply(enum Replies rpl_code, int requesting_client_fd, std::string extra_prefix, std::string msg_override, bool no_colon)
{
	std::map<enum Replies, std::string> rpl_msg;
	// rpl_msg[RPL_WELCOME]			= "Welcome to the <networkname> Network, <nick>[!<user>@<host>]";
	// rpl_msg[RPL_YOURHOST]			= "Your host is <servername>, running version <version>";
	// rpl_msg[RPL_CREATED]			= "This server was created <datetime>";
	// rpl_msg[RPL_MYINFO]				= "<client> <servername> <version> <available user modes> <available channel modes> [<channel modes with a parameter>]"; //
	rpl_msg[RPL_ISUPPORT]			= "are supported by this server";
	rpl_msg[RPL_LUSERCHANNELS]		= "channels formed";
	rpl_msg[RPL_NONE]				= "";
	rpl_msg[RPL_NOTOPIC]			= "No topic is set";
	// rpl_msg[RPL_TOPIC]				= "<topic>";
	// rpl_msg[RPL_TOPICWHOTIME]		= "<client> <channel> <nick> <setat>"; //
	// rpl_msg[RPL_INVITELIST]			= "<client> <channel>";
	rpl_msg[RPL_ENDOFINVITELIST]	= "End of /INVITE list";
	// rpl_msg[RPL_INVITING]			= "<client> <nick> <channel>";
	// rpl_msg[RPL_NAMREPLY]			= "[prefix]<nick>{ [prefix]<nick>}";
	rpl_msg[RPL_ENDOFNAMES]			= "End of /NAMES list";
	// rpl_msg[RPL_BANLIST]			= "<client> <channel> <mask> [<who> <set-ts>]";
	rpl_msg[RPL_ENDOFBANLIST]		= "End of channel ban list";
	// rpl_msg[RPL_MOTD]				= "<line of the motd>";
	// rpl_msg[RPL_MOTDSTART]			= "- <server> Message of the day - ";
	rpl_msg[RPL_ENDOFMOTD]			= "End of /MOTD command.";

	std::string message = ":" + _servername + " " + Log::str(rpl_code, 3, '0') + " " + _users[requesting_client_fd]->getNickname() + " ";
	if (extra_prefix.size() > 0)
		message += extra_prefix + " ";
	if (!no_colon)
		message += ":";
	if (msg_override != "")
		message += msg_override;
	else
		message += rpl_msg[rpl_code];
	sendClient(message, requesting_client_fd);
}

/// @brief Sends non-numeric replies to requesting client.
/// @param command The supported IRC command from requesting client.
/// @param requesting_client_fd The connection ID of the requesting client.
/// @param message The information required by the command (must include ':' if required by the message format)
/// @return A copy of the text of the message sent to the client.
std::string	Server::sendReply(std::string command, int requesting_client_fd, std::string message)
{
	return sendCommand(command, requesting_client_fd, requesting_client_fd, message);
}

/// @brief Sends a formatted message from one client to another. Generally used when a client command takes actions requiring other clients to be notified.
/// @param command The supported IRC command from requesting client.
/// @param source_client_fd The connection ID of the requesting client.
/// @param target_client_fd The connection ID of the client to notify of the request.
/// @param message  The information required by the command (must include ':' if required by the message format)
/// @return  A copy of the text of the message sent to the target client.
std::string Server::sendCommand(std::string command, int source_client_fd, int target_client_fd, std::string message)
{
	std::string reply = ":" + _users[source_client_fd]->getNickname() + " " + command + " " + message;
	sendClient(reply, target_client_fd);
	return reply;
}

// 474 ERR_BANNEDFROMCHAN not supported
// 476 ERR_BADCHANMASK not supported
// 405 ERR_TOOMANYCHANNELS not supported
// JOIN command
void	Server::joinChannel(const Command &cmd, int fromFd)
{
	if (cmd.channels.size() == 0)
		return sendError(ERR_NEEDMOREPARAMS, fromFd);

	std::string channel_name = "";
	for (std::size_t i = 0; i < cmd.channels.size(); i++)
	{
		channel_name = cmd.channels[i];

		// Check docs
		if (cmd.threw_error[i] && cmd.err_response[i] == ERR_NOSUCHNICK)
		{
			sendError(ERR_NOSUCHNICK, fromFd, channel_name);
			continue ;
		}
		if (!channelExists(channel_name))
		{
			Channel new_channel(channel_name);
			new_channel.addOperator(_users[fromFd]->getNickname());
			_channels.push_back(new_channel);
		}
		Channel *ch = getChannelByName(channel_name);

		if (ch->getHasPassword() && (cmd.keys.size() < i + 1 || ch->checkPassword(cmd.keys[i]) == false))
			sendError(ERR_BADCHANNELKEY, fromFd, channel_name);
		else if (ch->getHasLimit() && ch->getUsers().size() >= ch->getLimit())
			sendError(ERR_CHANNELISFULL, fromFd, channel_name);
		else if (ch->getIsInviteOnly() && Channel::containsUser(ch->getInvitedUsers(), _users[fromFd]->getNickname()) == false)
			sendError(ERR_INVITEONLYCHAN, fromFd, channel_name);
		else if (Channel::containsUser(ch->getInvitedUsers(), _users[fromFd]->getNickname()))
			ch->removeInvitedUser(_users[fromFd]->getNickname());
		ch->addUser(_users[fromFd]->getNickname());
		// sendChannel(":" + _users[fromFd] + " JOIN " + channel_name, channel_name, fromFd);
		// sendClient(":" + _users[fromFd] + " JOIN " + channel_name, fromFd);
		sendChannel(
			sendReply("JOIN", fromFd, channel_name),
			channel_name, fromFd);
		
		if (ch->getTopicSetAt().empty())
			sendReply(RPL_NOTOPIC, fromFd, channel_name);
		else
		{
			sendReply(RPL_TOPIC, fromFd, channel_name, ch->getTopic());
			sendReply(RPL_TOPICWHOTIME, fromFd, channel_name, ch->getTopicSetBy() + " " + ch->getTopicSetAt(), true);
		}
		sendReply(RPL_NAMREPLY, fromFd, "= " + channel_name, getNameList(ch));
		sendReply(RPL_ENDOFNAMES, fromFd, channel_name);
	}
}

void	Server::processMode(const Command &cmd, int fromFd)
{
	if (cmd.channels.empty())
		return sendError(ERR_NEEDMOREPARAMS, fromFd);
	
	std::string channel_name = cmd.channels[0];

	if (cmd.threw_error[0] && cmd.err_response[0] == ERR_NOSUCHNICK)
		return sendError(ERR_NOSUCHNICK, fromFd, channel_name);
	if (channelExists(channel_name) == false)
		return sendError(ERR_NOSUCHCHANNEL, fromFd, channel_name);
	
	Channel *ch = getChannelByName(channel_name);
	if (Channel::containsUser(ch->getUsers(), _users[fromFd]->getNickname()) == false)
		return sendError(ERR_NOTONCHANNEL, fromFd, channel_name);

	if (cmd.mode_operations.size() == 0)
		return sendReply(RPL_CHANNELMODEIS, fromFd, channel_name, ch->getModes());

	if (ch && Channel::containsUser(ch->getOperators(), _users[fromFd]->getNickname()) == false)
		return sendError(ERR_CHANOPRIVSNEEDED, fromFd, channel_name);

	// SET MODES
	else
	{
		std::size_t index = 0;
		std::string	processed_operations;
		std::string	processed_parameters;
		for (std::vector<std::string>::const_iterator it = cmd.mode_operations.begin(); it < cmd.mode_operations.end(); it++)
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
				processed_parameters += " " + cmd.mode_parameters[index];
				index++;
			}
			else if (*it == "-k")
			{
				if (cmd.mode_parameters.size() <= index || ch->checkPassword(cmd.mode_parameters[index]) == false )
				{
					index++;
					continue ;
				}
				ch->setHasPassword(false);
				ch->setPassword(cmd.mode_parameters[index]);
				processed_operations += *it;
				processed_parameters += " " + cmd.mode_parameters[index];
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
				processed_parameters += " " + cmd.mode_parameters[index];
				index++;
			}
			else if (*it == "-l")
			{
				ch->setHasLimit(false);
				processed_operations += *it;
			}
			else if (*it == "+o" || *it == "-o")
			{
				if (cmd.mode_parameters.size() <= index)
					continue ;
				if (!userExists(cmd.mode_parameters[index]))
				{
					sendError(ERR_NOSUCHNICK, fromFd, cmd.mode_parameters[index]);
					index++;
					continue ;
				}
				if (!Channel::containsUser(ch->getUsers(), cmd.mode_parameters[index]))
				{
					index++;
					continue ;
				}
				else if (*it == "+o" && !Channel::containsUser(ch->getOperators(), cmd.mode_parameters[index]))
					ch->addOperator(cmd.mode_parameters[index]);
				else if (*it == "-o" && Channel::containsUser(ch->getOperators(), cmd.mode_parameters[index]))
					ch->removeOperator(cmd.mode_parameters[index]);
				else
				{
					index++;
					continue ;
				}
				processed_operations += *it;
				processed_parameters += " " + cmd.mode_parameters[index];
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
				sendError(ERR_UNKNOWNMODE, fromFd, (*it).substr(1,1));
			}
		}
		sendChannel(
			sendReply("MODE", fromFd, channel_name + " " + processed_operations + " " + processed_parameters),
			channel_name, fromFd);
		// std::string	message = ":" + _users[fromFd] + " MODE " + channel_name + " " + processed_operations + " " + processed_parameters;
		// sendClient(message, fromFd);
		// sendChannel(message, channel_name, fromFd);
	}
	// 461 ERR_NEEDMOREPARAMS
	// 467 ERR_KEYSET
	// 502 ERR_USERSDONTMATCH
	// 501 ERR_UMODEUNKNOWNFLAG
	
	
	// 367 RPL_BANLIST
	// 368 RPL_ENDOFBANLIST
}

void	Server::sendAllClients(std::string response, int fromFd)
{
	for (std::size_t i = 0; i < _pfds.size(); i++)
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
		return sendError(ERR_NORECIPIENT, fromFd);
	if (cmd.message_set == false)
		return sendError(ERR_NOTEXTTOSEND, fromFd);

	for (std::size_t i = 0; i < cmd.channels.size(); i++)
	{
		if (channelExists(cmd.channels[i]))
			sendChannel(":" + _users[fromFd]->getNickname() + " PRIVMSG " + cmd.channels[i] + " :" + cmd.message, cmd.channels[i], fromFd);
		else
			sendError(ERR_NOSUCHNICK, fromFd, cmd.channels[i]); // Docs suggest this, but ERR_NOSUCHCHANNEL seems more appropriate
	}
	for (std::size_t i = 0; i < cmd.users.size(); i++)
	{
		if (cmd.threw_error[i] && cmd.err_response[i] == ERR_NOSUCHCHANNEL)
			sendError(ERR_NOSUCHCHANNEL, fromFd, cmd.users[i]);
		if (userExists(cmd.users[i]))
			sendCommand("PRIVMSG", fromFd, getUserFd(cmd.users[i]), ":" + cmd.message);
			// sendClient(":" + _users[fromFd] + " PRIVMSG " + cmd.message, getUserFd(cmd.users[i]));
		else
			sendError(ERR_NOSUCHNICK, fromFd, cmd.users[i]);
	}
}

// NOTICE command
void	Server::sendNotice(const Command &cmd, int fromFd)
{
	if (cmd.users.empty() && cmd.channels.empty())
		return ; // sendError(ERR_NORECIPIENT, fromFd);
	if (cmd.message_set == false)
		return ; // sendError(ERR_NOTEXTTOSEND, fromFd);

	for (std::size_t i = 0; i < cmd.channels.size(); i++)
	{
		if (channelExists(cmd.channels[i]))
			sendChannel(":" + _users[fromFd]->getNickname() + " NOTICE " + cmd.message, cmd.channels[0], fromFd);
		// else
		// 	sendError(ERR_NOSUCHNICK, fromFd, cmd.channels[i]); // Docs suggest this, but ERR_NOSUCHCHANNEL seems more appropriate
	}
	for (std::size_t i = 0; i < cmd.users.size(); i++)
	{
		if (userExists(cmd.users[i]))
			sendCommand("NOTICE", fromFd, getUserFd(cmd.users[0]), cmd.message);
			// sendClient(":" + _users[fromFd] + " NOTICE " + cmd.message, getUserFd(cmd.users[0]));
		// else
		// 	sendError(ERR_NOSUCHNICK, fromFd, cmd.users[i]);
	}
}

// KICK command
void	Server::kickUser(const Command &cmd, int fromFd)
{
	
	if (cmd.channels.size() == 0 || cmd.users.size() == 0)
		return sendError(ERR_NEEDMOREPARAMS, fromFd);

	std::string channel_name = cmd.channels[0];
	if (Channel::validChannelName(channel_name) == false)
		return sendError(ERR_BADCHANMASK, fromFd, channel_name);
	// Check docs vs no such channel
	if (cmd.threw_error[0] && cmd.err_response[0] == ERR_NOSUCHNICK)
		return sendError(ERR_NOSUCHNICK, fromFd, channel_name);
	Channel *ch = getChannelByName(channel_name);

	if (Channel::containsUser(ch->getUsers(), _users[fromFd]->getNickname()) == false)
		return sendError(ERR_NOTONCHANNEL, fromFd, channel_name);
	if (Channel::containsUser(ch->getOperators(), _users[fromFd]->getNickname()) == false)
		return sendError(ERR_CHANOPRIVSNEEDED, fromFd, channel_name);

	for (std::size_t i = 0; i < cmd.users.size(); i++)
	{
		if (cmd.threw_error[i + 1] && cmd.err_response[i + 1] == ERR_NOSUCHNICK)
		{
			sendError(ERR_NOSUCHNICK, fromFd, cmd.users[i]);
			continue ;
		}
		if (Channel::containsUser(ch->getUsers(), cmd.users[i]) == false)
		{
			sendError(ERR_USERNOTINCHANNEL, fromFd, cmd.users[i] + " " + channel_name);
			continue ;
		}
		// std::string message = ":" + _users[fromFd] + " KICK " + channel_name + " " + cmd.users[i];
		std::string message = channel_name + " " + cmd.users[i];
		if (cmd.message_set)
			message += " :" + cmd.message;
		else
			message += " :Kicked from " + channel_name + " by " + _users[fromFd]->getNickname();
		sendChannel(
			sendReply("KICK", fromFd, message),
			channel_name, fromFd);
		// sendClient(message, fromFd);
		// sendChannel(message, channel_name, fromFd);
		removeUserFromChannel(cmd.users[i], *ch);
	}
	if (ch->getUsers().empty())
		removeChannelFromServer(ch->getName());
}

// INVITE command
void	Server::inviteUser(const Command &cmd, int fromFd)
{
	if (cmd.channels.empty() && cmd.users.empty())
	{
		std::string channel_list = getInvitedChannels(_users[fromFd]->getNickname());
		if (!channel_list.empty())
			sendReply(RPL_INVITELIST, fromFd, "", channel_list);
		sendReply(RPL_ENDOFINVITELIST, fromFd);
		return ;
	}

	if (cmd.channels.empty())
		return sendError(ERR_NEEDMOREPARAMS, fromFd);

	std::string channel_name = cmd.channels[0];

	if ((cmd.threw_error[0] && cmd.err_response[0] == ERR_NOSUCHNICK) || !channelExists(channel_name))
		return sendError(ERR_NOSUCHCHANNEL, fromFd, channel_name);
	if (cmd.threw_error[1] && cmd.err_response[1] == ERR_NOSUCHNICK)
		return sendError(ERR_NOSUCHCHANNEL, fromFd, cmd.users[0]);
	Channel *ch = getChannelByName(channel_name);

	if (Channel::containsUser(ch->getUsers(), _users[fromFd]->getNickname()) == false)
		return sendError(ERR_NOTONCHANNEL, fromFd, channel_name);
	if (ch->getIsInviteOnly() && Channel::containsUser(ch->getOperators(), _users[fromFd]->getNickname()) == false)
		return sendError(ERR_CHANOPRIVSNEEDED, fromFd, channel_name);
	// 443 ERR_USERONCHAN
	if (ch && Channel::containsUser(ch->getUsers(), cmd.users[0]))
		return sendError(ERR_USERONCHANNEL, fromFd, channel_name);

	// 341 RPL_INVITING
	// RESPONSE
	if (!Channel::containsUser(ch->getInvitedUsers(), cmd.users[0]))
		ch->addInvitedUser(cmd.users[0]);
	sendReply(RPL_INVITING, fromFd, "", cmd.users[0] + " " + channel_name, true);
	sendCommand("INVITE", fromFd, getUserFd(cmd.users[0]), cmd.users[0] + " :" + channel_name);
}

// TOPIC command
void	Server::processTopic(const Command &cmd, int fromFd)
{
	if (cmd.channels.empty())
		return sendError(ERR_NEEDMOREPARAMS, fromFd);
	
	std::string channel_name = cmd.channels[0];

	if ((cmd.threw_error[0] && cmd.err_response[0] == ERR_NOSUCHCHANNEL) || channelExists(channel_name) == false)
		return sendError(ERR_NOSUCHCHANNEL, fromFd, channel_name);
	
	Channel *ch = getChannelByName(channel_name);

	if (ch && Channel::containsUser(ch->getUsers(), _users[fromFd]->getNickname()) == false)
		return sendError(ERR_NOTONCHANNEL, fromFd, channel_name);

	if (cmd.message_set == false)
	{
		if (ch->getTopicSetAt().empty())
			sendReply(RPL_NOTOPIC, fromFd, channel_name);
		else
		{
			sendReply(RPL_TOPIC, fromFd, channel_name, ch->getTopic());
			sendReply(RPL_TOPICWHOTIME, fromFd, channel_name, ch->getTopicSetBy() + " " + ch->getTopicSetAt(), true);
		}
		return ;
	}

	if (ch->getHasRestrictTopic() && Channel::containsUser(ch->getOperators(), _users[fromFd]->getNickname()) == false)
		return sendError(ERR_CHANOPRIVSNEEDED, fromFd, channel_name);
	
	ch->setTopic(cmd.message, _users[fromFd]->getNickname());
	sendChannel(
		sendReply("TOPIC", fromFd, channel_name + " " + cmd.message),
		channel_name, fromFd);
	// sendClient(":" + _users[fromFd] + " TOPIC " + channel_name + " " + cmd.message, fromFd);
	// sendChannel(":" + _users[fromFd] + " TOPIC " + channel_name + " " + cmd.message, channel_name, fromFd);
	
}

// PART command
void	Server::leaveChannel(const Command &cmd, int fromFd)
{
	if (cmd.channels.empty())
		return sendError(ERR_NEEDMOREPARAMS, fromFd);

	for (std::size_t i = 0; i < cmd.channels.size(); i++)
	{
		std::string channel_name = cmd.channels[i];

		if (cmd.threw_error[i] && cmd.err_response[i] == ERR_NOSUCHNICK)
		{
			sendError(ERR_NOSUCHNICK, fromFd, channel_name);
			continue ;
		}
		if (channelExists(channel_name) == false)
		{
			sendError(ERR_NOSUCHCHANNEL, fromFd, channel_name);
			continue ;
		}

		Channel	*ch = getChannelByName(channel_name);

		if (Channel::containsUser(ch->getUsers(), _users[fromFd]->getNickname()) == false)
		{
			sendError(ERR_NOTONCHANNEL, fromFd, channel_name);
			continue ;
		}

		// RESPONSE
		removeUserFromChannel(_users[fromFd]->getNickname(), *ch);
		sendChannel(
			sendReply("PART", fromFd, channel_name + " " + cmd.message),
			channel_name, fromFd);
		// std::string message = ":" + _users[fromFd] + " PART " + channel_name + " " + cmd.message;
		// sendClient(message, fromFd);
		// sendChannel(message, channel_name, fromFd);
		if (ch->getUsers().empty())
			removeChannelFromServer(ch->getName());
	}
}

// QUIT command
void	Server::quitServer(const Command &cmd, int fromFd)
{
	std::string	message = "ERROR :closing connection [Quit " + cmd.message + "]";
	sendClient(message, fromFd);
	message = ":" + _users[fromFd]->getNickname() + " QUIT :Quit " + cmd.message;
	sendAllClients(message, fromFd);
	delFromPfds(fromFd);
}

void	Server::checkPassword(const Command &cmd, int fromFd)
{
	if (!cmd.message_set)
		return sendError(ERR_NEEDMOREPARAMS, fromFd);
	if (cmd.message != _password)
		return sendError(ERR_PASSWDMISMATCH, fromFd);
	if (_users[fromFd]->isAuthenticated())
		return sendError(ERR_ALREADYREGISTRED, fromFd);

	if (!hasPassed(fromFd))
		_users[fromFd]->setAuthenticated();
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

/// @brief Gets the list of all users in a channel.
/// @param channel 
/// @return List of all users on channel, space separated with operators indicated with '@'
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

/// @brief Gets the list of all invited users to a channel
/// @param user 
/// @return Space separated list of users
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

std::size_t	Server::getCurrentRegisteredUsers() const
{
	std::size_t count = 0;
	for (std::map<int, User*>::const_iterator it = _users.begin(); it != _users.end(); it++)
	{
		if (it->second->isRegistered())
			count++;
	}
	return count;
}

