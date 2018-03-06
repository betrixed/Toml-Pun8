#ifndef _H_KEYTABLE
#define _H_KEYTABLE

/**
* Toml - or JSON or any kind of nested array storage,
* Toml specifies string - only keys. Even integers are to be converted to strings.
* Except for Array of KeyTables, in which case the "keys" are ordinal numbers from zero.
* The storage model in C++ - PHP-CPP terms is -
* KeyTable -- string key, Php::Value as value.
* Permitted values are scaler values, string values, inline nested arrays, inline nested tables
* and Array of Tables (AOT)
*
* Array of Table (AOT) --  vector of KeyTable objects - a TableList.
*
* As well, there are inline arrays, vectors of Php::Value, where each value in a single array is the same type,
* even if the type is array. The PHP version TOML parser uses a ValueList object for these.
* ValueList is therefore very similar to a TableList.  
* 
*/

/* Making a C++ compiled version of the TOML parser implies not going back to the PHP interpreter very much until the
   Array tree is complete, which means having C++ versions of all these container varients, which able
   to hold Php::Values, include the nested Array object instances.

   The PHP-CPP documentation has examples of implementations of ArrayAccess, shall try this first. One
   feature of PHP noted has been the difficulty of avoiding the forced conversion of "numeric" strings into
   integer keys. There are efficiency considerations that make it reasonable. 

   Of interest is that Php::Value can wrap the PHP-Array Implementation, and can be returned as an array value,
   so lets do that, and forgo implementing the full ArrayAccess interface, as we try the simplest implementation first.  

   As well as the public PHP - object interface, there will be KeyTable methods for use by the C++ world, which will not
   be using Php::Parameter arguments.

*/

#include <phpcpp.h>

#include "tomlbase.h"

// Php::Value also has a mapValue() method to return ValueMap
// string keys are forced by Value.stringValue()

class KeyTable : public TomlBase, public Php::Countable, 
				public Php::ArrayAccess//, public Php::Traversable
{
public:
	static const char* PHP_NAME;

	// Set a key value pair
	void setKV(Php::Parameters& params);
	// Get value accessed by key
	Php::Value getV(Php::Parameters& params);

	Php::Value hasK(Php::Parameters& params);
	// Remove value accessed by key
	void unsetK(Php::Parameters& params);
	// Return keys as Array 
	Php::Value getKeys();
	// Countable and ArrayAccess
	virtual long count() override;
	virtual bool offsetExists(const Php::Value &key) override;
	virtual void offsetSet(const Php::Value &key, const Php::Value &value) override;
	virtual Php::Value offsetGet(const Php::Value &key) override;
	virtual void offsetUnset(const Php::Value &key) override;
	//virtual Php::Iterator *getIterator() override;

	// Return the Array as stored
	Php::Value toArray();
public:
	void fn_setKV(std::string& key, Php::Value &val);
	Php::Value fn_getV(std::string& key);

	bool fn_hasK(std::string& key) const;
private:
	ValueMap _store;

};
/*
class KT_Iterator : public Php::Iterator {
	Php::Array         _array;
	// Has to be a pointer, unique_ptr usage.
	Php::ValueIterator* _iter; 

	

	KT_Iterator(KeyTable* base) : Php::Iterator(base), 
		_array(base->_store),
		_iter(nullptr) 
	{

	}
	virtual bool valid(){
	 	return (*_iter) != _array.end();
	}
	virtual Php::Value current() {
		return (*_iter)->second;
	}
	virtual Php::Value key() {
		return (*_iter)->first;
	}
	virtual void next() {
		(*_iter)++;
	}
	virtual void rewind() {
		_iter = & (_array.begin());
	}
};
*/
#endif
