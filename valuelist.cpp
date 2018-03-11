#include "valuelist.h"
#include "parameter.h"
#include "keytable.h"
#include <ostream>
#include <sstream>

const char* ValueList::PHP_NAME = "Pun\\ValueList";

const std::string CPunk::valuelist_classname(ValueList::PHP_NAME);

Php::Value ValueList::getTag() const
{
	return _tag;
}

void ValueList::setTag(Php::Parameters& param)
{
	if ((param.size()< 1)) {
		throw Php::Exception("setTag: Php Value expected");
	}
	_tag = param[0];
}

// It was bad design to do the value type checking and instanceOf inside ValueList
// In terms of efficiency should force TOML (class user) code to do it.
// Take advantage of the TOML base Tag property
// Values could be any Php kind of thing. Why limit ValueList just for them?
// Force caller to decide on action, exception calls, if not same.



void ValueList::fn_pushBack(Php::Value& val)
{
	_store.push_back(val);
}
void ValueList::pushBack(Php::Parameters& param)
{
	if (param.size() < 1) {
		throw Php::Exception("ValueList\\Pushback(value): missing value");
	}
	fn_pushBack(param[0]);
}
void ValueList::popBack()
{
	_store.pop_back();
}

static void checkIndex(int index, unsigned int vlen) {
	if (index < 0 || index >= (int) vlen) {
		throw Php::Exception("ValueList index out of range");
	}	
}
void ValueList::setV(Php::Parameters& param)
{
	int index = pun::check_Int(param,0);
	checkIndex(index, _store.size());
	pun::need_Value(param,1);
	_store[index] = param[1];
	
}
Php::Value ValueList::getV(Php::Parameters& params) const
{
	int index = pun::check_Int(params,0);
	checkIndex(index, _store.size());
	return Php::Value(_store[index]);
}


Php::Value ValueList::back() const
{
	if (_store.size() == 0) {
		throw Php::Exception("ValueList getLast on empty list");
	}
	int index = (int) _store.size() - 1;
	return Php::Value(_store[index]);
}

Php::Value ValueList::size() const
{
	return Php::Value((int)_store.size());
}

// Return the Array as stored
Php::Value ValueList::toArray()
{
	Php::Array result;
	// all the keys are integers, but values need to be checked
	int idx = 0;
	for( auto ait = _store.begin(); ait != _store.end(); idx++, ait++) {
		Php::Value& v = (*ait);
		if (v.isObject()) {
			if (v.instanceOf(ValueList::PHP_NAME)) {
				result[idx] = ((ValueList*) v.implementation())->toArray();
				continue;
			}
			else if (v.instanceOf(KeyTable::PHP_NAME)) {
				result[idx] = ((KeyTable*) v.implementation())->toArray();
				continue;
			}
		}
		result[idx] = v; // see what happens
	}
	return result;
}

int ValueList::fn_endIndex() {
	return (int) _store.size() - 1;
}

