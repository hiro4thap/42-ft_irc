#ifndef LOG_HPP
# define LOG_HPP

# include <string>
# include <iostream>
# include <sstream>

# define COLOR_NONE		""
# define COLOR_RED		"\033[0;31m"
# define COLOR_GREEN	"\033[0;32m"
# define COLOR_YELLOW	"\033[0;33m"
# define COLOR_BLUE		"\033[0;34m" 
# define COLOR_MAGENTA	"\033[0;35m"
# define COLOR_CYAN		"\033[0;36m"
# define COLOR_RESET	"\033[0m"


class	Log
{
	public:
		static void	out(std::string message, std::string color = COLOR_NONE, std::ostream& out = std::cout);
		static void	nl(std::string message = "", std::string color = COLOR_NONE, std::ostream& out = std::cout);
		static void	err(std::string message = "", std::string color = COLOR_RED);

		template <typename T>
		static std::string str(T t) 
		{
			std::stringstream	stream;
			stream << t;
			return (stream.str());
		}
		
		template <>
		std::string str<bool>(bool t)
		{
			if (t)
				return "true";
			return "false";
		}

	private:
		Log();
		Log(Log& instance);
		Log& operator=(Log copy);
		~Log();
		void	swap(Log& other);

		static void output(std::string message, std::string color = COLOR_NONE, std::ostream& out = std::cout);
};

#endif
