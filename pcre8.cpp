#include "pcre8.h"
#include "recap8.h"
#include "parameter.h"
#include <ostream>
#include "ustr8.h"

using namespace pun;


const char* Pcre8::PHP_NAME = "Pun\\Preg";

void
Pcre8::setup_ext(Php::Extension& ext)
{
    Php::Class<Pcre8> preg(Pcre8::PHP_NAME);
    preg.method<&Pcre8::__construct>("__construct", Php::Public);
    preg.method<&Pcre8::setPreg> ("setPreg", Php::Public);
    preg.method<&Pcre8::getPreg> ("getPreg", Php::Public);
    preg.method<&Pcre8::getId> ("getId", Php::Public);
    preg.method<&Pcre8::isCompiled> ("isCompiled", Php::Public);
    preg.method<&Pcre8::match> ("match", Php::Public);
    preg.method<&Pcre8::matchAll> ("matchAll", Php::Public);
    ext.add(std::move(preg));
}

Pcre8::Pcre8()
{

}

Pcre8::~Pcre8()
{
}

void Pcre8::__construct(Php::Parameters& params)
{
	(*this).setPreg(params);
}

Pcre8_share
Pcre8::fromParameters(Php::Parameters& params)
{
	int index = pun::check_IntString(params);
    const char* str = params[1];
    auto stringSize = params[1].size();

    Pcre8_share sp = pun::makeSharedRe(index, str, stringSize);
    return sp;
}
/** param 0 - id, param 1 - string pcre2 */

void Pcre8::setPreg(Php::Parameters& params)
{
	_imp = Pcre8::fromParameters(params);
}

void Pcre8::setImp(const Pcre8_share& sptr) {
	_imp = sptr;
}

Php::Value Pcre8::getPreg() const
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

 Php::Value
 Pcre8::matchAll(Php::Parameters& param)
 {
    UStr8* u8 = nullptr;;
    Pcre8_matchAll  matchSet;
    svx::string_view target;

    bool checked = true;
    if (param.size() < 1) {
        checked = false;
    }

    if (checked) {
        Php::Value& v = param[0];
        if (v.isObject()) {
            u8 = UStr8::get_UStr8(v);
            if (u8 != nullptr) {
                target = u8->fn_getView();
            }
        }
        else if (v.isString())
        {
            target = svx::string_view(v, v.size());
        }

        checked = target.size() > 0;
    }
    if (!checked) {
        throw Php::Exception("Empty target string argument");
    }
    auto sre = _imp.get();
    //Php::out << "Target " << target << std::endl;
    auto rct = sre->doMatchAll(target, matchSet);
    Php::Array result;
    if (rct > 0) {
        for(int i = 0; i < rct; i++) {
            Recap8* cap = new Recap8();
            cap->_match = std::move(matchSet[i]);
            result[i] = Php::Object(Recap8::PHP_NAME, cap);
        }
    }
    return result;
 }


 Php::Value
 Pcre8::match(Php::Parameters& param)
 {
    svx::string_view target;
    bool check = param.size() >= 1;
    if (check) {
        Php::Value& v = param[0];

        if (v.isString()) {
            target = svx::string_view(v, v.size());
        }
        else if (v.isObject())
        {
            UStr8* u8 = UStr8::get_UStr8(v);
            if (u8 != nullptr)
            {
                target = u8->fn_getView();
            }
        }
        check = target.size() > 0;
    }
    if (!check) {
        throw Php::Exception("Need non-empty string or UStr8 for 1st argument");
    }

    Php::Value result;
    Recap8* cap = nullptr;

    if (param.size() >= 2) {
        Php::Value& v = param[1];
        if (v.isObject()) {
            cap = Recap8::get_Recap8(v);
            if (cap != nullptr) {
                result = v;
            }
        }
    }
    if (cap == nullptr) {
        cap = new Recap8();
        result = Php::Object(Recap8::PHP_NAME, cap);
    }
    Pcre8_match matches;
    auto pimp = _imp.get();
    pimp->doMatch(target, matches);
    cap->_match = std::move(matches);
    return result;
 }
