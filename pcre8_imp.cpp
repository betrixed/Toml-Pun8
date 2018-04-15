#include "pcre8_imp.h"

#ifndef PHPCPP_H
#include <phpcpp.h>
#endif

#include <sstream>
#include <utility>
//#include <ostream>


//! Handle  pcre2 calls for first and all matches, free _matchData
class Pcre8_calls {
public:
    Pcre8_calls(pcre2_code* re) : _re(re), _matchData(nullptr), _ovector(nullptr) {

    }
    ~Pcre8_calls() {
        if (_matchData) {
            pcre2_match_data_free(_matchData);
            // _ovector is part of _matchData
            _ovector = nullptr;
            _matchData = nullptr;
        }
    }

    int doMatch(const svx::string_view &subject, Pcre8_match& matches)
    {
        if (_matchData == nullptr) {
            _matchData = pcre2_match_data_create_from_pattern(_re, nullptr);
        }
        auto start = (const unsigned char*) subject.data();
        auto slen = subject.size();

        auto rcount = pcre2_match(_re, start, slen, 0, 0, _matchData, nullptr);

        matches._slist.clear();

        if (rcount <= 0) {
            //std::string test(reinterpret_cast<const char*>(start), slen);
            return rcount;
        }

        if (_ovector == nullptr) {
            _ovector = pcre2_get_ovector_pointer(_matchData);
        }

        if (_ovector[0] > _ovector[1]) {
            return -1;
        }
        addMatchData( matches, start, rcount );
        return rcount;
    }

    int doMatchAll(const svx::string_view &seg, Pcre8_matchAll& matchSet)
    {
        uint32_t option_bits;
        uint32_t newline;
        // make 1 match results
        matchSet.resize(1);

        auto mcount = doMatch(seg, matchSet[0]);

        // follow the pcre2 demo exactly as possible
        if (mcount <= 0) {
            return 0;
        }
        pcre2_pattern_info(_re,PCRE2_INFO_ALLOPTIONS, &option_bits );

        bool isUTF8 = (option_bits & PCRE2_UTF) != 0;

        pcre2_pattern_info(_re,PCRE2_INFO_NEWLINE, &newline );
        bool crlf_is_newline = (newline == PCRE2_NEWLINE_ANY ||
                  newline == PCRE2_NEWLINE_CRLF ||
                  newline == PCRE2_NEWLINE_ANYCRLF);

        for(;;) {
            uint32_t options = 0;
            auto subject_length = seg.size();
            auto subject = (const unsigned char*) seg.data();

            PCRE2_SIZE start_offset = _ovector[1]; // end of previous match

            if (_ovector[0] == _ovector[1]) {
                if (_ovector[0] == subject_length)
                    break;
                options = PCRE2_NOTEMPTY_ATSTART | PCRE2_ANCHORED;
            }
            else {
                PCRE2_SIZE startchar = pcre2_get_startchar(_matchData);
                if (startchar >= subject_length)
                    break;
                start_offset = startchar + 1;
                if (isUTF8) {
                    for( ; start_offset < subject_length; start_offset++){
                        if ((subject[start_offset] & 0xC0) != 0x80)
                            break;
                    }
                }
            }

            // next match
            auto rcount = pcre2_match(
                _re, subject, subject_length,
                start_offset,
                options,
                _matchData,
                nullptr
            );

            if (rcount == PCRE2_ERROR_NOMATCH) {
                if (options == 0)
                    break; // all matches found
                _ovector[1] = start_offset + 1;
                if (crlf_is_newline && (start_offset < subject_length - 1) &&
                    subject[start_offset] == '\r' &&
                    subject[start_offset+1] == '\n')
                    _ovector[1] += 1;
                else if (isUTF8) {
                    while(_ovector[1] < subject_length) {
                        if ((subject[start_offset] & 0xC0) != 0x80)
                            break;
                        _ovector[1] += 1;
                    }
                }
                continue;
            }

            if (rcount <= 0) {
                //not recoverable
                return rcount;
            }

            if (_ovector[0] > _ovector[1]) {
                // see pcre2demo, "\\K was used in an assertion to set the match start after its end."
                return -1;
            }

            auto matchCount = matchSet.size();
            matchSet.resize(matchCount + 1);
            Pcre8_match& match = matchSet[matchCount];
            match._rcode = rcount;
            addMatchData(match, subject, rcount);
        }
        return matchSet.size();
    }
private:
    //! start: match subject, rcount: number of matches
    void addMatchData(Pcre8_match& matches, const unsigned char* start, int rcount) {
        for(int i = 0; i < rcount; i++)
        {
            auto offset = 2*i;
            auto substart = reinterpret_cast<const char*>(start + _ovector[offset]);
            size_t sublen = _ovector[offset+1] - _ovector[offset];
            matches._slist.push_back(std::move(std::string(substart, sublen)));
        }
    }
    // this is burrowed.
    pcre2_code* _re;
    // these are managed
    pcre2_match_data* _matchData;
    PCRE2_SIZE* _ovector;

};


IdList toIdList(const Php::Value& v) {

    IdList result;
    auto ct = v.size();
    result.reserve(ct);
    for(int i = 0; i < v.size(); i++)
    {
        result.push_back(v[i]);
    }
    return result;
}

void Pcre8_map::setRex(const Pcre8_share& reg)
{
    auto index = reg.get()->_id;
    auto pit = _map.find(index);
    if (pit != _map.end())
    {   // replace, delete old
        pit->second = reg;
    }
    else {

        _map.insert(std::pair<int, Pcre8_share>(index,reg));
    }
}

bool Pcre8_map::hasKey(int index) const
{
  auto pit = _map.find(index);
  return (pit != _map.end());
}

int
Pcre8_map::firstMatch(
    const svx::string_view& sv,
    const IdList& ids,
    Pcre8_match& matches)
{
    int result = 0;
    auto idend = ids.end();
    for (auto mid = ids.begin(); mid != idend; mid++) {
        result = this->match(sv,(*mid), matches);
        if (result > 0)
           break;
    }
    return result;
}

int Pcre8_map::match(const svx::string_view& sv,
                int mapId,
                Pcre8_match& matches)
{
    Pcre8_share reg;

    if (getRex(mapId, reg)) {
        return reg.get()->match(sv, matches);
    }
    return 0;
}

bool Pcre8_map::getRex(int index, Pcre8_share& reg) const
{
    auto pit = _map.find(index);
    if (pit != _map.end())
    {
        reg = pit->second;
        return true;
    }
    else
        return false;
}

int Pcre8_map::eraseRex(int index)
{
    return _map.erase(index);
}

Pcre8_imp::Pcre8_imp() : _re(nullptr), _id(0)
{
}

Pcre8_imp::~Pcre8_imp()
{
	(*this).setRex(nullptr);
}

void Pcre8_imp::setRex(pcre2_code* re) {
 	if (_re != nullptr) {
    //Php::out << "Freed Re" << std::endl;
		pcre2_code_free(_re);
	}
	_re = re;
}

bool Pcre8_imp::compile(std::string& _error)
{
	char const* sptr = _eStr.data();
 	auto pattern = reinterpret_cast<const unsigned char*>(sptr);
 	PCRE2_SIZE psize = _eStr.size();
 	int errornumber;
 	PCRE2_SIZE erroroffset;

 	_re = pcre2_compile(
  		pattern,
  		psize,
  		0,                     /* default options */
  		&errornumber,          /* for error number */
  		&erroroffset,          /* for error offset */
  		0
  		);

  	if (_re == nullptr) {
  		std::stringstream ss;
  		PCRE2_UCHAR buffer[256];

  		pcre2_get_error_message(errornumber, buffer, sizeof(buffer));
  		ss << "PCRE2 compilation failed at offset "
  				<< (int)erroroffset << std::endl;
  		ss << buffer << std::endl;

  		_error = ss.str();
  		return false;
  	}
  	else {
  		_error.clear();
  		return true;
  	}
}



int
Pcre8_imp::match(const svx::string_view& sv, Pcre8_match& matches)
{
    int rct;
    auto slen = sv.size();
    if (slen > 0) {
        Pcre8_calls  pre_call(_re);

        rct = pre_call.doMatch(sv, matches);

        if (rct > 0) {
            // _rcode to hold match mapId
            //Php::out << "Matched " << pimp->_id << std::endl;
            matches._rcode = this->_id;
            return matches._rcode;
        }
    }
    matches._rcode = 0;
    return 0;
}

int
Pcre8_imp::doMatchAll(const svx::string_view& sv, Pcre8_matchAll& matchSet)
{
    Pcre8_calls pre_call(_re);

    return pre_call.doMatchAll(sv, matchSet);
}

int
Pcre8_imp::doMatch(const svx::string_view& sv, Pcre8_match& matches)
{
    Pcre8_calls pre_call(_re);

    return pre_call.doMatch(sv, matches);
}

Pcre8_match::Pcre8_match() : _rcode(0)
{
}

Pcre8_match::Pcre8_match(const Pcre8_match &m)
  : _slist(m._slist), _rcode(m._rcode)
{
}

Pcre8_match &
Pcre8_match::operator=(const Pcre8_match &m)
{
  if (this != &m) {
    _rcode = m._rcode;
    _slist = m._slist;
  }
  return *this;
}

Pcre8_match::Pcre8_match(const Pcre8_match &&m)
  : _slist(std::move(m._slist)), _rcode(std::move(m._rcode))
{

}

Pcre8_match &
Pcre8_match::operator=(const Pcre8_match &&m)
{
   _slist = std::move(m._slist);
   _rcode = std::move(m._rcode);
   return *this;
}


Pcre8_share
pun::makeSharedRe(int mapId, const char* estr, unsigned int slen)
{
    Pcre8_share sp = std::make_shared<Pcre8_imp>();
    auto pimp = sp.get();
    pimp->_eStr.assign(estr,slen);
    pimp->_id = mapId;

    std::string errorMsg;

    if (!pimp->compile(errorMsg)) {
        throw Php::Exception(errorMsg);
    }
    return sp;
}


bool CharMap::hasKey(char32_t ix) const
{
  auto pit = _map.find(ix);
  return (pit != _map.end());
}

void CharMap::setKV(char32_t ix,  int tokenId)
{
    auto pit = _map.find(ix);
    if (pit != _map.end())
    {   // replace, delete old
        pit->second = tokenId;
    }
    else {
        _map.insert(std::pair<char32_t, int>(ix,tokenId));
    }
}

int  CharMap::getV(char32_t ix) const
{
    auto pit = _map.find(ix);
    if (pit != _map.end())
    {
         //Php::out << "Single " << (unsigned int) ix << " found " << pit->second << std::endl;
         return pit->second;
    }
    //Php::out << "Single " << (unsigned int) ix << " Not found " << std::endl;
    return 0;
}

int  CharMap::eraseV(char32_t ix)
{
  return _map.erase(ix);
}



