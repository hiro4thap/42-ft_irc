#include "../inc/Validater.hpp"

bool Validater::validateCommand(Command &cmd_out)
{
	if ()
}

// JOIN <channel>{,<channel>} [<key>{,<key>}]
bool Validater::join(Command &cmd_out)
{

}

// NICK <nickname>
bool Validater::nick(Command &cmd_out)
{
	if (cmd_out.parameters.size() > 1)
		return false;
	if (cmd_out.parameters.size() == 1)
	{
		std::string::iterator it = cmd_out.parameters[0].begin();
		for (; it != cmd_out.parameters[0].end(); it++)
		{
			
		}
		
	}
}

// PRIVMSG <target>{,<target>} <text to be sent>
bool Validater::privmsg(Command &cmd_out)
{
	
}

// KICK <channel> <user>{,<user>} [<comment>]
bool Validater::kick(Command &cmd_out)
{
	
}

// INVITE <nickname> <channel>
bool Validater::invite(Command &cmd_out)
{
	
}

// TOPIC <channel> [<topic>]
bool Validater::topic(Command &cmd_out)
{
	
}

// MODE <target> [<modestring> [<mode arguments>...]]
bool Validater::mode(Command &cmd_out)
{
	
}

// PART <channel>{,<channel>} [<reason>]
bool Validater::part(Command &cmd_out)
{
	
}

// QUIT <reason>
bool Validater::quit(Command &cmd_out)
{
	
}
