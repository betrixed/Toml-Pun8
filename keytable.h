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

class KeyTable : public TomlBase
{
public:
	static const char* PHP_NAME;

	// Set a key value pair
	void setKV(Php::Parameters& params);
	// Get value accessed by key
	Php::Value getV(Php::Parameters& params);
	// Remove value accessed by key
	void unsetV(Php::Parameters& params);
	// Return keys as Array 
	Php::Value getKeys();

	Php::Value count() const;
	// Return the Array as stored
	Php::Value toArray();
public:
	void fn_setKV(std::string& key, Php::Value &val);
	Php::Value fn_getV(std::string& key);

	bool hasKey(std::string& key) const;
private:
	Php::Array _store;

};

#endif
