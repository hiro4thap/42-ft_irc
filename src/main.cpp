#include "../inc/ft_irc.hpp"
#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <set>

static std::vector<std::string> tokenise(std::string input)
{
	std::vector<std::string> tokens;

	std::stringstream ss(input);

	std::string token;
	while (getline(ss, token, ' '))
	{
		if (!token.empty())
		{
			tokens.push_back(token);
		}
	}
	return tokens;
}






enum Replies
{
	RPL_NONE = 300,
	RPL_NOTOPIC = 331,
	RPL_TOPIC = 332,
	RPL_NAMERPLY = 353,
	RPL_ENDOFNAMES = 366,

	ERR_NOSUCHNICK = 401,
	ERR_NOSUCHCHNNEL = 403,
	ERR_CANNOTSENDTOCHAN = 404,
	ERR_TOOMANYCHANNELS = 405,
	ERR_TOOMANYTARGETS = 407,
	ERR_NOORIGIN = 409,
	ERR_NORECIPIENT = 411,
	ERR_NOTEXTTOSEND = 412,
	ERR_NOTOPLEVEL = 413,
	ERR_WILDTOPLEVEL = 414,
	ERR_UNKNOWNCOMMAND = 421,
	ERR_NONICKNAMEGIVEN = 431,
	ERR_ERRONEUSNICKNAME = 432,
	ERR_NICKNAMEINUSE = 433,
	ERR_NICKCOLLISION = 436,
	ERR_USERNOTINCHANNEL = 441,
	ERR_NOTONCHANNEL = 442,
	ERR_USERONCHANNEL = 443,
	ERR_USERDISABLED = 446,
	ERR_NOTREGISTERED = 451,
	ERR_NEEDMOREPARAMS = 461,
	ERR_ALREADYREGISTRED = 462,
	ERR_NOPERMFORHOST = 463,
	ERR_PASSWDMISMATCH = 464,
	ERR_YOUREBANNEDCREEP = 465,
	ERR_KEYSET = 467,
	ERR_CHANNELISFULL = 471,
	ERR_UNKNOWNMODE = 472,
	ERR_INVITEONLYCHAN = 473,
	ERR_BANNEDFROMCHAN = 474,
	ERR_BADCHANNELKEY = 475,
	ERR_NOPRIVILEGES = 481,
	ERR_CHANOPRIVSNEEDE = 482,

	ERR_UMODEUNKNOWNFLAG = 501,
	ERR_USERSDONTMATCH = 502
};

std::string send_reply(enum Replies reply, std::vector<std::string> tokens)
{
	if (ERR_NONICKNAMEGIVEN)
	{
		return ":No nickname given";
	}
	else if (RPL_TOPIC)
	{
		std::string channel = getChannel()
		return getChannel() + " :" + getTopic()
	}
}

std::vector<std::string> commandNick(std::vector<std::string> tokens)
{
	std::vector<std::string> response;

	if (tokens.size() == 1)
		response.push_back(send_reply(ERR_NONICKNAMEGIVEN, tokens));

}

std::vector<std::string> commandJoin(std::vector<std::string> tokens)
{
	std::vector<std::string> response;

	response.push_back(send_reply(RPL_TOPIC, tokens));
	response.push_back(send_reply(RPL_NAMERPLY, tokens));
	response.push_back(send_reply(RPL_ENDOFNAMES, tokens));


}

int main(void)
{
	std::set<std::string> commands;
	commands.insert("NICK");
	commands.insert("JOIN");
	commands.insert("PRIVMSG");
	commands.insert("KICK");
	commands.insert("INVITE");
	commands.insert("TOPIC");
	commands.insert("MODE");

	std::string input;
	getline(std::cin, input);

	std::vector<std::string> tokens = tokenise(input);
	if (commands.find(tokens.at(0)) != commands.end())
	{
		std::cout << tokens.at(0) << std::endl;
	}

	return 0;
}
