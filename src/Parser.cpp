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

	std::map<std::string, bool(*)(ParsedCommand&, Command&)>::iterator command_function = commands.find(cmd_in.command);

	if (command_function != commands.end())
	{
		return (command_function->second)(cmd_in, cmd_out);
	}
	return true; // ?
}

// JOIN <channel>{,<channel>} [<key>{,<key>}]
bool Parser::validateJoin(ParsedCommand &cmd_in, Command &cmd_out)
{
	std::string str;
	std::string::const_iterator it, test, end;
	if (cmd_in.parameters.size() > 0)
	{
		str = cmd_in.parameters[0];
		it = str.begin();
		end = str.end();
		while (is_char(it, ',') || it == str.begin())
		{
			if (is_char(it, ','))
				it++;
			test = it;
			if (!channel(test, end))
				return false; // ?
			cmd_out.channels.push_back(std::string(it, test));
			it = test;
		}
	}
	if (cmd_in.parameters.size() > 1)
	{
		str = cmd_in.parameters[1];
		it = str.begin();
		end = str.end();
		while (is_char(it, ',') || it == str.begin())
		{
			if (it != str.begin() || cmd_out.keys.size() > 0)
				it++;
			test = it;
			while (nonwhite(test) && !is_char(test, ','))
				test++;
			if (it == test)
				cmd_out.keys.push_back("");
			else
				cmd_out.keys.push_back(std::string(it, test));
			it = test;
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
		std::string::const_iterator end = cmd_in.parameters[0].end();
		if (!nick(it, end) || *it != '\0')
			return false;
		cmd_out.users.push_back(cmd_in.parameters[0]);
		return true;
	}
	return true;
}

// PRIVMSG <target>{,<target>} <text to be sent>
bool Parser::validatePrivmsg(ParsedCommand &cmd_in, Command &cmd_out)
{
	std::string::const_iterator it, end, test_target, test_channel, test_nick;
	if (cmd_in.parameters.size() > 0)
	{
		it = cmd_in.parameters[0].begin();
		end = cmd_in.parameters[0].end();
		
		while (is_char(it, ',') || it == cmd_in.parameters[0].begin())
		{
			if (is_char(it, ','))
				it++;
			test_target = it;
			if (!target(test_target, end))
				return false; // ?
			test_channel = it;
			test_nick = it;
			if (channel(test_channel, end))
			{
				cmd_out.channels.push_back(std::string(it, test_channel));
				it = test_channel;
			}
			else if (nick(test_nick, end))
			{
				cmd_out.users.push_back(std::string(it, test_nick));
				it = test_nick;
			}
		}
	}
	if (cmd_in.trailing.size() > 0)
	{
		cmd_out.message_set = true;
		cmd_out.message = cmd_in.trailing.substr(1, std::string::npos);
	}
	return true;
}

// KICK <channel> <user>{,<user>} [<comment>]
bool Parser::validateKick(ParsedCommand &cmd_in, Command &cmd_out)
{
	std::string str;
	std::string::const_iterator it, test, end;
	if (cmd_in.parameters.size() > 0)
	{
		str = cmd_in.parameters[0];
		it = str.begin();
		end = str.end();
		if (!channel(it, end) && *it != '\0')
			return false; // ?
		cmd_out.channels.push_back(cmd_in.parameters[0]);
	}
	if (cmd_in.parameters.size() > 1)
	{
		str = cmd_in.parameters[1];
		it = str.begin();
		end = str.end();
		while (is_char(it, ',') || it == str.begin())
		{
			if (it != str.begin())
				it++;
			test = it;
			if (!user(test, end))
				return false; // ?
			cmd_out.users.push_back(std::string(it, test));
			it = test;
		}
	}
	if (cmd_in.trailing.size() > 0)
	{
		cmd_out.message_set = true;
		cmd_out.message = cmd_in.trailing.substr(1, std::string::npos);
	}
	return true;
}

// INVITE <nickname> <channel>
bool Parser::validateInvite(ParsedCommand &cmd_in, Command &cmd_out)
{
	std::string str;
	std::string::const_iterator it, end;
	if (cmd_in.parameters.size() > 0)
	{
		str = cmd_in.parameters[0];
		it = str.begin();
		end = str.end();
		if (!nick(it, end) || *it != '\0')
			return false;
		cmd_out.users.push_back(str);
	}
	if (cmd_in.parameters.size() > 1)
	{
		str = cmd_in.parameters[1];
		it = str.begin();
		end = str.end();
		if (!channel(it, end) || *it != '\0')
			return false;
		cmd_out.channels.push_back(str);
	}
	return true;
}

// TOPIC <channel> [<topic>]
bool Parser::validateTopic(ParsedCommand &cmd_in, Command &cmd_out)
{
	std::string str;
	std::string::const_iterator it, end;
	if (cmd_in.parameters.size() > 0)
	{
		str = cmd_in.parameters[0];
		it = str.begin();
		end = str.end();
		if (!channel(it, end) || *it != '\0')
			return false;
		cmd_out.channels.push_back(str);
	}
	if (cmd_in.trailing.size() > 0)
	{
		cmd_out.message_set = true;
		cmd_out.message = cmd_in.trailing.substr(1, std::string::npos);
	}
	return true;
}

// MODE <target> [<modestring> [<mode arguments>...]]
//    Supported Modes: Channel: itkol
bool Parser::validateMode(ParsedCommand &cmd_in, Command &cmd_out)
{
	std::string str;
	std::string::const_iterator it, end;
	if (cmd_in.parameters.size() > 0)
	{
		str = cmd_in.parameters[0];
		it = str.begin();
		end = str.end();
		if (!channel(it, end) || *it != '\0')
			return false;
		cmd_out.channels.push_back(str);
	}
	if (cmd_in.parameters.size() > 1)
	{
		cmd_out.threw_error = !modestring(cmd_in.parameters[1], cmd_out);
	}
	std::size_t i = 2;
	while (i < cmd_in.parameters.size())
	{
		cmd_out.mode_parameters.push_back(cmd_in.parameters[i]);
		i++;
	}
	return true;
}

// <modestring>     ::= <+-> <mode> [<modestring>] 

bool Parser::modestring(std::string &modestring, Command &cmd_out)
{
	char operation = '+';

	std::string::const_iterator it, end;

	it = modestring.begin();
	end = modestring.end();
	while (it != end)
	{
		if (is_in(it, "+-"))
			operation = *it;
		else
		{
			std::string mode_op = "";
			mode_op += operation;
			mode_op += *it;
			cmd_out.mode_operations.push_back(mode_op);
		}
		it++;
	}
	return true;
}


// PART <channel>{,<channel>} [<reason>]
bool Parser::validatePart(ParsedCommand &cmd_in, Command &cmd_out)
{
	std::string str;
	std::string::const_iterator it, end, test;
	if (cmd_in.parameters.size() > 0)
	{
		str = cmd_in.parameters[0];
		it = str.begin();
		end = str.end();
		while (is_char(it, ',') || it == str.begin())
		{
			if (it != str.begin())
				it++;
			test = it;
			if (!channel(test, end))
				return false; // ?
			cmd_out.channels.push_back(std::string(it, test));
			it = test;
		}
	}
	if (cmd_in.trailing.size() > 0)
	{
		cmd_out.message_set = true;
		cmd_out.message = cmd_in.trailing.substr(1, std::string::npos);
	}
	return true;
}

// QUIT <reason>
bool Parser::validateQuit(ParsedCommand &cmd_in, Command &cmd_out)
{
	if (cmd_in.trailing.size() > 0)
	{
		cmd_out.message_set = true;
		cmd_out.message = cmd_in.trailing.substr(1, std::string::npos);
	}
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
	params(it, cmd_in);
	// if (!params(it, cmd_in))
	// {
	// 	return (error(it));
	// }
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
	while (!is_in(it, std::string("\0\r\n", 3)))
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
bool Parser::target(std::string::const_iterator &it, std::string::const_iterator &end)
{
	// _func_stack.push("target");
	if (!to(it, end))
	{
		return false;
	}
	if (is_char(it,','))
	{
		it++;
		if (!target(it, end))
		{
			return false;
			// return (error(it));
		}
	}
	return true;
}

// <to>         ::= <channel> | <user> '@' <servername> | <nick> | <mask>
bool Parser::to(std::string::const_iterator &it, std::string::const_iterator &end)
{
	// _func_stack.push("to");
	if (!channel(it, end))
	{
		/* if (user(it, end))
		{
			if (!is_char(it, '@'))
			{
				return false;
				// return (error(it));
			}
			if (!servername(it, end))
			{
				return false;
				// return (error(it));
			}
			return true;
		}
		else  */if (nick(it, end))
		{
			return true;
		}
		else if (mask(it, end))
		{
			return true;
		}
		else
			return false;
	}
	return true;
}

// <channel>    ::= ('#' | '&') <chstring>
bool Parser::channel(std::string::const_iterator &it, std::string::const_iterator &end)
{
	if (is_char(it,'#') == false && is_char(it, '&') == false)
		return false;
	it++;
	if (!chstring(it, end))
		return false;
		// return error(it);
	return true;
}

// <servername> ::= <host>
bool Parser::servername(std::string::const_iterator &it, std::string::const_iterator &end)
{
	return host(it, end);
}

// <host>       ::= see RFC 952 [DNS:4] for details on allowed hostnames
// <hname>      ::= <name>*["."<name>]
// <name>       ::= <letter>[*[<let-or-digit-or-hyphen>]<let-or-digit>]
bool Parser::host(std::string::const_iterator &it, std::string::const_iterator &end)
{
	// _func_stack.push("host");
	if (!name(it, end))
		return false;
	std::string::const_iterator test = it;
	while (is_char(it, '.'))
	{
		test++;
		if (!name(test, end))
			return false;
			// return (error(it));
	}
	return true;	
}

// <name>       ::= <letter> [*[let-or-digit-or-hyphen]<let-or-digit>]
bool Parser::name(std::string::const_iterator &it, std::string::const_iterator &end)
{
	// _func_stack.push("name");
	if (!letter(it))
	{
		return false;
	}
	it++;
	while (it != end && (letter(it) || number(it) || is_char(it, '-')))
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
bool Parser::mask(std::string::const_iterator &it, std::string::const_iterator &end)
{
	if (is_char(it,'#') == false && is_char(it, '$') == false)
		return false;
	if (!chstring(it, end))
		return false;
	return true;
}

// <chstring>   ::= <any 8bit code except SPACE, BELL, NUL, CR, LF and comma (',')>
bool Parser::chstring(std::string::const_iterator &it, std::string::const_iterator &end)
{
	std::string exclusion_string = std::string(" \b\0\r\n,", 6);
	if (is_in(it, exclusion_string))
		return false;
	it++;
	while (it != end && !is_in(it, exclusion_string))
	{
		it++;
		continue ;
	}
	return true;
}

// OTHER
// <user>       ::= <nonwhite> { <nonwhite> }
bool Parser::user(std::string::const_iterator &it, std::string::const_iterator &end)
{
	if (!nonwhite(it))
	{
		return false;
	}
	while (it != end && nonwhite(it))
	{
		it++;
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
		return true;
	return false;
}

// <nonwhite>   ::= <any 8bit code except SPACE (0x20), NUL (0x0), CR (0xd), and LF (0xa)>
bool Parser::nonwhite(std::string::const_iterator &it)
{
	return (!is_in(it, std::string(" \0\r\n", 4)));
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
