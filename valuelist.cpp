#include "valuelist.h"
#include "parameter.h"
#include "keytable.h"
#include <ostream>
#include <sstream>

const char* ValueList::PHP_NAME = "Pun\\ValueList";

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

std::string ValueList::fn_typeConflict(Php::Type odd)
{
	std::stringstream ss;

	ss << "Cannot add " << pun::getTypeName(odd) << " to list of " << pun::getTypeName(_type);
	return ss.str();
}

std::string ValueList::fn_classConflict(Php::Value& val)
{
	auto oddClass = Php::call("get_class", val);
	std::stringstream ss;

	ss << "Cannot add " << oddClass << " to list of " << _className;
	return ss.str();
}

void ValueList::fn_pushBack(Php::Value& val)
{
	int index = (int) _store.size();
	if (index == 0) {
		_type = val.type();
		if (_type == Php::Type::Object) {
			_className = Php::call("get_class", val);
		}
	}
	else {
		auto addType = val.type();
		if (addType != _type){
			// brain damage ahead
			if ((_type != Php::Type::True) && (_type != Php::Type::False) 
				&& (addType != Php::Type::False) && (addType != Php::Type::True))
					throw Php::Exception(fn_typeConflict(addType));
		}
		if (val.isObject()) {
			if (! val.instanceOf(_className, _className.size(), true)) {
				throw Php::Exception(fn_classConflict(val));
			}
		}
	}
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

Php::Value ValueList::getType()
{
	if (_type == Php::Type::Object) {
		return _className;
	}
	else {
		return _type;
	}
}

Php::Value ValueList::getV(Php::Parameters& params) const
{
	int index = pun::check_Int(params,0);
	if (index < 0 || index >= (int) _store.size()) {
		throw Php::Exception("ValueList index out of range");
	}
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

