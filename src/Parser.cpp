#include "../inc/Parser.hpp"

Parser::Parser()
{
	_supported_commands.insert("JOIN");
	_supported_commands.insert("NICK");
	_supported_commands.insert("PRIVMSG");
	_supported_commands.insert("KICK");
	_supported_commands.insert("INVITE");
	_supported_commands.insert("TOPIC");
	_supported_commands.insert("MODE");
	_supported_commands.insert("PART");
	_supported_commands.insert("QUIT");
}

void Parser::error(std::string::iterator &it, std::string &func)
{
	std::cout << "Error: " + func + "Error encountered at \'" + *it + "\'" << std::endl;
}

/*
Based on https://datatracker.ietf.org/doc/html/rfc1459
*/

// <message>  ::= [':' <prefix> <SPACE> ] <command> <params> <crlf>
bool Parser::message(std::string::iterator it)
{
	std::string::iterator test;
	Command cmd;
	std::string func = "message";
	if (is_char(it, ':'))
	{
		func += "->:";
		it++;
		if (!prefix(it))
		{
			func = func + "->" + "prefix";
			error(it, func);
			return false;
		}
		if (!space(it))
		{
			func = func + "->" + "space";
			error(it, func);
			return false;
		}
	}
	test = it;
	if (!command(test))
	{
		func = func + "->" + "command";
		error(it, func);
		return false;
	}
	cmd.command = std::string(it, test);
	if (_supported_commands.find(cmd.command) == _supported_commands.end())
		return false;
	it = test;
	if (!params(it, cmd))
	{
		func = func + "->" + "params";
		error(it, func);
		return false;
	}
	if (!crlf(it))
	{
		func = func + "->" + "crlf";
		error(it, func);
		return false;
	}

	std::cout << "Command: \"" << cmd.command << "\"" << std::endl;
	std::cout << "Parameter Count: " << cmd.parameters.size() << std::endl;
	for (std::size_t i = 0; i < cmd.parameters.size(); i++)
	{
		std::cout << "[" << i << "]: \"" << cmd.parameters[i] << "\"" << std::endl;
	}

	return true;
}

// <prefix>   ::= <servername> | <nick> [ '!' <user> ] [ '@' <host> ]
bool Parser::prefix(std::string::iterator &it)
{
	std::string func = "prefix";
	if(!servername(it))
	{
		if (nick(it))
		{
			func = func + "->" + "nick";
			if (is_char(it,'!'))
			{
				if (!user(it))
				{
					func = func + "->" + "is_char";
					func = func + "->" + "user";
					error(it, func);
					return false;
				}
			}
			if (is_char(it,'@'))
			{
				if (!host(it))
				{
					func = func + "->" + "is_char";
					func = func + "->" + "user";
					error(it, func);
					return false;
				}
			}
			return true;
		}
		return false;
	}
	return true;
}


// <command>  ::= <letter> { <letter> } | <number> <number> <number>
bool Parser::command(std::string::iterator &it)
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
bool Parser::space(std::string::iterator& it)
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
bool Parser::params(std::string::iterator& it, Command &cmd)
{
	std::string func = "params";
	std::string::iterator test;
	if (!space(it))
	{
		return false;
	}
	while (true)
	{
		test = it;
		if (is_char(it,':'))
		{
			it++;
			if (!trailing(it))
			{
				func = func + "->" + ":";
				func = func + "->" + "trailing";
				error(it, func);
				return false;
			}
		}
		else if (middle(test))
		{
			cmd.parameters.push_back(std::string(it, test));
			it = test;
			if (!params(it, cmd))
				break ;
			// if (!params(it))
			// {
			// 	func = func + "->" + "middle";
			// 	func = func + "->" + "params";
			// 	error(it, func);
			// 	return false;
			// }
		}
		else
			break;
	}
	return true;
}

// <middle>   ::= <Any *non-empty* sequence of octets not including SPACE
//                or NUL or CR or LF, the first of which may not be ':'>
bool Parser::middle(std::string::iterator& it)
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
bool Parser::trailing(std::string::iterator& it)
{
	while (*it != '\0' && *it != '\r' && *it != '\f')
	{
		it++;
	}
	return true;
}

// <crlf>     ::= CR LF
bool Parser::crlf(std::string::iterator& it)
{
	std::string func = "crlf";
	if (is_char(it, '\r') == false)
	{
		return false;
	}
	it++;
	if (is_char(it, '\n') == false)
	{
		func = func + "->" + "lf";
		error(it, func);
		return false;
	}
	it++;
	return true;
}

// <user>       ::= <nonwhite> { <nonwhite> }
bool Parser::user(std::string::iterator &it)
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
bool Parser::letter(std::string::iterator& it)
{
	if ((*it >= 'a' && *it <='z') || (*it >= 'A' && *it <= 'Z'))
		return true;
	return false;
}

// <number>     ::= '0' ... '9'
bool Parser::number(std::string::iterator& it)
{
	if (*it >='0' && *it <='9')
		return true;
	return false;
}

bool Parser::is_char(std::string::iterator& it, char c)
{
	if (*it == c)
		return true;
	return false;
}

// <special>    ::= '-' | '[' | ']' | '\' | '`' | '^' | '{' | '}'
bool Parser::is_in(std::string::iterator& it, std::string str)
{
	for (std::size_t i = 0; i < str.size(); i++)
	{
		if (is_char(it, str[i]))
			return true;
	}
	return false;
}

bool Parser::special(std::string::iterator& it)
{
	if (is_in(it, std::string("-[]\\`^{}", 8)))
	{
		it++;
		return true;
	}
	return true;
}

bool Parser::letter_digit(std::string::iterator& it)
{
	if (letter(it) || number(it))
		return true;
	return false;
}

bool Parser::letter_digit_dash(std::string::iterator& it)
{
	if (letter(it) || number(it) || is_char(it,'-'))
		return true;
	return false;
}

// <nonwhite>   ::= <any 8bit code except SPACE (0x20), NUL (0x0), CR (0xd), and LF (0xa)>
bool Parser::nonwhite(std::string::iterator& it)
{
	return is_in(it, std::string(" \0\r\n", 4));
}

// <servername> ::= <host>
bool Parser::servername(std::string::iterator &it)
{
	return host(it);
}

// <nick>       ::= <letter> { <letter> | <number> | <special> }
bool Parser::nick(std::string::iterator &it)
{
	if (!letter(it))
	{
		return false;
	}
	while (letter(it) || number(it) || special(it))
	{
		continue;
	}
	return true;
}

// <host>       ::= see RFC 952 [DNS:4] for details on allowed hostnames
// <hname> ::= <name>*["."<name>]
// <name>  ::= <letter>[*[<let-or-digit-or-hyphen>]<let-or-digit>]
bool Parser::host(std::string::iterator &it)
{
	std::string func = "host";
	if (!letter(it))
	{
		return false;
	}
	it++;
	if (letter_digit_dash(it) || letter_digit(it))
	{
		it++;
		while (letter_digit_dash(it))
		{
			it++;
			continue;
		}
		if (!letter_digit(it))
		{
			func = func + "->" + "letter_digit";
			error(it, func);
			return false;
		}
		return true;
	}
	return true;
	
}

// Target stuff
// <target>     ::= <to> [ "," <target> ]
bool Parser::target(std::string::iterator &it)
{
	std::string func = "target";
	if (!to(it))
	{
		return false;
	}
	if (is_char(it,','))
	{
		it++;
		if (!target(it))
		{
			func = func + "->" + "is_char";
			func = func + "->" + "target";
			error(it, func);
			return false;
		}
	}
	return true;
}

// <to>         ::= <channel> | <user> '@' <servername> | <nick> | <mask>
bool Parser::to(std::string::iterator &it)
{
	std::string func = "to";
	if (!channel(it))
	{
		if (user(it))
		{
			func = func + "->" + "user";
			if (!is_char(it, '@'))
			{
				func = func + "->" + "@";
				error(it, func);
				return false;
			}
			if (!servername(it))
			{
				func = func + "->" + "servername";
				error(it, func);
				return false;
			}
			return true;
		}
		else if (nick(it))
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
bool Parser::channel(std::string::iterator &it)
{
	if (is_char(it,'#') == false && is_char(it, '&') == false)
		return false;
	if (!chstring(it))
		return false;
	return true;
}

// <mask>       ::= ('#' | '$') <chstring>
bool Parser::mask(std::string::iterator &it)
{
	if (is_char(it,'#') == false && is_char(it, '$') == false)
		return false;
	if (!chstring(it))
		return false;
	return true;
}

// <chstring>   ::= <any 8bit code except SPACE, BELL, NUL, CR, LF and comma (',')>
bool Parser::chstring(std::string::iterator &it)
{
	if (is_in(it, std::string(" \b\0\r\n,", 6)))
		return false;
	return true;
}

/* 
// Based on https://modern.ircdocs.horse/

bool Parser::message(std::string::iterator it)
{
	if ('@')
	{
		if (!tags())
		{
			error(it, func);
			return false;
		}
		if (!space())
		{
			error(it, func);
			return false;
		}
	}
	if (':')
	{
		if (!source())
		{
			error(it, func)
			return false;
		}
		if (!space())
		{
			error(it, func);
			return false;
		}
	}
	if (!command())
		return false;
	if (!parameters())
		return false;
	if (!crlf())
		return false;
	return true;
}

bool Parser::space(std::string::iterator it)
{
	int count = 0;
	while (' ')
	{
		count ++;
		continue ;
	}
	if (count > 0)
		return true;
	return false;
}

bool Parser::crlf(std::string::iterator it)
{
	if ("\r\f")
		return true;
	return false;
}

bool Parser::tags(std::string::iterator it)
{
	if (!tag())
		return false;
	while (';')
	{
		if (!tag())
		{
			error(it, func);
			return false;
		}
	}
	return true;
}

bool Parser::tag(std::string::iterator it)
{
	if (!key())
		return false;
	if ('=')
	{
		if (!escaped_value())
		{
			error(it, func);
			return false;
		}
	}
	return true;
}

bool Parser::key(std::string::iterator it)
{
	if (client_prefix())
	{

	}
	if (vendor())
	{
		if (!'/')
		{
			error(it, func);
			return false;
		}
	}
	if (!seq_alpha_num_dash())
	{
		return false;
	}
	return true;
}

bool Parser::client_prefix(std::string::iterator it)
{
	if (!'+')
		return false;
	return true;
}

bool Parser::escaped_value(std::string::iterator it)
{
	if (!seq_not_null_cr_lf_semi_sp())
	{
		return false;
	}
	return true;
}

bool Parser::vendor(std::string::iterator it)
{
	if (!host())
		return false;
	return true;
}

bool Parser::source(std::string::iterator it)
{
	if (!servername())
	{
		if (nickname())
		{
			if ('!')
			{
				if (!user())
				{
					error(it, func);
					return false;
				}
			}
			if ('@')
			{
				if (!host())
				{
					error(it, func);
					return false;
				}
			}
			return true;
		}
		else 
			return false;
	}
}

// Needs to be sequence?
bool Parser::seq_not_null_cr_lf_semi_sp(std::string::iterator it)
{
	if ('\0\r\f; ')
		return false;
	return true;
}

// Unclearly defined?
bool Parser::nickname(std::string::iterator it)
{
	// Chan type?
	if (!seq_not_null_cr_lf_semi_sp())
}


bool Parser::user(std::string::iterator it)
{
	if (!seq_not_null_cr_lf_sp())
	{
		return false;
	}
	return true;
}

// letter* / 3digit
// Valid command / command code
bool Parser::command(std::string::iterator it)
{
	return true;
}

bool Parser::parameters(std::string::iterator it)
{
	while(space())
	{
		if (':')
		{
			if (!trailing())
			{
				error(it, func);
				return false;
			}
			return true;
		}
		if (!middle())
		{
			error(it, func);
			return false;
		}
	}
	return true;	
}

bool Parser::nospcrlfcl(std::string::iterator it)
{
	//<sequence of any characters except NUL, CR, LF, colon (`:`) and SPACE>
	return true;
}

bool Parser::middle(std::string::iterator it)
{
	if (!nospcrlfcl())
	{
		return false;
	}
	while (':' || ' ' || nospcrlfcl())
		continue;
	return true;
}

bool Parser::trailing(std::string::iterator it)
{
	while (':' || ' ' || nospcrlfcl())
		continue;
	return true;
} */


