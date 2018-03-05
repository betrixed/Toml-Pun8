#include "valuelist.h"
#include "parameter.h"

const char* ValueList::PHP_NAME = "Pun\\ValueList";

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
		if (addType != _type) {
			throw Php::Exception("ValueList\\Pushback(value): value type conflict");
		}
		if (_type == Php::Type::Object) {
			if (! val.instanceOf(_className, _className.size(), true)) {
				throw Php::Exception("ValueList\\Pushback(value): class name conflict");
			}
		}
	}
	_store[index] = val;
}
void ValueList::pushBack(Php::Parameters& param)
{
	if (param.size() < 1) {
		throw Php::Exception("ValueList\\Pushback(value): missing value");
	}
	fn_pushBack(param[0]);
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
	return Php::Value(_store[index]);
}

Php::Value ValueList::getLast() const
{
	int index = (int) _store.size() - 1;
	return Php::Value(_store[index]);
}

Php::Value ValueList::count() const
{
	return Php::Value(_store.size());
}

// Return the Array as stored
Php::Value ValueList::toArray()
{
	return _store;
}

int ValueList::fn_endIndex() {
	return (int) _store.size() - 1;
}

