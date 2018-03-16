#ifndef _H_PCRE8
#define _H_PCRE8

#include <phpcpp.h>
#include "pcre8_imp.h"

// Php object wrap a single PCRE2

namespace pun {
    class Pcre8 : public Php::Base {
    public:
        static const char* PHP_NAME;

    	Pcre8();
        ~Pcre8();
        void __construct(Php::Parameters& params);
        void setIdString(Php::Parameters& params);
        
        void setImp(const Pcre8_share& sptr);
        const Pcre8_share& getImp() const { return _imp; }

        Pcre8_share static fromParameters(Php::Parameters& params);
        
        Php::Value match(Php::Parameters& params);
        Php::Value getString() const;
        Php::Value getId() const;
        Php::Value isCompiled() const;
    private:
        Pcre8_share  _imp;
    };

};

#endif