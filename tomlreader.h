
#ifndef _H_TOML_READER
#define _H_TOML_READER

#include <phpcpp.h>

#include "parameter.h"
#include "token8.h"

#include <vector>
#include "ustr_data.h"

#include "pcre8_imp.h"

// globals for TomlReader, shareable, for a static class member.

namespace pun {

	class KeyTable;
	class Token8Stream;
	class ValueList;

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
			Integer, //5
			Quote3,
			Quote1,
			Apost3,
			Apost1,
			Newline, // 10
			Space,
			LSquare, //12
			RSquare,
			LCurly,
			RCurly,
			Comma,
			Dot,
			BareKey, // 18
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
		static Php::Value parseFile(Php::Parameters& param);
		static Php::Value getUseVersion();
		static Php::Value getTomlVersion();
		TomlReader();
		~TomlReader();

		Php::Value parse(Php::Parameters &param);
        void setData(Str_ptr& data) { _src = data; }
	protected:
        Php::Value doParse();
		void popExpSet();
		void pushExpSet(int id);
		void setExpSet(int id);
		void syntaxError(const char* msg);
		void valueError(const char* msg, const std::string& value);

		void syntaxError(const std::string& msg);
		void arrayMatchError(pun::Pype spunt, pun::Pype punt);

		void  parseComment();

		void  parseKeyValue();
		void  parseTablePath();

		int  finishLine();

		void parseKeyName(std::string& name);

		void parseArray(ValueList* vlist);

		void parseInlineTable();

		void parseValue(Php::Value& val, pun::Pype& punt);

		void parseQString(std::string& sval);
		void parseMLQString(std::string& sval);
		void parseLitString(std::string& sval);
		void parseMLString(std::string& sval);

        std::string escString(svx::string_view sv);

		void invalidEscChar(char eChar);
		void fn_checkFullMatch(const std::string& target, const std::string& cap);
		void throw_notFullMatch(const std::string& target, const std::string& cap);
		//bool fn_moveLiteralStr(svx::string_view& view);

		void parseDateTime(Php::Value& val);
		void parseFloat(Php::Value& val, Pcre8_match& matches);
		void parseFloatExp(Php::Value& val);
		void parseInteger(std::string& val);

		int fn_getExpSetId() const { return _expSetId; }
	private:

		unsigned int checkBOM(const char* sptr, unsigned int slen);
		enum {
			eKey,
			eScaler,
			eLString,
			eMLString,
			eBString
		};

		Rex*	_myrex;
        Str_ptr _src;

		KeyTable* _table;
		Token8	  _token;

		KeyTable* 	 _root;
		Php::Value   _v_root;

		Token8Stream* _ts;
		Php::Value    _v_ts;

		int     _stackTop;
		int     _expSetId;
		IdList	_expStack;

		std::string  _valueBuf;
		svx::string_view  _valueText;
		Re8map_share    _remap;




	};

};

#endif
