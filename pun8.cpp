
#include "pun8.h"
#include "pcre8.h"
#include "re8map.h"
#include "recap8.h"
#include "ucode8.h"
#include "parameter.h"

#include <sstream>
#include <ostream>
const char* Pun8::PHP_NAME = "Pun\\Pun8";



Pun8::Pun8() : _index(0), _size(0), _myChar(0)
{
    _remap = std::make_shared<Pcre8_map>();
}

Pun8::~Pun8()
{
    //Php::out << "Deleted Pun8 " << std::endl;
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
        const char* cptr = params[0];

        _size = params[0].size();

        _mystr.assign(cptr, _size);
    } 
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

Php::Value Pun8::peekChar() {
    if (_index < _size) {
        char const* buf = _mystr.data();
        auto oldIndex = _index;
        if (ucode8Fore(buf, _size, oldIndex, _myChar)) {
            return Php::Value(&buf[_index], oldIndex -_index);
        }
    }
    return Php::Value(false);
}



Php::Value Pun8::nextChar() {
    if (_index < _size) {
        char const* buf = _mystr.data();
        auto oldIndex = _index;
        if (ucode8Fore(buf, _size, _index, _myChar)) {
            return Php::Value(&buf[oldIndex], _index - oldIndex);
        }
    }
    return Php::Value(false);
}

Php::Value 
Pun8::matchIdRex8(Php::Parameters& params)
{
    Pcre8* p8 = pun::check_Pcre8(params,0);
    Recap8* cap = pun::option_Recap8(params,1);
    if (cap == nullptr) {
        throw Php::Exception("Need Arguments of (IdRex8, Recap8)");
    }
    auto  sp = p8->getImp();
    
    Pcre8_match result;

    int count = this->matchSP(sp, result);
    cap->_match = std::move(result);
    return Php::Value(count);
}

int 
Pun8::matchSP(Pcre8_share& sp, Pcre8_match& matches)
{
    char const* buf;
    int rct;
    auto pimp = sp.get();
    
    if (_index < _size) {

        buf = _mystr.data();
        buf += _index;
        //Php::out << "target: " << buf << " with " << pimp->_eStr << std::endl;
        rct = pimp->doMatch(
                 reinterpret_cast<const unsigned char*>(buf),
                 _size - _index, 
                  matches);
        if (rct > 0) {
            // _rcode to hold match mapId
            //Php::out << "Matched " << pimp->_id << std::endl;
            matches._rcode = pimp->_id;
            return matches._rcode;
        }
    }
    matches._rcode = 0;
    return 0;
}


void Pun8::setRe8map(Php::Parameters& params)
{
    Re8map* obj = pun::check_Re8map(params);
    _remap = obj->getImp();
}

Php::Value 
Pun8::matchMapId(Php::Parameters& params)
{
    int id = pun::check_Int(params,0);
    Recap8* cap = pun::option_Recap8(params,1);
    if (cap == nullptr) {
        throw Php::Exception("Need Arguments of (Integer, Recap8)");
    }
    auto map = _remap.get();
     Pcre8_share sp;
    if (!map->getRex(id, sp)) {
        throw Php::Exception("No PCRE2 expression found at index");
    }
    Pcre8_match matches;

    int count = this->matchSP(sp, matches); 

    cap->_match = std::move(matches);

    return Php::Value(count);
}

Php::Value Pun8::getIdList() {
    Php::Value result;
    this->fn_copyIdList(result);
    return result;
}
void Pun8::setIdList(Php::Parameters& params)
{
    auto isArray = pun::option_Array(params, 0); 
    if (!isArray) {
        throw Php::Exception("Need (Array of integer)");
    }
    const Php::Value& v = params[0];
    auto ct = v.size();
    _idlist.clear();
    _idlist.reserve(ct);
    for(int i = 0; i < v.size(); i++)
    {
        _idlist.push_back(v[i]);
    }
}

Php::Value 
Pun8::firstMatch(Php::Parameters& params)
{
    Recap8* cap = pun::option_Recap8(params,0);
    if (cap == nullptr) {
        throw Php::Exception("Need Argument of (Recap8)");
    }

    auto result = fn_firstMatch(cap->_match);
    return Php::Value(result);
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
    int index = pun::check_Int(params,0);

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

int Pun8::fn_peekChar()
{
    if (_index < _size) {
        char const* buf = _mystr.data();
        auto oldIndex = _index;
        if (ucode8Fore(buf, _size, oldIndex, _myChar)) {
            return oldIndex - _index;;
        }
    }
    return 0;
}

int Pun8::fn_firstMatch(Pcre8_match& matches)
{
    auto pimp = _remap.get();
    //Php::out << "Number of expressions = " << pimp->_map.size() << std::endl;
    auto mapend = pimp->_map.end();

    auto mid = _idlist.begin();
    auto idend = _idlist.end();
    while (mid != idend) {
        int index = (*mid);
        mid++;
        auto pit = pimp->_map.find(index);
        if (pit != mapend)
        {
            if (this->matchSP(pit->second, matches))
            {
                return index;
            }
        }
    }
    matches._slist.clear();
    matches._rcode = 0;
    return 0;
}

 void Pun8::fn_copyIdList(Php::Value& v)
 {
    auto idle = _idlist.begin();
    auto idend = _idlist.end();
    int idx = 0;
    while (idle != idend) {
        v[idx] = *idle;
        idle++;
        idx++;
    }
 }