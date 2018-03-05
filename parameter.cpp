#include "parameter.h"

#include "pcre8.h"
#include "recap8.h"
#include "re8map.h"
#include "token8.h"

#include <sstream>
#include <ostream>
#include <cstdint>

#include "ucode8.h"
void pun::hexUniStr8(const std::string& hexval, std::ostream& os)
{
    char32_t val = (char32_t) strtol(hexval.data(), 0, 16);
    EncodeUTF8 ec8;

    ec8.encode(val);
    os << ec8.result;
}


std::string 
pun::missingParameter(const char* shouldBe, unsigned int offset)
{
    std::stringstream ss;

    ss << "Parameter " << offset << " should be " << shouldBe;
    return ss.str();
}

std::string 
pun::invalidCharacter(char32_t unc8)
{
	std::stringstream ss;

    ss << "Control code in stream: chr(" << (std::uint_least32_t ) unc8 << ")";
    return ss.str();	
}

Recap8* 
pun::option_Recap8(Php::Parameters& params, unsigned int offset)
{
    auto ct = params.size();
    if (offset >= ct) {
        return nullptr;
    }
    const Php::Value& object = params[offset];   
    if (!object.instanceOf(Recap8::PHP_NAME)) {
        throw Php::Exception("Parameter 1 should be Recap8 object");
    } 

    Recap8 *obj = (Recap8 *)object.implementation();
    return obj;    
}

Re8map* 
pun::check_Re8map(Php::Parameters& params, unsigned int offset)
{
    auto ct = params.size();

    if (offset < ct) {
        const Php::Value& object = params[offset];   
        if (object.instanceOf(Re8map::PHP_NAME)) {
            Re8map *obj = (Re8map *)object.implementation();
            return obj;  
        }
    }
    throw Php::Exception(pun::missingParameter("Re8map object",offset));
  
}


Pcre8* 
pun::check_Pcre8(Php::Parameters& params, unsigned int offset)
{
    auto ct = params.size();

    if (offset < ct) {
        const Php::Value& object = params[offset];   
        if (object.instanceOf(Pcre8::PHP_NAME)) {
            Pcre8 *obj = (Pcre8 *)object.implementation();
            return obj;
        }
    }
    throw Php::Exception(pun::missingParameter("Pcre8 object", offset));
}

 
bool pun::check_String(Php::Parameters& params,unsigned int offset)
{
    auto ct = params.size();
    if (offset >= ct) {
        throw Php::Exception(pun::missingParameter("String", offset));
    }    
    return params[offset].isString();
}

bool pun::option_Array(Php::Parameters& params, unsigned int offset)
{
    auto ct = params.size();
    if (offset >= ct) {
        return false;
    }
    return params[offset].isArray();
}

bool pun::option_Int(Php::Parameters& params,unsigned int offset)
{
    auto ct = params.size();
    return (offset < ct);
}

int 
pun::check_Int(Php::Parameters& params,unsigned int offset)
{
    auto ct = params.size();
    if (ct < 1) {
        throw Php::Exception(pun::missingParameter("Integer", offset));
    }
    return params[0];
}

int 
pun::check_IntString(Php::Parameters& params)
{
    
    auto ct = params.size();
    if (ct < 2) {
        throw Php::Exception(pun::missingParameter("String",2));
    }
    return params[0];
}

Token8* 
pun::check_Token8(Php::Parameters& params, unsigned int offset)
{
    auto ct = params.size();

    if (offset < ct) {
        const Php::Value& object = params[offset];   
        if (object.instanceOf(Token8::PHP_NAME)) {
            Token8 *obj = (Token8 *)object.implementation();
            return obj;
        }
    }
    throw Php::Exception(pun::missingParameter("Token8 object", offset));
}