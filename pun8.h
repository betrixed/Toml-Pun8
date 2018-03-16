#ifndef _H_PUN8
#define _H_PUN8

#include <phpcpp.h>
#include "pcre8_imp.h"

#include "tomlbase.h"

namespace pun {
// look ahead
    class Pcre8;
    class Re8map;
    class Recap8;
    class Token8;

    // this requires at least C++11 , to mandate std::string contiguous storage and data(), c_str() equivalance.
    class Pun8 : public TomlBase {
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

        Php::Value getValue() const;

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
        Php::Value matchMapId(Php::Parameters& params);

        // Match to a Pcre8 object
        Php::Value matchIdRex8(Php::Parameters& params);

        // Get current byte offset into string
        Php::Value getOffset() const;

        // set the current byte offset. 
        void setOffset(Php::Parameters& params);

        // add increment to the current byte offset
        void addOffset(Php::Parameters& params);

        // size of character buffer
        Php::Value  size() const { return (int) _mystr.size(); }
        
        // Get current artifical end position, within actual end.
        Php::Value  getRangeEnd() const { return (int) _size; }

        // Set artifical end string position, up to actual size.
        void setRangeEnd(Php::Parameters& params);

        // Fetch next character, update "Code" value and move the byte offset
        Php::Value nextChar();

        // Fetch next character, update "Code" value but do not change the byte offset
        Php::Value peekChar();

        // list of map ids to iterate
        void setIdList(Php::Parameters& params);

        // return array of ordered map id's
        Php::Value getIdList(); 

        // return BOM as string for UTF16 on this platform
        Php::Value bomUTF16();

        // return BOM as string for UTF8 
        Php::Value bomUTF8();

        // Return PHP string converted to platform UTF16, no BOM
        Php::Value asUTF16();

        Php::Value getTag() const;
        void setTag(Php::Parameters& param);

        Php::Value __toString();

        // See if String holds a BOM, return a "BOM-name" or empty string
        Php::Value getBOMId();

        // convert if none UTF8 contents to UTF8. 
        // Return false if cannot convert
        // Regular expression and traversal functions only work with UTF8.
        // Return an offset to first none BOM, if a UTF8 BOM exists.
        Php::Value ensureUTF8(); 

        // erase block of characters, given start offset, and length.
        // packs string, and moves up characters to fill in missing block
        // Resets offset to 0, and rangeEnd to string size
        void erase(Php::Parameters& param);
    public:
        int fn_peekChar();
        int fn_firstMatch(Pcre8_match& matches);
        int fn_matchRegId(int id, Pcre8_match& matches);
        void fn_copyIdList(Php::Value& v);
        void fn_setString(const char* ptr, unsigned int len);
        void fn_setString(std::string &&m);
        void fn_setString(const std::string& s);
        std::string fn_beforeChar(char32_t c) const;
        const std::string& fn_getValue() const;

        void fn_setRe8map(Re8map_share& smap);
        std::string& str();

    private:

        friend class Token8Stream;

        /* get byte size and code value of next character */

        std::string      _mystr;
        unsigned int    _index;
        unsigned int    _size;
        char32_t        _myChar;
        Re8map_share    _remap;
        IdList          _idlist;  // current id list for first match

        // This returns the id in Pcre8_share and result, or 0
        int matchSP(Pcre8_share& sp, Pcre8_match& result);
    };
};

#endif
