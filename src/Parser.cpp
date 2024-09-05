
void error()
{

}


bool message()
{
	if (':')
	{
		if (!prefix())
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
	if (!command())
	{
		error();
		return false;
	}
	if (!params())
	{
		error();
		return false;
	}
	if (!crlf())
	{
		error();
		return false;
	}
	return true;
}

bool prefix()
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

bool command()
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

bool space()
{
	if (!' ')
		return false;
	while (' ')
		continue;
	return true;
}


bool params()
{
	if (!space())
	{
		return false;
	}
	while (true)
	{
		if (':')
		{
			if (!trailing())
			{
				error();
				return false;
			}
		}
		else if (middle())
		{
			if (!params())
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

bool middle()
{
	// <Any *non-empty* sequence of octets not including SPACE or NUL or CR or LF, the first of which may not be ':'>
	return true;
}

bool trailing()
{
	// <Any, possibly *empty*, sequence of octets not including NUL or CR or LF>
	return true;
}

bool crlf()
{
	if (!'\r')
	{
		return false;
	}
	if (!'\f')
	{
		error();
		return false;
	}
	return true;
}


bool user()
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

bool letter()
{
	if (>='a' && <='z' || >='A' && <='Z')
		return true;
	return false;
}

bool number()
{
	if (>='0' && <='9')
		return true;
	return false;
}

bool special()
{
	if (in "-[]\`^{}")
		return true;
	return false;
}

bool letter_digit()
{
	if (letter() || number())
		return true;
	return false;
}

bool letter_digit_dash()
{
	if (letter() || number() || '-')
		return true;
	return false;
}


bool nonwhite()
{
	if (' ' || '\0' || '\r' || '\f')
		return false;
	return true;
}

bool servername()
{
	return host();
}

bool nick()
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

bool host()
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
bool target()
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


bool to()
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


bool channel()
{
	if (!'#' && !'&')
		return false;
	if (!chstring())
		return false;
	return true;
}

bool mask()
{
	if (!'#' && !'$')
		return false;
	if (!chstring())
		return false;
	return true;
}

bool chstring()
{
	if (in " \b\0\r\f,")
		return false;
	return true;
}








/* 
// Based on https://modern.ircdocs.horse/

bool message()
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

bool space()
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

bool crlf()
{
	if ("\r\f")
		return true;
	return false;
}

bool tags()
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

bool tag()
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

bool key()
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

bool client_prefix()
{
	if (!'+')
		return false;
	return true;
}

bool escaped_value()
{
	if (!seq_not_null_cr_lf_semi_sp())
	{
		return false;
	}
	return true;
}

bool vendor()
{
	if (!host())
		return false;
	return true;
}

bool source()
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
bool seq_not_null_cr_lf_semi_sp()
{
	if ('\0\r\f; ')
		return false;
	return true;
}

// Unclearly defined?
bool nickname()
{
	// Chan type?
	if (!seq_not_null_cr_lf_semi_sp())
}


bool user()
{
	if (!seq_not_null_cr_lf_sp())
	{
		return false;
	}
	return true;
}

// letter* / 3digit
// Valid command / command code
bool command()
{
	return true;
}

bool parameters()
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

bool nospcrlfcl()
{
	//<sequence of any characters except NUL, CR, LF, colon (`:`) and SPACE>
	return true;
}

bool middle()
{
	if (!nospcrlfcl())
	{
		return false;
	}
	while (':' || ' ' || nospcrlfcl())
		continue;
	return true;
}

bool trailing()
{
	while (':' || ' ' || nospcrlfcl())
		continue;
	return true;
} */


