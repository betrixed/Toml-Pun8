#include "recap8.h"
#include "pun8.h"
#include "parameter.h"

const char* Recap8::PHP_NAME = "Pun\\Recap8";

Php::Value 
Recap8::count() const
{
	int result =  _match._slist.size();
	return Php::Value(result);
}

Php::Value 
Recap8::getCap(Php::Parameters& params) const
{
	int offset = pun::check_Int(params,0);

	if (offset >= 0 && offset < (int) _match._slist.size())
	{
		return Php::Value(_match._slist[offset]);
	}
	else {
		throw new Php::Exception("Recap8 Offset out of range");
	}
}

