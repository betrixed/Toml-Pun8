#include "re8map.h"
#include "pcre8.h"
#include "ucode8.h"
#include "parameter.h"
#include "ustr8.h"
#include "idlist.h"
#include "recap8.h"
#include <sstream>

using namespace pun;

const char* Re8map::PHP_NAME = "Pun\\Re8map";

void
Re8map::setup_ext(Php::Extension& ext)
{
    Php::Class<Re8map> re8(Re8map::PHP_NAME);
    re8.method<&Re8map::setIdRex> ("setIdRex");
    re8.method<&Re8map::hasIdRex> ("hasIdRex");
    re8.method<&Re8map::unsetIdRex> ("unsetIdRex");
    re8.method<&Re8map::getIdRex> ("getIdRex");
    re8.method<&Re8map::addMapIds> ("addMapIds");
    re8.method<&Re8map::getIds> ("getIds");
    re8.method<&Re8map::count> ("count");
    re8.method<&Re8map::firstMatch> ("firstMatch");
    ext.add(std::move(re8));
}

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
    pun::check_Int(params);
    int index = params[0];
    Pcre8_share sp;
    auto map = _remap.get();

    if (!map->getRex(index,sp))
    {
        return Php::Value(false);
    }

    auto p8 = new Pcre8();
    p8->setImp(sp);
    auto result = Php::Object(Pcre8::PHP_NAME, p8);
    return result;
}

Php::Value Re8map::addMapIds(Php::Parameters& params)
{
	int mapIndex;
	int shared = 0;
	Re8map* obj = pun::check_Re8map(params,0);
	auto mFrom = obj->getImp().get();
	auto mTo = _remap.get();


	if (pun::option_Array(params,1)) {
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
	pun::check_Int(params);
	int index = params[0];
	auto map = _remap.get();
	return Php::Value(map->hasKey(index));
}

// Remove key value and expression data from map
Php::Value Re8map::unsetIdRex(Php::Parameters& params)
{
	pun::check_Int(params);
	int index = params[0];
	auto map = _remap.get();
	return Php::Value(map->eraseRex(index));
}

Php::Value
Re8map::firstMatch(Php::Parameters& param)
{
    UStr8* u8 = nullptr;;
    Recap8* caps = nullptr;;
    IntList*  intlist = nullptr;
    IdList    ids;

    bool checked = true;
    if (param.size() < 3) {
        checked = false;
    }
    if (checked && (u8 = UStr8::get_UStr8(param[0])) == nullptr) {
        checked = false;
    }
    if (checked && (caps = Recap8::get_Recap8(param[1])) == nullptr) {
        checked = false;
    }
    if (checked) {
        Php::Value& idArray = param[2];
        if (idArray.isArray()) {
            ids = toIdList(idArray);
        }
        else {
            checked = ((intlist = IntList::get_IntList(param[2])) != nullptr);
        }
    }
    if (!checked) {
        throw Php::Exception("firstMatch(UStr8,Recap8,List<int>) parameter missing");
    }

    auto spm = _remap.get();
    const IdList& tests = (intlist==nullptr) ? ids : intlist->_store;

    return spm->firstMatch(u8->fn_getView(), tests, caps->_match );

}

