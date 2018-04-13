#include "pcre8.h"
#include "recap8.h"
#include "parameter.h"
#include <ostream>

using namespace pun;


const char* Pcre8::PHP_NAME = "Pun\\IdRex8";


Pcre8::Pcre8()
{

}

Pcre8::~Pcre8()
{
}

void Pcre8::__construct(Php::Parameters& params)
{
	(*this).setIdString(params);
}

Pcre8_share
Pcre8::fromParameters(Php::Parameters& params)
{
	int index = pun::check_IntString(params);
    const char* str = params[1];
    auto stringSize = params[1].size();

    Pcre8_share sp = pun::makeSharedRe(index, str, stringSize);
    return sp;
}
/** param 0 - id, param 1 - string pcre2 */

void Pcre8::setIdString(Php::Parameters& params)
{
	_imp = Pcre8::fromParameters(params);
}

void Pcre8::setImp(const Pcre8_share& sptr) {
	_imp = sptr;
}

Php::Value Pcre8::getString() const
{
	auto pimp = _imp.get();
	return Php::Value(pimp->_eStr);
}

Php::Value Pcre8::getId() const
{
	auto pimp = _imp.get();
	return Php::Value(pimp->_id);
}

 Php::Value Pcre8::isCompiled() const
 {
 	return Php::Value(_imp.get()->isCompiled());
 }

 Php::Value Pcre8::match(Php::Parameters& params)
 {
    pun::check_String(params,0);
    int offset =  pun::option_Int(params,1);
    Recap8* cap = pun::option_Recap8(params,2);

    Pcre8_match matches;
    auto pimp = _imp.get();
    char const* buf = params[0];
    auto size = params[0].size();

    if (offset < size) {
        buf += offset;
        size -= offset;
        pimp->doMatch(reinterpret_cast<const unsigned char*>(buf), size, matches);
    }
    if (cap == nullptr) {
        cap = new Recap8();
    }
    cap->_match = std::move(matches);

    Php::Value result = Php::Object(Recap8::PHP_NAME, cap);
    return result;
 }
