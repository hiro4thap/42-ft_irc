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
	Command cmd_out;
	cmd_out.command = "";
	cmd_out.err_response = 0;
	cmd_out.threw_error = false;
	cmd_out.message_set = false;
	cmd_out.message = "";
	if (parser.message(input, cmd_out))	
	{
		std::cout << "Valid" << std::endl;
		std::cout << "Command:        \"" << cmd_out.command << "\"" << std::endl;
		std::cout << "Users Count:     " << cmd_out.users.size() << std::endl;
		for (std::size_t i = 0; i < cmd_out.users.size(); i++)
		{
			std::cout << "    [" << i << "]:            \"" << cmd_out.users[i] << "\"" << std::endl;
		}
		std::cout << "Channel Count:   " << cmd_out.channels.size() << std::endl;
		for (std::size_t i = 0; i < cmd_out.channels.size(); i++)
		{
			std::cout << "    [" << i << "]:            \"" << cmd_out.channels[i] << "\"" << std::endl;
		}
		std::cout << "Key Count:       " << cmd_out.keys.size() << std::endl;
		for (std::size_t i = 0; i < cmd_out.keys.size(); i++)
		{
			std::cout << "    [" << i << "]:            \"" << cmd_out.keys[i] << "\"" << std::endl;
		}
		std::cout << "Modes Set:       " << cmd_out.mode_operations.size() << std::endl;
		for (std::size_t i = 0; i < cmd_out.mode_operations.size(); i++)
		{
			std::cout << "    [" << i << "]:            \"" << cmd_out.mode_operations[i] << "\"" << std::endl;
		}
		if (cmd_out.mode_operations.size() > 0)
		{
			std::cout << "Mode Parameters:       " << cmd_out.mode_parameters.size() << std::endl;
			for (std::size_t i = 0; i < cmd_out.mode_parameters.size(); i++)
			{
				std::cout << "    [" << i << "]:            \"" << cmd_out.mode_parameters[i] << "\"" << std::endl;
			}
		}

		std::cout << "Message Set:     " << ((cmd_out.message_set) ? "True" : "False") << std::endl;
		if (cmd_out.message_set)
			std::cout << "    Message:    \"" << cmd_out.message << "\"" << std::endl;	
		std::cout << "Threw Error:     " << ((cmd_out.threw_error) ? "True" : "False") << std::endl;
		if (cmd_out.threw_error)
			std::cout << "    Error Code:  " << cmd_out.err_response << std::endl;
	}
	else
		std::cout << "Invalid" << std::endl;;
	std::cout << std::endl;
}

int main(void)
{
	std::cout << "Testing JOIN" << std::endl;
	test("JOIN #Blah");
	test("JOIN #foo,#bar fubar,foobar");
	test("JOIN #no-pass,#pass ,password");
	test("JOIN #no-pass,#no-pass-2,#pass ,,password");
	std::cout << std::endl;

	std::cout << "Testing NICK" << std::endl;
	test("NICK Hal");
	std::cout << std::endl;

	std::cout << "Testing PRIVMSG" << std::endl;
	test("PRIVMSG Jeremy,Hiro :Hello there!");
	test("PRIVMSG Angel :yes I'm receiving it !receiving it !'u>(768u+1n) .br");
	std::cout << std::endl;

	std::cout << "Testing KICK" << std::endl;
	test("KICK #Blah Hal");
	std::cout << std::endl;

	std::cout << "Testing INVITE" << std::endl;
	test("INVITE Wiz #Twilight_Zone");
	std::cout << std::endl;

	std::cout << "Testing TOPIC" << std::endl;
	test("TOPIC #test :another topic");
	test("TOPIC #test :");
	test("TOPIC #test");
	std::cout << std::endl;

	std::cout << "Testing MODE" << std::endl;
	test("MODE #test");
	test("MODE #blah +o User");
	test("MODE #a +ok test pass2");
	test("MODE #a +ko test pass2");
	test("MODE #a +o test test2");
	test("MODE #a +o test +o test2");
	test("MODE #a +oo test test2");
	test("MODE #a +o+o test test2");
	test("MODE #a +o-o test test2");
	test("MODE #a -o+o test test2");
	test("MODE #a -k+i-i pass test");
	std::cout << std::endl;

	std::cout << "Testing PART" << std::endl;
	test("PART #test,#test2 :Logging off");
	test("PART #twilight_zone ");
	test("PART #oz-ops,&group5");
	std::cout << std::endl;

	std::cout << "Testing QUIT" << std::endl;
	test("QUIT :Gone to have lunch");
	std::cout << std::endl;

	std::cout << "Testing other" << std::endl;
	test("MOTD");
	std::cout << std::endl;
	
	
	
	
	
	
	
	return 0;
}
