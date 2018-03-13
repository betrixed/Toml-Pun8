
#include "keytable.h"
#include "valuelist.h"
#include "parameter.h"
#include <ostream>
#include <sstream>

const char* KeyTable::PHP_NAME = "Pun\\KeyTable";
const std::string CPunk::keytable_classname(KeyTable::PHP_NAME);

long KeyTable::count() 
{
	return _store.size();
}

bool KeyTable::offsetExists(const Php::Value & key)
{
	return _store.find(key.stringValue()) != _store.end();
}

void KeyTable::offsetSet(const Php::Value &key, const Php::Value &value)
{
	_store[key.stringValue()] = value;
}

Php::Value KeyTable::offsetGet(const Php::Value &key)
{
	return _store[key.stringValue()];
}

void KeyTable::offsetUnset(const Php::Value &key)
{
	_store.erase(key.stringValue());
}
	/*
Php::Iterator *KeyTable::getIterator()
{
    // construct a new map iterator on the heap
    // the (PHP-CPP library will delete it when ready)
    return new KT_Iterator(this);
}
*/
void KeyTable::fn_setKV(std::string& key, Php::Value& val)
{

	_store[key] = val;
}

void KeyTable::setKV(Php::Parameters& params)
{
	if (params.size() < 2) {
		throw Php::Exception("KeyTable->setKV(key, value) missing a parameter ");
	}
	Php::Value& key = params[0];
	_store[key.stringValue()] = params[1];
}

Php::Value KeyTable::fn_getV(std::string& key){
	return _store[key];
}

Php::Value KeyTable::getV(Php::Parameters& params)
{
	if (params.size() < 1) {
		throw Php::Exception("KeyTable->getV(key) missing parameter ");
	}
	return _store[params[0].stringValue()];
}
// Remove value accessed by key
void KeyTable::unsetK(Php::Parameters& params)
{
	if (params.size() < 1) {
		throw Php::Exception("KeyTable->unsetV(key) missing parameter ");
	}
	_store.erase(params[0].stringValue());
}
// Return keys as Php Array 
Php::Value KeyTable::getKeys()
{
	Php::Value result;
	int idx = 0;
	for(auto ait = _store.begin(); ait != _store.end(); ait++) 
	{
		result[ idx ] = ait->first;
		idx++;
	}
	return result;
}

Php::Value KeyTable::hasK(Php::Parameters& params) {
	pun::check_String(params,0);
	return (_store.find(params[0].stringValue()) != _store.end());
}

Php::Value KeyTable::getTag() const
{
	return _tag;
}

void KeyTable::setTag(Php::Parameters& param)
{
	if ((param.size()< 1)) {
		throw Php::Exception("setTag: Php Value expected");
	}
	_tag = param[0];
}


bool KeyTable::fn_hasK(std::string& key) const
{
	return (_store.find(key) != _store.end());
}
/*
Php::Value KeyTable::count() const
{
	return Php::Value(_store.size());
}
*/

// Return the _store as Array
Php::Value KeyTable::toArray()
{
	Php::Array result;
	for(auto ait = _store.begin(); ait != _store.end(); ait++) 
	{
		Php::Value& val = ait->second;
		if (val.isObject()) {
			if (val.instanceOf(KeyTable::PHP_NAME)) {
				KeyTable* kt = (KeyTable*) val.implementation();
				result[ait->first] = kt->toArray();
				continue;
			}
			else if (val.instanceOf(ValueList::PHP_NAME)) {
				ValueList* vlist = (ValueList*) val.implementation();
				result[ait->first] = vlist->toArray();
				continue;
			}
		}
		result[ ait->first ] = ait->second;
	}
	return result;
}

Php::Value KeyTable::__toString() {
	std::stringstream ss;

	ss << "KeyTable [ tag " << _tag << " size " << _store.size() << "]";
	return ss.str();
}