#ifndef _H_PARAMETER
#define _H_PARAMETER

#ifndef PHPCPP_H
#include <phpcpp.h>
#endif


#ifdef __has_include
# if __has_include(<string_view>)
#  include <string_view>
#  if __cplusplus >= 201703L || defined(_LIBCPP_STRING_VIEW)
#   define ABSL_HAVE_STD_STRING_VIEW 1
#  endif
# endif
#endif

#ifndef ABSL_HAVE_STD_STRING_VIEW 
#    include <experimental/string_view>
          namespace svx = std::experimental;
#else
     namespace svx = std;
#endif

#include <ostream>



struct CPunk {
     static const std::string keytable_classname;
     static const std::string valuelist_classname;
     static const std::string datetime_classname;
};

namespace pun {

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
     Recap8* option_Recap8(Php::Parameters& params, unsigned int offset=0);

     void hexUniStr8(svx::string_view hexval, std::ostream& os);
	
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
};



#endif