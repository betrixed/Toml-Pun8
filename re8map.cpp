#include "re8map.h"

#include "pun8.h"
#include "pcre8.h"
#include "ucode8.h"
#include <sstream>

const std::string Re8map::PHP_NAME = "Pun\\Re8map";

Re8map::Re8map()
{
	_remap = std::make_shared<Pcre8_map>();
}

Re8map::~Re8map()
{
}

Php::Value 
Re8map::setIdRex(Php::Parameters& params)
{
    auto sp = Pcre8::fromParameters(params);
    _remap.get()->setRex(sp);
    return Php::Value(params[0]);
}

Php::Value 
Re8map::getIds() const
{
    Php::Value result;
    auto map = _remap.get();

    auto m1 = map->_map.begin();
    auto mend = map->_map.end();
	int i = 0;
    while(m1 != mend) {
        result[i] = Php::Value(m1->first);
        ++i;
        ++m1;
    }
    return result;
}

Php::Value 
Re8map::getIdRex(Php::Parameters& params) 
{
    Pun8::check_Int(params);
    int index = params[0];
    Pcre8_share sp;
    auto map = _remap.get();

    if (!map->getRex(index,sp))
    {
        return Php::Value(false);
    }

    auto p8 = new Pcre8();
    p8->setImp(sp);
    auto result = Php::Object(Pcre8::PHP_NAME.data(), p8);
    return result;
}

Php::Value Re8map::shareMap(Php::Parameters& params)
{
	int mapIndex;
	int shared = 0;
	Re8map* obj = Pun8::check_Re8map(params,0);
	auto mFrom = obj->getImp().get();
	auto mTo = _remap.get();
	

	if (Pun8::option_Array(params,1)) {
		auto& Ar = params[1];
		for(auto& iter : Ar)
		{
			mapIndex = iter.second;
			if (mFrom->hasKey(mapIndex) && !mTo->hasKey(mapIndex)) {
				Pcre8_share fresh;
				mFrom->getRex(mapIndex,fresh);
				mTo->setRex(fresh);
				shared++;
			}
		}
	}
	else {
		// Assign all the keys
		auto pit = mFrom->_map.begin();
		auto pend = mFrom->_map.end();
		while(pit != pend) {
			mapIndex = pit->first;
			if (mFrom->hasKey(mapIndex) && !mTo->hasKey(mapIndex)) {
				Pcre8_share fresh;
				mFrom->getRex(mapIndex,fresh);
				mTo->setRex(fresh);
				shared++;
			}
		}
	}
	return Php::Value(shared);
}

// Has key value in map
Php::Value Re8map::count() const
{
	auto map = _remap.get();
	return Php::Value((int)map->_map.size());
}
// Has key value in map
Php::Value Re8map::hasIdRex(Php::Parameters& params) const
{
	Pun8::check_Int(params);
	int index = params[0];
	auto map = _remap.get();
	return Php::Value(map->hasKey(index));
}

// Remove key value and expression data from map
Php::Value Re8map::unsetIdRex(Php::Parameters& params)
{
	Pun8::check_Int(params);
	int index = params[0];
	auto map = _remap.get();
	return Php::Value(map->eraseRex(index));
}
