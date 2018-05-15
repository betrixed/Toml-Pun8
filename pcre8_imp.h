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
    StringList  _slist;
    int         _rcode;
    Pcre8_match();

    svx::string_view capture(unsigned int i) const {
        const std::string& cap = _slist[i];
        return svx::string_view(cap.data(), cap.size());
    }

    Pcre8_match(const Pcre8_match &m);
    Pcre8_match &operator=(const Pcre8_match &m);

    Pcre8_match(const Pcre8_match&& m);
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
    int         _id;
    std::string _eStr;

    Pcre8_imp();
    ~Pcre8_imp();

    pcre2_code* getRex() { return _re; }
    void setRex(pcre2_code* re);
    //! return first match,  or negative error code
    int doMatch(const svx::string_view& sv, Pcre8_match& matches);
    //! multiple matches, so there is an array of Pcre8_match
    int doMatchAll(const svx::string_view& sv, Pcre8_matchAll& matches);

    int match(const svx::string_view& sv, Pcre8_match& matches);
    bool compile(std::string& _error);
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
	RexMap	_map;

	int match(const svx::string_view& sv,
                int mapId,
                Pcre8_match& matches);
    int firstMatch(
        const svx::string_view& sv,
        const IdList& ids,
        Pcre8_match& matches);

    bool hasKey(int index) const;
	void setRex(const Pcre8_share& re);
	bool getRex(int index, Pcre8_share& re) const;
	int  eraseRex(int index);
};

/*!
    Wrap a map of a unicode char32_t key to arbitrary integer "token" id.
*/
class CharMap {
public:
    RexSingles _map;
    bool hasKey(char32_t index) const;
    void setKV(char32_t ix,  int tokenId);
    int  getV(char32_t ix) const;
    int  eraseV(char32_t ix);
};

typedef std::shared_ptr<Pcre8_map> Re8map_share;
typedef std::shared_ptr<CharMap> CharMap_sp;

namespace pun {
    Pcre8_share makeSharedRe(int mapId, const char* estr, unsigned int slen);
};

#endif
