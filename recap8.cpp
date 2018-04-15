#include "recap8.h"
#include "parameter.h"


using namespace pun;

const char* Recap8::PHP_NAME = "Pun\\Recap8";

void
Recap8::setup_ext(Php::Extension& ext)
{
    Php::Class<Recap8> cap8(Recap8::PHP_NAME);
    cap8.method<&Recap8::getCap> ("getCap");
    ext.add(std::move(cap8));
}

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



long
Recap8::count()
{
	return (long) _match._slist.size();
}

bool
Recap8::offsetExists(const Php::Value &key) {
    if (key.isNumeric()) {
        long val = key.numericValue();
        return (val >= 0 && val < (long) _match._slist.size());
    }
    return false;
}

Php::Value
Recap8::offsetGet(const Php::Value& key)
{
    if (key.isNumeric()) {
        long val = key.numericValue();
        if (val >= 0 && val < (long) _match._slist.size())
        {
            return _match._slist[val];
        }
    }
    throw Php::Exception("offsetGet out of range");
}

Php::Value
Recap8::getCap(Php::Parameters& params) const
{
	long offset = pun::check_Int(params,0);

	if (offset >= 0 && offset < (long) _match._slist.size())
	{
		return Php::Value(_match._slist[offset]);
	}
    throw new Php::Exception("Recap8 Offset out of range");

}

