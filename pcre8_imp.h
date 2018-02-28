#ifndef _H_PCRE8_IMP
#define _H_PCRE8_IMP

#define PCRE2_CODE_UNIT_WIDTH 8
#include <pcre2.h>
#include <unordered_map>
#include <vector>
#include <memory>

typedef std::vector<std::string> StringList;

class Pcre8_match {
public:
    StringList  _slist;
    int         _rcode;
    Pcre8_match();
    
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

class Pcre8_map {
public:
	RexMap	_map;

    bool hasKey(int index) const;
	void setRex(Pcre8_share& re);
	bool getRex(int index, Pcre8_share& re) const;
	int  eraseRex(int index);
};

typedef std::shared_ptr<Pcre8_map> Re8map_share;

#endif