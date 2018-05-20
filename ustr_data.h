#ifndef _H_USTR_DATA
#define _H_USTR_DATA

#ifndef _H_TEXT
#include "text.h"
#endif

#include <memory>

/*!
    UStrData is meant to be a read-only
    segment of text, enforced to be UTF-8 if necessary.
    It may or may not have a BOM at the beginning, if it was read from a file.
    The string_view component is meant to exclude a BOM, or restrict range,
    for common operations, such as fetch.
*/
struct UStrData {
    //! Actual data
	std::string 	 _data;
	//! Adjusted view
	svx::string_view _view;

	//! make it UTF-8 if necessary, hide bom if any, from _view
	void ensureUTF8();
    //! setup data and view
	void assign(const char* p, uint64_t slen);
	//! setup data and view
	void assign(std::string&& rh);
	//! Set _view to entire _data
    void reset_view();
    //! Fetch UTF-8 character, as UTF-32, and as multibyte range in string_view.
	char32_t fetch(uint64_t offset, svx::string_view& v);

	//! return UTF-16 in std::string
	std::string asUTF16();
	//! return BOM string, if any
	std::string getBOMId();

	//! return native UTF-16 BOM string
	static  std::string bomUTF16();
	//! return native UTF-8  BOM string
	static  std::string bomUTF8();

	//! Substring as string_view
	svx::string_view substr(uint64_t begin, uint64_t len);
};

typedef std::shared_ptr<UStrData> Str_ptr;


#endif
