#include "../inc/ft_irc.hpp"
#include "../inc/Server.hpp"

int main()
{
	Server	server(8080, "pass"); //TODO: take args
	int fd = server.getSocketFd();
	if (fd == -1)
		return 1;
	server.launch(fd);

	return 0;
}

