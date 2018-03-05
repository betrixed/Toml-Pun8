#ifndef _H_TOMLBASE
#define _H_TOMLBASE

#include <phpcpp.h>


class TomlTag {
public:
	std::string _part;
	bool		_isAOT;
	bool		_objAOT;
	bool		_implicit;
};

class TomlBase : public Php::Base {
private:
	TomlTag 			_tag;
public:
	TomlTag& tomlTag() { return _tag; }
	virtual int fn_endIndex() { return 0; }
};

#endif
