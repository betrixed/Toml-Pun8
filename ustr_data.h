#ifndef _H_USTR_DATA
#define _H_USTR_DATA

#ifndef _H_TEXT
#include "text.h"
#endif

#include <memory>

struct UStrData {
	std::string 	 _data;
	svx::string_view _view;

	// return true if found a BOM
	void ensureUTF8();
	void assign(const char* p, uint64_t slen);
	void assign(std::string&& rh);
    void reset_view();

	char32_t fetch(uint64_t offset, svx::string_view& v);

	std::string asUTF16();
	std::string getBOMId();

	static std::string bomUTF16();
	static std::string bomUTF8();

	// start offset and length
	svx::string_view substr(uint64_t begin, uint64_t len);
};

typedef std::shared_ptr<UStrData> Str_ptr;


#endif
