#include "token8.h"

const char* Token8::PHP_NAME = "Pun\\Token8";


Token8::Token8() : _id(0), _line(0), _isSingle(false)
{

}

Token8::Token8(const Token8 &m) 
: _value(m._value), _id(m._id), _line(m._line), _isSingle(m._isSingle)
{

}

Token8 &
Token8::operator=(const Token8 &m)
{
	if (this != &m) {
    _value = m._value;
    _id = m._id;
    _line = m._line;
    _isSingle = m._isSingle;
  }
  return *this;
}

Token8::Token8(const Token8&& m)
: _value(std::move(m._value)), _id(m._id), _line(m._line), _isSingle(m._isSingle)
{

}

Token8 &
Token8::operator=(const Token8 &&m)
{
	_value = std::move(m._value);
	_id = m._id;
	_line = m._line;
	_isSingle = m._isSingle;

	return *this;
}


Php::Value 
Token8::getValue() const
{
	return Php::Value(_value.data(), _value.size());
}

Php::Value 
Token8::getId() const
{
	return Php::Value(_id);
}

Php::Value 
Token8::getLine() const
{
	return Php::Value(_line);
}

Php::Value 
Token8::isSingle() const
{
	return Php::Value(_isSingle);
}

	

