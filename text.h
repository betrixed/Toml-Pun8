#ifndef _H_TEXT
#define _H_TEXT

#include <string>
#include <vector>

typedef std::vector<std::string> StringList;

typedef int64_t intp_t; // PHP on 64 bit integers

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

#include <phpcpp.h>

#ifndef _H_PUN_TYPE
#include "puntype.h"
#endif

namespace pun {
    //! Tries to do thing as PHP version.
	std::string uncamelize(const std::string& s, char sep);
	//! Tries to do thing as PHP version.
	StringList explode(const std::string& sep, const std::string& toSplit);
	//! Tries to do thing as PHP version.
	std::string str_replace(
				const std::string& from,
				const std::string& to,
				const std::string& subject);

    /*! Return the number of replacements.
        If return zero, the result is unchanged
        else result contains new string with replacements
        Argument src can be view of original result, or any other string.
    */
    unsigned int replaceAll(svx::string_view src,
                                const svx::string_view& out,
                                const svx::string_view& in,
                                std::string& result);

    void replaceVar_object(Php::Value& obj, Php::Value& lookup);

    bool replaceVar_str(const svx::string_view& src,
                                std::string& result,
                                Php::Value& lookup);

    void replaceVar_ValueArray(ValueArray& items, Php::Value& lookup);

    void replaceVar_ValueMap(ValueMap& items, Php::Value& lookup);

};

#endif
