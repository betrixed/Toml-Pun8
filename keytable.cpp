
#include "keytable.h"
#include "valuelist.h"
#include "parameter.h"
#include <ostream>
#include <sstream>

using namespace pun;

const char* KeyTable::PHP_NAME = "Pun\\KeyTable";
const std::string CPunk::keytable_classname(KeyTable::PHP_NAME);


KeyTable* KeyTable::get_KeyTable(Php::Value& v)
{
    if (v.instanceOf(CPunk::keytable_classname))
    {
        return (KeyTable*) v.implementation();
    }
    return nullptr;
}

void
KeyTable::setup_ext(Php::Extension& ext, Php::Interface& if1, Php::Interface& if2) {
    Php::Class<KeyTable> keytab(KeyTable::PHP_NAME);
    keytab.implements(if1);
    keytab.implements(if2);
    //keytab.extends(tbase);

    keytab.method<&KeyTable::setKV> ("set");
    keytab.method<&KeyTable::get> ("get");
    keytab.method<&KeyTable::unsetK> ("unsetKey");
    keytab.method<&KeyTable::hasK> ("exists");
    keytab.method<&KeyTable::clear> ("clear");

    keytab.method<&KeyTable::merge> ("merge", {
        Php::ByVal("store", KeyTable::PHP_NAME)
    });
    //keytab.method<&KeyTable::merge> ("merge");
    keytab.method<&KeyTable::size> ("size");

    keytab.method<&KeyTable::__construct> ("__construct");
    keytab.method<&KeyTable::toArray> ("toArray");
    keytab.method<&KeyTable::setTag> ("setTag",
        { Php::ByVal("tag") }
    );
    keytab.method<&KeyTable::getTag> ("getTag");

    keytab.method<&KeyTable::replaceVars> ("replaceVars");

    ext.add(std::move(keytab));
}

void KeyTable::__construct(Php::Parameters& param) {
    if (param.size() > 0) {
        Php::Value& v = param[0];

        if (v.isArray()) {
            fn_merge(v);
        }
        else if (v.isObject())
        {
            auto obj = KeyTable::get_KeyTable(v);
            fn_merge(obj);
        }
    }
}

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



Php::Value KeyTable::get(Php::Parameters& param)
{
	auto ct = param.size();
	if (ct < 1) {
		throw Php::Exception("KeyTable->get(key, alternate) missing key");
	}
	auto fit = _store.find(param[0].stringValue());
	if (fit != _store.end())
		return fit->second;
	if (ct < 2) {
		throw Php::Exception("KeyTable->get(key, alternate) missing alternate ");
	}
	return param[1];
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

Php::Value
KeyTable::fn_object()
{
	return Php::Object(PHP_NAME, this);
}

void KeyTable::fn_unserialize(std::istream& ins)
{
	size_t keyct;
	char   check;
	std::string key;

	ins.read( (char*) &keyct, sizeof(keyct));
	//Php::out << "fn_unserialize keyct is " << keyct << std::endl;
	ins >> check;
	//Php::out << "fn_unserialize check " << check << std::endl;
	for(size_t i = 0; i < keyct; i++) {
		pun::unserialize_str(key, ins);
		//Php::out << "fn_unserialize key  " << key << std::endl;
		Php::Value val;
		pun::unserialize(val, ins);
		_store[key] = val;
	}
	pun::unserialize(_tag, ins);
	ins >> check;
	//Php::out << "fn_unserialize check2 " << check << std::endl;
}

void KeyTable::fn_serialize(std::ostream& out)
{
	auto keyct = _store.size();
	out.write((const char*) &keyct,sizeof(keyct));
	//Php::out << "KT fn_serialize " << std::endl;
	out << '{';
	for(auto ait = _store.begin(); ait != _store.end(); ait++)
	{
		const std::string& key = ait->first;
		//Php::out << "fn_serialize key" << key << std::endl;
		pun::serialize_str(key.data(), key.size(), out);
		pun::serialize(ait->second, out);

	}
	pun::serialize(_tag, out);
	out << '}';
}

std::string
KeyTable::serialize()
{
	std::stringstream out;
	//Php::out << "KT serialize " << std::endl;
	out << 'K';
	fn_serialize(out);

	return out.str();
}



void KeyTable::unserialize(const char *input, size_t size)
{
	std::string buffer(input,size);
	//Php::out << "KT unserialize " << std::endl;

	std::istringstream ins(buffer);
	char check;
	ins >> check;
	//Php::out << "Check " << check << std::endl;
	fn_unserialize(ins);
}


Php::Value KeyTable::__get(const Php::Value &name) const
{
	auto fit = _store.find(name.stringValue());

	if (fit != _store.end()) {
		return fit->second;
	}
	else
		return Php::Value();
}

void KeyTable::__set(const Php::Value &name, const Php::Value &value)
{
	_store[name.stringValue()] = value;
}

bool KeyTable::__isset(const Php::Value &name) const
{
	return _store.find(name.stringValue()) != _store.end();
}

void KeyTable::__unset(const Php::Value &name)
{
	_store.erase(name.stringValue());
}


void
KeyTable::throw_mergeFail(const std::string& key) {
	throw Php::Exception("KeyTable merge fail : key clash on " + key);
}
// if merge fails at some point,
// throw exception, leaving merge with incomplete state.
void
KeyTable::fn_merge(KeyTable* other)
{
	auto zit = other->end();

	for(auto ait = other->begin(); ait != zit; ait++)
	{
		const std::string& key = ait->first;
		auto hereNow = _store.find(key);

		if (hereNow != _store.end()) {
			// key clashes only allowed for matching tables
			KeyTable* kt = pun::castKeyTable(hereNow->second);
			if (kt) {
				KeyTable* okt = pun::castKeyTable(ait->second);
				if (okt) {
					kt->fn_merge(okt);
					continue;
				}
			}
			throw_mergeFail(key);
		}
		else {
			_store[key] = ait->second;
		}
	}
}



// only works for another KeyTable, and for KeyTable, ValueList members
// complicated by TOML that ValueList and KeyTable can't be merged together,
// if occupy same key.

Php::Value
KeyTable::merge(Php::Parameters& param) {
    bool ok = true;
	if (param.size() > 0) {
        Php::Value& v = param[0];

        if (v.isArray()) {
            fn_merge(v);
        }
        else if (v.isObject()) {
            auto obj = KeyTable::get_KeyTable(v);
            if (obj != nullptr) {
                fn_merge(obj);
            }
            else {
                ok = false;
            }
        }
	}
	else {
        ok = false;
	}
	if (!ok) {
        throw Php::Exception("KeyTable merge  needs array or KeyTable as argument");
	}
	return fn_object();
}

void KeyTable::fn_merge(const Php::Value& fromArray)
{
    for( auto &iter : fromArray) {
		const Php::Value& kval = iter.first;

		std::string key = kval.stringValue();
        if (key.size() > 0) {
            // empty keys not allowed.
            const Php::Value& val = iter.second;
            if (val.isArray()) {
                KeyTable* subTable = new KeyTable();
                Php::Value table(Php::Object(KeyTable::PHP_NAME, subTable));
                subTable->fn_merge(val);
                _store[key] = table;
            }
            else {
                _store[key] = val;
            }
        }
    }
}

Php::Value
KeyTable::intf_merge(Php::Value& obj)
{
	KeyTable* other = castKeyTable(obj);
	fn_merge(other);
	return fn_object();
}

void KeyTable::replaceVars(Php::Parameters& param)
{
    bool check = (param.size() > 0);
    KeyTable*   searchKT = nullptr;
    Php::Value  searchArray;

    if (check) {
        Php::Value& v = param[0];
        if (v.isArray()) {
            searchArray = v;
        }
        else if (v.isObject()) {
            searchKT = KeyTable::get_KeyTable(v);
            check = (searchKT != nullptr);
        }
    }
    if (!check) {
        throw Php::Exception("replaceVars needs a string lookup array or object");
    }
    replaceVar_ValueMap(_store,param[0]);
}
