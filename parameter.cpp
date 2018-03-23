#include "parameter.h"

#include "valuelist.h"

#include "pcre8.h"
#include "recap8.h"
#include "re8map.h"
#include "token8.h"

#include <sstream>
#include <ostream>
#include <cstdint>

#include "ucode8.h"
#include "keytable.h"


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
union SerUnion {
    double dval;
    int64_t ival;
};

const std::string CPunk::datetime_classname = "DateTime";

using namespace pun;


void 
pun::unserialize(Php::Value& val, std::istream& ins)
{
    char check;
    std::string sval;
    SerUnion cval;

    ins >> check;
    switch(check) {
    case 'B':
        ins >> check;
        if (check == '1') {
            val = Php::Value(true);
        }
        else {
            val = Php::Value(false);
        }
        break;
    case 'S':
        pun::unserialize_str(sval, ins);
        val = std::move(sval);
        break;
    case 'F':
        ins.read( ( char*) &cval.dval, sizeof(double));
        val = cval.dval;
        break;
    case 'I':
        ins.read( ( char*) &cval.ival, sizeof(int64_t));
        val = cval.ival;
        break;
    case 'D':
    case 'A':
    case 'O':
        pun::unserialize_str(sval, ins);
        val = Php::call("unserialize", sval);
        break;
    case 'K': {
            auto ktp = new pun::KeyTable();
            ktp->fn_unserialize(ins);
            val = ktp->fn_object();
        }
        break;
    case 'V': {
            auto vlp = new pun::ValueList();
            vlp->fn_unserialize(ins);
            val = vlp->fn_object();
        }
        break;
    case 'N': 
        val = Php::Value();
        break;
    
    }
}

Pype 
pun::getPype(Php::Value& val) {
    auto t = pun::getPype(val.type());
    if (t == pun::tObject) {
        if (val.instanceOf(CPunk::keytable_classname)) {
            return pun::tKeyTable;
        }
        else if (val.instanceOf(CPunk::valuelist_classname)) {
            return pun::tValueList;
        }
        else if (val.instanceOf(CPunk::datetime_classname)) {
            return pun::tDateTime;
        }
    }
    return t;
}

pun::Pype pun::getPype(Php::Type t) {
    switch(t) {
    case Php::Type::String:
        return pun::tString;
    case Php::Type::Numeric:
        return pun::tInteger;
    case Php::Type::Float:
        return pun::tFloat;
    case Php::Type::Array:
        return pun::tArray;
    case Php::Type::Object:
        return pun::tObject;
    case Php::Type::False:
    case Php::Type::True:
        return pun::tBool;
    case Php::Type::Null:
        return pun::tNull;
    case Php::Type::Resource:
        return pun::tResource;
    case Php::Type::Reference:
        return pun::tReference;
    case Php::Type::Undefined:        
    default:
        return pun::tUndefined;
    }
}

const char* pun::getPypeId(Pype t)
{
    switch(t) {
    case pun::tString:
        return "string";
    case pun::tInteger:
        return "integer";
    case pun::tFloat:
        return "float";
    case pun::tKeyTable:
        return "table";
    case pun::tValueList:
        return "list";
    case pun::tDateTime:
        return "datetime";
    case pun::tArray:
        return "array";
    case pun::tObject:
        return "object";
    case pun::tBool:
        return "boolean";
    case pun::tNull:
        return "null";
    case pun::tResource:
        return "resource";
    case pun::tReference:
        return "reference";
    case pun::tUndefined:        
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

KeyTable* pun::castKeyTable(Php::Value& val) {
    if  (!val.isObject())
        return nullptr;
    Php::Base* base = val.implementation();
    return  dynamic_cast<KeyTable*>(base);
}

void pun::hexUniStr8(svx::string_view hexval, std::ostream& os)
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

KeyTable* 
pun::check_KeyTable(Php::Parameters& params, unsigned int offset)
{
    auto ct = params.size();

    if (offset < ct) {
        KeyTable *obj = castKeyTable(params[offset]);
            if (obj)
                return obj;  
    }
    throw Php::Exception(pun::missingParameter("KeyTable object", offset));

}


void pun::serialize_valueList(Php::Base* base, std::ostream& out)
{
    pun::ValueList* vlist = ( pun::ValueList* ) base;
    vlist->fn_serialize(out);
}

void pun::serialize_keyTable(Php::Base* base, std::ostream& out)
{
    pun::KeyTable* ktab = ( pun::KeyTable* ) base;
    ktab->fn_serialize(out);
}

void 
pun::serialize_str(const char* s, size_t slen, std::ostream& out) {
    out.write((const char*) &slen, sizeof(slen));
    out.write(s, slen);
}




void 
pun::unserialize_str(std::string& cval, std::istream& ins)
{
    size_t slen;
    ins.read(( char*) &slen, sizeof(slen));
    //Php::out << "unser. str len " << slen << std::endl;
    cval.assign(slen,'0');
    ins.read(( char*) cval.data(), slen);
    //Php::out << "unser. str " << cval << std::endl;
}



void 
pun::serialize(Php::Value& val, std::ostream& out)
{
    auto ptype = pun::getPype(val);
    SerUnion cval;

    std::string sval;
    Php::Value result;

    switch(ptype) {
    case pun::tBool:
        // All our types are distinguishable on capital letter,
        // so 
        out << 'B' << (val.boolValue() ? '1' : '0');
        break;
    case pun::tString:
        out << 'S';
        pun::serialize_str(val, val.size(), out);
        break;

    case pun::tFloat:
        out << 'F';
        cval.dval = val.floatValue();
        out.write((const char*) &cval.dval, sizeof(double));
        break;

    case pun::tInteger:
        out << 'I';
        cval.ival = val.numericValue();
        out.write((const char*) &cval.ival, sizeof(cval.ival));
        //Php::out << "Write tInteger" << std::endl;
        break;

    case pun::tDateTime:
        out << 'D';
        result = Php::call("serialize", val);
        pun::serialize_str(result, result.size(), out);
        break;

    case pun::tValueList:
        out << 'V';
        pun::serialize_valueList(val.implementation(),out);
        break;

    case pun::tKeyTable:
        out << 'K';
        pun::serialize_keyTable(val.implementation(),out);
        break;

    case pun::tArray:
        out << 'A';
        result = Php::call("serialize", val);
        pun::serialize_str(result, result.size(), out);
        break;

    case pun::tNull:
        out << 'N'; // stands for itself
        break;

    case pun::tObject:
    default:
        out << 'O';
        result = Php::call("serialize", val);
        pun::serialize_str(result, result.size(), out);
        break;

    }
}