#ifndef _H_TOMLBASE
#define _H_TOMLBASE

#include <phpcpp.h>

#include "pcre8_imp.h"

namespace pun {

/*!
    PathTag has no declared methods accessible by script.
    It is used by internal TOML parsing.
*/
class PathTag : public Php::Base{
public:
	static const char* PHP_NAME;
	bool		_objAOT;  // ValueList for Array of Tables
	bool		_implicit; // Implicity created by Toml Path, not yet used as KeyTable

	Php::Value __toString();

	PathTag() : _objAOT(false), _implicit(true) {

	}
};

/*!
    A base class for ValueList and KeyTable, for the Tagable interface
*/
class TomlBase : public Php::Base {
protected:
	Php::Value 			_tag;
public:

	static const char* PHP_NAME;

	static void setup_ext(Php::Extension& ext);

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

/*!
Temporary created during Table path parse
*/

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
