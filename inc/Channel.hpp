#ifndef CHANNEL_HPP
# define CHANNEL_HPP

# include <iostream>

class	Channel
{
public:
	Channel(const std::string name);
	~Channel();
	void				setTopic(const std::string topic);
	const std::string	&getTopic() const;
	void				setPassword(const std::string password);
	bool				checkPassword(const std::string password) const;
	void				setIsInviteOnly(bool value);
	bool				getIsInviteOnly() const;
	void				setHasRestrictTopic(bool value);
	bool				getHasRestrictTopic() const;
	void				setHasPassword(bool value);
	bool				getHasPassword() const;
	void				setHasLimit(bool value);
	bool				getHasLimit() const;
	//void				addUser(const std::string &user);
	//void				removeUser(const std::string &user);

private:
	std::string	_name;
	std::string	_topic;
	std::string _password;
	bool		_is_invite_only;		//i
	bool		_has_restrict_topic;	//t
	bool		_has_password;			//k
	bool		_has_limit;				//l
	//vector<std::string>	_users;
	//vector<std::string>	_operators;
};

#endif
