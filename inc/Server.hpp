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
# include <string>

enum Replies
{
	RPL_NONE = 300,
	RPL_NOTOPIC = 331,
	RPL_TOPIC = 332,
	RPL_NAMERPLY = 353,
	RPL_ENDOFNAMES = 366,

	ERR_NOSUCHNICK = 401,
	ERR_NOSUCHCHANNEL = 403,
	ERR_CANNOTSENDTOCHAN = 404,
	ERR_TOOMANYCHANNELS = 405,
	// ERR_TOOMANYTARGETS = 407,
	// ERR_NOORIGIN = 409,
	ERR_NORECIPIENT = 411,
	ERR_NOTEXTTOSEND = 412,
	// ERR_NOTOPLEVEL = 413,
	// ERR_WILDTOPLEVEL = 414,
	ERR_INPUTTOOLONG = 417,
	ERR_UNKNOWNCOMMAND = 421,
	ERR_NONICKNAMEGIVEN = 431,
	ERR_ERRONEUSNICKNAME = 432,
	ERR_NICKNAMEINUSE = 433,
	// ERR_NICKCOLLISION = 436,
	ERR_USERNOTINCHANNEL = 441,
	ERR_NOTONCHANNEL = 442,
	ERR_USERONCHANNEL = 443,
	// ERR_USERDISABLED = 446,
	ERR_NOTREGISTERED = 451,
	ERR_NEEDMOREPARAMS = 461,
	ERR_ALREADYREGISTRED = 462,
	// ERR_NOPERMFORHOST = 463,
	ERR_PASSWDMISMATCH = 464,
	// ERR_YOUREBANNEDCREEP = 465,
	// ERR_KEYSET = 467,
	ERR_CHANNELISFULL = 471,
	ERR_UNKNOWNMODE = 472,
	ERR_INVITEONLYCHAN = 473,
	// ERR_BANNEDFROMCHAN = 474,
	ERR_BADCHANNELKEY = 475,
	ERR_BADCHANMASK = 476,
	ERR_NOPRIVILEGES = 481,
	ERR_CHANOPRIVSNEEDED = 482,

	ERR_UMODEUNKNOWNFLAG = 501,
	// ERR_USERSDONTMATCH = 502,

	ERR_INVALIDKEY = 525
};

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
	std::vector<int>			_passed_fds;

	void			addToPfds(int fd);
	void			delFromPfds(int fromFd);
	Channel			*getChannelByName(const std::string &name);
	bool			removeChannelFromServer(const std::string &channel_name);

	void			sendError(enum Replies err_code, const Command &cmd, int requesting_client_fd, std::string extra_prefix = "");

	bool			hasPassed(int fd);	

	void			setNickname(const Command &cmd, int fromFd);
	void			joinChannel(const Command &cmd, int fromFd);
	void			sendMessage(const Command &cmd, int fromFd);
	void			kickUser(const Command &cmd, int fromFd);
	void			inviteUser(const Command &cmd, int fromFd);
	void			processTopic(const Command &cmd, int fromFd);
	void			processMode(const Command &cmd, int fromFd);
	void			leaveChannel(const Command &cmd, int fromFd);
	void			quitServer(const Command &cmd, int fromFd);
	void			checkPassword(const Command &cmd, int fromFd);

	void			processCommand(std::string command, int fd);
	void			sendClient(std::string response, int toFd);
	void			sendChannel(std::string response, const std::string &channel, int fromFd);
	void			sendAllClients(std::string response, int fromFd);

	bool			userExists(const std::string &value);
	int				getUserFd(const std::string &value);
	bool			channelExists(const std::string &channel_str);
	bool			removeUserFromChannel(const std::string &user, Channel &channel);

	const std::string	getNameList(const Channel *channel) const;
	const std::string	getInvitedChannels(const std::string &user) const;
};

#endif
