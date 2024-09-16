#include "../inc/Parser.hpp"

Parser::Parser()
{
	_supported_commands.insert("JOIN");		// JOIN <channel>{,<channel>} [<key>{,<key>}]
	_supported_commands.insert("NICK");		// NICK <nickname>
	_supported_commands.insert("PRIVMSG");	// PRIVMSG <target>{,<target>} <text to be sent>
	_supported_commands.insert("KICK");		// KICK <channel> <user>{,<user>} [<comment>]
	_supported_commands.insert("INVITE");	// INVITE <nickname> <channel>
	_supported_commands.insert("TOPIC");	// TOPIC <channel> [<topic>]
	_supported_commands.insert("MODE");		// MODE <target> [<modestring> [<mode arguments>...]]
	_supported_commands.insert("PART");		// PART <channel>{,<channel>} [<reason>]
	_supported_commands.insert("QUIT");		// QUIT <reason>
}




bool Parser::error(std::string::const_iterator &it)
{
	if (_func_stack.size() == 0)
		return false;
	std::string location = _func_stack.top();
	_func_stack.pop();
	while (this->_func_stack.size() > 0)
	{
		location += ("<-" + _func_stack.top());
		_func_stack.pop();
	}
	std::cout << "Error: " + location + ": Error encountered at \'" + *it + "\'" << std::endl;
	return false;
}

bool Parser::out(bool value)
{
	_func_stack.pop();
	return value;
}

bool Parser::validateCommand(ParsedCommand &cmd_in, Command &cmd_out)
{
	std::size_t s1 = cmd_in.command.size();
	std::size_t s2 = cmd_out.command.size();
	cmd_out.command = cmd_in.command;
	(void) s1;
	(void) s2;

	std::map<std::string, bool(*)(ParsedCommand&, Command&)> commands;
	commands["JOIN"] = &validateJoin;
	commands["NICK"] = &validateNick;
	commands["PRIVMSG"] = &validatePrivmsg;
	commands["KICK"] = &validateKick;
	commands["INVITE"] = &validateInvite;
	commands["TOPIC"] = &validateTopic;
	commands["MODE"] = &validateMode;
	commands["PART"] = &validatePart;
	commands["QUIT"] = &validateQuit;

	try
	{
		return (commands[cmd_in.command])(cmd_in, cmd_out);
	}
	catch (std::exception e)
	{
		return false;
	}
}

// JOIN <channel>{,<channel>} [<key>{,<key>}]
bool Parser::validateJoin(ParsedCommand &cmd_in, Command &cmd_out)
{
	std::string::const_iterator it, test;
	if (cmd_in.parameters.size() > 0)
	{
		it = cmd_in.parameters[0].begin();
		test = it;
		if (!channel(test))
			return false; // ?
		cmd_out.channels.push_back(std::string(it, test));
		it = test;
		while (is_char(it, ','))
		{
			it++;
			test = it;
			channel(test);
			cmd_out.channels.push_back(std::string(it, test));
		}
	}
	if (cmd_in.parameters.size() > 1)
	{
		it = cmd_in.parameters[1].begin();
		test = it;
		if (!channel(test))
			return false; // ?
		cmd_out.channels.push_back(std::string(it, test));
		it = test;
		while (is_char(it, ','))
		{
			it++;
			test = it;
			channel(test);
			cmd_out.channels.push_back(std::string(it, test));
		}
	}
	return true;
}

// NICK <nickname>
bool Parser::validateNick(ParsedCommand &cmd_in, Command &cmd_out)
{

	if (cmd_in.parameters.size() > 0)
	{
		std::string::const_iterator it = cmd_in.parameters[0].begin();
		if (!nick(it) || *it != '\0')
			return false;
		cmd_out.users.push_back(cmd_in.parameters[0]);
		return true;
	}
	return true;
}

// PRIVMSG <target>{,<target>} <text to be sent>
bool Parser::validatePrivmsg(ParsedCommand &cmd_in, Command &cmd_out)
{
	std::string::const_iterator it, test;
	if (cmd_in.parameters.size() > 0)
	{
		it = cmd_in.parameters[0].begin();
		test = it;
		if (!target(test))
			return false; // ?
		cmd_out.channels.push_back(std::string(it, test));
		it = test;
		while (is_char(it, ','))
		{
			it++;
			test = it;
			target(test);
			cmd_out.channels.push_back(std::string(it, test));
		}
	}
	cmd_out.message = cmd_in.trailing;
	return true;
}

// KICK <channel> <user>{,<user>} [<comment>]
bool Parser::validateKick(ParsedCommand &cmd_in, Command &cmd_out)
{
	std::string::const_iterator it, test;
	if (cmd_in.parameters.size() > 0)
	{
		it = cmd_in.parameters[1].begin();
		if (!channel(it) && *it != '\0')
			return false; // ?
		cmd_out.channels.push_back(cmd_in.parameters[0]);
	}
	if (cmd_in.parameters.size() > 1)
	{
		it = cmd_in.parameters[1].begin();
		test = it;
		if (!user(test))
			return false; // ?
		cmd_out.channels.push_back(std::string(it, test));
		it = test;
		while (is_char(it, ','))
		{
			it++;
			test = it;
			user(test);
			cmd_out.channels.push_back(std::string(it, test));
		}
	}
	cmd_out.message = cmd_in.trailing;
	return true;
}

// INVITE <nickname> <channel>
bool Parser::validateInvite(ParsedCommand &cmd_in, Command &cmd_out)
{
	if (cmd_in.parameters.size() > 0)
	{
		std::string::const_iterator it = cmd_in.parameters[0].begin();
		if (!nick(it) || *it != '\0')
			return false;
		cmd_out.users.push_back(cmd_in.parameters[0]);
	}
	if (cmd_in.parameters.size() > 1)
	{
		std::string::const_iterator it = cmd_in.parameters[1].begin();
		if (!channel(it) || *it != '\0')
			return false;
		cmd_out.channels.push_back(cmd_in.parameters[1]);
	}
	return true;
}

// TOPIC <channel> [<topic>]
bool Parser::validateTopic(ParsedCommand &cmd_in, Command &cmd_out)
{
	if (cmd_in.parameters.size() > 0)
	{
		std::string::const_iterator it = cmd_in.parameters[0].begin();
		if (!channel(it) || *it != '\0')
			return false;
		cmd_out.channels.push_back(cmd_in.parameters[0]);
	}
	cmd_out.message = cmd_in.trailing;
	return true;
}

// MODE <target> [<modestring> [<mode arguments>...]]
bool Parser::validateMode(ParsedCommand &cmd_in, Command &cmd_out)
{
	(void) cmd_in;
	(void) cmd_out;
	return true;
}

// PART <channel>{,<channel>} [<reason>]
bool Parser::validatePart(ParsedCommand &cmd_in, Command &cmd_out)
{
	std::string::const_iterator it, test;
	if (cmd_in.parameters.size() > 0)
	{
		it = cmd_in.parameters[0].begin();
		test = it;
		if (!channel(test))
			return false; // ?
		cmd_out.channels.push_back(std::string(it, test));
		it = test;
		while (is_char(it, ','))
		{
			it++;
			test = it;
			channel(test);
			cmd_out.channels.push_back(std::string(it, test));
		}
	}
	cmd_out.message = cmd_in.trailing;
	return true;
}

// QUIT <reason>
bool Parser::validateQuit(ParsedCommand &cmd_in, Command &cmd_out)
{
	cmd_out.message = cmd_in.trailing;
	return true;
}

/*
Based on https://datatracker.ietf.org/doc/html/rfc1459
*/

// <message>  ::= [':' <prefix> <SPACE> ] <command> <params> <crlf>
bool Parser::message(std::string message, Command &cmd_out)
{
	_func_stack.push("message");
	ParsedCommand cmd_in;

	std::string::const_iterator it = message.begin();
	std::string::const_iterator test = it;

	// Client to not send a source/prefix	
	if (is_char(it, ':'))
		return false;
	test = it;
	if (!command(test))
	{
		return (error(it));
	}
	cmd_in.command = std::string(it, test);
	if (_supported_commands.find(cmd_in.command) == _supported_commands.end())
		return false;
	it = test;
	if (!params(it, cmd_in))
	{
		return (error(it));
	}
	if (!crlf(it))
	{
		return (error(it));
	}
	bool result = validateCommand(cmd_in, cmd_out);
	return result;
}

// // <prefix>   ::= <servername> | <nick> [ '!' <user> ] [ '@' <host> ]
// bool Parser::prefix(std::string::const_iterator &it)
// {
// 	_func_stack.push("prefix");
// 	if(!servername(it))
// 	{
// 		if (nick(it))
// 		{
// 			if (is_char(it,'!'))
// 			{
// 				if (!user(it))
// 				{
// 					return error(it);
// 				}
// 			}
// 			if (is_char(it,'@'))
// 			{
// 				if (!host(it))
// 				{
// 					return error(it);
// 				}
// 			}
// 			return true;
// 		}
// 		return false;
// 	}
// 	return true;
// }


// <command>  ::= <letter> { <letter> } | <number> <number> <number>
bool Parser::command(std::string::const_iterator &it)
{
	if (!letter(it))
	{
		if (number(it) && number(it) && number(it))
		{
			return true;
		}
		return false;
	}
	it++;
	while (letter(it))
	{
		it++;
		continue ;
	}
	return true;
}

// <SPACE>    ::= ' ' { ' ' }
bool Parser::space(std::string::const_iterator& it)
{
	if (*it != ' ')
		return false;
	it++;
	while (*it == ' ')
	{
		it++;
		continue;
	}
	return true;
}

// <params>   ::= <SPACE> [ ':' <trailing> | <middle> <params> ]
bool Parser::params(std::string::const_iterator& it, ParsedCommand &cmd_out)
{
	_func_stack.push("params");
	std::string::const_iterator test;

	if (!space(it))
		return false;
	while (true)
	{
		test = it;
		if (is_char(test,':'))
		{
			test++;
			if (!trailing(test))
				return (error(it));
			cmd_out.trailing = std::string(it, test);
			it = test;
			break ;
		}
		else if (middle(test))
		{
			cmd_out.parameters.push_back(std::string(it, test));
			it = test;
			if (!params(it, cmd_out))
				break ;
		}
		else
			break;
	}
	return true;
}

// <middle>   ::= <Any *non-empty* sequence of octets not including SPACE
//                or NUL or CR or LF, the first of which may not be ':'>
bool Parser::middle(std::string::const_iterator &it)
{
	int count = 0;
	if (is_char(it, ':'))
		return false;
	while (is_in(it, std::string(" \0\r\n", 4)) == false)
	{
		it++;
		count++;
	}
	if (count > 0)
		return true;
	return false;
}

// <trailing> ::= <Any, possibly *empty*, sequence of octets not including NUL or CR or LF>
bool Parser::trailing(std::string::const_iterator &it)
{
	while (is_in(it, std::string("\0\r\n", 3)))
	{
		it++;
	}
	return true;
}

// <crlf>     ::= CR LF
bool Parser::crlf(std::string::const_iterator &it)
{
	_func_stack.push("crlf");
	if (is_char(it, '\r') == false)
	{
		return false;
	}
	it++;
	if (is_char(it, '\n') == false)
	{
		return (error(it));
	}
	it++;
	return true;
}

// TARGETS
// <target>     ::= <to> [ "," <target> ]
bool Parser::target(std::string::const_iterator &it)
{
	// _func_stack.push("target");
	if (!to(it))
	{
		return false;
	}
	if (is_char(it,','))
	{
		it++;
		if (!target(it))
		{
			return false;
			// return (error(it));
		}
	}
	return true;
}

// <to>         ::= <channel> | <user> '@' <servername> | <nick> | <mask>
bool Parser::to(std::string::const_iterator &it)
{
	// _func_stack.push("to");
	if (!channel(it))
	{
		if (user(it))
		{
			if (!is_char(it, '@'))
			{
				return false;
				// return (error(it));
			}
			if (!servername(it))
			{
				return false;
				// return (error(it));
			}
			return true;
		}
		else if (nick(it,))
		{
			return true;
		}
		else if (mask(it))
		{
			return true;
		}
		else
			return false;
	}
	return true;
}

// <channel>    ::= ('#' | '&') <chstring>
bool Parser::channel(std::string::const_iterator &it)
{
	if (is_char(it,'#') == false && is_char(it, '&') == false)
		return false;
	it++;
	if (!chstring(it))
		return false;
		// return error(it);
	return true;
}

// <servername> ::= <host>
bool Parser::servername(std::string::const_iterator &it)
{
	return host(it);
}

// <host>       ::= see RFC 952 [DNS:4] for details on allowed hostnames
// <hname>      ::= <name>*["."<name>]
// <name>       ::= <letter>[*[<let-or-digit-or-hyphen>]<let-or-digit>]
bool Parser::host(std::string::const_iterator &it)
{
	// _func_stack.push("host");
	if (!name(it))
		return false;
	std::string::const_iterator test = it;
	while (is_char(it, '.'))
	{
		test++;
		if (!name(test))
			return false;
			// return (error(it));
	}
	return true;	
}

// <name>       ::= <letter> [*[let-or-digit-or-hyphen]<let-or-digit>]
bool Parser::name(std::string::const_iterator &it)
{
	// _func_stack.push("name");
	if (!letter(it))
	{
		return false;
	}
	it++;
	while (letter(it) || number(it) || is_char(it, '-'))
	{
		std::string::const_iterator test = it;
		if (is_char(it, '-'))
		{
			test++;
			// if 'it' is the last character, return error.
			if (!(letter(test) || number(test) || is_char(test, '-')))
				return false;
				// return (error(it));
		}
		it++;
	}
	return true;
}

// <nick>       ::= <letter> { <letter> | <number> | <special> }
bool Parser::nick(std::string::const_iterator &it, std::string::const_iterator &end)
{
	if (!letter(it))
	{
		return false;
	}
	it++;
	while (letter(it) || number(it) || special(it))
	{
		it++;
		if (it == end)
			break ;
		continue;
	}
	return true;
}

// <mask>       ::= ('#' | '$') <chstring>
bool Parser::mask(std::string::const_iterator &it)
{
	if (is_char(it,'#') == false && is_char(it, '$') == false)
		return false;
	if (!chstring(it))
		return false;
	return true;
}

// <chstring>   ::= <any 8bit code except SPACE, BELL, NUL, CR, LF and comma (',')>
bool Parser::chstring(std::string::const_iterator &it)
{
	std::string exclusion_string = std::string(" \b\0\r\n,", 6);
	if (is_in(it, exclusion_string))
		return false;
	it++;
	while (!is_in(it, exclusion_string))
	{
		it++;
		continue ;
	}
	return true;
}

// OTHER
// <user>       ::= <nonwhite> { <nonwhite> }
bool Parser::user(std::string::const_iterator &it)
{
	if (!nonwhite(it))
	{
		return false;
	}
	while (nonwhite(it))
	{
		continue;
	}
	return true;
}

// <letter>     ::= 'a' ... 'z' | 'A' ... 'Z'
bool Parser::letter(std::string::const_iterator &it)
{
	if ((*it >= 'a' && *it <='z') || (*it >= 'A' && *it <= 'Z'))
		return true;
	return false;
}

// <number>     ::= '0' ... '9'
bool Parser::number(std::string::const_iterator &it)
{
	if (*it >='0' && *it <='9')
		return true;
	return false;
}

// <special>    ::= '-' | '[' | ']' | '\' | '`' | '^' | '{' | '}'
bool Parser::special(std::string::const_iterator &it)
{
	if (is_in(it, std::string("-[]\\`^{}", 8)))
	{
		it++;
		return true;
	}
	return true;
}

// <nonwhite>   ::= <any 8bit code except SPACE (0x20), NUL (0x0), CR (0xd), and LF (0xa)>
bool Parser::nonwhite(std::string::const_iterator &it)
{
	return is_in(it, std::string(" \0\r\n", 4));
}


// HELPERS
bool Parser::is_char(std::string::const_iterator &it, char c)
{
	if (*it == c)
		return true;
	return false;
}


bool Parser::is_in(std::string::const_iterator &it, std::string str)
{
	for (std::size_t i = 0; i < str.size(); i++)
	{
		if (is_char(it, str[i]))
			return true;
	}
	return false;
}
