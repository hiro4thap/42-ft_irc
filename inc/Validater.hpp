#ifndef VALIDATER_HPP
# define VALIDATER_HPP

# include <iostream>
# include <string>
# include <vector>

struct Command
{
	std::string command;
	std::vector<std::string> parameters;
	std::vector<std::string> target;
};

class Validater
{
	public:
		static bool validateCommand(Command &cmd_out);
	private:
		static bool join(Command &cmd_out);
		static bool nick(Command &cmd_out);
		static bool privmsg(Command &cmd_out);
		static bool kick(Command &cmd_out);
		static bool invite(Command &cmd_out);
		static bool topic(Command &cmd_out);
		static bool mode(Command &cmd_out);
		static bool part(Command &cmd_out);
		static bool quit(Command &cmd_out);
};

#endif