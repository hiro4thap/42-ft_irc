#ifndef SERVER_HPP
# define SERVER_HPP

# include "ft_irc.hpp"

# include <cstring>
# include <iostream>

class	Server
{
public:
	Server();
	~Server();
	Server(unsigned int port, std::string passwrod);
	int		getSocketFd();
	void	launch(int serverSocket);
	
private:
	struct pollfd	*_pfds;
	unsigned int	_port;
	std::string		_password;
	unsigned int	_capacity;
	unsigned int	_size;
	// vector<Channel>	_channels;

	void			addToPfds(int fd);
	void			delFromPfds(int index);
};

#endif
