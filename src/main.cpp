#include "ft_irc.hpp"
#include "Server.hpp"

int main()
{
	Server	server(8080, "pass");
	int fd = server.getSocketFd();
	if (fd == -1)
		return 1;
	server.launch(fd);

    return 0;
}

