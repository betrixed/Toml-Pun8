#include "ustr8.h"
#include "ucode8.h"
#include <ostream>

using namespace pun;

const char* UStr8::PHP_NAME = "Pun\\UStr8";

void
UStr8::setup_ext(Php::Extension& ext)
{
    Php::Class<UStr8> ustr8(UStr8::PHP_NAME);
    ustr8.method<&UStr8::__construct>("__construct", Php::Public);
    ustr8.method<&UStr8::share>("share", Php::Public);
    ustr8.method<&UStr8::setString> ("setString", Php::Public);
    ustr8.method<&UStr8::asUTF16> ("asUTF16", Php::Public);
    ustr8.method<&UStr8::getBOMId> ("getBOMId", Php::Public);
    ustr8.method<&UStr8::ensureUTF8> ("ensureUTF8", Php::Public);
    ustr8.method<&UStr8::getBegin> ("getBegin", Php::Public);
    ustr8.method<&UStr8::getEnd> ("getEnd", Php::Public);
    ustr8.method<&UStr8::size> ("size", Php::Public);
    ustr8.method<&UStr8::setRange> ("setRange", Php::Public);
    ustr8.method<&UStr8::setEnd> ("setEnd", Php::Public);
    ustr8.method<&UStr8::value> ("value", Php::Public);

    ustr8.method<&UStr8::peekChar> ("peekChar", Php::Public);
    ustr8.method<&UStr8::nextChar> ("nextChar", Php::Public);
    ustr8.method<&UStr8::replaceAll> ("replaceAll", Php::Public);
    ustr8.method<&UStr8::endsWith> ("endsWith", Php::Public);
    ustr8.method<&UStr8::beginsWith> ("startsWith", Php::Public);
    ustr8.method<&UStr8::pushBack> ("pushBack", Php::Public);
    ustr8.method<&UStr8::popBack> ("popBack", Php::Public);

    ustr8.method<&UStr8::bomUTF16> ("bomUTF16", Php::Public);
    ustr8.method<&UStr8::bomUTF8> ("bomUTF8", Php::Public);
    ext.add(std::move(ustr8));


}

Php::Value
UStr8::make_UStr8(std::string& s) {
    UStr8* u8 = new UStr8();
    u8->fn_setString(std::move(s));
    return Php::Object(UStr8::PHP_NAME, u8);
}

UStr8::UStr8() : _index(0), _size(0)
{
    _str = std::make_shared<UStrData>();
}

UStr8::UStr8(UStr8& p)
    : _str(p._str), _index(p._index), _size(p._size)
{
}

UStr8* UStr8::get_UStr8(Php::Value& val)
{
    if (val.isObject()) {
        if (val.instanceOf(UStr8::PHP_NAME)) {
            return (UStr8*) val.implementation();
        }
    }
    return nullptr;
}

void UStr8::__construct(Php::Parameters& param)
{
    (*this).setString(param);
}

svx::string_view
UStr8::param_getView(Php::Value& v)
{
    if (v.isString()) {
        return svx::string_view(v, v.size());
    }
    else {
        UStr8* u8 = get_UStr8(v);
        return u8->fn_getView();
    }
    return svx::string_view();
}

Php::Value
UStr8::replaceAll(Php::Parameters& param)
{
    svx::string_view target;
    svx::string_view instr;
    bool checked =  (param.size() >= 2);
    UStr8 *saveObj = nullptr;

    if (checked) {
        target = param_getView(param[0]);
        instr = param_getView(param[1]);
        if (param.size() >= 3) {
            saveObj = UStr8::get_UStr8(param[2]);
        }
    }
    svx::string_view src = fn_getView();
    std::string result;

    auto rct = pun::replaceAll(src, target, instr, result);

    if (rct == 0) {
        // make a copy, because replaceAll did not
        result .assign(src.data(), src.size());
    }
    if (saveObj) {
        saveObj->fn_setString(std::move(result));
        return param[2];
    }
    else {
        return UStr8::make_UStr8(result);
    }
}

Php::Value UStr8::__toString()
{
    svx::string_view result = fn_getView();
    return Php::Value(result.data(), result.size());
}


Php::Value
UStr8::share() {
    UStr8* cp = new UStr8(*this);
    return Php::Object(UStr8::PHP_NAME, cp);
}

// Reset existing string, or provide another
void UStr8::setString(Php::Parameters& param)
{
    if (!param.empty() && param[0].isString()) {
        const char* cptr = param[0];
        fn_setString(cptr, param[0].size());
    }
}

Php::Iterator *
UStr8::getIterator()
{
	return new UStr_Iterator(this, _index, _size);
}

void UStr8::fn_setBegin(Php::Value& val)
{
	if (val.isNumeric()) {
		_index = val.numericValue();
	}
	else
		throw Php::Exception("Range begin not integer");
	auto slen = _str.get()->_view.size();

	if (_index > slen) {
		_index = slen;
	}
}

void UStr8::fn_setEnd(Php::Value& val)
{
	if (val.isNumeric()) {
		_size = val.numericValue();
	}
	else
		throw Php::Exception("Range end not integer");
	auto slen = _str.get()->_view.size();

	if (_size > slen) {
		_size = slen;
	}
}

Php::Value
UStr8::peekChar(Php::Parameters& param) {
    if (_index < _size) {
        auto ustr = _str.get();
        svx::string_view seq;
        char32_t code = ustr->fetch(_index,seq);

        if (code != INVALID_CHAR) {
        	if (param.size() > 0 && param[0].type() == Php::Type::Reference) {
        		param[0] = (int) code;
        	}
            Php::Value(seq.data(), seq.size());
        }
    }
    return Php::Value(false);
}

Php::Value
UStr8::nextChar(Php::Parameters& param) {
    if (_index < _size) {
        auto ustr = _str.get();
        svx::string_view seq;

        char32_t code = ustr->fetch(_index,seq);
        if (code != INVALID_CHAR) {
        	if (param.size() > 0 && param[0].type() == Php::Type::Reference) {
        		param[0] = (int) code;
        	}
        	_index += seq.size();
            return Php::Value(seq.data(), seq.size());
        }
    }
    return Php::Value(false);
}

void UStr8::setRange(Php::Parameters& param)
{
	if (param.size() > 0) {
		fn_setBegin(param[0]);
	}
	else {
		throw Php::Exception("Expect integer as parameter 1");
	}
	if (param.size() > 1) {
		fn_setEnd(param[1]);
	}
}

void UStr8::setEnd(Php::Parameters& param)
{
	if (param.size() > 0) {
		fn_setEnd(param[0]);
	}
	else {
		throw Php::Exception("Expect integer as parameter 1");
	}
}


void UStr8::fn_setString(std::string&& m)
{
    if (_str.use_count() > 1) {
        _str = std::make_shared<UStrData>();
    }
    auto sp = _str.get();
    sp->assign(std::move(m));
    _index = 0;
    _size = sp->_view.size();
}

void UStr8::fn_setString(const char* ptr, unsigned int len)
{
    std::string buffer(ptr, len);
    fn_setString(std::move(buffer));
}

// return BOM for UTF16 as string on this platform
Php::Value
UStr8::bomUTF16()
{
    uint16_t bom = 0xFEFF;
    std::string bomstr( (const char* )&bom, 2);
    return  Php::Value(std::move(bomstr));
}

// return BOM for UTF8  as string
Php::Value
UStr8::bomUTF8()
{
    return ("\xEF\xBB\xBF");
}

// Return PHP string converted to platform UTF16
Php::Value
UStr8::asUTF16() const
{
    std::string result = _str.get()->asUTF16();
    return Php::Value(std::move(result));
}

Php::Value
UStr8::getBOMId()
{
	return _str.get()->getBOMId();
}

void
UStr8::ensureUTF8()
{
	auto sp = _str.get();
	sp->ensureUTF8();
    _index = 0;
    _size = sp->_view.size();
}

/*! Return boolean if argument is last part of string */
Php::Value
UStr8::endsWith(Php::Parameters& param)
{
    svx::string_view src = fn_getView();
    bool checked = (param.size() >= 1);
    if (checked) {
        svx::string_view target = param_getView(param[0]);
        auto target_offset = target.size();
        if (target_offset > src.size()) {
            return false;
        }
        if (src.substr(src.size() - target_offset,target_offset )==target) {
            return true;
        }
    }
    return false;
}
/*! Return boolean if argument is first part of string */
Php::Value
UStr8::beginsWith(Php::Parameters& param)
{
    svx::string_view src = fn_getView();
    bool checked = (param.size() >= 1);
    if (checked) {
        svx::string_view target = param_getView(param[0]);
        auto target_offset = target.size();
        if (target_offset > src.size()) {
            return false;
        }
        if (src.substr(0,target_offset )==target) {
            return true;
        }
    }
    return false;
}

/*! Push the string argument onto the end. This resets the view range. */
void
UStr8::pushBack(Php::Parameters& param)
{

    bool checked = param.size() >= 1;
    if (checked) {
       auto seg = param_getView(param[0]);
       auto sp = _str.get();
       sp->_data.append(seg.data(), seg.size());
       //Php::out << "Add " << seg << std::endl;

       sp->reset_view();
       //Php::out << sp->_view << std::endl;
       _index = 0;
       _size = sp->_view.size();
    }
}

/*! Reduce length by so many characters.  This resets the view range. */
void
UStr8::popBack(Php::Parameters& param)
{
    bool checked = param.size() >= 1;
    if (checked) {
        checked = param[0].isNumeric();
        if (checked) {
            auto value = param[0].numericValue();
            if (value > 0) {
                auto sp = _str.get();
                std::string& data = sp->_data;
                auto oldsize = data.size();
                if (oldsize > (std::string::size_type) value) {
                    data.resize(oldsize - value);
                }
                else {
                    data.clear();
                }
                sp->reset_view();
                _index = 0;
                _size = sp->_view.size();
            }
        }
    }
}

Php::Value UStr8::value() const
{
	auto sp = _str.get();
	auto view = sp->substr(_index, _size - _index);
	return Php::Value(view.data(), view.size());
}

svx::string_view
UStr8::fn_getView()
{
    auto sp = _str.get();
	return sp->substr(_index, _size - _index);
}

