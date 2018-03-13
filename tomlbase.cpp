#include "tomlbase.h"

const char* PathTag::PHP_NAME = "Pun\\PathTag";

const char* TomlBase::PHP_NAME = "Pun\\TomlBase";

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
