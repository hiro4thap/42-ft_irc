#ifndef SERVER_HPP
# define SERVER_HPP

# include "ft_irc.hpp"
# include "Channel.hpp"

# include <cstring>
# include <iostream>
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
	struct pollfd				*_pfds;
	unsigned int				_port;
	std::string					_password;
	unsigned int				_capacity;
	unsigned int				_size;
	std::vector<Channel>		_channels;
	std::map<int, std::string>	_users;

	void			addToPfds(int fd);
	void			delFromPfds(int index);
	bool			checkPassword(const std::string &password) const;
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
	void			setMode(const std::string &channel, const char mode, int fd, const std::string &limit, const std::string &user);
	// PART command
	void			leaveChannel(const std::string &channel, int fd, const std::string &reason = "");
	// QUIT command
	void			quitServer(int fd, const std::string &comment = "");

	void			processCommand(std::string command, int fd);
	void			sendClient(std::string response, int toFd, int fromFd);

	bool			valueExits(const std::string &value);
	int				findKey(const std::string &value);
	bool			channelExists(const std::string &channel);
};

#endif
