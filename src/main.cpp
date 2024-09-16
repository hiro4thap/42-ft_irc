#include "../inc/ft_irc.hpp"
#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <set>

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


void test(std::string message)
{
	std::cout << "Received \"" + message + "\"" << std::endl;
	std::string input = message + "\r\n";

	Parser parser;
	std::cout << "Message is: ";
	if (parser.message(input.begin()))
		std::cout << "Valid" << std::endl;
	else
		std::cout << "Invalid" << std::endl;;
	std::cout << std::endl;
}

int main(void)
{
	test("NICK Hal");
	test("MOTD");
	test("JOIN #Blah");
	test("KICK #Blah Hal");
	test("JOIN #foo,#bar fubar,foobar");
	return 0;
}
