#include "parameter.h"

#include "pcre8.h"
#include "recap8.h"
#include "re8map.h"
#include "token8.h"

#include <sstream>
#include <ostream>
#include <cstdint>

#include "ucode8.h"

/*
Undefined       =   0,  // Variable is not set
    Null            =   1,  // Null will allow any type
    False           =   2,  // Boolean false
    True            =   3,  // Boolean true
    Numeric         =   4,  // Integer type
    Float           =   5,  // Floating point type
    String          =   6,  // A string obviously
    Array           =   7,  // An array of things
    Object          =   8,  // An object
    Resource        =   9,  // A resource
    Reference       =  10,  // Reference to another value (can be any type!)
    Constant        =  11,  // A constant value
    ConstantAST     =  12,  // I think an Abstract Syntax tree, not quite sure
*/

const std::string CPunk::datetime_classname = "DateTime";

pun::Type 
pun::getPunType(Php::Value& val) {
    auto t = pun::getPunType(val.type());
    if (t == pun::Type::Object) {
        if (val.instanceOf(CPunk::keytable_classname)) {
            return pun::Type::KeyTable;
        }
        else if (val.instanceOf(CPunk::valuelist_classname)) {
            return pun::Type::ValueList;
        }
        else if (val.instanceOf(CPunk::datetime_classname)) {
            return pun::Type::DateTime;
        }
    }
    return t;
}

pun::Type pun::getPunType(Php::Type t) {
    switch(t) {
    case Php::Type::String:
        return pun::Type::String;
    case Php::Type::Numeric:
        return pun::Type::Integer;
    case Php::Type::Float:
        return pun::Type::Float;
    case Php::Type::Array:
        return pun::Type::Array;
    case Php::Type::Object:
        return pun::Type::Object;
    case Php::Type::False:
    case Php::Type::True:
        return pun::Type::Bool;
    case Php::Type::Null:
        return pun::Type::Null;
    case Php::Type::Resource:
        return pun::Type::Resource;
    case Php::Type::Reference:
        return pun::Type::Reference;
    case Php::Type::Undefined:        
    default:
        return pun::Type::Undefined;
    }
}

const char* pun::getPunTName(Type t)
{
    switch(t) {
    case pun::Type::String:
        return "string";
    case pun::Type::Integer:
        return "integer";
    case pun::Type::Float:
        return "float";
    case pun::Type::KeyTable:
        return "table";
    case pun::Type::ValueList:
        return "list";
    case pun::Type::DateTime:
        return "datetime";
    case pun::Type::Array:
        return "array";
    case pun::Type::Object:
        return "object";
    case pun::Type::Bool:
        return "boolean";
    case pun::Type::Null:
        return "null";
    case pun::Resource:
        return "resource";
    case pun::Type::Reference:
        return "reference";
    case pun::Type::Undefined:        
    default:
        return "undefined";
    }

}

const char* pun::getTypeName(Php::Type ptype)
{
    switch(ptype) {
    case Php::Type::String:
        return "string";
    case Php::Type::Numeric:
        return "integer";
    case Php::Type::Float:
        return "float";
    case Php::Type::Array:
        return "array";
    case Php::Type::Object:
        return "object";
    case Php::Type::False:
    case Php::Type::True:
        return "boolean";
    case Php::Type::Null:
        return "null";
    case Php::Type::Resource:
        return "resource";
    case Php::Type::Reference:
        return "reference";
    case Php::Type::Undefined:        
    default:
        return "undefined";
    }
}


void pun::hexUniStr8(std::string_view hexval, std::ostream& os)
{
    char32_t val = (char32_t) strtol(hexval.data(), 0, 16);
    EncodeUTF8 ec8;

    ec8.encode(val);
    //Php::out << val << " is " << ec8.result << std::endl;
    os << ec8.result;
}


void pun::need_Value(Php::Parameters& param, unsigned int offset)
{
    if (param.size() < offset) {
        throw Php::Exception(missingParameter("Value", offset));
    }
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
        if (object.isObject() && object.instanceOf(Re8map::PHP_NAME)) {
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
        if (object.isObject() && object.instanceOf(Pcre8::PHP_NAME)) {
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
        if (object.isObject() && object.instanceOf(Token8::PHP_NAME)) {
            Token8 *obj = (Token8 *)object.implementation();
            return obj;
        }
    }
    throw Php::Exception(pun::missingParameter("Token8 object", offset));
}