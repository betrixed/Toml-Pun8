#ifndef _H_PCRE8_IMP
#define _H_PCRE8_IMP

#ifndef  _H_PARAMETER
#include "parameter.h"
#endif

#define PCRE2_CODE_UNIT_WIDTH 8
#include <pcre2.h>
#include <unordered_map>
#include <vector>
#include <memory>


#ifndef _H_TEXT
#include "text.h"
#endif

typedef std::vector<int>  IdList;
IdList toIdList(const Php::Value& v);

/*!
    Internal store of regular expression captures as std::vector<std::string>
*/
class Pcre8_match {
public:
    //! Vector of capture strings
    StringList  _slist;
    //! code returned by libpcre2-8, number of captures
    int         _rcode;
    Pcre8_match();
    //! return view of capture number i
    svx::string_view capture(unsigned int i) const {
        const std::string& cap = _slist[i];
        return svx::string_view(cap.data(), cap.size());
    }

    //! Copy construct
    Pcre8_match(const Pcre8_match &m);
    //! Assign
    Pcre8_match &operator=(const Pcre8_match &m);

    //! Move construct - this takes ownership of capture list
    Pcre8_match(const Pcre8_match&& m);
    //! Assign move - takes ownership
    Pcre8_match &operator=(const Pcre8_match &&m);
};

typedef std::vector<Pcre8_match> Pcre8_matchAll;


/*!
    Wrap calls to libpcre2-8, and manage the compiled regular expression
*/
class Pcre8_imp {
private:
    pcre2_code *_re;
public:
    //! Mapped token Id
    int         _id;
    //! The expression to compile, note the usual '/' for start and end expression may need to be left out.
    std::string _eStr;

    Pcre8_imp();
    ~Pcre8_imp();

    //! probably not needed, the compiled regular expression pointer
    pcre2_code* getRex() { return _re; }

    //! probably not needed
    void setRex(pcre2_code* re);
    //! return first match,  or negative error code
    int doMatch(const svx::string_view& sv, Pcre8_match& matches);
    //! multiple matches, so there is an array of Pcre8_match
    int doMatchAll(const svx::string_view& sv, Pcre8_matchAll& matches);

    //! Put expression results in string list - matches.
    int match(const svx::string_view& sv, Pcre8_match& matches);

    //! Get libpcre2-8 to compile the expression.
    bool compile(std::string& _error);

    //! A compiled expression pointer exists
    bool isCompiled() { return _re != nullptr; }
};

typedef std::shared_ptr<Pcre8_imp> Pcre8_share;

typedef std::unordered_map<int, Pcre8_share> RexMap;

typedef std::unordered_map<char32_t, int> RexSingles;


/*!
    Interface to map of shared pointers to compiled regular expressions, key
    is integer.
*/
class Pcre8_map {
public:
    //! map of Id to shared pointer
	RexMap	_map;

    //! see if the mapped PCRE id has matches, return how many.
	int match(const svx::string_view& sv,
                int mapId,
                Pcre8_match& matches);

    //! From ordered list of Ids, return results of first match.
    int firstMatch(
        const svx::string_view& sv,
        const IdList& ids,
        Pcre8_match& matches);

    //!  Mapped Id exists
    bool hasKey(int index) const;

    //! Enter the shared pointer PCRE in the map
	void setRex(const Pcre8_share& re);

	//! If mapped Id exists, set argument re to its shared pointer.
	bool getRex(int index, Pcre8_share& re) const;

	//! Remove the mapped Id / shared pointer entry
	int  eraseRex(int index);
};

/*!
    Wrap a map of a unicode char32_t key to arbitrary integer "token" id.
*/
class CharMap {
public:
    //! map std::unordered_map<char32_t, int> this class wraps
    RexSingles _map;
    //! Interrogate the map
    bool hasKey(char32_t index) const;
    //! map the character to integer value
    void setKV(char32_t ix,  int tokenId);
    //! return current mapped character integer value
    int  getV(char32_t ix) const;
    //! erase mapped character entry
    int  eraseV(char32_t ix);
};

typedef std::shared_ptr<Pcre8_map> Re8map_share;
typedef std::shared_ptr<CharMap> CharMap_sp;

namespace pun {
    Pcre8_share makeSharedRe(int mapId, const char* estr, unsigned int slen);
};

#endif
