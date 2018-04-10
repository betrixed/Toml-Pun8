#ifndef _H_PARAMETER
#define _H_PARAMETER

#ifndef PHPCPP_H
#include <phpcpp.h>
#endif

#ifndef _H_TEXT
#include "text.h"
#endif

#include <ostream>
#include <map>

typedef std::map<std::string, Php::Value> ValueMap;

struct CPunk {
     static const std::string keytable_classname;
     static const std::string valuelist_classname;
     static const std::string datetime_classname;
};

namespace pun {

     class KeyTable;
     class Re8map;
     class Recap8;
     class Token8;
     class Pcre8;

     enum Pype {
          tNull,
          tBool,
          tInteger,
          tFloat,
          tString,
          tDateTime,
          tValueList,
          tKeyTable,
          tArray,
          tObject,
          tResource,
          tReference,
          tUndefined,
     };

     Pype getPype(Php::Value& val);

     Pype getPype(Php::Type t);
     const char* getPypeId(Pype t);


     Pcre8* check_Pcre8(Php::Parameters& params,unsigned int offset=0);
     Token8* check_Token8(Php::Parameters& params, unsigned int offset=0);
     Re8map* check_Re8map(Php::Parameters& params, unsigned int offset=0);
     KeyTable* check_KeyTable(Php::Parameters& params, unsigned int offset=0);

     KeyTable* castKeyTable(Php::Value& val);

     Recap8* option_Recap8(Php::Parameters& params, unsigned int offset=0);

     std::string hexUniStr8(const svx::string_view& hexval);
     void hexUniStr8(const svx::string_view& hexval, std::ostream& os);

	bool option_Array(Php::Parameters& params, unsigned int offset=0);

     const char* getTypeName(Php::Type);

     int check_IntString(Php::Parameters& params);
     void need_Value(Php::Parameters& params,unsigned int offset=0);

     bool check_String(Php::Parameters& params,unsigned int offset = 0);
     int check_Int(Php::Parameters& params,unsigned int offset = 0);
     bool option_Int(Php::Parameters& params,unsigned int offset);

     std::string missingParameter(const char* shouldBe, unsigned int offset);
     std::string invalidCharacter(char32_t unc8);

     void serialize_keyTable(Php::Base* base, std::ostream& out);
     void serialize_valueList(Php::Base* base, std::ostream& out);

     void serialize_str(const char* s, size_t slen, std::ostream& out);
     void unserialize_str(std::string& cval, std::istream& ins);

     void serialize(Php::Value& val, std::ostream& out);
     void unserialize(Php::Value& val, std::istream& ins);

     Php::Array to_array(const ValueMap &vmap);
};



#endif
