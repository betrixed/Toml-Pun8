#include "idlist.h"

#include "parameter.h"
using namespace pun;

const char* IntList::PHP_NAME = "Pun\\IntList";

IntList* IntList::get_IntList(Php::Value& val)
{
    if (val.isObject()) {
        if (val.instanceOf(IntList::PHP_NAME)) {
            return (IntList*) val.implementation();
        }
    }
    return nullptr;
}

void
IntList::setup_ext(Php::Extension& ext)
{
    Php::Class<IntList> cext(IntList::PHP_NAME);
    cext.method<&IntList::__construct>("__construct", Php::Public);
    cext.method<&IntList::setArray> ("setArray", Php::Public);
    cext.method<&IntList::pushBack> ("pushBack", Php::Public);
    cext.method<&IntList::popBack> ("popBack", Php::Public);
    cext.method<&IntList::getV> ("getV", Php::Public);

    cext.method<&IntList::setV> ("setV", Php::Public);
    cext.method<&IntList::back> ("back", Php::Public);
    cext.method<&IntList::size> ("size", Php::Public);
    cext.method<&IntList::clear> ("clear", Php::Public);
    cext.method<&IntList::toArray> ("toArray", Php::Public);


    ext.add(std::move(cext));
}


void
IntList::__construct(Php::Parameters& param)
{
	setArray(param);
}
// set list from PHP array of integers
void
IntList::setArray(Php::Parameters& param)
{
	auto isArray = pun::option_Array(param, 0);
    if (!isArray) {
        throw Php::Exception("Need (Array of integer)");
    }
    const Php::Value& v = param[0];
    auto ct = v.size();
    _store.clear();
    _store.reserve(ct);
    for(int i = 0; i < v.size(); i++)
    {
        _store.push_back(v[i]);
    }
}

 void
 IntList::fn_copyIdList(Php::Value& v)
 {
    auto idle = _store.begin();
    auto idend = _store.end();
    int idx = 0;
    while (idle != idend) {
        v[idx] = *idle;
        idle++;
        idx++;
    }
 }

// return list as PHP array of integers
Php::Value
IntList::toArray()
{
	Php::Value result;
    this->fn_copyIdList(result);
    return result;
}


void IntList::fn_pushBack(Php::Value& val)
{
	_store.push_back(val);
}
void IntList::pushBack(Php::Parameters& param)
{
	if (param.size() < 1) {
		throw Php::Exception("ValueList\\Pushback(value): missing value");
	}
	fn_pushBack(param[0]);
}
void IntList::popBack()
{
	_store.pop_back();
}
static void checkIndex(int index, unsigned int vlen) {
	if (index < 0 || index >= (int) vlen) {
		throw Php::Exception("IntList index out of range");
	}
}
void
IntList::setV(Php::Parameters& param)
{
	int index = pun::check_Int(param,0);
	checkIndex(index, _store.size());
	pun::need_Value(param,1);
	_store[index] = param[1];

}
Php::Value
IntList::getV(Php::Parameters& params) const
{
	int index = pun::check_Int(params,0);
	checkIndex(index, _store.size());
	return Php::Value(_store[index]);
}


Php::Value
IntList::back() const
{
	if (_store.size() == 0) {
		throw Php::Exception("ValueList getLast on empty list");
	}
	int index = (int) _store.size() - 1;
	return Php::Value(_store[index]);
}

Php::Value
IntList::size() const
{
	return Php::Value((int)_store.size());
}
