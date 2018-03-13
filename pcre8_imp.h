#ifndef _H_PCRE8_IMP
#define _H_PCRE8_IMP

#define PCRE2_CODE_UNIT_WIDTH 8
#include <pcre2.h>
#include <unordered_map>
#include <vector>
#include <memory>
#include <string_view>

typedef std::vector<std::string> StringList;
typedef std::vector<int>  IdList;

class Pcre8_match {
public:
    StringList  _slist;
    int         _rcode;
    Pcre8_match();
    
    std::string_view capture(unsigned int i) const {
        const std::string& cap = _slist[i];
        return std::string_view(cap.data(), cap.size());
    }

    Pcre8_match(const Pcre8_match &m);
    Pcre8_match &operator=(const Pcre8_match &m);

    Pcre8_match(const Pcre8_match&& m);
    Pcre8_match &operator=(const Pcre8_match &&m);
};

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
    // return number of matches, or negative error code
    int doMatch(const unsigned char* start, unsigned int slen, Pcre8_match& matches);

    bool compile(std::string& _error);
    bool isCompiled() { return _re != nullptr; }
};

typedef std::shared_ptr<Pcre8_imp> Pcre8_share;

typedef std::unordered_map<int, Pcre8_share> RexMap;

typedef std::unordered_map<char32_t, int> RexSingles;

class Pcre8_map {
public:
	RexMap	_map;

    bool hasKey(int index) const;
	void setRex(const Pcre8_share& re);
	bool getRex(int index, Pcre8_share& re) const;
	int  eraseRex(int index);
};

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