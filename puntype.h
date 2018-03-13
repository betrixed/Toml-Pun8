#ifndef _H_PUN_TYPE
#define _H_PUN_TYPE

#ifndef PHPCPP_H
#include <phpcpp.h>
#endif

#include "parameter.h"

class PunType : public Php::Base {
public:
	static const char* PHP_NAME;

	pun::Type 	 	_type;				// Php::Type is an enum
	//Php::Value      _className; 		// is a string, didn't know how to do this cheaply

	enum {	
		Different = 0,
		Same = 1
	};

	PunType() : _type(pun::Type::Null) {}
	void fromValue(Php::Parameters& param);
	Php::Value isMatch(Php::Parameters& param);


	Php::Value type() const;
	//Php::Value getClass() const;
	Php::Value name() const;

	Php::Value __toString()  {
		return name();
	}

	void fn_fromValue(Php::Value& val);
	

	//static std::string fn_ValueConflict(const ValueTag &tag, const Php::Value& val, int matchEnum);
};

#endif