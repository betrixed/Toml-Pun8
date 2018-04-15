#ifndef _H_RECAP8
#define _H_RECAP8

#include <phpcpp.h>
#include "pcre8_imp.h"

// Php object wrap a single PCRE2

namespace pun {

class Recap8 : public Php::Base, public Php::ArrayAccess, public Php::Countable {
public:
    static const char* PHP_NAME;
    static Recap8* get_Recap8(Php::Value& val);

    static void setup_ext(Php::Extension& ext);
    /*!
        Fetch capture value: argument int from 0 to count-1
    */
    Php::Value getCap(Php::Parameters& params) const;

    virtual long count() override;
	virtual bool offsetExists(const Php::Value &key) override;
	virtual Php::Value offsetGet(const Php::Value &key) override;
	//! Array access parameters are read only
    virtual void offsetUnset(const Php::Value &key) override {}
	virtual void offsetSet(const Php::Value &key, const Php::Value &value) override {}
public:
    Pcre8_match	_match;
};

};

#endif
