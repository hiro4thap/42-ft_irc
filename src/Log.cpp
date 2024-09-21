#include "../inc/Log.hpp"

Log::Log()
{
}

Log::Log(Log& instance)
{
	(void) instance;
}

Log& Log::operator= (Log copy)
{
	this->swap(copy);
	return (*this);
}

Log::~Log()
{
}

void	Log::swap(Log& other)
{
	(void) other;
}

void	Log::out(std::string message, std::string color, std::ostream& out)
{
	output(message, color);
	out.flush();
}

void	Log::nl(std::string message, std::string color, std::ostream& out)
{
	output(message, color, out);
	out << std::endl;
}

void	Log::err(std::string message, std::string color)
{
	output(message, color, std::cerr);
	std::cerr << std::endl;
}

void	Log::output(std::string message, std::string color, std::ostream& out)
{
	out << color << message <<COLOR_RESET; 
}
