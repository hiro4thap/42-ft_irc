#include "Channel.hpp"

Channel::Channel(const std::string name):
	_name(name),
	_is_invite_only(false),
	_has_restrict_topic(false),
	_has_password(false),
	_has_limit(false)
{
}

Channel::~Channel()
{
}

const std::string	&Channel::getName() const
{
	return _name;
}

void	Channel::setTopic(const std::string topic)
{
	_topic = topic;
}

const std::string	&Channel::getTopic() const
{
	return _topic;
}

void	Channel::setPassword(const std::string password)
{
	_password = password;
}

bool	Channel::checkPassword(const std::string password) const
{
	return (_password == password);
}

void	Channel::setIsInviteOnly(bool value)
{
	_is_invite_only = value;
}

bool	Channel::getIsInviteOnly() const
{
	return _is_invite_only;
}

void	Channel::setHasRestrictTopic(bool value)
{
	_has_restrict_topic = value;
}

bool	Channel::getHasRestrictTopic() const
{
	return _has_restrict_topic;
}

void	Channel::setHasPassword(bool value)
{
	_has_password = value;
}

bool	Channel::getHasPassword() const
{
	return _has_password;
}

void	Channel::setHasLimit(bool value)
{
	_has_limit = value;
}

bool	Channel::getHasLimit() const
{
	return _has_limit;
}

void	Channel::setLimit(std::size_t value)
{
	this->_limit = value;
}

std::size_t	Channel::getLimit() const
{
	return this->_limit;
}

const std::vector<std::string>	&Channel::getUsers() const
{
	return _users;
}

void	Channel::addUser(const std::string &user)
{
	_users.push_back(user);
}

void	Channel::removeUser(const std::string &user)
{
	std::vector<std::string>::iterator it = std::find(_users.begin(), _users.end(), user);
	if (it != _users.end())
		_users.erase(it);
}

const std::vector<std::string>	&Channel::getOperators() const
{
	return _operators;
}

void	Channel::addOperator(const std::string &user)
{
	_operators.push_back(user);
}

void	Channel::removeOperator(const std::string &user)
{
	std::vector<std::string>::iterator it = std::find(_operators.begin(), _operators.end(), user);
	if (it != _operators.end())
		_operators.erase(it);
}

const std::vector<std::string>	&Channel::getInvitedUsers() const
{
	return _invited_users;
}

void	Channel::addInvitedUser(const std::string &user)
{
	_invited_users.push_back(user);
}

void	Channel::removeInvitedUser(const std::string &user)
{
	std::vector<std::string>::iterator it = std::find(_invited_users.begin(), _invited_users.end(), user);
	if (it != _invited_users.end())
		_invited_users.erase(it);
}

bool	Channel::containsUser(const std::vector<std::string> &user_list, std::string &user)
{
	for (std::vector<std::string>::const_iterator it = user_list.begin(); it != user_list.end(); it++)
	{
		if (*it == user)
			return true;
	}
	return false;
}

static bool chstring(std::string::const_iterator it)
{
	if (is_in(it, " \b\0\r\f,"))
		return false;
	return true;
}

static bool is_char(std::string::const_iterator& it, char c)
{
	if (*it == c)
		return true;
	return false;
}

static bool is_in(std::string::const_iterator& it, std::string str)
{
	for (int i = 0; i < str.size(); i++)
	{
		if (is_char(it, str[i]))
			return true;
	}
	return false;
}

bool	Channel::validChannelName(const std::string &name)
{
	std::string::const_iterator it = name.begin();
	if ((*it) != '#' && (*it) != '&')
		return false;
	it++;
	if (!chstring(it))
		return false;
	while (chstring(it))
		it++;
	return true;
}
