#ifndef _H_PUN8
#define _H_PUN8

#include <phpcpp.h>
#include "pcre8_imp.h"

// look ahead
class Pcre8;
class Re8map;
class Recap8;

class Pun8 : public Php::Base {
public:
    static const char* PHP_NAME;

	Pun8();
	~Pun8();

    // Install PCRE2, Return integer id, the first parameter.
	Php::Value setIdRex(Php::Parameters& params);

     // Return Pcre8 object by Id
    Php::Value getIdRex(Php::Parameters& params);

    // Get array of integer id's in map
	Php::Value getIds() const;

   

    // Same parameters as setString
    void __construct(Php::Parameters& params);

    // Set the string for iteration
    void setString(Php::Parameters& params);

    // Return 32 unicode of last nextChar step
    Php::Value getCode() const;

    // use the PCRE2 map, shared with Re8map object
    // Note their is no getRe8map
    void setRe8map(Php::Parameters& params);

    Php::Value firstMatch(Php::Parameters& params);
    // Return matches array from a PCRE2 match
    // starting from current offsetm using map key
    Php::Value matchIdRex(Php::Parameters& params);

    // Match to a Pcre8 object
    Php::Value matchPcre8(Php::Parameters& params);

    // Get current byte offset into string
    Php::Value getOffset() const;

    // set the current byte offset. 
    void setOffset(Php::Parameters& params);

    // add increment to the current byte offset
    void addOffset(Php::Parameters& params);

    // move the byte offset to next character
    Php::Value nextChar();

    static bool option_Array(Php::Parameters& params, unsigned int offset=0);
    static Re8map* check_Re8map(Php::Parameters& params, unsigned int offset=0);
    static Recap8* option_Recap8(Php::Parameters& params, unsigned int offset=0);
    static Pcre8* check_Pcre8(Php::Parameters& params,unsigned int offset=0);
    static int check_IntString(Php::Parameters& params);
    static bool check_String(Php::Parameters& params,unsigned int offset = 0);
    static int check_Int(Php::Parameters& params,unsigned int offset = 0);
    static bool option_Int(Php::Parameters& params,unsigned int offset);
    static std::string missingParameter(const char* shouldBe, unsigned int offset);

    
private:
    Php::Value      _mystr;
    unsigned int    _index;
    unsigned int    _size;
    char32_t        _myChar;
    Re8map_share    _remap;
    bool matchSP(Pcre8_share& sp, Pcre8_match& result);
};

#endif
