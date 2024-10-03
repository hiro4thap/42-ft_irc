#include "inc/Bot.hpp"


void test(std::string input)
{
	User bot_user = User(2);
	Bot bot = Bot(2, "#Dice");
	bot.setupBot(&bot_user);

	Command cmd = bot.proccessMessage(input);
	if (!cmd.threw_error[0])
		Log::nl(":" + bot_user.getNickname() + " " + cmd.command + " " + cmd.users[0] + " :" + cmd.message);
}


int main()
{
	srand(time(0));
	test(":Jeremy PRIVMSG :r 2d6");
	test(":Jeremy PRIVMSG :r 7d1");
	test(":Jeremy PRIVMSG :r 13d5");
	test(":Jeremy PRIVMSG :r d100");
	test(":Jeremy PRIVMSG :r 0d2");
	test(":Jeremy PRIVMSG :r 6d6 Fireball");
	test(":Jeremy PRIVMSG :d60");
	test(":Jeremy PRIVMSG :r 20d6 Falling damage");
	test(":Jeremy PRIVMSG :r 21d4");
	test(":Jeremy PRIVMSG :r d101 Dalmations");
	return 0;
}