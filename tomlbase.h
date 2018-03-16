#ifndef _H_TOMLBASE
#define _H_TOMLBASE

#include <phpcpp.h>

namespace pun {

class PathTag : public Php::Base{
public:
	static const char* PHP_NAME;
	bool		_objAOT;  // ValueList for Array of Tables
	bool		_implicit; // Implicity created by Toml Path, not yet used as KeyTable

	Php::Value __toString();
	
	PathTag() : _objAOT(false), _implicit(true) {

	}
};

typedef std::map<std::string, Php::Value> ValueMap;
typedef std::vector<Php::Value> ValueArray;

class TomlBase : public Php::Base {
protected:
	Php::Value 			_tag;
public:

	static const char* PHP_NAME;
	virtual int fn_endIndex() { return 0; }
	// interface for PHP
	Php::Value getTag() const;
	void setTag(Php::Parameters& param);
public:
	void fn_setTag(Php::Value&& v) { _tag = std::move(v); }
	Php::Value&  fn_getTag() { return _tag; }
	
	PathTag*  fn_getPathTag();
	void	  fn_setPathTag(PathTag* tag);
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

}; // end namespace pun

#endif
