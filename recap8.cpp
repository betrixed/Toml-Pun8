#include "recap8.h"
#include "pun8.h"

const char* Recap8::PHP_NAME = "Pun\\Recap8";

Php::Value 
Recap8::count() const
{
	int result =  (_match._rcode >= 0) ? _match._rcode : 0;
	return Php::Value(result);
}

Php::Value 
Recap8::getCap(Php::Parameters& params) const
{
	int offset = Pun8::check_Int(params,0);
	if (offset >= 0 && offset < _match._rcode)
	{
		return Php::Value(_match._slist[offset]);
	}
	else {
		throw new Php::Exception("Recap8 Offset out of range");
	}
}

