
#include "keytable.h"


const char* KeyTable::PHP_NAME = "Pun\\KeyTable";


void KeyTable::fn_setKV(std::string& key, Php::Value& val)
{
	_store[key] = val;
}

void KeyTable::setKV(Php::Parameters& params)
{
	if (params.size() < 2) {
		throw Php::Exception("KeyTable->setKV(key, value) missing a parameter ");
	}

	_store[params[0]] = params[1];
}

Php::Value KeyTable::fn_getV(std::string& key){
	return _store[key];
}

Php::Value KeyTable::getV(Php::Parameters& params)
{
	if (params.size() < 1) {
		throw Php::Exception("KeyTable->getV(key) missing parameter ");
	}
	return _store[params[0]];
}
// Remove value accessed by key
void KeyTable::unsetV(Php::Parameters& params)
{
	if (params.size() < 1) {
		throw Php::Exception("KeyTable->unsetV(key) missing parameter ");
	}
	_store.unset(params[0]);
}
// Return keys as Array 
Php::Value KeyTable::getKeys()
{
	Php::Value result;
	int idx = 0;
	for( auto &iter : _store) {

		result[ idx ] = iter.first;
		idx++;
	}
	return result;
}

bool KeyTable::hasKey(std::string& key) const
{
	return _store.contains(key);
}

Php::Value KeyTable::count() const
{
	return Php::Value(_store.size());
}
// Return the Array as stored
Php::Value KeyTable::toArray()
{
	return _store;
}