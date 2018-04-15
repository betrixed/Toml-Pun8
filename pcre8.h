#ifndef _H_PCRE8
#define _H_PCRE8

#include <phpcpp.h>
#include "pcre8_imp.h"

// Php object wrap a single PCRE2

namespace pun {
    class Pcre8 : public Php::Base {
    public:
        static const char* PHP_NAME;
        static void setup_ext(Php::Extension& ext);

    	Pcre8();
        ~Pcre8();

        //! function __construct(int id, string regex);
        void __construct(Php::Parameters& params);

        //! function setPreg(int id, string regex);
        void setPreg(Php::Parameters& params);

        Pcre8_share static fromParameters(Php::Parameters& params);

        //! function matchAll(UStr8 | String) : array of Recap8
        Php::Value matchAll(Php::Parameters& params);

        //! function match(UStr8 | String [, Recap8]) : Recap8;
        Php::Value match(Php::Parameters& params);

        //! function getPreg : string;
        Php::Value getPreg() const;

        //! function getId : int;
        Php::Value getId() const;

        //! function isCompiled : boolean;
        Php::Value isCompiled() const;

    public:
        void setImp(const Pcre8_share& sptr);
        const Pcre8_share& getImp() const { return _imp; }
    private:
        Pcre8_share  _imp;
    };

};

#endif
