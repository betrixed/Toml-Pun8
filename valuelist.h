#ifndef _H_ValueList
#define _H_ValueList

#include "tomlbase.h"

class ValueList : public TomlBase {
public:
	static const char* PHP_NAME;

	void pushBack(Php::Parameters& param);
	Php::Value getV(Php::Parameters& params) const;
	Php::Value getLast() const;
	Php::Value count() const;
	// Return the Array as stored
	Php::Value toArray();
	Php::Value getType();
	
public:
	void fn_pushBack(Php::Value& vtype);
	int fn_endIndex() override;
	Php::Value fn_getLast() const;
	
private:
	Php::Array 		_store;
	Php::Value      _className;
	Php::Type 	 	_type;
};
#endif