#ifndef SERVER_HPP
# define SERVER_HPP

# include "ft_irc.hpp"
# include "Channel.hpp"

# include <cstring>
# include <iostream>
# include <algorithm>
# include <vector>
# include <map>
# include <sstream>

class	Server
{
public:
	Server();
	~Server();
	Server(unsigned int port, std::string passwrod);
	int		getSocketFd();
	void	launch(int serverSocket);
	
private:
	struct pollfd				*_pfds; //TODO: type is to be changed as vector<struct pollfd>
	unsigned int				_port;
	std::string					_password;
	unsigned int				_capacity;
	unsigned int				_size;
	std::vector<Channel>		_channels;
	std::map<int, std::string>	_users;

	void			addToPfds(int fd);
	void			delFromPfds(int fromFd);
	bool			checkPassword(const std::string &password) const;
	Channel			*getChannelByName(const std::string &name);
	// NICK command
	void			setNickname(const std::string &nickname, int fd);
	// JOIN command
	void			joinChannel(const std::string &channel, int fd, const std::string &password = "");
	// PRIVMSG command
	void			sendToChannel(const std::string &channel, const std::string &message ,int fd);
	void			sendToUser(const std::string &user, const std::string &message, int fd);
	// KICK command
	void			kickUser(const std::string &channel, const std::string &user, int fd, const std::string &comment = "");
	// INVITE command
	void			inviteUser(const std::string &channel, const std::string &user, int fd);
	// TOPIC command
	void			setTopic(const std::string &channel, int fd, const std::string topic = "");
	// MODE command
	void			setMode(const std::string &channel, const std::string mode, int fd, const std::string &parameters = "");
	// PART command
	void			leaveChannel(const std::string &channel, int fd, const std::string &reason = "");
	// QUIT command
	void			quitServer(int fd, const std::string &comment = "");

	void			processCommand(std::string command, int fd);
	void			sendClient(std::string response, int toFd);
	void			sendChannel(std::string response, const std::string &channel, int fromFd);
	void			sendAllClients(std::string response, int fromFd);

	bool			userExists(const std::string &value);
	int				getUserFd(const std::string &value);
	bool			channelExists(const std::string &channel_str, Channel **channel = 0);
	const std::string	getNameList(const Channel *channel) const;
	const std::string	getInvitedChannels(const std::string &user) const;
};

#endif
