#ifndef _H_ValueList
#define _H_ValueList

#include "tomlbase.h"

class ValueList : public TomlBase {
public:
	static const char* PHP_NAME;

	void pushBack(Php::Parameters& param);
	void popBack();
	Php::Value getV(Php::Parameters& params) const;
	Php::Value back() const;
	Php::Value size() const;
	// Return the Array as stored
	Php::Value toArray();
	Php::Value getType();
	Php::Value getTag() const;
	void setTag(Php::Parameters& param);
public:
	void fn_pushBack(Php::Value& vtype);
	int fn_endIndex() override;
	Php::Value fn_getLast() const;
	std::string fn_typeConflict(Php::Type odd);
	std::string fn_classConflict(Php::Value& val);
private:
	ValueArray 		_store;
	// if storing objects, require same class name
	// store it to avoid more calls to "get_class"
	Php::Value      _className; 
	// require same value type
	Php::Type 	 	_type;
};
#endif