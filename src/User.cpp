#include "../inc/User.hpp"

User::User(int fd): 
	_socket_fd(fd), _authenticated(false), _registered(false), _registration_state(CONNECTED),
	_nickname(""), _username(""), _real_name("")
{
}

User::~User()
{

}

bool    User::isAuthenticated() const
{
	return _authenticated;
}

void    User::setAuthenticated()
{
	_authenticated = true;
}

bool    User::isRegistered() const
{
	return _registered;
}

void    User::setRegistered()
{
	_registered = true;
}

Registration_State	User::getRegistrationState() const
{
	return _registration_state;
}

void    User::setRegistrationState(Registration_State state)
{
	if (_registration_state == CONNECTED)
		_registration_state = state;
	if ((_registration_state == PROVIDED_NICK && state == PROVIDED_USER)
		|| (_registration_state == PROVIDED_USER && state == PROVIDED_NICK))
		_registration_state = REGISTERED;
}

int User::getFd() const
{
	return _socket_fd;
}

const std::string&	User::getNickname() const
{
	return _nickname;
}

const std::string&	User::getUsername() const
{
	return _username;
}

const std::string&	User::getRealName() const
{
	return _real_name;
}

void    User::setNickname(const std::string& name)
{
	_nickname = name;
}

void	User::setUsername(const std::string& name)
{
	_username = name;
}

void	User::setRealName(const std::string& name)
{
	_real_name = name;
}
