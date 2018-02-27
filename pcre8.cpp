#include "pcre8.h"
#include "pun8.h"
#include <ostream>

const std::string Pcre8::PHP_NAME = "Pun\\IdRex8";

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
	Pun8::check_IntString(params);

    int index = params[0];
    const char* str = params[1];
    int stringSize = params[1].size();

    Pcre8_share sp = std::make_shared<Pcre8_imp>();
    auto pimp = sp.get();
    pimp->_eStr.assign(str,stringSize);
    pimp->_id = index;

    std::string errorMsg;

    if (!pimp->compile(errorMsg)) {
        throw Php::Exception(errorMsg);
    }
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
    Pun8::check_String(params,0);
    int offset =  (Pun8::option_Int(params,1)) ? (int) params[1] : 0;

    Php::Value result;
    Pcre8_match matches;
    auto pimp = _imp.get();
    char const* buf = params[0];
    auto size = params[0].size();

    if (offset < size) {
        buf += offset;
        size -= offset;
        int rct = pimp->doMatch(reinterpret_cast<const unsigned char*>(buf), size, matches);

        if (rct > 0) {
            // return array of strings
            for(int i = 0; i < rct; i++)
            {
                result[i] = matches._slist[i];
            }
            return result;
        }
    }
    result = false;
    return result;
 }