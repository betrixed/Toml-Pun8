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

namespace pun {
	std::string uncamelize(const std::string& s, char sep);
	StringList explode(const std::string& sep, const std::string& toSplit);
	std::string str_replace(
				const std::string& from,
				const std::string& to,
				const std::string& subject);

};

#endif