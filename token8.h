#ifndef _H_TOKEN
#define _H_TOKEN

#include <phpcpp.h>
#include "parameter.h"


namespace pun {

    class Token8: public Php::Base {
    public:
        static const char* PHP_NAME;

        Token8();

    	Token8(const Token8 &m);
        Token8 &operator=(const Token8 &m);

        Token8(const Token8&& m);
        Token8 &operator=(const Token8 &&m);


        Php::Value getValue() const;
        Php::Value getId() const;
        Php::Value getLine() const;
        Php::Value isSingle() const;

    	svx::string_view _value; // view of original source string
    	int				 _id;
    	int    			 _line;
    	bool			 _isSingle;
    };

};
#endif
