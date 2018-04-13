#ifndef _H_RECAP8
#define _H_RECAP8

#include <phpcpp.h>
#include "pcre8_imp.h"

// Php object wrap a single PCRE2

namespace pun {

class Recap8 : public Php::Base {
public:
    static const char* PHP_NAME;
    static Recap8* get_Recap8(Php::Value& val);

    Php::Value count() const;
    Php::Value getCap(Php::Parameters& params) const;
public:
    Pcre8_match	_match;
};

};

#endif
