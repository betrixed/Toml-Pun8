#ifndef _H_PARAMETER
#define _H_PARAMETER

#ifndef PHPCPP_H
#include <phpcpp.h>
#endif

#ifndef _H_PCRE8_IMP
#include "pcre8_imp.h"
#endif


#include <ostream>

class Re8map;
class Recap8;
class Pcre8;
class Token8;
class KeyTable;
class ValueList;
class Token8Stream;


struct CPunk {
     static const std::string keytable_classname;
     static const std::string valuelist_classname;
     static const std::string datetime_classname;
};

namespace pun {

     enum Type {
          Null,
          Bool,
          Integer,
          Float,
          String,
          DateTime,
          ValueList,
          KeyTable,
          Array,
          Object,
          Resource,
          Reference,
          Undefined,
     };

     Type getPunType(Php::Value& val);

     Type getPunType(Php::Type t);
     const char* getPunTName(Type t);

     void hexUniStr8(const std::string& hexval, std::ostream& os);
	 Token8* check_Token8(Php::Parameters& params, unsigned int offset=0);
	 bool option_Array(Php::Parameters& params, unsigned int offset=0);
     Re8map* check_Re8map(Php::Parameters& params, unsigned int offset=0);
     Recap8* option_Recap8(Php::Parameters& params, unsigned int offset=0);
     const char* getTypeName(Php::Type);
     Pcre8* check_Pcre8(Php::Parameters& params,unsigned int offset=0);
     int check_IntString(Php::Parameters& params);
     void need_Value(Php::Parameters& params,unsigned int offset=0);

     bool check_String(Php::Parameters& params,unsigned int offset = 0);
     int check_Int(Php::Parameters& params,unsigned int offset = 0);
     bool option_Int(Php::Parameters& params,unsigned int offset);
     std::string missingParameter(const char* shouldBe, unsigned int offset);
     std::string invalidCharacter(char32_t unc8);
	

};



#endif