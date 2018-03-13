#ifndef _H_ValueList
#define _H_ValueList

#include "tomlbase.h"
#include "parameter.h"

// ValueList should not be crippled by placing "all the same thing" checks
// on each call to push. Since TomlBase has a "Tag" facility, force the 
// tag usage and value type matching here, for use by Toml Arrays which must be
// all of the same type, and the parser code will have to do a check
// Drawback is there is an extra PHP object tag for every 
// Array representation. 


class VL_Iterator : public Php::Iterator {
	ValueArray &_ref;
	ValueArray::const_iterator _iter;

public:
	VL_Iterator(Php::Base* pobj, ValueArray& aRef)
		: Php::Iterator(pobj), _ref(aRef), _iter(aRef.begin()) 
	{
	}

	virtual ~VL_Iterator() {}

	virtual bool valid() override
	{
		return _iter != _ref.end();
	}

	virtual Php::Value current() override 
	{
		return (*_iter);
	}

	virtual Php::Value key() override
	{
		return (int) (_iter - _ref.begin());
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

class ValueList : public TomlBase, public Php::Countable, 
				public Php::Traversable {
public:
	static const char* PHP_NAME;

	void pushBack(Php::Parameters& param);
	void popBack();
	Php::Value getV(Php::Parameters& params) const;
	void setV(Php::Parameters& param);

	Php::Value back() const;
	virtual long  count() override { return (long) _store.size(); }

	Php::Value size() const;
	void clear() { _store.clear(); } 

	virtual Php::Iterator *getIterator() override 
	{
		return new VL_Iterator( this, _store);
	}

	// Return the Array as stored
	Php::Value toArray();
	

	Php::Value getTag() const;
	void setTag(Php::Parameters& param);

	Php::Value __toString();
	

public:
	void fn_pushBack(Php::Value& vtype);
	int fn_endIndex() override;
	Php::Value fn_getLast() const;
	//std::string fn_typeConflict(Php::Type odd);
	//std::string fn_classConflict(Php::Value& val);
	unsigned int fn_size() { return _store.size(); }
private:
	ValueArray 		_store;
	// if storing objects, require same class name
	// store it to avoid more calls to "get_class"
	//Php::Value      _className; 
	// require same value type
	//Php::Type 	 	_type;
};
#endif