#ifndef PARSER_HPP
# define PARSER_HPP

# include <string>
# include <iostream>
# include <set>
# include <vector>
# include <stack>

struct Command
{
	std::string command;
	std::vector<std::string> parameters;
};

class Parser
{
	public:
		Parser();

		//  message ::= ['@' <tags> SPACE] [':' <source> SPACE] <command> <parameters> <crlf>
		bool message(const std::string message, Command &cmd_out);
	
	private: 
		bool error(std::string::const_iterator &it);

		// SPACE ::=  ' ' *( ' ' )   ; space character(s)
		bool space(std::string::const_iterator &it);

		// crlf ::=  '\r' '\n'        ; "carriage return" "linefeed"
		bool crlf(std::string::const_iterator &it);

		// TAGS
		// <tags> ::= <tag> [';' <tag>]*
		bool tags(std::string::const_iterator &it);

		// <tag> ::= <key> ['=' <escaped value>]
		bool tag(std::string::const_iterator &it);

		// <key> ::= [ <client_prefix> ] [ <vendor> '/' ] <sequence of letters, digits, hyphens (`-`)>
		bool key(std::string::const_iterator &it);

		// <client_prefix> ::= '+'
		bool clientPrefix(std::string::const_iterator &it);

		// <escaped value> ::= <sequence of any characters except NUL, CR, LF, semicolon (`;`) and SPACE>
		bool escapedValue(std::string::const_iterator &it);

		// <vendor> ::= <host>
		bool vendor(std::string::const_iterator &it);

		// SOURCE
		// source ::=  <servername> / ( <nickname> [ "!" <user> ] [ "@" <host> ] )
		bool source(std::string::const_iterator &it);

		// nick ::=  <any characters except NUL, CR, LF, chantype character, and SPACE> <possibly empty sequence of any characters except NUL, CR, LF, and SPACE>
		bool nick(std::string::const_iterator &it);

		// user ::=  <sequence of any characters except NUL, CR, LF, and SPACE>
		bool user(std::string::const_iterator &it);

		// COMMAND
		// command ::=  letter* / 3digit
		bool command(std::string::const_iterator &it);

		// PARAMETERS
		// parameters ::=  *( SPACE middle ) [ SPACE ":" trailing ]
		bool parameters(std::string::const_iterator &it);

		// nospcrlfcl ::=  <sequence of any characters except NUL, CR, LF, colon (`:`) and SPACE>
		bool nospcrlfcl(std::string::const_iterator &it);

		// middle ::=  nospcrlfcl *( ":" / nospcrlfcl )
		bool middle(std::string::const_iterator &it);

		// trailing ::=  *( ":" / " " / nospcrlfcl )
		bool trailing(std::string::const_iterator &it);


		// 
		bool letter(std::string::const_iterator& it);

		bool digit(std::string::const_iterator& it);

		bool is_char(std::string::const_iterator& it, char c);

		bool is_in(std::string::const_iterator& it, std::string str);

		bool special(std::string::const_iterator& it);

		std::set<std::string>		_supported_commands;
		std::string					_message;
		std::stack<std::string>		_func_stack;

		bool out(bool value);
};

#endif
