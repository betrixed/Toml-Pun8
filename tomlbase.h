#ifndef _H_TOMLBASE
#define _H_TOMLBASE

#include <phpcpp.h>


class TomlTag {
public:
	//std::string _part;
	//bool		_isAOT;
	bool		_objAOT;  // ValueList for Array of Tables
	bool		_implicit; // Implicity created by Toml Path, not yset used as KeyTable
};


typedef std::map<std::string, Php::Value> ValueMap;
typedef std::vector<Php::Value> ValueArray;

class TomlBase : public Php::Base {
private:
	TomlTag 			_tag;
public:
	TomlTag& tomlTag() { return _tag; }
	virtual int fn_endIndex() { return 0; }
};

// Temporary created during Table path parse

class TomlPartTag {
public:
	std::string _part;
	TomlBase* 	_base;
	bool		_isAOT;

	TomlPartTag(bool isAOT) : _base(nullptr), _isAOT(isAOT)
	{
	}
};

#endif
