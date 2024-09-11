#ifndef PARSER_HPP
# define PARSER_HPP

# include <string>
# include <iostream>
# include <set>
# include <vector>

struct Command
{
	std::string command;
	std::vector<std::string> parameters;
};

class Parser
{
	public:
		Parser();
		bool message(std::string::iterator it);
	
	private: 
		void error(std::string::iterator &it, std::string &func);

		bool prefix(std::string::iterator &it);
		bool command(std::string::iterator &it);
		bool space(std::string::iterator& it);
		bool params(std::string::iterator& it, Command &cmd);
		bool middle(std::string::iterator& it);
		bool trailing(std::string::iterator& it);
		bool crlf(std::string::iterator& it);
		bool user(std::string::iterator &it);
		bool letter(std::string::iterator& it);
		bool number(std::string::iterator& it);
		bool is_char(std::string::iterator& it, char c);
		bool is_in(std::string::iterator& it, std::string str);
		bool special(std::string::iterator& it);
		bool letter_digit(std::string::iterator& it);
		bool letter_digit_dash(std::string::iterator& it);
		bool nonwhite(std::string::iterator& it);
		bool servername(std::string::iterator &it);
		bool nick(std::string::iterator &it);
		bool host(std::string::iterator &it);
		bool target(std::string::iterator &it);
		bool to(std::string::iterator &it);
		bool channel(std::string::iterator &it);
		bool mask(std::string::iterator &it);
		bool chstring(std::string::iterator &it);

		std::set<std::string> _supported_commands;
};

#endif
