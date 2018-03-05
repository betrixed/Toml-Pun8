#include "tomlreader.h"
#include "keytable.h"
#include "valuelist.h"
#include "re8map.h"
#include "ucode8.h"
#include "token8stream.h"

#include <ostream>
#include <sstream>
#include <limits>
#include <algorithm>

const char* const cRexBool = "^(true|false)";
const char* const cDateTime = "^(\\d{4}-\\d{2}-\\d{2}(T\\d{2}:\\d{2}:\\d{2}(\\.\\d{6})?(Z|-\\d{2}:\\d{2})?)?)";
const char* const cFloatExp = "^([+-]?((\\d_?)+([\\.](\\d_?)*)?)([eE][+-]?(_?\\d_?)+))";
const char* const cFloatDot = "^([+-]?((\\d_?)+([\\.](\\d_?)*)))";
const char* const cInteger = "^([+-]?((\\d_?)+([\\.](\\d_?)*)))";

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
const char* const cNo_0Digit = "^(0\\d+)";
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
	mptr->setRex(pun::makeSharedRe(Rex::No_0Digit, cNo_0Digit, strlen(cNo_0Digit) ));
	mptr->setRex(pun::makeSharedRe(Rex::Float_E, cFloat_E, strlen(cFloat_E) ));

	_singles.reserve(31);

	_singles.insert( { 
		{"=", Rex::Equal}, 
		{"[", Rex::LSquare},
		{"]", Rex::RSquare},
		{".", Rex::Dot},
		{".", Rex::Comma},
		{"{", Rex::LCurly},
		{"}", Rex::RCurly},
		{"'", Rex::Apost1},
		{"#", Rex::Hash},
		{"\\", Rex::Escape},
		{" ", Rex::Space},
		{"\t", Rex::Space},
		{"\f", Rex::Space},
		{"\b", Rex::Space}
	});

	_keyRe = std::move(std::vector<int>( {Rex::Space, Rex::BareKey, Rex::Integer}));
	_valueRe = std::move(std::vector<int>( {Rex::Bool, Rex::DateTime, Rex::FloatExp, Rex::FloatDot, Rex::Integer}));
	_estrRe = std::move(std::vector<int>( {Rex::Space, Rex::NoEscape, Rex::EscapedChar, Rex::Quote3}));
	_lstrRe = std::move(std::vector<int>( {Rex::LitString }));
	_mlstrRe = std::move(std::vector<int>( {Rex::LitString, Rex::Apost3}));
}


Rex::~Rex()
{
	delete _re8;
}
TomlReader::~TomlReader()
{
	delete _ts;
	delete _root;
}

TomlReader::TomlReader() : 
	_myrex(nullptr), _ts(nullptr), 
	_root(nullptr), _table(nullptr), 
	_stackTop(0), _expSetId(0)
{
	_myrex = Rex::getGlobalRex();
	_root = new KeyTable();
	_table  = _root;
	_ts = new Token8Stream();
	_ts->fn_setEOS(Rex::EOS);
	_ts->fn_setEOL(Rex::Newline);
	_ts->fn_setUnknown(Rex::AnyChar);
	_ts->fn_setRe8map(_myrex->_re8);
	_valueText.fn_setRe8map(_myrex->_re8->_remap);
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

Php::Value TomlReader::parse(Php::Parameters& param)
{
	pun::check_String(param,0);
	_ts->setString((const char*) param[0], param[0].size());
	_table = _root;

	auto tokenId = _ts->fn_moveNextId();
	while (tokenId != Rex::EOS)
	{
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
			tokenId = parseTablePath();
			break;
		default:
			syntaxError("Expect Key = , [Path] or # Comment");
			break;
		}
	}
	return Php::Object(KeyTable::PHP_NAME,_root);
}

void TomlReader::syntaxError(const std::string& s) {
	syntaxError(s.data());
}

void TomlReader::syntaxError(const char* msg)
{
	_ts->fn_getToken(_token);
	std::stringstream ss;

	ss << "Error line " <<  _token._line << ": " << msg;

	if (_token._value.size() > 0) {
		ss << ". Value { " << _token._value << " }.";
	}
	else {
		ss << ".";
	}
	throw Php::Exception(ss.str());
}

void  TomlReader::parseComment()
{
	_ts->fn_moveRegId(Rex::CommentStuff);
}

int TomlReader::finishLine() {
	_ts->fn_moveRegId(Rex::HashComment);
	int result = _ts->fn_moveNextId();
	if (result != Rex::Newline && result != Rex::EOS) {
		syntaxError("Expected NEWLINE or EOS");
	}
	return result;
}

void TomlReader::parseKeyName(std::string& name)
{
	auto id = _ts->fn_getId();
	switch(id) {
	case Rex::BareKey:
		 _ts->fn_moveValue(name);
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

void TomlReader::parseQString(std::string& val)
{
	this->pushExpSet(eBString);
	auto id = _ts->fn_moveNextId();
	std::stringstream result;

	while(id != Rex::Quote1) {
		if (id == Rex::Newline || id == Rex::EOS || id == Rex::Escape)
		{
			this->syntaxError("parseEscString: Unfinished string value");
		}
		else if (id == Rex::EscapedChar) {
			this->parseEscChar(result);
		}
		else {
			result << _ts->fn_moveValue(val);
		}
		id = _ts->fn_moveNextId();
	}
	this->popExpSet();

	val = std::move(result.str());
	return;
}

void TomlReader::invalidEscChar(char eChar) {
	std::stringstream ss;
	ss << "Invalid escaped character chr(" << int(eChar) << ")";
	syntaxError(ss.str());	
}


void TomlReader::parseLitString(std::string& val)
{
	std::stringstream result;

	_ts->fn_peekToken(&_token);
	while (_token._id != Rex::Apost1) {
		if (_token._id == Rex::Newline || _token._id == Rex::EOS) {
			syntaxError("String value missing closing quote ( ' )");
		}
		if (_ts->fn_moveRegId(Rex::LitString)) {
			_ts->fn_moveValue(val);

			result << val;
		}
		else {
			syntaxError("Bad literal string value");
		}
		_ts->fn_peekToken(&_token);
	}
	_ts->fn_acceptToken(&_token);
	val = result.str();
}

void  TomlReader::parseEscChar(std::ostream& os)
{
	char val = _token._value.at(1);
	switch(val) {
	case 'n':
		os << '\n';
		break;
	case 't':
		os << '\t';
		break;
	case 'r':
		os << '\r';
		break;
	case 'b':
		os << '\b';
		break;
	case 'f':
		os << '\f';
		break;
	case '\"':
		os << '\"';
		break;
	case '\\':
		os << '\\';
		break;
	case 'u':
		pun::hexUniStr8(_token._value.substr(2,4),os);
		break;
	case 'U':
		pun::hexUniStr8(_token._value.substr(2,8),os);
		break;
	default:
		invalidEscChar(val);
		break;
	}
	return;
}

void  TomlReader::parseMLString(std::string& val)
{
	std::stringstream result;

	pushExpSet(eMLString);

	auto id = _ts->fn_moveNextId();
	if (id == Rex::Newline) {
		id = _ts->fn_moveNextId();
	}
	bool doLoop = true;
	while(doLoop) {
		switch(id) {
		case Rex::Newline:
			result << std::endl;
			id = _ts->fn_moveNextId();
			break;
		case Rex::Apost3:
			doLoop = false;
			break;
		case Rex::EOS:
			syntaxError("Missing ''' at end");
			break;
		default:
			result << _ts->fn_moveValue(val);
			id = _ts->fn_moveNextId();
			break;
		}
	}
	val = std::move(result.str());
	popExpSet();
}


void  TomlReader::parseKeyValue()
{
	std::string keyName;

	this->parseKeyName(keyName);

	if (_table->hasKey(keyName)) {
		this->syntaxError("Duplicate key");
	}

	if (! _ts->fn_moveRegId(Rex::SpacedEqual) )
	{
		this->syntaxError("Expected: equal { = }");
	}
	_ts->fn_peekToken(&_token);
	Php::Value rhsValue;
	if (_token._id == Rex::LSquare) {
		_ts->fn_acceptToken(&_token);	
		ValueList* vlist = new ValueList();
		rhsValue = Php::Object(ValueList::PHP_NAME, vlist);
		parseArray(vlist);
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
		parseValue(rhsValue);
	}
	_table->fn_setKV(keyName, rhsValue);
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
			ValueList *subList = new ValueList();
			Php::Value subArray(Php::Object(ValueList::PHP_NAME,subList));
			vlist->fn_pushBack(subArray);
			parseArray(subList);
		}
		else {
			Php::Value scaler;
			parseValue(scaler);
			vlist->fn_pushBack(scaler);
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
			case Rex::RSquare:
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


void TomlReader::fn_checkFullMatch(const std::string& target, const std::string& cap) 
{
	if (target.size() > cap.size()) {
		std::stringstream ss;
		ss << "Value { " << cap << " } is not full match";
		syntaxError(ss.str());
	}
}
// assume peekToken as just occurred
void TomlReader::parseValue(Php::Value& val)
{
	std::string sval;
	if (_token._id == Rex::Apost1 )
	{
		
		if (_ts->fn_moveRegId(Rex::Apost3)) {
			
			parseMLString(sval);
			
		}
		else {
			_ts->fn_acceptToken(&_token);
			parseLitString(sval);
		}
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
		val = sval;
		return;
	}
	if (_ts->fn_moveRegId(Rex::AnyValue)) {
		_valueText.fn_setString(_ts->fn_getValue());
	}
	else {
		syntaxError("No value after = ");
	}
	Pcre8_match matches;

	int ct = _valueText.fn_matchRegId(Rex::Bool, matches);
	if (ct > 1) {
		val =  (matches._slist[1] == "true") ? true : false;
		return;
	}
	ct = _valueText.fn_matchRegId(Rex::DateTime, matches);
	if (ct > 1) {
		fn_checkFullMatch(_valueText.str(), matches._slist[1]);
		parseDateTime(val);
		return;
	}
	ct = _valueText.fn_matchRegId(Rex::FloatExp, matches);
	if (ct > 1) {
		fn_checkFullMatch(_valueText.str(), matches._slist[1]);
		parseFloatExp(val);
		return;
	}
	ct = _valueText.fn_matchRegId(Rex::FloatDot, matches);
	if (ct > 1) {
		fn_checkFullMatch(_valueText.str(), matches._slist[1]);
		parseFloat(val,matches);
		return;
	}
	ct = _valueText.fn_matchRegId(Rex::Integer, matches);
	if (ct > 1) {
		fn_checkFullMatch(_valueText.str(), matches._slist[1]);
		parseInteger(sval);
		val = sval;
		return;
	}
	std::stringstream ss;
	ss << "No value type match found for " << _valueText.str();
	syntaxError(ss.str());
}

void TomlReader::parseInteger(std::string& val)
{
	Pcre8_match matches;
	if (_valueText.fn_matchRegId(Rex::Dig_Dig, matches)) {
		syntaxError("Invalid integer: Underscore must be between digits");
	}
	std::string extract = _valueText.str();
	extract.erase(std::remove(extract.begin(), extract.end(), '_'), extract.end());

	_valueText.fn_setString(extract);
	if (_valueText.fn_matchRegId(Rex::No_0Digit, matches)) {
		syntaxError("Invalid integer: Leading zeros not allowed");
	}
	val = _valueText.str();
}

void TomlReader::parseFloatExp(Php::Value& val)
{
	Pcre8_match matches;
	if (_valueText.fn_matchRegId(Rex::Float_E, matches)) {
		syntaxError("Invalid float with exponent: Underscore must be between digits");
	}
	std::string extract = _valueText.str();
	extract.erase(std::remove(extract.begin(), extract.end(), '_'), extract.end());

	_valueText.fn_setString(extract);
	if (_valueText.fn_matchRegId(Rex::No_0Digit, matches)) {
		syntaxError("Invalid float with exponent: Underscore must be between digits");
	}
	val = std::stof(_valueText.str());
}


void TomlReader::parseFloat(Php::Value& val, Pcre8_match& matches)
{
	if (matches._slist.size() < 5) {
		syntaxError("Wierd Float Capture");
	}

	if (matches._slist[4].size() <= 1) {
		syntaxError("Float needs at least one digit after decimal point");
	}

	if (_valueText.fn_matchRegId(Rex::Dig_Dig, matches)) {
		syntaxError("Invalid float: Underscore must be between digits");
	}
	std::string extract = _valueText.str();
	extract.erase(std::remove(extract.begin(), extract.end(), '_'), extract.end());

	_valueText.fn_setString(extract);
	if (_valueText.fn_matchRegId(Rex::No_0Digit, matches)) {
		syntaxError("Invalid float: Leading zeros not allowed");
	}
	val = std::stof(_valueText.str());
}
void TomlReader::parseDateTime(Php::Value& val)
{
	val = Php::call("DateTime", Php::Value(_valueText.str()));
}

void TomlReader::parseMLQString(std::string& sval) {
	pushExpSet(eBString);

	std::stringstream result;

	auto id = _ts->fn_moveNextId();
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
			result << " ";
			id = _ts->fn_moveNextId();
			break;
		case Rex::Newline:
			result << std::endl;
			id = _ts->fn_moveNextId();
			break;
		case Rex::EscapedChar:
			parseEscChar(result);
			id = _ts->fn_moveNextId();
			break;
		default:
			result << _ts->fn_getValue();
			id = _ts->fn_moveNextId();
			break;
		}
	}
	sval = std::move(result.str());
	popExpSet();
}

static std::string sfn_makePath(std::vector<TomlBase*> & parts, bool withIndex = true)
{
	std::stringstream result;

	auto ait = parts.begin();
	auto zit = parts.end();
	while (ait != zit) {
		TomlBase* b = *ait;
		auto tag = b->tomlTag();
		if (tag._objAOT) {
			result << "[" << tag._part;
			if (withIndex) {
				result << "/" << b->fn_endIndex();
			}
			result << "]";
		}
		else {
			result << "{" << tag._part << "}";
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

void TomlReader::parseObjectPath()
{
	std::string partName;
	bool isAOT = false;
	bool hitNew = false;

	std::vector<TomlBase*> parts;
	int  dotCount = 0;
	int  aotLength = 0;
	unsigned int firstNew = std::numeric_limits<unsigned int>::max();
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
			parseKeyName(partName);
			if (dotCount < 1 && parts.size() > 0) {
				syntaxError("Expected a '.' after path part");
			}
			dotCount = 0;
			if (pObj->hasKey(partName)) {
				val = pObj->fn_getV(partName);
				if (val.isObject()) {
					// There are only two kinds of object inserted here, both are TomlBase;
					testObj = reinterpret_cast<TomlBase*>(val.implementation());
				}
				else {
					std::stringstream ss;
					ss << "Duplicate key path: " << sfn_makePath(parts) << "." << partName;
					syntaxError(ss.str());
				}
				auto tag = testObj->tomlTag();
				tag._isAOT = isAOT;
				if (tag._objAOT) {
					aotLength++;
					tableList = reinterpret_cast<ValueList*>( testObj);
					val = tableList->getLast();
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
					firstNew = parts.size();
				}
				// first branch declaration has to be correct
				if (isAOT) {
					aotLength++;
					tableList = sfn_setNewAOT(partName, pObj);
					pObj = sfn_pushNewTable(tableList);
					testObj = reinterpret_cast<TomlBase*>(tableList);
				}
				else {
					pObj = sfn_pushKeyTable(partName, pObj);
					testObj = reinterpret_cast<TomlBase*>(pObj);
				}
				auto tag = testObj->tomlTag();
				tag._part = partName;
				tag._isAOT = isAOT;
				tag._objAOT = isAOT;
				// cannot set implicit yet, do not know the ending
			}
			parts.push_back(testObj);
			id = _ts->fn_moveNextId();
			break;
		}
	}
	if (parts.size() == 0) {
		syntaxError("Table path cannot be empty");
	}
	if (!hitNew) {
		auto tag = testObj->tomlTag();
		if (tag._objAOT) {
			if (tag._isAOT) {
				pObj = sfn_pushNewTable(reinterpret_cast<ValueList*>(testObj));
			}
			else {
				std::stringstream ss;
				ss << "Table path mismatch with " << sfn_makePath(parts, false);
				syntaxError(ss.str());
			}
		}
		else {
			if (tag._implicit) {
				tag._implicit = false;
			}
			else {
				std::stringstream ss;
				ss << "Duplicate key path: " << sfn_makePath(parts, false);
				syntaxError(ss.str());
			}
		}
	}
	else {
		auto partsCt = (unsigned int) parts.size() - 1;

		while ( firstNew < partsCt ) {
			testObj = parts[firstNew];
			auto tag = testObj->tomlTag();
			tag._implicit = true;
			firstNew++;
		}
	}
	_table = pObj;
}

int  TomlReader::parseTablePath()
{
	parseObjectPath();

	return finishLine();
}
