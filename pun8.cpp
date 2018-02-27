
#include "pun8.h"
#include "pcre8.h"
#include "re8map.h"
#include "ucode8.h"
#include <sstream>

const std::string Pun8::PHP_NAME = "Pun\\Pun8";

Pun8::Pun8() : _index(0), _size(0), _myChar(0)
{
    _remap = std::make_shared<Pcre8_map>();
}

Pun8::~Pun8()
{

}

void Pun8::__construct(Php::Parameters& params)
{
    (*this).setString(params);
}

// Reset existing string, or provide another
void Pun8::setString(Php::Parameters& params)
{
     _index = 0;
     _myChar = 0;

    if (!params.empty()) {
        _mystr = params[0];
        _size = params[0].size();
    } 
}

Re8map* Pun8::check_Re8map(Php::Parameters& params, unsigned int offset)
{
    auto ct = params.size();
    if (offset >= ct) {
        throw Php::Exception("Parameter 1 missing: Re8map object");
    }
    Php::Value object = params[offset];   
    if (!object.instanceOf(Re8map::PHP_NAME)) {
        throw Php::Exception("Parameter 1 should be Re8map object");
    } 

    Re8map *obj = (Re8map *)object.implementation();
    return obj;    
}
Pcre8* Pun8::check_Pcre8(Php::Parameters& params, unsigned int offset)
{
    auto ct = params.size();
    if (offset >= ct) {
        throw Php::Exception("Parameter 1 missing: Pcre8 object");
    }
    Php::Value object = params[offset];   
    if (!object.instanceOf(Pcre8::PHP_NAME)) {
        throw Php::Exception("Parameter 1 should be Pcre8 object");
    } 

    Pcre8 *obj = (Pcre8 *)object.implementation();
    return obj;
}

 void Pun8::check_String(Php::Parameters& params,unsigned int offset)
 {
    auto ct = params.size();
    if (offset >= ct) {
        std::stringstream ss;
        ss << "Missing Parameter " << offset << ": String";
        throw Php::Exception(ss.str().data());
    }    

 }

 bool Pun8::option_Array(Php::Parameters& params, unsigned int offset)
 {
    auto ct = params.size();
    if (offset >= ct) {
        return false;
    }
    return true;
 }

 bool Pun8::option_Int(Php::Parameters& params,unsigned int offset)
 {
    auto ct = params.size();
    return (offset < ct);
 }


void Pun8::check_Int(Php::Parameters& params,unsigned int offset)
{
    auto ct = params.size();
    if (ct < 1) {
        std::stringstream ss;
        ss << "Missing Parameter " << offset << ": Integer";
        throw Php::Exception(ss.str().data());
    }    
}
void Pun8::check_IntString(Php::Parameters& params)
{
    Pun8::check_Int(params);
    auto ct = params.size();
    if (ct < 2) {
        throw Php::Exception("Parameter 2 missing: String - PCRE2 expression");
    }
}


Php::Value Pun8::getCode() const {
    return Php::Value(int(_myChar));
}

Php::Value Pun8::getOffset() const {
    return Php::Value(int(_index));
}

void Pun8::setOffset(Php::Parameters& params) {
    if (!params.empty()) {
        _index = (int) params[0];
    }
}

void Pun8::addOffset(Php::Parameters& params) {
    if (!params.empty()) {
        _index += (int) params[0];
    }
}

Php::Value Pun8::nextChar() {
    if (_index < _size) {
        char const* buf = _mystr;
        int oldIndex = _index;
        if (ucode8Fore(buf, _size, _index, _myChar)) {
            return Php::Value(&buf[oldIndex], _index - oldIndex);
        }
    }
    return Php::Value(false);
}

Php::Value 
Pun8::matchPcre8(Php::Parameters& params)
{
    Pcre8* p8 = Pun8::check_Pcre8(params);
    auto  sp = p8->getImp();
    Php::Value result;
    this->Pun8::matchSP(sp, result);
    return result;
}

bool 
Pun8::matchSP(Pcre8_share& sp, Php::Value& result)
{
    Pcre8_match matches;
    char const* buf;
    int rct;
    auto pimp = sp.get();
    
    if (_index < _size) {
        buf = _mystr;
        buf += _index;
        rct = pimp->doMatch(
                 reinterpret_cast<const unsigned char*>(buf),
                 _size - _index, 
                  matches);

        if (rct > 0) {
        // return array of strings
            for(int i = 0; i < rct; i++)
            {
                result[i] = matches._slist[i];
            }
            return true;
        }
    }
    result = false;
    return false;

}


void Pun8::setRe8map(Php::Parameters& params)
{
    Re8map* obj = Pun8::check_Re8map(params);
    _remap = obj->getImp();
}

Php::Value 
Pun8::matchIdRex(Php::Parameters& params)
{
    Pun8::check_Int(params);
    int id = params[0];
   
    auto map = _remap.get();
     Pcre8_share sp;
    if (!map->getRex(id, sp)) {
        throw Php::Exception("No PCRE2 expression found at index");
    }
    Php::Value result;

    this->Pun8::matchSP(sp, result); 
    return result;
}

Php::Value 
Pun8::setIdRex(Php::Parameters& params)
{
    auto sp = Pcre8::fromParameters(params);
    auto map = _remap.get();
    map->setRex(sp);
    return Php::Value(params[0]);
}

Php::Value 
Pun8::getIds() const
{
    Php::Value result;
    auto map = _remap.get();
    auto m1 = map->_map.begin();
    auto mend = map->_map.end();
    int i = 0;
    while(m1 != mend) {
        result[i] = Php::Value(m1->first);
        ++i;
        ++m1;
    }
    return result;
}

Php::Value 
Pun8::getIdRex(Php::Parameters& params) 
{
    Pun8::check_Int(params);
    int index = params[0];
    Pcre8_share sp;
    auto map = _remap.get();
    if (!map->getRex(index,sp))
    {
        return Php::Value(false);
    }

    auto p8 = new Pcre8();
    p8->setImp(sp);
    auto result = Php::Object(Pcre8::PHP_NAME.data(), p8);
    return result;
}
