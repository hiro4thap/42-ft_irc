#ifndef CHANNEL_HPP
# define CHANNEL_HPP

# include <iostream>
# include <sstream>
# include <vector>
# include <ctime>
# include <algorithm>

# include "Log.hpp"

typedef std::vector<std::string>::const_iterator user_it; 

class	Channel
{
public:
	Channel(const std::string name);
	~Channel();
	const std::string	&getName() const;
	void				setTopic(const std::string &topic, const std::string &setter);
	const std::string	&getTopic() const;
	const std::string	&getTopicSetBy() const;
	const std::string	&getTopicSetAt() const;
	void				setPassword(const std::string &password);
	const std::string	&getPassword() const;
	bool				checkPassword(const std::string &password) const;
	void				setIsInviteOnly(bool value);
	bool				getIsInviteOnly() const;
	void				setHasRestrictTopic(bool value);
	bool				getHasRestrictTopic() const;
	void				setHasPassword(bool value);
	bool				getHasPassword() const;
	void				setHasLimit(bool value);
	bool				getHasLimit() const;
	void				setLimit(std::size_t value);
	std::size_t			getLimit() const;
	const std::string	getModes() const;
	const std::vector<std::string>	&getUsers() const;
	void				addUser(const std::string &user);
	void				removeUser(const std::string &user);
	void				replaceUser(const std::string &old_name, const std::string &new_name);
	const std::vector<std::string>	&getOperators() const;
	void				addOperator(const std::string &user);
	void				removeOperator(const std::string &user);
	void				replaceOperator(const std::string &old_name, const std::string &new_name);
	const std::vector<std::string>	&getInvitedUsers() const;
	void				addInvitedUser(const std::string &user);
	void				removeInvitedUser(const std::string &user);
	void				replaceInvitedUser(const std::string &old_name, const std::string &new_name);

	static bool			containsUser(const std::vector<std::string> &user_list, const std::string &user);
	static bool			validChannelName(const std::string &name);
	bool				operator==(const Channel &rhs) const;
	bool				operator!=(const Channel &rhs) const;

private:
	std::string	_name;
	std::string	_topic;
	std::string	_topic_set_by;
	std::string	_topic_set_at;
	std::string	_password;
	std::size_t	_limit;
	bool		_is_invite_only;		//i
	bool		_has_restrict_topic;	//t
	bool		_has_password;			//k
	bool		_has_limit;				//l
	std::vector<std::string>	_users;
	std::vector<std::string>	_operators;
	std::vector<std::string>	_invited_users;
};

#endif
