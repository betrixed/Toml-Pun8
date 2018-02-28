
#include "pun8.h"
#include "pcre8.h"
#include "re8map.h"
#include "recap8.h"
#include "ucode8.h"
#include <sstream>

const char* Pun8::PHP_NAME = "Pun\\Pun8";

std::string 
Pun8::missingParameter(const char* shouldBe, unsigned int offset)
{
    std::stringstream ss;

    ss << "Parameter " << offset << " should be " << shouldBe;
    return ss.str();
}

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

Recap8* 
Pun8::option_Recap8(Php::Parameters& params, unsigned int offset)
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
Pun8::check_Re8map(Php::Parameters& params, unsigned int offset)
{
    auto ct = params.size();

    if (offset < ct) {
        const Php::Value& object = params[offset];   
        if (object.instanceOf(Re8map::PHP_NAME)) {
            Re8map *obj = (Re8map *)object.implementation();
            return obj;  
        }
    }
    throw Php::Exception(missingParameter("Re8map object",offset));
  
}


Pcre8* 
Pun8::check_Pcre8(Php::Parameters& params, unsigned int offset)
{
    auto ct = params.size();

    if (offset < ct) {
        const Php::Value& object = params[offset];   
        if (object.instanceOf(Pcre8::PHP_NAME)) {
            Pcre8 *obj = (Pcre8 *)object.implementation();
            return obj;
        }
    }
    throw Php::Exception(missingParameter("Pcre8 object", offset));
}

 
bool Pun8::check_String(Php::Parameters& params,unsigned int offset)
{
    auto ct = params.size();
    if (offset >= ct) {
        throw Php::Exception(missingParameter("String", offset));
    }    
    return params[offset].isString();
}

bool Pun8::option_Array(Php::Parameters& params, unsigned int offset)
{
    auto ct = params.size();
    if (offset >= ct) {
        return false;
    }
    return params[offset].isArray();
}

bool Pun8::option_Int(Php::Parameters& params,unsigned int offset)
{
    auto ct = params.size();
    return (offset < ct);
}

int 
Pun8::check_Int(Php::Parameters& params,unsigned int offset)
{
    auto ct = params.size();
    if (ct < 1) {
        throw Php::Exception(missingParameter("Integer", offset));
    }
    return params[0];
}

int 
Pun8::check_IntString(Php::Parameters& params)
{
    
    auto ct = params.size();
    if (ct < 2) {
        throw Php::Exception(missingParameter("String",2));
    }
    return params[0];
}

Php::Value 
Pun8::getCode() const {
    return Php::Value(int(_myChar));
}

Php::Value 
Pun8::getOffset() const {
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
    Pcre8* p8 = Pun8::check_Pcre8(params,0);
    Recap8* cap = Pun8::option_Recap8(params,1);

    auto  sp = p8->getImp();
    
    Pcre8_match result;

    this->Pun8::matchSP(sp, result);

    if (cap == nullptr) {
        cap = new Recap8(); 
    }
    cap->_match = std::move(result);
    Php::Value obj = Php::Object(Recap8::PHP_NAME, cap);
    return obj;
}

bool 
Pun8::matchSP(Pcre8_share& sp, Pcre8_match& matches)
{
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
        return (rct > 0);
    }
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
    int id = Pun8::check_Int(params,0);
    Recap8* cap = Pun8::option_Recap8(params,1);

    auto map = _remap.get();
     Pcre8_share sp;
    if (!map->getRex(id, sp)) {
        throw Php::Exception("No PCRE2 expression found at index");
    }
    Pcre8_match matches;

    this->Pun8::matchSP(sp, matches); 

    if (cap == nullptr) {
        cap = new Recap8();
    }
    cap->_match = std::move(matches);

    Php::Value result = Php::Object(Recap8::PHP_NAME, cap);
    return result;
}

Php::Value 
Pun8::firstMatch(Php::Parameters& params)
{
    auto isArray = Pun8::option_Array(params, 0);
    Recap8* cap = option_Recap8(params,1);
    if (!isArray || (cap == nullptr)) {
        throw Php::Exception("Need Arguments of (Array, Recap8)");
    }
    const Php::Value& v = params[0];
    Pcre8_match matches;
    auto pimp = _remap.get();
    auto mapend = pimp->_map.end();
    for(int i = 0; i < v.size(); i++)
    {
        int index = v[i];
        auto pit = pimp->_map.find(index);
        if (pit != mapend)
        {
            if (this->matchSP(pit->second, matches))
            {
                cap->_match = std::move(matches);
                return Php::Value(index);
            }
        }
    }
    return Php::Value(0);
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
    int index = Pun8::check_Int(params,0);

    Pcre8_share sp;
    auto map = _remap.get();
    if (!map->getRex(index,sp))
    {
        return Php::Value(false);
    }

    auto p8 = new Pcre8();
    p8->setImp(sp);
    auto result = Php::Object(Pcre8::PHP_NAME, p8);
    return result;
}
