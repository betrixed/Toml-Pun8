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
#include <iostream>

// Php::Value also has a mapValue() method to return ValueMap
// string keys are forced by Value.stringValue()

namespace pun {

/*!
    Implement PHP interator internals for KeyTable
*/
class KT_Iterator : public Php::Iterator {
	ValueMap &_ref;
	ValueMap::const_iterator _iter;
public:
	KT_Iterator(Php::Base* pobj, ValueMap& mapRef)
		: Php::Iterator(pobj), _ref(mapRef), _iter(mapRef.begin())
	{
	}

	virtual ~KT_Iterator() {}

	virtual bool valid() override
	{
		return _iter != _ref.end();
	}

	virtual Php::Value current() override
	{
		return _iter->second;
	}

	virtual Php::Value key() override
	{
		return _iter->first;
	}

	virtual void next() override
	{
		_iter++;
	}

	virtual void rewind() override
	{
		_iter = _ref.begin();
	}
};

/*! KeyTable is a PHP class wrapper of a std::map<std::string,Php::Value> , where
    Php::Value is a wrapper for any value, known as a Z_VAL.
    All keys are strings.
    From PHP, array access [] is possible, foreach works as expected,
    the count() function returns the number of keys.
    and serialize and unserialize works.
    Php 'foreach' always traverses in the default alphabetical order, as
    std::map is implemented with a red-black tree container.

    KeyTable also has a Tag property, to get or set with any Php::Value.
    Keytable is serialiable, and can return its values as an Array,
    or be set from an Array.
    KeyTable access time grows O(log N).
    Its keys and values are not directly visible to PHP xdebug variable inspection.

    KeyTable differs in behaviour from Phalcon\Config class, which it is loosely meant to be
    a substitute for.
    Phalcon\Config stores numeric (integer) keys, as integer, even if set as string.
    Phalcon\Config foreach traversal order is undefined for a given set of keys.
    Phalcon\Config access stores values as PHP object properties table, which uses a hash, quasi O(1).


*/
class KeyTable : public TomlBase, public Php::Countable,
				public Php::ArrayAccess, public Php::Traversable, public Php::Serializable
{
public:
	static const char* PHP_NAME;

	static KeyTable* get_KeyTable(Php::Value& v);

    static void setup_ext(Php::Extension& ext /*, Php::Interface& if1, Php::Interface& if2*/);

    //! __construct([array]);  from optional array
    void __construct(Php::Parameters& param);

	// Set a key value pair
	void setKV(Php::Parameters& params);
	// Get value accessed by key
	Php::Value getV(Php::Parameters& params);

	// Get value or return supplied default
	Php::Value get(Php::Parameters& params);

    // Follow string of keys separated by '.'
	Php::Value path(Php::Parameters& param);

	Php::Value hasK(Php::Parameters& params);
	// Remove value accessed by key
	void unsetK(Php::Parameters& params);
	// Return keys as Array
	Php::Value getKeys();

	Php::Value getTag() const;
	void setTag(Php::Parameters& param);
	// Countable and ArrayAccess, as done by PHP-CPP
	virtual long count() override;
	virtual bool offsetExists(const Php::Value &key) override;
	virtual Php::Value offsetGet(const Php::Value &key) override;
	virtual void offsetUnset(const Php::Value &key) override;
	virtual void offsetSet(const Php::Value &key, const Php::Value &value) override;
	// For Traversable
	virtual Php::Iterator *getIterator() override
	{
		return new KT_Iterator( this, _store);
	}



	Php::Value  size() const { return (long) _store.size(); }
	void clear() { _store.clear(); }
	// Return the Array as stored
	Php::Value toArray();
	// recursive merge
    /*!
        function merge(array) : KeyTable;
        Recursive add all values from array.
        Convert numeric keys into strings.
        Convert values which are sub-arrays into KeyTable objects.

        function fromArray(array) : KeyTable;
    */

	/*! function merge(KeyTable) : KeyTable;
	 Recursive add keys from KeyTable argument
	 values are not copied, references are copied.
	 */

	Php::Value merge(Php::Parameters& param);


    //Php::Value fromArray(Php::Parameters& param);


	Php::Value intf_merge(Php::Value& obj);

	auto begin() { return _store.begin(); }
	auto end() { return _store.end(); }

	Php::Value __toString();

	virtual std::string serialize();
	virtual void unserialize(const char *input, size_t size);

	Php::Value __get(const Php::Value &name) const;
	void __set(const Php::Value &name, const Php::Value &value);
	bool __isset(const Php::Value &name) const;
	void __unset(const Php::Value &name);
    /*!
        function replaceVars() : void;
        Replace each segment ${name} of a string value
        with the looked up value
        in a KeyTable or Array
    */
	void replaceVars(Php::Parameters& param);

public:
	void fn_setKV(std::string& key, Php::Value &val);
	Php::Value fn_getV(std::string& key);

	void fn_merge(const Php::Value& fromArray);

	void fn_merge(KeyTable* other);
	void throw_mergeFail(const std::string& key) ;

	void fn_unserialize(std::istream& ins);
	void fn_serialize(std::ostream& os);

	bool fn_hasK(std::string& key) const;

	Php::Value fn_object();
    ValueMap& fn_store() { return _store; }
protected:
	ValueMap _store;

};

const std::string keytable_classname;

}; // end namespace pun
#endif
