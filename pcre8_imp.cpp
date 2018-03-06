#include "pcre8_imp.h"

#ifndef PHPCPP_H
#include <phpcpp.h>
#endif

#include <sstream>
#include <utility>
//#include <ostream>

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
Pcre8_imp::doMatch(const unsigned char* start, unsigned int slen, Pcre8_match& matches)
{
  auto match_data = pcre2_match_data_create_from_pattern(_re, nullptr);

  auto rcount = pcre2_match(_re, start, slen, 0, 0, match_data, nullptr);

  matches._slist.clear();
  matches._rcode = rcount;

  if (rcount <= 0) {
      //std::string test(reinterpret_cast<const char*>(start), slen);
      pcre2_match_data_free(match_data);
      return rcount;
  }

  auto ovector = pcre2_get_ovector_pointer(match_data);

  if (ovector[0] > ovector[1]) {
    // can't handle this at all
    pcre2_match_data_free(match_data);
    return -1;
  }

  for(int i = 0; i < rcount; i++)
  {
    auto offset = 2*i;
    auto substart = reinterpret_cast<const char*>(start + ovector[offset]);
    size_t sublen = ovector[offset+1] - ovector[offset];
    matches._slist.push_back(std::move(std::string(substart, sublen)));
  }
  pcre2_match_data_free(match_data);
  return rcount;
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



