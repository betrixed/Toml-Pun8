#include "tomlbase.h"

using namespace pun;

const char* PathTag::PHP_NAME = "Pun\\PathTag";

const char* TomlBase::PHP_NAME = "Pun\\TomlBase";

void
TomlBase::setup_ext(Php::Extension& ext)
{
    Php::Class<PathTag> tag(PathTag::PHP_NAME);
    ext.add(std::move(tag));

    Php::Class<TomlBase> tbase(TomlBase::PHP_NAME);
    tbase.method<&TomlBase::setTag> ("setTag");
    tbase.method<&TomlBase::getTag> ("getTag");
    ext.add(std::move(tbase));
}

Php::Value PathTag::__toString()
{
	std::string txt = "PathTag(" + std::to_string((int) _objAOT)
		+ "," + std::to_string((int) _implicit) + ")";


	return Php::Value(std::move(txt));
}

Php::Value TomlBase::getTag() const
{
	return _tag;
}

void TomlBase::setTag(Php::Parameters& param)
{
	if ((param.size()< 1)) {
		throw Php::Exception("setTag: Php Value expected");
	}
	_tag = param[0];
}



PathTag*  TomlBase::fn_getPathTag()
{
	if (_tag.isObject() && _tag.instanceOf(PathTag::PHP_NAME)) {
		return (PathTag* ) _tag.implementation();
	}
	return nullptr;
}


void TomlBase::fn_setPathTag(PathTag* tag)
{
	_tag = Php::Value(Php::Object(PathTag::PHP_NAME, tag));
}
