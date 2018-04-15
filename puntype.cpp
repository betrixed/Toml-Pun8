#include "puntype.h"
#include "parameter.h"

using namespace pun;

const char* PunType::PHP_NAME = "Pun\\Type";

void PunType::fromValue(Php::Parameters& param)
{
	pun::need_Value(param);
	_type = pun::getPype(param[0]);
}


void PunType::fn_fromValue(Php::Value& val) {
	_type = pun::getPype(val);
}

Php::Value
PunType::type() const
{
	return _type;
}


Php::Value
PunType::name() const
{
	return pun::getPypeId(_type);
}

Php::Value PunType::isMatch(Php::Parameters& param)
{
	pun::need_Value(param);
	return (_type == pun::getPype(param[0])) ? Same : Different;
}
