#ifndef _H_RECAP8
#define _H_RECAP8

#include <phpcpp.h>
#include "pcre8_imp.h"


namespace pun {
//! Php object represents the captures as string list, returned by regular expression match

class Recap8 : public Php::Base, public Php::ArrayAccess, public Php::Countable {
public:
    static const char* PHP_NAME;
    static Recap8* get_Recap8(Php::Value& val);

    static void setup_ext(Php::Extension& ext);
    /*!
        Fetch capture value: argument int from 0 to count-1
    */
    Php::Value getCap(Php::Parameters& params) const;

    //! Number of captures stored
    virtual long count() override;
    //! Operators for PHP interface ArrayAccess, are not directly callable
    //! If this capture index exists
	virtual bool offsetExists(const Php::Value &key) override;
	//! return value for this capture index
	virtual Php::Value offsetGet(const Php::Value &key) override;
	//! Write operators for PHP interface ArrayAccess are nothing stubs
	//! This won't do anything
    virtual void offsetUnset(const Php::Value &key) override {}
    //! This won't do anything
	virtual void offsetSet(const Php::Value &key, const Php::Value &value) override {}
public:
    Pcre8_match	_match;
};

};

#endif
