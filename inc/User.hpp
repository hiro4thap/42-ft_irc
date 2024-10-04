#ifndef USER_HPP
# define USER_HPP

# include <string>

enum Registration_State
{
	CONNECTED,
	PROVIDED_NICK,
	PROVIDED_USER,
	REGISTERED
};

class User
{
	public:
		User(int fd);
		~User();

		bool				isAuthenticated() const;
		void				setAuthenticated();
		bool				isRegistered() const;
		void 				setRegistered();

		Registration_State	getRegistrationState() const;
		void				setRegistrationState(Registration_State state);
		bool				getHasConnected() const;
		void				setHasDisconnected();
		int					getFd() const;

		const std::string&	getNickname() const;
		const std::string&	getUsername() const;
		const std::string&	getRealName() const;
		void				setNickname(const std::string& name);
		void				setUsername(const std::string& name);
		void				setRealName(const std::string& name);

	private:
		// Connection
		const int			_socket_fd;
		bool				_authenticated;
		bool				_registered;
		Registration_State	_registration_state;
		bool				_has_disconnected;
		
		// User details
		std::string			_nickname;
		std::string			_username;
		std::string			_real_name;

};

#endif
