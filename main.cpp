#include <phpcpp.h>
#include "ucode8.h"
#include "pun8.h"
#include "pcre8.h"
#include "re8map.h"
#include "recap8.h"
#include "token8stream.h"
#include "keytable.h"
#include "valuelist.h"
#include "tomlreader.h"
/**
* Iterate through a UTF-8 string,
* Keep an internal array index to beginning of
* next character, as a byte index
* Allow the internal array index to be set, or get
* holds a string as a PHP value object
*/


/**
 *  tell the compiler that the get_module is a pure C function
 */

/**
 *  Function that is called by PHP right after the PHP process
 *  has started, and that returns an address of an internal PHP
 *  strucure with all the details and features of your extension
 *
 *  @return void*   a pointer to an address that is understood by PHP
 */

extern "C" {

PHPCPP_EXPORT void *get_module() 
{
    // static(!) Php::Extension object that should stay in memory
    // for the entire duration of the process (that's why it's static)
    static Php::Extension extension("punicode", "1.0");
    
    // @todo    add your own functions, classes, namespaces to the extension
    Php::Class<Pun8> punic(Pun8::PHP_NAME);

    punic.method<&Pun8::__construct>("__construct");
    
    // using regular expression id map.
    punic.method<&Pun8::matchMapId> ("matchMapId");
    punic.method<&Pun8::matchIdRex8> ("matchIdRex8");
    punic.method<&Pun8::firstMatch> ("firstMatch");
    
    punic.method<&Pun8::setIdRex> ("setIdRex");
    punic.method<&Pun8::getIdRex> ("getIdRex");
    punic.method<&Pun8::getIds> ("getIds");
    punic.method<&Pun8::setRe8map> ("setRe8map");
    punic.method<&Pun8::setIdList> ("setIdList");
    punic.method<&Pun8::getIdList> ("getIdList");

    // iteration of managed utf-8 string
    punic.method<&Pun8::setString> ("setString");
    punic.method<&Pun8::nextChar> ("nextChar");
    punic.method<&Pun8::peekChar> ("peekChar");
    punic.method<&Pun8::getCode> ("getCode");
    punic.method<&Pun8::getOffset> ("getOffset");
    punic.method<&Pun8::setOffset> ("setOffset");
    punic.method<&Pun8::addOffset> ("addOffset");

    extension.add(std::move(punic));
 
    Php::Class<Pcre8> preg(Pcre8::PHP_NAME);
    preg.method<&Pcre8::__construct>("__construct");
    preg.method<&Pcre8::setIdString> ("setIdString");
    preg.method<&Pcre8::getString> ("getString");
    preg.method<&Pcre8::getId> ("getId");
    preg.method<&Pcre8::isCompiled> ("isCompiled");
    preg.method<&Pcre8::match> ("match");
    
    extension.add(std::move(preg));

    Php::Class<Re8map> re8(Re8map::PHP_NAME);
    re8.method<&Re8map::setIdRex> ("setIdRex");
    re8.method<&Re8map::hasIdRex> ("hasIdRex");
    re8.method<&Re8map::unsetIdRex> ("unsetIdRex");
    re8.method<&Re8map::getIdRex> ("getIdRex");
    re8.method<&Re8map::addMapIds> ("addMapIds");
    re8.method<&Re8map::getIds> ("getIds");
    re8.method<&Re8map::count> ("count");
    extension.add(std::move(re8));

    Php::Class<Recap8> cap8(Recap8::PHP_NAME);
    cap8.method<&Recap8::count> ("count");
    cap8.method<&Recap8::getCap> ("getCap");
    extension.add(std::move(cap8));

    Php::Class<Token8Stream> t8s(Token8Stream::PHP_NAME);
    t8s.method<&Token8Stream::setEOSId> ("setEOSId");
    t8s.method<&Token8Stream::setEOLId> ("setEOLId");
    t8s.method<&Token8Stream::setUnknownId> ("setUnknownId");

    t8s.method<&Token8Stream::setExpSet> ("setExpSet");
    t8s.method<&Token8Stream::getExpSet> ("getExpSet");
    t8s.method<&Token8Stream::setSingles> ("setSingles");
    t8s.method<&Token8Stream::setRe8map> ("setRe8map");
    t8s.method<&Token8Stream::setInput> ("setInput");


    t8s.method<&Token8Stream::hasPendingTokens> ("hasPendingTokens");
    t8s.method<&Token8Stream::getToken> ("getToken");
    t8s.method<&Token8Stream::getLine> ("getLine");
    t8s.method<&Token8Stream::getValue> ("getValue");
    t8s.method<&Token8Stream::getId> ("getId");
    t8s.method<&Token8Stream::getOffset> ("getOffset");
    t8s.method<&Token8Stream::beforeEOL> ("beforeEOL");


    t8s.method<&Token8Stream::peekToken> ("peekToken");
    t8s.method<&Token8Stream::acceptToken> ("acceptToken");
    t8s.method<&Token8Stream::moveNextId> ("moveNextId");
    t8s.method<&Token8Stream::moveRegex> ("moveRegex");
    t8s.method<&Token8Stream::moveRegId> ("moveRegId");
    extension.add(std::move(t8s));

    Php::Class<Token8> token(Token8::PHP_NAME);
    token.method<&Token8::getValue> ("getValue");
    token.method<&Token8::getId> ("getId");
    token.method<&Token8::getLine> ("getLine");
    token.method<&Token8::isSingle> ("isSingle");
    extension.add(std::move(token));

    Php::Class<KeyTable> keytab(KeyTable::PHP_NAME);
    keytab.method<&KeyTable::setKV> ("setKV");
    keytab.method<&KeyTable::getV> ("getV");
    keytab.method<&KeyTable::unsetV> ("unsetV");
    keytab.method<&KeyTable::count> ("count");
    keytab.method<&KeyTable::toArray> ("toArray");
    extension.add(std::move(keytab));

    Php::Class<ValueList> valList(ValueList::PHP_NAME);
    valList.method<&ValueList::pushBack> ("pushBack");
    valList.method<&ValueList::getV> ("getV");
    valList.method<&ValueList::getLast> ("getLast");
    valList.method<&ValueList::count> ("count");
    valList.method<&ValueList::toArray> ("toArray");
    valList.method<&ValueList::getType> ("getType");
    extension.add(std::move(valList));

    Php::Class<TomlReader> rdr(TomlReader::PHP_NAME);
    rdr.method<&TomlReader::parse> ("parse");
    extension.add(std::move(rdr));

    return extension;
}

}