#ifndef _H_TOML_READER
#define _H_TOML_READER

#include <phpcpp.h>

#include "parameter.h"
#include "token8.h"
#include "pun8.h"

// globals for TomlReader, shareable, for a static class member.

class Rex : public Php::Base {
public:
	// enums ids, for mixture of regular expressions
	// and single characters.
	enum {
		Bad = 0,
		Equal,
		Bool,
		DateTime,
		EOS,
		Integer,
		Quote3,
		Quote1,
		Apost3,
		Apost1,
		Newline,
		Space,
		LSquare,
		RSquare,
		LCurly,
		RCurly,
		Comma,
		Dot,
		BareKey,
		EscapedChar,
		Escape,
		NoEscape,
		FloatExp,
		FloatDot,
		Hash,
		LitString,
		SpacedEqual,
		CommentStuff,
		HashComment,
		AnyValue,
		Dig_Dig,
		No_0Digit,
		Float_E,
		AnyChar,
	};
	
	Re8map*	_re8;
	IdList	_keyRe;
	IdList  _valueRe;
	IdList  _estrRe;
	IdList  _lstrRe;
	IdList  _mlstrRe;
	CharMap_sp	_singles;

	Rex();
	~Rex();

	
	static Rex* getGlobalRex();

	private: 
		static Rex* _globalRex;

};


class TomlReader : public Php::Base {
public:
	static const char* PHP_NAME;


	TomlReader();
	~TomlReader();

	Php::Value parse(Php::Parameters& param);
	

protected:
	void popExpSet();
	void pushExpSet(int id);
	void setExpSet(int id);
	void syntaxError(const char* msg);
	void syntaxError(const std::string& msg);
	void  parseComment();

	void  parseKeyValue();
	int  parseTablePath();
	int  finishLine();

	void parseObjectPath();
	
	void parseKeyName(std::string& name);
	
	void parseArray(ValueList* vlist);
	
	void parseInlineTable();
	
	void parseValue(Php::Value& val);

	void parseQString(std::string& val);
	void parseMLQString(std::string& val);
	void parseLitString(std::string& val);
	void parseMLString(std::string& val);

	void parseEscChar(std::ostream& os);
	
	void invalidEscChar(char eChar);
	void fn_checkFullMatch(const std::string& target, const std::string& cap);

	void parseDateTime(Php::Value& val);
	void parseFloat(Php::Value& val, Pcre8_match& matches);
	void parseFloatExp(Php::Value& val);
	void parseInteger(std::string& val);
private:


	enum {
		eKey,
		eScaler,
		eLString,
		eMLString,
		eBString
	};

	Rex*	_myrex;
	
	
	KeyTable* _table;
	Token8	  _token;

	KeyTable* 	 _root;
	Php::Value   _v_root;

	Token8Stream* _ts;
	Php::Value    _v_ts;
	
	int     _stackTop;
	int     _expSetId;
	IdList	_expStack;
	Pun8	_valueText;
	

};

#endif