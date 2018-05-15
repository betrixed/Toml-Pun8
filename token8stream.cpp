//tokenstream.cpp
#include "token8stream.h"
#include "token8.h"
#include "parameter.h"
#include "pcre8.h"
#include "ucode8.h"
#include "ustr8.h"
#include <ostream>

using namespace pun;

const char* Token8Stream::PHP_NAME = "Pun\\Token8Stream";

void Token8Stream::setup_ext(Php::Extension &ext)
{
    Php::Class<Token8Stream> t8s(Token8Stream::PHP_NAME);

    t8s.method<&Token8Stream::setEOSId> ("setEOSId");
    t8s.method<&Token8Stream::setEOLId> ("setEOLId");
    t8s.method<&Token8Stream::setUnknownId> ("setUnknownId");

    t8s.method<&Token8Stream::setExpSet> ("setExpSet");
    t8s.method<&Token8Stream::getExpSet> ("getExpSet");
    t8s.method<&Token8Stream::setSingles> ("setSingles");
    t8s.method<&Token8Stream::setRe8map> ("setRe8map");
    t8s.method<&Token8Stream::setInput> ("setInput");


    t8s.method<&Token8Stream::hasPendingTokens> ("hasPendingTokens");
    t8s.method<&Token8Stream::getToken> ("getToken");
    t8s.method<&Token8Stream::getLine> ("getLine");
    t8s.method<&Token8Stream::getValue> ("getValue");
    t8s.method<&Token8Stream::getId> ("getId");
    t8s.method<&Token8Stream::getOffset> ("getOffset");
    t8s.method<&Token8Stream::beforeEOL> ("beforeEOL");


    t8s.method<&Token8Stream::peekToken> ("peekToken");
    t8s.method<&Token8Stream::acceptToken> ("acceptToken");
    t8s.method<&Token8Stream::moveNextId> ("moveNextId");
    t8s.method<&Token8Stream::moveRegex> ("moveRegex");
    t8s.method<&Token8Stream::moveRegId> ("moveRegId");
    ext.add(std::move(t8s));
}

Token8Stream::Token8Stream()
{
	_flagLF = false;
	_tokenLine = 0;
    _str = std::make_shared<UStrData>();
	_eolId = 0;
	_eosId = 0;
	_unknownId = 0;

}
Token8Stream::~Token8Stream()
{
}

void Token8Stream::checkLineFeed(Token8* token)
{
	if (token->_unicode == 13)
	{
		// skip and expect a line feed
		_index += 1;
		fn_peekChar(token);
	}
	if (token->_unicode == 10) {
		_flagLF = true;
		token->_id = _eolId;
		token->_value = svx::string_view();
		token->_line = _tokenLine;
		token->_isSingle = true;
		return;
	}
	throw Php::Exception(pun::invalidCharacter(token->_unicode));
}

uint64_t
Token8Stream::fn_peekChar(Token8* token)
{
    if (_index < _size) {
        auto unc8 = _str.get()->fetch(_index, token->_value);
        if (unc8 != INVALID_CHAR) {
            token->_unicode = unc8;
            return token->_value.size();
        }
    }
    token->_id = _eosId;
    token->_unicode = INVALID_CHAR;
    token->_value = svx::string_view();
    token->_isSingle = true;

    return 0;
}

unsigned char
Token8Stream::fn_peekByte() const
{
	if (_size > _index)
	{
		return (unsigned char) _str.get()->_view[_index];
	}
	throw Php::Exception("PeekByte past end of string");
}

uint64_t
Token8Stream::fn_size() const
{
	return _str.get()->_view.size();
}

Php::Value
Token8Stream::getIdList() {
    Php::Value result;
    this->fn_copyIdList(result);
    return result;
}


void Token8Stream::fn_copyIdList(Php::Value& v)
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


void
Token8Stream::setIdList(Php::Parameters& params)
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

void Token8Stream::setRe8map(Php::Parameters& params)
{
    Re8map* obj = pun::check_Re8map(params);
    _remap = obj->getImp();
}

const char*
Token8Stream::fn_data() const
{
	return _str.get()->_view.data();
}

void Token8Stream::fn_addOffset(unsigned int offset)
{
	_index += offset;
}

unsigned char
Token8Stream::fn_movePeekByte()
{
	_index++;
	return fn_peekByte();
}


svx::string_view Token8Stream::fn_substr(size_t start, size_t slen)
{
	return svx::string_view(_str.get()->_view.data() + start, slen);
}
void Token8Stream::fn_peekToken(Token8* token) {
	auto nextCt = fn_peekChar(token);
	if (nextCt==0) {
		token->_line = _tokenLine;
	}
	else if (token->_unicode < 20) {
		this->checkLineFeed(token);
	}
	else {
		token->_line = _tokenLine;
		token->_isSingle = false;
		if (nextCt == 1) {
			auto cmap = _singles.get();
			auto fit = cmap->getV(token->_unicode);
			if (fit) {
				token->_id = fit;
				token->_isSingle = true;
			}
			else {
				token->_id = _unknownId;
			}
		}
		else {
			token->_id = _unknownId;
		}
	}
}
// Argument is Token8 to put next value into, return same token object
Php::Value Token8Stream::peekToken(Php::Parameters& params)
{
	Token8* token = pun::check_Token8(params,0);
	fn_peekToken(token);
	return Php::Value(Php::Object(Token8::PHP_NAME, token));
}

void Token8Stream::fn_acceptToken(Token8* token)
{
	if (_flagLF) {
		_flagLF = false;
		_tokenLine += 1;
	}
	_token = std::move(*token);
	_token._line = _tokenLine;
	if (_token._id == _eosId) {
		return;
	}
	else if (_token._id == _eolId) {
		_index++;
		return;
	}
	_index += _token._value.size();
}

void Token8Stream::acceptToken(Php::Parameters& params)
{
	Token8* token = pun::check_Token8(params,0);
	fn_acceptToken(token);
}

int  Token8Stream::fn_moveNextId() {
	if (_flagLF) {
		_flagLF = false;
		_tokenLine += 1;
		_token._line = _tokenLine;
	}
	auto nextCt = fn_peekChar(&_token);
	//Php::out << "peekChar " << nextCt << std::endl;
	if (nextCt==0) {
		_token._id = _eosId;
	}
	else if (_token._unicode < 20) {
		this->checkLineFeed(&_token);
		_index++;
	}
	else {
		//Php::out << std::endl << "Peek: " << _token._unicode <<  " size " << nextCt << std::endl;
		auto matchId = fn_firstMatch(_caps);
		if (_caps._slist.size() > 1) {
			_token._id = _caps._rcode;
			_token._value = _caps.capture(1);
			_token._isSingle = false;
			auto advance = _caps._slist[0].size();

			//Php::out << std::endl << "0: " << _caps._slist[0] << std::endl;
			//Php::out << "1: " << _caps._slist[1] << std::endl;
			//Php::out << "size " << advance << std::endl;
			//Php::out.flush();
			_index += advance;
		}
		else {			// no capture,
			if (matchId != 0) {
				throw Php::Exception("Match Id without 2 captures");
			}
			_index += nextCt;
			_token._isSingle = false;
			if (nextCt == 1) {
				auto cmap = _singles.get();
				auto fit = cmap->getV(_token._unicode);
				if (fit) {
					_token._id = fit;
					_token._isSingle = true;
				}
				else {
					_token._id = _unknownId;
				}
			}
			else {
				_token._id = _unknownId;
			}
		}
	}
	_token._line = _tokenLine;
	return _token._id;
}


int
Token8Stream::fn_firstMatch(Pcre8_match& matches)
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

Php::Value
Token8Stream::moveNextId()
{
	return Php::Value(fn_moveNextId());
}


int
Token8Stream::matchSP(Pcre8_share& sp, Pcre8_match& matches)
{
    char const* buf;
    int rct;
    auto pimp = sp.get();

    if (_index < _size) {

        buf = _str.get()->_view.data();
        buf += _index;
        //Php::out << "target: " << buf << " with " << pimp->_eStr << std::endl;
        rct = pimp->doMatch(svx::string_view(buf, _size - _index), matches);
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
Php::Value
Token8Stream::moveRegex(Php::Parameters& params)
{
	Pcre8* p8 = pun::check_Pcre8(params,0);
    if (p8 == nullptr) {
        throw Php::Exception("Need Argument of (IdRex8)");
    }
    auto  sp = p8->getImp();
    bool result = false;
    auto code = matchSP(sp, _caps);
    if (code != 0 && _caps._slist.size() > 1) {
    	_token._value = _caps.capture(1);
    	auto advance = _caps._slist[0].size();
    	_index += advance;
    	_token._id = _unknownId;
    	_token._isSingle = false;
    	result = true;
    }
    return Php::Value(result);
}



bool Token8Stream::fn_moveRegId(int id)
{
	auto map = _remap.get();
    Pcre8_share sp;
    if (!map->getRex(id, sp)) {
        throw Php::Exception("No PCRE2 expression found at index");
    }
    bool result = false;
    auto code = matchSP(sp, _caps);
    if (code != 0 && _caps._slist.size() > 1) {
    	_token._value = _caps.capture(1);
    	auto advance = _caps._slist[0].size();
    	_index += advance;
    	_token._id = _unknownId;
    	_token._isSingle = false;
    	result = true;
    }
    return result;
}

Php::Value Token8Stream::moveRegId(Php::Parameters& params)
{
	int id = pun::check_Int(params,0);
    return Php::Value(this->fn_moveRegId(id));
}

void Token8Stream::setEOSId(Php::Parameters& params)
{
	int id = pun::check_Int(params,0);
	_eosId = id;
}

void Token8Stream::setEOLId(Php::Parameters& params)
{
	int id = pun::check_Int(params,0);
	_eolId = id;
}

void Token8Stream::setUnknownId(Php::Parameters& params)
{
	int id = pun::check_Int(params,0);
	_unknownId = id;
}

void Token8Stream::setExpSet(Php::Parameters& params)
{
	setIdList(params);
}

Php::Value Token8Stream::getExpSet()
{
	Php::Value result;
	fn_copyIdList(result);
	return result;
}
// argument is associative array, of string => id
// convert to char32_t -> id
void Token8Stream::setSingles(Php::Parameters& params)
{
	bool isArray = pun::option_Array(params,0);

	if (!isArray) {
		throw Php::Exception(pun::missingParameter("Array(string => int)", 0));
	}
	const Php::Value& v = params[0];

	if (!_singles) {
		_singles = std::make_shared<CharMap>();
	}
	auto cmap = _singles.get();

	for( auto &iter : v) {
		const Php::Value& sval = iter.first;
		const char* cp = sval;
		auto cpsize = sval.size();
		if (cpsize > 0) {
			char32_t code = cp[0];
			int id = iter.second;
			//Php::out << "Cmap setKV " << cp << " "<< code << " id " << id << std::endl;
			cmap->setKV(code,id);
		}
	}
}

Php::Value
Token8Stream::getOffset() const {
    return Php::Value(int(_index));
}


Php::Value
Token8Stream::beforeEOL()  {
	return Php::Value(fn_beforeChar(10));
}

void
Token8Stream::fn_setString(const char* ptr, uint64_t len)
{
    // if _str is shared, need to copy - on - write
    if (_str.use_count() > 1) {
        _str = std::make_shared<UStrData>();
    }
    _str.get()->assign(ptr,len);
    _index = 0;
    _size = len;
    _flagLF = true;
}

void Token8Stream::setString(const char* ptr, unsigned int len)
{
	fn_setString(ptr,len);
}

void Token8Stream::fn_setString(Str_ptr& sp)
{
    _str = sp;
    _size = _str.get()->_view.size();
    _index = 0;
    _flagLF = true;
}

void Token8Stream::setInput(Php::Parameters& param)
{
    if (param.size() > 0) {
        Php::Value& val = param[0];
        if (val.isString()) {
            fn_setString(val, val.size());
        }
        else if (val.isObject()) {
            if (val.instanceOf(UStr8::PHP_NAME)) {
                UStr8* obj = (UStr8*) val.implementation();
                fn_setString(obj->_str);

            }
        }
    }
}

Php::Value Token8Stream::hasPendingTokens() const
{
	bool result = (_token._id != _eosId);
	return Php::Value(result);
}


svx::string_view
Token8Stream::fn_getValue()
{
	return _token._value;
}

void Token8Stream::fn_setSingles(CharMap_sp& sp)
{
	_singles = sp;
}

uint64_t
Token8Stream::fn_getOffset() const
{
	return _index;
}

Token8*
Token8Stream::fn_getToken(Token8 &token)
{
	token = _token;
	return &token;
}

Php::Value Token8Stream::getToken(Php::Parameters& params) const
{
	Token8* token = pun::check_Token8(params,0);
	*token = _token;
	return Php::Value(Php::Object(Token8::PHP_NAME, token));
}

Php::Value Token8Stream::getLine() const
{
	return Php::Value((int) _tokenLine);
}

Php::Value Token8Stream::getValue() const
{
	return Php::Value(_token._value.data(), _token._value.size());
}

Php::Value Token8Stream::getId() const
{
	return Php::Value(_token._id);
}

void Token8Stream::setExpSet(const IdList& list) {
	_idlist = list;
}

std::string
Token8Stream::fn_beforeChar(char32_t c) const
{
    auto offset = _index;
    //auto prev = offset;
    auto test = INVALID_CHAR;
    svx::string_view seq;
    auto ustr = _str.get();
    while(test != c) {
        test = ustr->fetch(offset, seq);

        //clen = ucode8Fore(bptr+offset, _size-offset, test );
        if (test == INVALID_CHAR)
        {
            break;
        }
        offset += seq.size();
    }

    return std::string(ustr->_view.data() + _index, offset - _index);
}
