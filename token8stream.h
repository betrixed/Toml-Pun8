#ifndef _H_TOKEN_STREAM
#define _H_TOKEN_STREAM


#include <unordered_map>
#include <vector>

#include "pun8.h"

// integrated functions of Pun8 class
// external interface based on PHP version
// 
#include "token8.h"



class Token8Stream : public Php::Base {
public:
    static const char* PHP_NAME;

    Token8Stream();
    ~Token8Stream();

    // set means of various integer token ids
    // End of Stream/String
    void setEOSId(Php::Parameters& params);

    // Newline
    void setEOLId(Php::Parameters& params);
    // Not a mapped id, not a singles id
    void setUnknownId(Php::Parameters& params);

    // Set ordered array of integers
    void setExpSet(Php::Parameters& params);
    // Get ordered array of integer map Ids
    
    Php::Value getExpSet();

    // set array of chr => integer
    void setSingles(Php::Parameters& params);

    // set sharable expression map object
    void setRe8map(Php::Parameters& params);

    // set input string for processing
    void setInput(Php::Parameters& params);

    // EOS not yet encountered 
    Php::Value hasPendingTokens() const;

    // Pass a Token8, get Id back, values in Token
    Php::Value getToken(Php::Parameters& params) const;

    // get rest of line for debugging
    
    Php::Value beforeEOL();

    Php::Value getOffset() const;



    // Get source line number
    Php::Value getLine() const;

    // get current expression capture, or string chr
    Php::Value getValue() const;

    // get Id
    Php::Value getId() const;

    // first argument is Token8 - return same object with values
    Php::Value peekToken(Php::Parameters& params);

    // first argument is Token8, after a call to peekToken
    // advance the stream with byteoffset implied by Token
    void	   acceptToken(Php::Parameters& params);
    Php::Value moveNextId();

    // move the distance or not of a individual PCRE8
    Php::Value moveRegex(Php::Parameters& params);

    // move the distance or not of mapped PCRE8 id
    Php::Value moveRegId(Php::Parameters& params);

private:
	void			checkLineFeed(Token8* token);
    unsigned int    offsetToChar(char32_t stopChar);

	Pun8			_input;
    Token8          _token;

    Pcre8_match		_caps;

    SingleMap		_singles;

    int    _unknownId;
    int    _eolId;
    int    _eosId;
    unsigned int    _tokenLine;
    bool            _flagLF;
};

#endif