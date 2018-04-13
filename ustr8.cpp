#include "ustr8.h"
#include "ucode8.h"

using namespace pun;

const char* UStr8::PHP_NAME = "Pun\\UStr8";

void
UStr8::setup_ext(Php::Extension& ext)
{
    Php::Class<UStr8> ustr8(UStr8::PHP_NAME);
    ustr8.method<&UStr8::__construct>("__construct");
    ustr8.method<&UStr8::share>("share");
    ustr8.method<&UStr8::setString> ("setString");
    ustr8.method<&UStr8::asUTF16> ("asUTF16");
    ustr8.method<&UStr8::getBOMId> ("getBOMId");
    ustr8.method<&UStr8::ensureUTF8> ("ensureUTF8");
    ustr8.method<&UStr8::getBegin> ("getBegin");
    ustr8.method<&UStr8::getEnd> ("getEnd");
    ustr8.method<&UStr8::size> ("size");
    ustr8.method<&UStr8::setRange> ("setRange");
    ustr8.method<&UStr8::setEnd> ("setEnd");
    ustr8.method<&UStr8::value> ("value");

    // 2 static methods
    ustr8.method<&UStr8::bomUTF16> ("bomUTF16");
    ustr8.method<&UStr8::bomUTF8> ("bomUTF8");
    ext.add(std::move(ustr8));
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


void UStr8::fn_setString(const char* ptr, unsigned int len)
{
     _index = 0;
     _size = len;
     auto sp = _str.get();
     sp->assign(ptr,len);
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

