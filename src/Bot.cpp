#include "../inc/Bot.hpp"

Bot::Bot():_fd(2), _channel("#Dice")
{}

Bot::Bot(int fd, const std::string& channel): _fd(fd), _channel(channel)
{
}

Bot::~Bot()
{}

void	Bot::setupBot(User* bot)
{
	bot->setAuthenticated();
	bot->setRegistered();
	bot->setRegistrationState(PROVIDED_NICK);
	bot->setRegistrationState(PROVIDED_USER);
	bot->setNickname("DiceBot");
}


static std::vector<std::string> tokenise(std::string input)
{
	std::vector<std::string> tokens;

	std::stringstream ss(input);

	std::string token;
	while (getline(ss, token, ' '))
	{
		if (!token.empty())
		{
			tokens.push_back(token);
		}
	}
	return tokens;
}

struct dice_roll
{
	int number_of_dice;
	int number_of_sides;
};

struct dice_roll parseDice(const std::string dice_roll)
{
	std::string::const_iterator it = dice_roll.begin();

	struct dice_roll d;
	d.number_of_dice = 0;
	d.number_of_sides = 0;

	while (it != dice_roll.end() && (*it >= '0' && *it <= '9'))
	{
		d.number_of_dice += 10 * (*it);
		it++;
	}
	if (it == dice_roll.end() || *it != 'd')
	{
		d.number_of_dice = -1;
		d.number_of_sides = -1;
		return d;
	}
	it++;
	if (it == dice_roll.end() || *it < '0' || *it > '9')
	{
		d.number_of_dice = -1;
		d.number_of_sides = -1;
		return d;
	}
	while (it != dice_roll.end() && (*it >= '0' && *it <= '9'))
	{
		d.number_of_dice += 10 * (*it);
		it++;
	}
	if (*it == '\0' || it == dice_roll.end())
		return d;
	else
	{
		d.number_of_dice = -1;
		d.number_of_sides = -1;
		return d;
	}
}

const std::string diceRoll(int num_dice, int num_sides)
{
	if (num_sides == 0)
		return std::string("Cannot roll zero sided dice");
	if (num_dice == 0)
		return std::string("No dice rolled");
	
	std::string output = Log::str(num_dice) + "d" + Log::str(num_sides) + "=";
	std::string individual_rolls = "";
	int sum = 0;
	for (int i = 0; i < num_dice; i++)
	{
		int result = rand() % num_sides + 1;
		sum += result;
		if (individual_rolls.empty() == false)
			individual_rolls += ",";
		individual_rolls += Log::str(result);
	}
	output += Log::str(sum) + "(" + individual_rolls + ")";
	return output;
}


Command	Bot::proccessMessage(const std::string& input) const
{
	Command cmd;
	cmd.threw_error.push_back(true);

	std::size_t second_pos = input.find(":", 1);
	if (second_pos == std::string::npos)
		return cmd;
	
	std::string header = input.substr(1, second_pos - 1);
	std::string message = input.substr(second_pos + 1, std::string::npos);

	// Validate header

	std::vector<std::string> header_tokens = tokenise(header);
	if (header_tokens.size() < 2)
		return cmd;

	std::string from = header_tokens[0];
	std::string	command = header_tokens[1];
	if (command != "PRIVMSG")
		return cmd;
	if (header_tokens.size() == 3 && header_tokens[2] != _channel)
		return cmd;

	// Validate message
	std::vector<std::string> message_tokens = tokenise(message);
	if (message_tokens.size() < 2 || message_tokens[0] != "r")
		return cmd;
	std::string dice_request = message_tokens[1];
	std::string description = dice_request.substr(message_tokens[0].size() + 1 + message_tokens[1].size() + 1, std::string::npos);

	struct dice_roll dice = parseDice(dice_request);
	if (dice.number_of_dice == -1 && dice.number_of_sides == -1)
		return cmd;
	std::string dice_result = diceRoll(dice.number_of_dice, dice.number_of_sides);

	cmd.threw_error.at(0) = false;
	cmd.message_set = true;
	cmd.message = dice_result;
	cmd.command = "PRIVMSG";
	cmd.users.push_back(from);
	if (header_tokens.size() == 3)
		cmd.channels.push_back(_channel);
	return cmd;
}

int	Bot::getFd() const
{
	return _fd;
}

const std::string&	Bot::getChannel() const
{
	return _channel;
}
