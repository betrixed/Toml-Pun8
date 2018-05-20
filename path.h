#ifndef _H_PATH
#define _H_PATH

#ifndef _H_PUN_TYPE
#include "puntype.h"
#endif

#ifndef _H_TEXT
#include "text.h"
#endif
/*!
    Little class to handle directory paths
    which may or may not be wanted to end in the system file path
    convention for directory separator.
*/

namespace pun {
class Path : public Php::Base {
protected:
	static std::string OSPathSep;
	static std::string OtherSep;
public:
    static void setup_ext(Php::Extension& ext);
    static const char* PHP_NAME;

    //! return native system directory separator
	static Php::Value sep();
	//! String or UStr8 arguments fullstring, test start
    static Php::Value startsWith(Php::Parameters& param);
    //! String or UStr8 arguments: fullstring, test end
    static Php::Value endsWith(Php::Parameters& param);
    //! rectify slash direction
	static Php::Value native(Php::Parameters& param);
	//! ensure system directory separator at end
	static Php::Value endSep(Php::Parameters& param);
	//! ensure no system directory separator at end
	static Php::Value noEndSep(Php::Parameters& param);
protected:
    //! return true if a changed value placed in second argument result, else use first argument
	static bool fn_native(svx::string_view& src, std::string& result);
};

};

#endif
