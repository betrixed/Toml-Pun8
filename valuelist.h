#ifndef _H_ValueList
#define _H_ValueList

#include "tomlbase.h"

class ValueList : public TomlBase {
public:
	static const char* PHP_NAME;

	void pushBack(Php::Parameters& param);
	void popBack();
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
	ValueArray 		_store;
	// if storing objects, require same class name
	// store it to avoid more calls to "get_class"
	Php::Value      _className; 
	// require same value type
	Php::Type 	 	_type;
};
#endif