#ifndef BOT_HPP
# define BOT_HPP

# include <iostream>
# include <sstream>
# include <string>
# include "User.hpp"
# include "Parser.hpp"
# include "Log.hpp"


class Bot
{
	public:
		Bot();
		Bot(int fd, const std::string& channel);
		~Bot();
		
		void setupBot(User* bot);
		Command proccessMessage(const std::string& message) const;

		int getFd() const;
		const std::string& getChannel() const;
	private:
		const int _fd;
		const std::string _channel;
};

#endif
