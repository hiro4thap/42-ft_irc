#include <string>

void error()
{

}

/*
Based on https://datatracker.ietf.org/doc/html/rfc1459
*/

// <message>  ::= [':' <prefix> <SPACE> ] <command> <params> <crlf>
bool message(std::string::iterator it)
{
	if (*it == ':')
	{
		if (!prefix(it + 1))
		{
			error();
			return false;
		}
		if (!space(it + 1))
		{
			error();
			return false;
		}
	}
	if (!command(it + 1))
	{
		error();
		return false;
	}
	if (!params(it + 1))
	{
		error();
		return false;
	}
	if (!crlf(it + 1))
	{
		error();
		return false;
	}
	return true;
}

// <prefix>   ::= <servername> | <nick> [ '!' <user> ] [ '@' <host> ]
bool prefix(std::string::iterator it)
{
	if(!servername())
	{
		if (nick())
		{
			if ('!')
			{
				if (!user())
				{
					error();
					return false;
				}
			}
			if ('@')
			{
				if (!host())
				{
					error();
					return false;
				}
			}
			return true;
		}
		return false;
	}
	return true;
}


// <command>  ::= <letter> { <letter> } | <number> <number> <number>
bool command(std::string::iterator it)
{
	if (!letter())
	{
		if (number() && number() && number())
		{
			return true;
		}
		return false;
	}
	while (letter())
	{
		continue ;
	}
	return true;
}

// <SPACE>    ::= ' ' { ' ' }
bool space(std::string::iterator& it)
{
	if (*it != ' ')
		return false;
	it++;
	while (*it == ' ')
	{
		it++;
		continue;
	}
	return true;
}

// <params>   ::= <SPACE> [ ':' <trailing> | <middle> <params> ]
bool params(std::string::iterator& it)
{
	if (!space(it))
	{
		return false;
	}
	while (true)
	{
		if ((*it) == ':')
		{
			it++;
			if (!trailing(it))
			{
				error();
				return false;
			}
		}
		else if (middle(it))
		{
			if (!params(it))
			{
				error();
				return false;
			}
		}
		else
			break;
	}
	return true;
}

// <middle>   ::= <Any *non-empty* sequence of octets not including SPACE
//                or NUL or CR or LF, the first of which may not be ':'>
bool middle(std::string::iterator& it)
{
	int count = 0;
	if (*it == ':')
		return false;
	while (*it != ' ' && *it != '\0' && *it != '\r' && *it != '\f')
	{
		it++;
		count++;
	}
	if (count > 0)
		return true;
	return false;
}

// <trailing> ::= <Any, possibly *empty*, sequence of octets not including NUL or CR or LF>
bool trailing(std::string::iterator& it)
{
	while (*it != '\0' && *it != '\r' && *it != '\f')
	{
		it++;
	}
	return true;
}

// <crlf>     ::= CR LF
bool crlf(std::string::iterator& it)
{
	if ((*it) != '\r')
	{
		return false;
	}
	it++;
	if ((*it) != '\f')
	{
		error();
		return false;
	}
	it++;
	return true;
}

// <user>       ::= <nonwhite> { <nonwhite> }
bool user(std::string::iterator it)
{
	if (!nonwhite())
	{
		return false;
	}
	while (nonwhite())
	{
		continue;
	}
	return true;
}

// <letter>     ::= 'a' ... 'z' | 'A' ... 'Z'
bool letter(std::string::iterator& it)
{
	if ((*it >= 'a' && *it <='z') || (*it >= 'A' && *it <= 'Z'))
		return true;
	return false;
}

// <number>     ::= '0' ... '9'
bool number(std::string::iterator& it)
{
	if (*it >='0' && *it <='9')
		return true;
	return false;
}

bool is_char(std::string::iterator& it, char c)
{
	if (*it == c)
		return true;
	return false;
}

// <special>    ::= '-' | '[' | ']' | '\' | '`' | '^' | '{' | '}'
bool is_in(std::string::iterator& it, std::string str)
{
	for (int i = 0; i < str.size(); i++)
	{
		if (is_char(it, str[i]))
			return true;
	}
	return false;
}

bool special(std::string::iterator& it)
{
	if (is_in("-[]\\`^{}"))
	{
		it++;
		return true;
	}
	return true;
}

bool letter_digit(std::string::iterator& it)
{
	if (letter(it) || number(it))
		return true;
	return false;
}

bool letter_digit_dash(std::string::iterator& it)
{
	if (letter(it) || number(it) || is_char(it,'-'))
		return true;
	return false;
}

// <nonwhite>   ::= <any 8bit code except SPACE (0x20), NUL (0x0), CR (0xd), and LF (0xa)>
bool nonwhite(std::string::iterator& it)
{
	return is_in(it, " \0\r\f")
}

// <servername> ::= <host>
bool servername(std::string::iterator it)
{
	return host();
}

// <nick>       ::= <letter> { <letter> | <number> | <special> }
bool nick(std::string::iterator it)
{
	if (!letter())
	{
		return false;
	}
	while (letter() || number() || special())
	{
		continue;
	}
	return true;
}

// <host>       ::= see RFC 952 [DNS:4] for details on allowed hostnames
// <hname> ::= <name>*["."<name>]
// <name>  ::= <letter>[*[<let-or-digit-or-hyphen>]<let-or-digit>]
bool host(std::string::iterator it)
{
	if (!letter())
	{
		return false;
	}
	if (letter_digit_dash() || letter_digit())
	{
		while (letter_digit_dash())
		{
			continue;
		}
		if (!letter_digit())
		{
			error();
			return false;
		}
		return true;
	}
	return true;
	
}


// Target stuff
// <target>     ::= <to> [ "," <target> ]
bool target(std::string::iterator it)
{
	if (!to())
	{
		return false;
	}
	if (',')
	{
		if (!target())
		{
			error();
			return false;
		}
	}
	return true;
}

// <to>         ::= <channel> | <user> '@' <servername> | <nick> | <mask>
bool to(std::string::iterator it)
{
	if (!channel())
	{
		if (user())
		{
			if (!'@')
			{
				error();
				return false;
			}
			if (!servername())
			{
				error();
				return false;
			}
			return true;
		}
		else if (nick())
		{
			return true;
		}
		else if (mask())
		{
			return true;
		}
		else
			return false;
	}
	return true;
}

// <channel>    ::= ('#' | '&') <chstring>
bool channel(std::string::iterator it)
{
	if ((*it) != '#' && (*it) != '&')
		return false;
	if (!chstring())
		return false;
	return true;
}

// <mask>       ::= ('#' | '$') <chstring>
bool mask(std::string::iterator it)
{
	if (!'#' && !'$')
		return false;
	if (!chstring())
		return false;
	return true;
}

// <chstring>   ::= <any 8bit code except SPACE, BELL, NUL, CR, LF and comma (',')>
bool chstring(std::string::iterator it)
{
	if (in " \b\0\r\f,")
		return false;
	return true;
}








/* 
// Based on https://modern.ircdocs.horse/

bool message(std::string::iterator it)
{
	if ('@')
	{
		if (!tags())
		{
			error();
			return false;
		}
		if (!space())
		{
			error();
			return false;
		}
	}
	if (':')
	{
		if (!source())
		{
			error()
			return false;
		}
		if (!space())
		{
			error();
			return false;
		}
	}
	if (!command())
		return false;
	if (!parameters())
		return false;
	if (!crlf())
		return false;
	return true;
}

bool space(std::string::iterator it)
{
	int count = 0;
	while (' ')
	{
		count ++;
		continue ;
	}
	if (count > 0)
		return true;
	return false;
}

bool crlf(std::string::iterator it)
{
	if ("\r\f")
		return true;
	return false;
}

bool tags(std::string::iterator it)
{
	if (!tag())
		return false;
	while (';')
	{
		if (!tag())
		{
			error();
			return false;
		}
	}
	return true;
}

bool tag(std::string::iterator it)
{
	if (!key())
		return false;
	if ('=')
	{
		if (!escaped_value())
		{
			error();
			return false;
		}
	}
	return true;
}

bool key(std::string::iterator it)
{
	if (client_prefix())
	{

	}
	if (vendor())
	{
		if (!'/')
		{
			error();
			return false;
		}
	}
	if (!seq_alpha_num_dash())
	{
		return false;
	}
	return true;
}

bool client_prefix(std::string::iterator it)
{
	if (!'+')
		return false;
	return true;
}

bool escaped_value(std::string::iterator it)
{
	if (!seq_not_null_cr_lf_semi_sp())
	{
		return false;
	}
	return true;
}

bool vendor(std::string::iterator it)
{
	if (!host())
		return false;
	return true;
}

bool source(std::string::iterator it)
{
	if (!servername())
	{
		if (nickname())
		{
			if ('!')
			{
				if (!user())
				{
					error();
					return false;
				}
			}
			if ('@')
			{
				if (!host())
				{
					error();
					return false;
				}
			}
			return true;
		}
		else 
			return false;
	}
}

// Needs to be sequence?
bool seq_not_null_cr_lf_semi_sp(std::string::iterator it)
{
	if ('\0\r\f; ')
		return false;
	return true;
}

// Unclearly defined?
bool nickname(std::string::iterator it)
{
	// Chan type?
	if (!seq_not_null_cr_lf_semi_sp())
}


bool user(std::string::iterator it)
{
	if (!seq_not_null_cr_lf_sp())
	{
		return false;
	}
	return true;
}

// letter* / 3digit
// Valid command / command code
bool command(std::string::iterator it)
{
	return true;
}

bool parameters(std::string::iterator it)
{
	while(space())
	{
		if (':')
		{
			if (!trailing())
			{
				error();
				return false;
			}
			return true;
		}
		if (!middle())
		{
			error();
			return false;
		}
	}
	return true;	
}

bool nospcrlfcl(std::string::iterator it)
{
	//<sequence of any characters except NUL, CR, LF, colon (`:`) and SPACE>
	return true;
}

bool middle(std::string::iterator it)
{
	if (!nospcrlfcl())
	{
		return false;
	}
	while (':' || ' ' || nospcrlfcl())
		continue;
	return true;
}

bool trailing(std::string::iterator it)
{
	while (':' || ' ' || nospcrlfcl())
		continue;
	return true;
} */


