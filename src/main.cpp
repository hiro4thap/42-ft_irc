#include "../inc/ft_irc.hpp"
#include "../inc/Server.hpp"

int	to_int(const std::string &str)
{
	std::stringstream	ss(str);
	int	num;
	ss >> num;
	if (ss.fail() || !ss.eof())
		return -1;
	return num;
}

int main(int argc, char *argv[])
{
	if (argc != 3)
	{
		std::cout << "Invalid arguments. Follow the usage below" << std::endl;
		std::cout << "./ircserv <port> <password>" << std::endl;
		return 1;
	}
	int	port = to_int(argv[1]);
	if (port < 0)
	{
		std::cout << "Invalid arguments. The port number should be a positive integer" << std::endl;
		return 1;
	}
	std::string	password = std::string(argv[2]);
	Server	server(port, password);
	int fd = server.getSocketFd();
	if (fd == -1)
		return 1;
	server.launch(fd);

	return 0;
}

