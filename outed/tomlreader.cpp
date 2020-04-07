#include "tomlreader.h"
#include "keytable.h"
#include "valuelist.h"
#include "re8map.h"
#include "ucode8.h"
#include "ustr8.h"
#include "token8stream.h"

#include <ostream>
#include <sstream>
#include <fstream>
#include <limits>
#include <algorithm>
#include <cstdint>

using namespace pun;

const char* TomlReader::PHP_NAME = "Pun\\TomlReader";

void TomlReader::setup_ext(Php::Extension& ext)
{
    Php::Class<TomlReader> rdr(TomlReader::PHP_NAME);

    rdr.method<&TomlReader::parse> ("parse");
    rdr.method<&TomlReader::parseFile>("parseFile");
    rdr.method<&TomlReader::getUseVersion>("getUseVersion");
    rdr.method<&TomlReader::getTomlVersion>("getTomlVersion");
    ext.add(std::move(rdr));
}

const char* const cRexBool = "^(true|false)";
const char* const cDateTime = "^(\\d{4}-\\d{2}-\\d{2}(T\\d{2}:\\d{2}:\\d{2}(\\.\\d{6})?(Z|-\\d{2}:\\d{2})?)?)";
const char* const cFloatExp = "^([+-]?((\\d_?)+([\\.](\\d_?)*)?)([eE][+-]?(_?\\d_?)+))";
const char* const cFloatDot = "^([+-]?((\\d_?)+([\\.](\\d_?)*)))";
const char* const cInteger = "^([+-]?(\\d_?)+)";

const char* const cQuote3 = "^(\"\"\")";
const char* const cApost3 = "^(\'\'\')";
const char* const cSpace = "^(\\h+)";
const char* const cBareKey = "^([-A-Z_a-z0-9]+)";

const char* const cEscapedChar = "^(\\\\(n|t|r|f|b|\\\"|\\\\|u[0-9A-Fa-f]{4,4}|U[0-9A-Fa-f]{8,8}))";
const char* const cNoEscape = "^([^\\x{0}-\\x{19}\\x{22}\\x{5C}]+)";
const char* const cLitString = "^([^\\x{0}-\\x{19}\\x{27}]+)";

const char* const cAnyValue = "^([^\\s\\]\\},]+)";
const char* const cSpacedEqual = "^(\\h*=\\h*)";
const char* const cCommentStuff = "^(\\V*)";
const char* const cHashComment = "^(\\h*#\\V*|\\h*)";

const char* const cDig_Dig = "^([^\\d]_[^\\d])|(_$)";
//const char* const cNo_0Digit = "^(0\\d+)";
const char* const cFloat_E = "([^\\d]_[^\\d])|_[eE]|[eE]_|(_$)";

#ifdef ZTS

/**
 *  Mutex so that getGlobalRex construction only happens once
 *  by one process at a time
 *  @var    std::mutex
 */
	std::mutex mutexRexGlobal;

#endif



Rex* Rex::_globalRex;

Rex* Rex::getGlobalRex()
{
#ifdef ZTS

    // lock the mutex
    std::unique_lock<std::mutex> lock(mutexRexGlobal);
#endif
    if (Rex::_globalRex == nullptr) {
    	Rex::_globalRex = new Rex();
    }
    return Rex::_globalRex;
}
Rex::Rex()
{
	_re8 = new Re8map();

	auto mptr = _re8->_remap.get();

	mptr->setRex(pun::makeSharedRe(Rex::Bool, cRexBool, strlen(cRexBool) ));
	mptr->setRex(pun::makeSharedRe(Rex::DateTime, cDateTime, strlen(cDateTime) ));
	mptr->setRex(pun::makeSharedRe(Rex::FloatExp, cFloatExp, strlen(cFloatExp) ));
	mptr->setRex(pun::makeSharedRe(Rex::FloatDot, cFloatDot, strlen(cFloatDot) ));
	mptr->setRex(pun::makeSharedRe(Rex::Integer, cInteger, strlen(cInteger) ));

	mptr->setRex(pun::makeSharedRe(Rex::Quote3, cQuote3, strlen(cQuote3) ));
	mptr->setRex(pun::makeSharedRe(Rex::Apost3, cApost3, strlen(cApost3) ));
	mptr->setRex(pun::makeSharedRe(Rex::Space, cSpace, strlen(cSpace) ));
	mptr->setRex(pun::makeSharedRe(Rex::BareKey, cBareKey, strlen(cBareKey) ));

	mptr->setRex(pun::makeSharedRe(Rex::EscapedChar, cEscapedChar, strlen(cEscapedChar) ));
	mptr->setRex(pun::makeSharedRe(Rex::NoEscape, cNoEscape, strlen(cNoEscape) ));
	mptr->setRex(pun::makeSharedRe(Rex::LitString, cLitString, strlen(cLitString) ));

	mptr->setRex(pun::makeSharedRe(Rex::AnyValue, cAnyValue, strlen(cAnyValue) ));
	mptr->setRex(pun::makeSharedRe(Rex::SpacedEqual, cSpacedEqual, strlen(cSpacedEqual) ));
	mptr->setRex(pun::makeSharedRe(Rex::CommentStuff, cCommentStuff, strlen(cCommentStuff) ));
	mptr->setRex(pun::makeSharedRe(Rex::HashComment, cHashComment, strlen(cHashComment) ));

	mptr->setRex(pun::makeSharedRe(Rex::Dig_Dig, cDig_Dig, strlen(cDig_Dig) ));
	//mptr->setRex(pun::makeSharedRe(Rex::No_0Digit, cNo_0Digit, strlen(cNo_0Digit) ));
	mptr->setRex(pun::makeSharedRe(Rex::Float_E, cFloat_E, strlen(cFloat_E) ));

	_singles = std::make_shared<CharMap>();

	auto cmap = _singles.get();
	cmap->_map.reserve(31);

	cmap->_map.insert( {
		{'=', Rex::Equal},
		{'[', Rex::LSquare},
		{']', Rex::RSquare},
		{'.', Rex::Dot},
		{',', Rex::Comma},
		{'{', Rex::LCurly},
		{'}', Rex::RCurly},
		{'\'', Rex::Apost1},
		{'\"', Rex::Quote1},
		{'#', Rex::Hash},
		{'\\', Rex::Escape},
		{' ', Rex::Space},
		{'\t', Rex::Space},
		{'\f', Rex::Space},
		{'\b', Rex::Space}
	});

	_keyRe = std::move(std::vector<int>( {Rex::Space, Rex::BareKey, Rex::Integer}));
	_valueRe = std::move(std::vector<int>( {Rex::Bool, Rex::DateTime, Rex::FloatExp, Rex::FloatDot, Rex::Integer}));
	_estrRe = std::move(std::vector<int>( {Rex::Space, Rex::NoEscape, Rex::EscapedChar, Rex::Quote3}));
	_lstrRe = std::move(std::vector<int>( {Rex::LitString }));
	_mlstrRe = std::move(std::vector<int>( {Rex::LitString, Rex::Apost3}));
}


Rex::~Rex()
{

}
TomlReader::~TomlReader()
{

}

Php::Value
TomlReader::getUseVersion()
{
	return "Pun\\TomlReader 0.4 PHP-CPP 2.0.0";
}

Php::Value
TomlReader::getTomlVersion()
{
	return "0.4 Toml Specification";
}

Php::Value
TomlReader::parseFile(Php::Parameters& param)
{
	pun::check_String(param,0);
	std::string path = param[0];

	std::ifstream in(path, std::ios::binary);

	std::string contents{std::istreambuf_iterator<char>{in}, {}};

    auto sp = std::make_shared<UStrData>();
    sp.get()->assign(std::move(contents));
    sp.get()->ensureUTF8();

	auto parser = new TomlReader();
	parser->setData(sp);
	return parser->doParse();
}

TomlReader::TomlReader() :
	_myrex(nullptr),
	_table(nullptr),
	_root(nullptr),
	_stackTop(0),
	_expSetId(0)
{
	_myrex = Rex::getGlobalRex();
    _src = std::make_shared<UStrData>();

	// Reference counted objects ??

	_ts = new Token8Stream();
	_v_ts = Php::Object(Token8Stream::PHP_NAME, _ts);

	//
	_ts->fn_setEOS(Rex::EOS);
	_ts->fn_setEOL(Rex::Newline);
	_ts->fn_setUnknown(Rex::AnyChar);

	_remap = _myrex->_re8->_remap;

	_ts->fn_setMap(_remap);
	_ts->fn_setSingles(_myrex->_singles);

	this->setExpSet(eKey);
}

void TomlReader::popExpSet()
{
	int temp = _expStack.back();
	_expStack.pop_back();
	setExpSet(temp);
}

void TomlReader::pushExpSet(int id)
{
	_expStack.push_back(_expSetId);
	this->setExpSet(id);
}

void TomlReader::setExpSet(int id)
{
	_expSetId = id;
	switch(id) {
		case eKey:
			_ts->setExpSet(_myrex->_keyRe);
			break;
		case eScaler:
			_ts->setExpSet(_myrex->_valueRe);
			break;
		case eLString:
			_ts->setExpSet(_myrex->_lstrRe);
			break;
		case eBString:
			_ts->setExpSet(_myrex->_estrRe);
			break;
		case eMLString:
			_ts->setExpSet(_myrex->_mlstrRe);
			break;
		default:
			throw Php::Exception("TomlReader::setExpSet invalid constant");
			break;
	}
}

//* try to ensure entire input is UTF-8, convert, rewrite as necessary.
//* if original UTF-8 with BOM, leave BOM in place, return offset.



Php::Value TomlReader::parse(Php::Parameters &param)
{
    if (param.size() < 1) {
        throw Php::Exception("Expected string or UStr8 object");
    }
    Php::Value& val = param[0];
    if (val.isString()) {
        _src.get()->assign(val, val.size());
    }
    else if (val.isObject()) {
        if (val.instanceOf(UStr8::PHP_NAME)) {
            auto u8 = (UStr8*) val.implementation();
            _src = u8->_str;
        }
    }

	return doParse();
}


Php::Value TomlReader::doParse()
{
	try {

		_ts->fn_setString(_src);
		//Php::out << "Source length " << _src.get()->_view.size() << std::endl;
		_root = new KeyTable();
		_v_root = Php::Object(KeyTable::PHP_NAME, _root);
		_table  = _root;

		auto tokenId = _ts->fn_moveNextId();
		unsigned int loopct = 0;
		while (tokenId != Rex::EOS)
		{
			loopct++;
			//Php::out << loopct << ": " << _ts->fn_beforeChar(10) << " ExpSet " << fn_getExpSetId() << std::endl;
			//Php::out << "id: " << tokenId <<  " pos: " <<  _ts->fn_getOffset() << std::endl;
			switch(tokenId)
			{
			case Rex::Hash:
				parseComment();
				tokenId = _ts->fn_moveNextId();
				break;
			case Rex::Space:
			case Rex::Newline:
				tokenId = _ts->fn_moveNextId();
				break;
			case Rex::Quote1:
			case Rex::BareKey:
			case Rex::Apost1:
			case Rex::Integer:
				parseKeyValue();
				tokenId = finishLine();
				break;
			case Rex::LSquare:
				parseTablePath();
				tokenId = finishLine();
				break;
			default:
				syntaxError("Expect Key = , [Path] or # Comment");
				break;
			}
		}
	}
	// some exceptions don't know the parse context, so all line, value
	// context information to be added here.
	catch (Php::Exception& oh_no) {
			std::stringstream ss;
			ss << "Toml Parse at line " << _ts->getLine() << ". " << oh_no.what();
			const std::string& value = _ts->getValue();
			if (value.size() > 0) {
				ss << ". Value { " << value << " }.";
			}
			else {
				ss << ".";
			}
			throw Php::Exception(ss.str());
	}
	return _v_root;
}

void TomlReader::syntaxError(const std::string& s) {
	syntaxError(s.data());
}

void TomlReader::valueError(const char* emsg, const std::string& value) {
	std::string msg = emsg;
	msg += ". Value { " + value + " }";
	throw Php::Exception(msg);
}
void TomlReader::syntaxError(const char* msg)
{
	throw Php::Exception(msg);
}

void  TomlReader::parseComment()
{
	_ts->fn_moveRegId(Rex::CommentStuff);
}

int TomlReader::finishLine() {
	// Php::out << "Finish pos: " <<  _ts->fn_getOffset() << std::endl;
	_ts->fn_moveRegId(Rex::HashComment);
	int result = _ts->fn_moveNextId();
	if (result != Rex::Newline && result != Rex::EOS) {
		syntaxError("Expected NEWLINE or EOS");
	}
	// Php::out << "Line end pos: " <<  _ts->fn_getOffset() << " " << _ts->fn_getValue() << std::endl;
	return result;
}

void TomlReader::parseKeyName(std::string& name)
{
	auto id = _ts->fn_getId();
	switch(id) {
	case Rex::BareKey: {
		 	auto sv = _ts->fn_getValue();
		 	name.assign(sv.data(), sv.size());
		 }
		 //_ts->fn_moveValue(name);
		 break;
	case Rex::Quote1:
		this->parseQString(name);
		break;
	case Rex::Apost1:
		this->parseLitString(name);
		break;
	case Rex::Integer:
		this->parseInteger(name);
		break;
	default:
		this->syntaxError("Improper Key");
		break;
	}

}

void TomlReader::parseQString(std::string& sval)
{
	this->pushExpSet(eBString);
	auto id = _ts->fn_moveNextId();
    svx::string_view temp;
	while(id != Rex::Quote1) {
		if (id == Rex::Newline || id == Rex::EOS || id == Rex::Escape)
		{
			this->syntaxError("parseEscString: Unfinished string value");
		}
		else if (id == Rex::EscapedChar) {
			sval += escString(_ts->fn_getValue());
		}
		else {
            temp = _ts->fn_getValue();
			sval.append(temp.data(), temp.size());
		}
		id = _ts->fn_moveNextId();
	}
	this->popExpSet();
	return;
}

void TomlReader::invalidEscChar(char eChar) {
	std::stringstream ss;
	ss << "Invalid escaped character chr(" << int(eChar) << ")";
	syntaxError(ss.str());
}
/*
bool TomlReader::fn_moveLiteralStr(svx::string_view& view) {
	Token8 token;
	bool loop = true;
	auto before = _ts->fn_getOffset();
	while(loop) {
		// only detects singles, EOS, Newline
		_ts->fn_peekToken(&token);
		switch(token._id) {
		case Rex::Apost1:
		case Rex::Newline:
		case Rex::EOS:
			loop = false;
			break;
		case Rex::AnyChar:
		default:
			char32_t check = _ts->fn_getChar32();
			if (check < 0x20) {
				loop = false;
			}
			else {
				_ts->fn_acceptToken(&token);
			}
			break;
		}

	}
	auto after =  _ts->fn_getOffset();
	if (after > before) {
		view = _ts->fn_substr(before, after - before);
		return true;
	}
	return false;
}
*/
void TomlReader::parseLitString(std::string& sval)
{
	//std::stringstream result;
	//svx::string_view  value;

	_ts->fn_peekToken(&_token);
	while (_token._id != Rex::Apost1) {
		if (_token._id == Rex::Newline || _token._id == Rex::EOS) {
			syntaxError("String value missing closing quote ( ' )");
		}
		// a literal string is just no control characters,
		// and no / (x27)
		if (_ts->fn_moveRegId(Rex::LitString)) {
            svx::string_view temp = _ts->fn_getValue();
			sval.append(temp.data(), temp.size()) ;
		}
		//if (this->fn_moveLiteralStr(value)) {
		//	result << value;
		//}
		else {
			syntaxError("Bad literal string value");
		}
		_ts->fn_peekToken(&_token);
	}
	_ts->fn_acceptToken(&_token);
}

std::string
TomlReader::escString(svx::string_view sval) {
    std::string result;

	//Php::out << "parseEscChar len " << sval.size() << ": " << sval << std::endl;
	char val = sval.at(1);
	switch(val) {
	case 'n':
		result += "\n";
		break;
	case 't':
		result += '\t';
		break;
	case 'r':
		result += '\r';
		break;
	case 'b':
		result += '\b';
		break;
	case 'f':
		result += '\f';
		break;
	case '\"':
		result += '\"';
		break;
	case '\\':
		result += '\\';
		break;
	case 'u':
		result = pun::hexUniStr8(sval.substr(2,4));
		break;
	case 'U':
		result = pun::hexUniStr8(sval.substr(2,8));
		break;
	default:
		invalidEscChar(val);
		break;
	}
	return result;
}

void
TomlReader::parseMLString(std::string& sval)
{
//	std::stringstream result;
    svx::string_view temp;

	pushExpSet(eMLString);

	auto id = _ts->fn_moveNextId();
	if (id == Rex::Newline) {
		id = _ts->fn_moveNextId();
	}
	bool doLoop = true;
	while(doLoop) {
		switch(id) {
		case Rex::Newline:
            //TODO:  platform newline?
			sval += "\n";
			id = _ts->fn_moveNextId();
			break;
		case Rex::Apost3:
			doLoop = false;
			break;
		case Rex::EOS:
			syntaxError("Missing ''' at end");
			break;
		default:
            temp = _ts->fn_getValue();
			sval.append(temp.data(), temp.size());
			id = _ts->fn_moveNextId();
			break;
		}
	}
	popExpSet();
}


void  TomlReader::parseKeyValue()
{
	std::string keyName;

	this->parseKeyName(keyName);

	if (_table->fn_hasK(keyName)) {
		this->syntaxError("Duplicate key");
	}

	if (! _ts->fn_moveRegId(Rex::SpacedEqual) )
	{
		this->syntaxError("Expected: equal { = }");
	}
	_ts->fn_peekToken(&_token);
	Php::Value rhsValue;
	//Php::out << "KeyVal id " << _token._id << " = " <<  _token._value << std::endl;
	if (_token._id == Rex::LSquare) {
		_ts->fn_acceptToken(&_token);
		// ValueList throws exceptions without line context

			ValueList* vlist = new ValueList();
			rhsValue = Php::Object(ValueList::PHP_NAME, vlist);
			parseArray(vlist);
			//Php::out << "End parse Array" << std::endl;
	}
	else if (_token._id == Rex::LCurly) {
		_ts->fn_acceptToken(&_token);
		KeyTable* ktab = new KeyTable();
		rhsValue = Php::Object(KeyTable::PHP_NAME, ktab);
		auto oldTable = _table;
		_table = ktab;
		parseInlineTable();
		_table = oldTable;
	}
	else {
		pun::Pype punt; // anything goes here
		parseValue(rhsValue, punt);
	}

	_table->fn_setKV(keyName, rhsValue);
	//Php::out << "Set " << keyName << " value " << rhsValue.debugZval() << std::endl;
}

void TomlReader::arrayMatchError(pun::Pype spunt, pun::Pype punt)
{
	std::stringstream ss;

	ss << "Cannot add value of type " << pun::getPypeId(punt)
	   << " to ValueList of type " << pun::getPypeId(spunt);
	throw Php::Exception(ss.str());

}
void TomlReader::parseArray(ValueList *vlist)
{
	_ts->fn_peekToken(&_token);

	bool doLoop = true;
	while(doLoop) {
		switch(_token._id) {
		case Rex::Space:
			_ts->fn_moveRegId(Rex::Space);
			_ts->fn_peekToken(&_token);
			break;
		case Rex::Newline:
			_ts->fn_acceptToken(&_token);
			_ts->fn_peekToken(&_token);
			break;
		case Rex::Hash:
			_ts->fn_acceptToken(&_token);
			parseComment();
			_ts->fn_peekToken(&_token);
			break;
		case Rex::EOS:
			_ts->fn_acceptToken(&_token);
			syntaxError("parseArray: missing bracket - ]");
			break;
		case Rex::RSquare:
			_ts->fn_acceptToken(&_token);
		default:
			doLoop = false;
			break;
		}
	}

	while(_token._id != Rex::RSquare)
	{
		if (_token._id == Rex::LSquare) {
			_ts->fn_acceptToken(&_token);
			auto ct = vlist->fn_size();
			if (ct == 0) {
				vlist->fn_setTag(Php::Value(pun::tValueList));
			}
			else {
				Php::Value &spun = vlist->fn_getTag();
				pun::Pype spunt = (pun::Pype)((int) spun);
				if (spunt != pun::tValueList) {
					arrayMatchError(spunt, pun::tValueList);
				}
			}
			ValueList *subList = new ValueList();
			Php::Value subArray(Php::Object(ValueList::PHP_NAME,subList));
			vlist->fn_pushBack(subArray);
			vlist->fn_setTag(Php::Value(pun::tValueList));
			parseArray(subList);

		}
		else {
			Php::Value val;
			pun::Pype  punt;

			parseValue(val,punt);
			auto ct = vlist->fn_size();
			if (ct == 0) {
				vlist->fn_setTag(Php::Value(punt));
			}
			else {
				Php::Value &spun = vlist->fn_getTag();
				pun::Pype spunt = (pun::Pype)((int) spun);
				if (punt != spunt) {
					arrayMatchError(spunt, punt);
				}
			}
			vlist->fn_pushBack(val);
		}
		doLoop = true;
		bool gotComma = false;

		_ts->fn_peekToken(&_token);
		while(doLoop) {
			switch(_token._id) {
			case Rex::Space:
				_ts->fn_moveRegId(Rex::Space);
				_ts->fn_peekToken(&_token);
				break;
			case Rex::Newline:
				_ts->fn_acceptToken(&_token);
				_ts->fn_peekToken(&_token);
				break;
			case Rex::Hash:
				_ts->fn_acceptToken(&_token);
				parseComment();
				_ts->fn_peekToken(&_token);
				break;
			case Rex::Comma:
				if (gotComma) {
					syntaxError("parseArray: No value between commas");
				}
				else {
					gotComma = true;
				}
				_ts->fn_acceptToken(&_token);
				_ts->fn_peekToken(&_token);
				break;
			case Rex::RSquare:
				//Php::out << "End of Array" << std::endl;
				_ts->fn_acceptToken(&_token);
			default:
				doLoop = false;
				break;
			}
		}
	}
}

void TomlReader::parseInlineTable()
{
	auto id = _ts->fn_moveNextId();
	if (id == Rex::Space) {
		id = _ts->fn_moveNextId();
	}

	if (id != Rex::RCurly) {
		parseKeyValue();
		id = _ts->fn_moveNextId();
		if (id == Rex::Space){
			id = _ts->fn_moveNextId();
		}
	}
	while(id == Rex::Comma) {
		id = _ts->fn_moveNextId();
		if (id == Rex::Space){
			id = _ts->fn_moveNextId();
		}
		parseKeyValue();
		id = _ts->fn_moveNextId();
		if (id == Rex::Space){
			id = _ts->fn_moveNextId();
		}
	}
	if (id != Rex::RCurly) {
		syntaxError("Inline table expected final bracket - } ");
	}

}

void TomlReader::throw_notFullMatch(const std::string& target, const std::string& cap)
{
	std::string msg = "Value { ";
	msg += cap + " } is not full match for { " + target + " }";
	syntaxError(msg);
}
void TomlReader::fn_checkFullMatch(const std::string& target, const std::string& cap)
{
	if (target.size() > cap.size()) {
		throw_notFullMatch(target, cap);
	}
}

// assume peekToken as just occurred, return value in val.
void TomlReader::parseValue(Php::Value& val, pun::Pype& punt)
{
	std::string sval;
	//Php::out << "Val id " << _token._id << " = " <<  _token._value << std::endl;
	if (_token._id == Rex::Apost1 )
	{

		if (_ts->fn_moveRegId(Rex::Apost3)) {

			parseMLString(sval);

		}
		else {
			_ts->fn_acceptToken(&_token);
			parseLitString(sval);
		}
		punt = pun::tString;
		val = sval;
		return;
	}
	else if (_token._id == Rex::Quote1) {

		if (_ts->fn_moveRegId(Rex::Quote3))
		{
			parseMLQString(sval);
		}
		else {
			_ts->fn_acceptToken(&_token);

			parseQString(sval);
		}
		punt = pun::tString;
		val = sval;
		return;
	}
	//Php::out << "value " << _ts->fn_beforeChar(10) << std::endl;
	if (!_ts->fn_moveRegId(Rex::AnyValue)) {
		syntaxError("No value after = ");
	}
	else {
		_valueText = _ts->fn_getValue();
		Pcre8_match matches;
        auto ct = _remap.get()->match(_valueText, Rex::Integer, matches);
		auto valueLen = _valueText.size();
		if (ct > 1) {
			const std::string& match = matches._slist[1];
			if (match.size() == valueLen) {
				parseInteger(sval);
				val = std::stol(sval);
				punt = pun::tInteger;
				return;
			}
		}
		ct = _remap.get()->match(_valueText, Rex::FloatExp, matches);
		if (ct > 1) {
			const std::string& match = matches._slist[1];
			if (match.size() == valueLen) {
				parseFloatExp(val);
				punt = pun::tFloat;
			}
			return;
		}
		ct = _remap.get()->match(_valueText, Rex::FloatDot, matches);
		if (ct > 1) {
			const std::string& match = matches._slist[1];
			if (match.size() == valueLen)
			{
				parseFloat(val,matches);
				punt = pun::tFloat;
				return;
			}
		}
		ct = _remap.get()->match(_valueText, Rex::Bool, matches);
		if (ct > 1) {
			const std::string& match = matches._slist[1];
			if (match.size() == valueLen)
			{
				bool bresult =  (match == "true") ? true : false;
				punt = pun::tBool;
		    	val = bresult;
				return;
			}
		}
		ct = _remap.get()->match(_valueText, Rex::DateTime, matches);
		if (ct > 1) {
			const std::string& match = matches._slist[1];
			if (match.size() == valueLen)
			{
				parseDateTime(val);
				punt = pun::tDateTime;
				return;
			}
			else {
				throw_notFullMatch(std::string(_valueText.data(), _valueText.size()), match);
			}
		}
	}
	auto sview = _ts->fn_getValue();
	std::string errMsg = "No value type match found for " + std::string(sview.data(), sview.size());
	// Undo (_ts->fn_getValue()) before throwing.
	//Php::out << "Restored = " << _ts->fn_getValue() << std::endl;
	syntaxError(errMsg);
}

void TomlReader::parseInteger(std::string& val)
{
	Pcre8_match matches;
	// don't need expression check if no underscores in string
	// reference to full underlying string
	// skip the sign
	svx::string_view int_str(_valueText);
	auto firstChar = int_str[0];
    uint64_t prefix = 0;
	if (firstChar == '-' || firstChar == '+') {
		int_str.remove_prefix(1);
		prefix = 1;
	}

	auto fpos = int_str.find_first_of('_');

	if (fpos != svx::string_view::npos) {
		bool valid = true;
		if (fpos == 0) {
			valid = false;
		}
		else {
			fpos = int_str.find_last_of('_');
			if (fpos == int_str.size()-1) {
				valid = false;
			}
		}
		val.assign(_valueText.data(), _valueText.size());
		if (!valid) {
			valueError("Invalid integer: Underscore must be between digits", val);
		}

		val.erase(std::remove(val.begin(), val.end(), '_'), val.end());
        int_str = svx::string_view(val.data() + prefix, val.size() - prefix);
	}
	else {
        val.assign(_valueText.data(), _valueText.size());
	}
	if (int_str[0] == '0' && int_str.size() > 1){
		valueError("Invalid integer: Leading zeros not allowed", val);
	}
}

void TomlReader::parseFloatExp(Php::Value& val)
{
	Pcre8_match matches;
	auto ct = _remap.get()->match(_valueText, Rex::Float_E, matches);
    std::string copy(_valueText.data(), _valueText.size());

	if (ct > 0) {
		valueError("Invalid float with exponent: Underscore must be between digits", copy);
	}
	copy.erase(std::remove(copy.begin(), copy.end(), '_'), copy.end());

	_valueText = svx::string_view(copy.data(), copy.size());
	ct = _remap.get()->match(_valueText, Rex::No_0Digit, matches);
	if (ct > 0) {
		valueError("Invalid float with exponent: Underscore must be between digits", copy);
	}
	//Php::Value sval = _valueText.str();
	//val = sval.floatValue();
	val = std::stod(copy);
}


void TomlReader::parseFloat(Php::Value& val, Pcre8_match& matches)
{
    std::string copy(_valueText.data(), _valueText.size());
	if (matches._slist.size() < 5) {
		valueError("Wierd Float Capture",copy);
	}

	if (matches._slist[4].size() <= 1) {
		valueError("Float needs at least one digit after decimal point", copy);
	}

    auto ct = _remap.get()->match(_valueText, Rex::Dig_Dig, matches);
    if (ct > 0) {
		valueError("Invalid float: Underscore must be between digits", copy);
	}
	copy.erase(std::remove(copy.begin(), copy.end(), '_'), copy.end());

	_valueText = svx::string_view(copy.data(), copy.size());
	ct = _remap.get()->match(_valueText, Rex::No_0Digit, matches);
	if (ct > 0) {
		valueError("Invalid float: Leading zeros not allowed",copy);
	}
	//Php::Value sval = _valueText.str();
	//val = sval.floatValue();
	val = std::stod(copy);

}
void TomlReader::parseDateTime(Php::Value& val)
{
    std::string copy(_valueText.data(), _valueText.size());

	val = Php::Object("DateTime", copy);
}

void TomlReader::parseMLQString(std::string& sval) {
    svx::string_view temp;

	pushExpSet(eBString);

	auto id = _ts->fn_moveNextId();
	// Newline straight after opening quotes is ignored
	if (id == Rex::Newline) {
		id = _ts->fn_moveNextId();
	}
	bool doLoop = true;
	while(doLoop) {
		switch(id) {
		case Rex::Quote3:
			doLoop = false;
			break;
		case Rex::EOS:
			syntaxError("Missing \"\"\" at end");
			break;
		case Rex::Escape:
			do {
				id = _ts->fn_moveNextId();
			}
			while(id == Rex::Space || id == Rex::Newline || id == Rex::Escape);
			break;
		case Rex::Space:
			sval +=  ' ';
			id = _ts->fn_moveNextId();
			break;
		case Rex::Newline:
			sval += "\n";
			id = _ts->fn_moveNextId();
			break;
		case Rex::EscapedChar:
            sval += escString(_ts->fn_getValue());
			id = _ts->fn_moveNextId();
			break;
		default:
            temp = _ts->fn_getValue();
            sval.append(temp.data(), temp.size());
			id = _ts->fn_moveNextId();
			break;
		}
	}
	popExpSet();
}

// make a descriptive path for generating an exception
static std::string sfn_makePath(std::vector<TomlPartTag> & parts, bool withIndex = true)
{
	std::stringstream result;

	auto ait = parts.begin();
	auto zit = parts.end();
	while (ait != zit) {
		TomlBase* b = ait->_base;
		auto tag = b->fn_getPathTag();
		if (tag->_objAOT) {
			result << "[" << ait->_part;
			if (withIndex) {
				result << "/" << b->fn_endIndex();
			}
			result << "]";
		}
		else {
			result << "{" << ait->_part << "}";
		}
		ait++;
	}
	return result.str();
}

static KeyTable* sfn_pushKeyTable(std::string& key, KeyTable* ktab)
{
	KeyTable* result = new KeyTable();
	Php::Value val( std::move(Php::Object(KeyTable::PHP_NAME, result)));
	ktab->fn_setKV(key, val);
	return result;
}

static ValueList* sfn_setNewAOT(std::string& key, KeyTable* ktab)
{
	ValueList* v = new ValueList();
	Php::Value val( std::move(Php::Object(ValueList::PHP_NAME, v)));
	ktab->fn_setKV(key, val);
	return v;
}
static KeyTable* sfn_pushNewTable(ValueList* vlist) {
	KeyTable* result = new KeyTable();
	Php::Value val(Php::Object(KeyTable::PHP_NAME, result));
	vlist->fn_pushBack(val);
	return result;
}

void TomlReader::parseTablePath()
{
	std::string partName;
	bool isAOT = false;
	bool hitNew = false;
	std::vector<TomlPartTag> parts;
	int  dotCount = 0;
	int  aotLength = 0;
	TomlBase*  testObj = nullptr;
	ValueList* tableList = nullptr;
	KeyTable*  pObj = _root;
	Php::Value val;

	auto id = _ts->fn_moveNextId();
	bool doLoop = true;

	while(doLoop) {
		switch(id) {
		case Rex::Hash:
			syntaxError("Unexpected '#' in path");
			break;
		case Rex::Equal:
			syntaxError("Unexpected '=' in path");
			break;
		case Rex::Space:
			id = _ts->fn_moveNextId();
			break;
		case Rex::EOS:
			doLoop = false;
			break;
		case Rex::Newline:
			syntaxError("New line in unfinished path");
			break;
		case Rex::RSquare:
			if (isAOT) {
				if (aotLength == 0) {
					syntaxError("AOT Segment cannot be empty");
				}
				isAOT = false;
				aotLength = 0;
				id = _ts->fn_moveNextId();
			}
			else {
				doLoop = false;
			}
			break;
		case Rex::LSquare:
			if (dotCount < 1 && parts.size() > 0) {
				syntaxError("Expected a '.' after path part");
			}
			if (isAOT) {
				syntaxError("Too many consecutive [ in path");
			}
			id = _ts->fn_moveNextId();
			isAOT = true;
			break;
		case Rex::Dot:
		    if (dotCount > 0) {
		    	syntaxError("Too many consecutive . . in path");
		    }
			dotCount++;
			id = _ts->fn_moveNextId();
			break;
		case Rex::Quote1:
		default:
			TomlPartTag otag(isAOT);
			parseKeyName(otag._part);
			if (dotCount < 1 && parts.size() > 0) {
				syntaxError("Expected a '.' after path part");
			}
			dotCount = 0;
			if (pObj->fn_hasK(otag._part)) {
				// There's a key-value stored already
				val = pObj->fn_getV(otag._part);
				if (val.isObject()) {
					// There are only two kinds of object inserted here, both are TomlBase;
					testObj = reinterpret_cast<TomlBase*>(val.implementation());
				}
				else {
					// its a value and we want to set an object
					std::stringstream ss;
					ss << "Duplicate key path: " << sfn_makePath(parts) << "." << otag._part;
					syntaxError(ss.str());
				}
				auto tag = testObj->fn_getPathTag();

				if (tag->_objAOT) {
					aotLength++;
					tableList = reinterpret_cast<ValueList*>( testObj);
					val = tableList->back();
					pObj = reinterpret_cast<KeyTable*>(val.implementation());
				}
				else {
					pObj = reinterpret_cast<KeyTable*>(testObj);
				}
			}
			else {
				// make new branch
				if (!hitNew) {
					hitNew = true;
				}
				// first branch declaration has to be correct
				if (isAOT) {
					aotLength++;
					tableList = sfn_setNewAOT(otag._part, pObj);
					pObj = sfn_pushNewTable(tableList);
					testObj = reinterpret_cast<TomlBase*>(tableList);
				}
				else {
					pObj = sfn_pushKeyTable(otag._part, pObj);
					testObj = reinterpret_cast<TomlBase*>(pObj);
				}
				PathTag* tag = new PathTag();

				//tag._part = partName;
				tag->_objAOT = isAOT;
				tag->_implicit = true;

				testObj->fn_setPathTag(tag);
			}
			otag._base = testObj;

			parts.push_back(std::move(otag));
			id = _ts->fn_moveNextId();
			break;
		}
	}
	if (parts.size() == 0) {
		syntaxError("Table path cannot be empty");
	}
	if (!hitNew) {
		// check last object && last parts value
		TomlPartTag& otag = parts.back();
		auto tag = testObj->fn_getPathTag();
		if (tag->_objAOT) {
			if (otag._isAOT) {
				pObj = sfn_pushNewTable(reinterpret_cast<ValueList*>(testObj));
			}
			else {
				// Have an AOT already, but just asked for plain table
				std::stringstream ss;
				ss << "Table path mismatch with " << sfn_makePath(parts, false);
				syntaxError(ss.str());
			}
		}
		else {
			// Table part created earlier, allow if it was just implicit creation
			if (tag->_implicit) {
				tag->_implicit = false; // won't allow this to happen again
			}
			else {
				std::stringstream ss;
				ss << "Duplicate key path: " << sfn_makePath(parts, false);
				syntaxError(ss.str());
			}
		}
	}
	else {
		auto tag = testObj->fn_getPathTag();
		tag->_implicit = false; // end of path cannot be implicit
	}
	_table = pObj;
	//Php::out << "New table path is " << sfn_makePath(parts, true) << std::endl;
}

