#include "recap8.h"
#include "parameter.h"


using namespace pun;

const char* Recap8::PHP_NAME = "Pun\\Recap8";

Recap8*
Recap8::get_Recap8(Php::Value& val)
{
    if (val.isObject()) {
        if (val.instanceOf(Recap8::PHP_NAME)) {
            return (Recap8*) val.implementation();
        }
    }
    return nullptr;
}

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

