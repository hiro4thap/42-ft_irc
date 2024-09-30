#ifndef PARSER_HPP
# define PARSER_HPP

# include <iostream>
# include <string>
# include <vector>
# include <stack>
# include <set>
# include <map>

struct ParsedCommand
{
	std::string					command;
	std::vector<std::string>	parameters;
	std::string					trailing;
};

struct Command
{
	std::string					command;
	std::vector<std::string>	users;
	std::vector<std::string>	channels;
	std::vector<std::string>	keys;

	std::vector<std::string>	mode_operations;
	std::vector<std::string>	mode_parameters;

	bool						message_set;
	std::string					message;

	std::vector<bool> 			threw_error;
	std::vector<int>			err_response;
};

class Parser
{
	public:
		Parser();

		// <message>  ::= [':' <prefix> <SPACE> ] <command> <params> <crlf>
		bool message(std::string message, Command &cmd_out);
	
	private: 
		bool error(std::string::const_iterator &it);
		
		static bool validateCommand(ParsedCommand &cmd_in, Command &cmd_out);
		static bool validateJoin(ParsedCommand &cmd_in, Command &cmd_out);
		static bool validateUser(ParsedCommand &cmd_in, Command &cmd_out);
		static bool validateNick(ParsedCommand &cmd_in, Command &cmd_out);
		static bool validatePrivmsg(ParsedCommand &cmd_in, Command &cmd_out);
		static bool validateNotice(ParsedCommand &cmd_in, Command &cmd_out);
		static bool validateKick(ParsedCommand &cmd_in, Command &cmd_out);
		static bool validateInvite(ParsedCommand &cmd_in, Command &cmd_out);
		static bool validateTopic(ParsedCommand &cmd_in, Command &cmd_out);
		static bool validateMode(ParsedCommand &cmd_in, Command &cmd_out);
		static bool validatePart(ParsedCommand &cmd_in, Command &cmd_out);
		static bool validateQuit(ParsedCommand &cmd_in, Command &cmd_out);
		static bool validatePass(ParsedCommand &cmd_in, Command &cmd_out);

		// <prefix>   ::= <servername> | <nick> [ '!' <user> ] [ '@' <host> ]
		// bool prefix(std::string::const_iterator &it);
		// <command>  ::= <letter> { <letter> } | <number> <number> <number>
		bool command(std::string::const_iterator &it);
		// <SPACE>    ::= ' ' { ' ' }
		bool space(std::string::const_iterator& it);
		// <params>   ::= <SPACE> [ ':' <trailing> | <middle> <params> ]
		bool params(std::string::const_iterator& it, ParsedCommand &cmd);

		// <middle>   ::= <Any *non-empty* sequence of octets not including SPACE or NUL or CR or LF, the first of which may not be ':'>
		bool middle(std::string::const_iterator& it);
		// <trailing> ::= <Any, possibly *empty*, sequence of octets not including NUL or CR or LF>
		bool trailing(std::string::const_iterator& it);

		// <crlf>     ::= CR LF
		bool crlf(std::string::const_iterator& it);

		// TARGETS
		// <target>     ::= <to> [ "," <target> ]
		static bool target(std::string::const_iterator &it, std::string::const_iterator &end);
		// <to>         ::= <channel> | <user> '@' <servername> | <nick> | <mask>
		static bool to(std::string::const_iterator &it, std::string::const_iterator &end);
		// <channel>    ::= ('#' | '&') <chstring>
		static bool channel(std::string::const_iterator &it, std::string::const_iterator &end);
		// <servername> ::= <host>
		static bool servername(std::string::const_iterator &it, std::string::const_iterator &end);
		// <host>       ::= see RFC 952 [DNS:4] for details on allowed hostnames
		//				::= <name>*["."<name>]
		static bool host(std::string::const_iterator &it, std::string::const_iterator &end);
		// <name>       ::= <letter> [*[let-or-digit-or-hyphen]<let-or-digit>]
		static bool name(std::string::const_iterator &it, std::string::const_iterator &end);
		// <nick>       ::= <letter> { <letter> | <number> | <special> }
		static bool nick(std::string::const_iterator &it, std::string::const_iterator &end);
		// <mask>       ::= ('#' | '$') <chstring>
		static bool mask(std::string::const_iterator &it, std::string::const_iterator &end);
		// <chstring>   ::= <any 8bit code except SPACE, BELL, NUL, CR, LF and comma (',')>
		static bool chstring(std::string::const_iterator &it, std::string::const_iterator &end);

		static bool modestring(std::string &modestring, Command &cmd_out);

		// OTHER
		// <user>       ::= <nonwhite> { <nonwhite> }
		static bool user(std::string::const_iterator &it, std::string::const_iterator &end);
		// <username>	::= 1*(<not in "\0\r\n @">)
		static bool username(std::string::const_iterator &it, std::string::const_iterator &end);
		// <letter>     ::= 'a' ... 'z' | 'A' ... 'Z'
		static bool letter(std::string::const_iterator& it);
		// <number>     ::= '0' ... '9'
		static bool number(std::string::const_iterator& it);
		// <special>    ::= '-' | '[' | ']' | '\' | '`' | '^' | '{' | '}'
		static bool special(std::string::const_iterator& it);
		// <nonwhite>   ::= <any 8bit code except SPACE (0x20), NUL (0x0), CR (0xd), and LF (0xa)>
		static bool nonwhite(std::string::const_iterator& it);

		// HELPERS
		static bool is_char(std::string::const_iterator& it, char c);
		static bool is_in(std::string::const_iterator& it, std::string str);
		
		std::set<std::string>	_supported_commands;
		std::string				_message;
		std::stack<std::string>	_func_stack;

		bool out(bool value);
};

#endif
