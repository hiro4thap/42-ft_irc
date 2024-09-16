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

bool Parser::out(bool value)
{
	_func_stack.pop();
	return value;
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


// <letter>     ::= 'a' ... 'z' | 'A' ... 'Z'
bool Parser::letter(std::string::const_iterator &it)
{
	if ((*it < 'a' || *it > 'z') && (*it < 'A' || *it > 'Z'))
		return false;
	it++;
	return true;
}

// <number>     ::= '0' ... '9'
bool Parser::digit(std::string::const_iterator &it)
{
	if (*it < '0' || *it > '9')
		return false;
	it++;
	return true;
}

bool Parser::is_char(std::string::const_iterator &it, char c)
{
	if (*it != c)
		return false;
	it++;
	return true;
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

bool Parser::special(std::string::const_iterator &it)
{
	if (is_in(it, std::string("-[]\\`^{}", 8)))
		return true;
	return true;
}


// Based on https://modern.ircdocs.horse/
//  message ::= ['@' <tags> SPACE] [':' <source> SPACE] <command> <parameters> <crlf>
bool Parser::message(const std::string message, Command &cmd_out)
{
	this->_message = message;
	std::string::const_iterator it = message.begin();
	std::string::const_iterator test = it;

	_func_stack.push("message");

	if (is_char(test, '@'))
	{
		if (!tags(test))
			return error(it);
		if (!space(it))
			return error(it);
	}
	if (is_char(test, ':'))
	{
		if (!source(it))
			return error(it);
		if (!space(it))
			return error(it);
	}
	if (!command(it))
		return out(false);
	if (!parameters(it))
		return out(false);
	if (!crlf(it))
		return out(false);
	return out(true);
}

// SPACE ::=  ' ' *( ' ' )   ; space character(s)
bool Parser::space(std::string::const_iterator &it)
{
	_func_stack.push("space");
	if (!is_char(it, ' '))
		return (out(false));
	while (is_char(it, ' '))
		continue ;
	return (out(true));
}

// crlf ::=  '\r' '\n'        ; "carriage return" "linefeed"
bool Parser::crlf(std::string::const_iterator &it)
{
	_func_stack.push("crlf");
	if (!is_char(it, '\r'))
		return (out(false));
	if (!is_char(it, '\n'))
		return error(it);
	return (out(true));
}

// TAGS
// <tags> ::= <tag> [';' <tag>]*
bool Parser::tags(std::string::const_iterator &it)
{
	_func_stack.push("tags");
	if (!tag(it))
		return (out(false));
	while (is_char(it, ';'))
	{
		if (!tag(it))
			return error(it);
	}
	return (out(true));
}

// <tag> ::= <key> ['=' <escaped value>]
bool Parser::tag(std::string::const_iterator &it)
{
	_func_stack.push("tag");
	if (!key(it))
		return (out(false));
	if (is_char(it, '='))
	{
		if (!escapedValue(it))
			return error(it);
	}
	return (out(true));
}

// <key> ::= [ <client_prefix> ] [ <vendor> '/' ] <sequence of letters, digits, hyphens (`-`)>
bool Parser::key(std::string::const_iterator &it)
{
	_func_stack.push("key");
	std::string::const_iterator test = it;
	if (clientPrefix(test))
		it = test;
	test = it;
	if (vendor(test))
	{
		it = test;
		if (!is_char(it, '/'))
			return (error(it));
	}
	test = it;
	if (!(letter(test) || digit(test) || is_char(test, '-')))
		return (out(false));
	while (letter(test) || digit(test) || is_char(test, '-'))
		continue ;
	return out(true);
}

// <client_prefix> ::= '+'
bool Parser::clientPrefix(std::string::const_iterator &it)
{
	if (is_char(it, '+'))
		return (true);
	return (false);
}

// <escaped value> ::= <sequence of any characters except NUL, CR, LF, semicolon (`;`) and SPACE>
bool Parser::escapedValue(std::string::const_iterator &it)
{
	_func_stack.push("escapedValue");
	std::string exclusion_string = std::string("\0\r\n; ", 5);
	if (is_in(it, exclusion_string))
		return (out(false));
	while (is_in(it, exclusion_string))
		continue ;
	return (out(true));
}

// <vendor> ::= <host>
bool Parser::vendor(std::string::const_iterator &it)
{
	_func_stack.push("vendor");
}

// SOURCE
// source ::=  <servername> / ( <nickname> [ "!" <user> ] [ "@" <host> ] )
bool Parser::source(std::string::const_iterator &it)
{
	_func_stack.push("source");
}

// nick ::=  <any characters except NUL, CR, LF, chantype character, and SPACE> <possibly empty sequence of any characters except NUL, CR, LF, and SPACE>
bool Parser::nick(std::string::const_iterator &it)
{
	_func_stack.push("nick");
	std::string exclusion_string = std::string("\0\r\n ", 4);
	if (is_in(it, exclusion_string))
		return (out(false));
	while (is_in(it, exclusion_string))
		continue ;
	return (out(true));
}

// user ::=  <sequence of any characters except NUL, CR, LF, and SPACE>
bool Parser::user(std::string::const_iterator &it)
{
	_func_stack.push("user");
}

// COMMAND
// command ::=  letter* / 3digit
bool Parser::command(std::string::const_iterator &it)
{
	_func_stack.push("command");
}

// PARAMETERS
// parameters ::=  *( SPACE middle ) [ SPACE ":" trailing ]
bool Parser::parameters(std::string::const_iterator &it)
{
	_func_stack.push("parameters");
}

// nospcrlfcl ::=  <sequence of any characters except NUL, CR, LF, colon (`:`) and SPACE>
bool Parser::nospcrlfcl(std::string::const_iterator &it)
{
	_func_stack.push("nospcrlfcl");
}

// middle ::=  nospcrlfcl *( ":" / nospcrlfcl )
bool Parser::middle(std::string::const_iterator &it)
{
	_func_stack.push("middle");
}

// trailing ::=  *( ":" / " " / nospcrlfcl )
bool Parser::trailing(std::string::const_iterator &it)
{
	_func_stack.push("trailing");
}


