#include <phpcpp.h>
#include "ucode8.h"
//#include "pun8.h"
#include "pcre8.h"
#include "re8map.h"
#include "recap8.h"
#include "token8stream.h"
#include "keytable.h"
#include "valuelist.h"
#include "tomlreader.h"
#include "puntype.h"
#include "ustr8.h"
#include "idlist.h"

using namespace pun;

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
    static Php::Extension extension("pun8", "1.0");


    Php::Interface mergeable("Mergeable");

// define an interface method
    mergeable.method("merge", {
        Php::ByVal("store", KeyTable::PHP_NAME)
    });
    mergeable.method("toArray");

    extension.add(std::move(mergeable));
// registration methods
    UStr8::setup_ext(extension);
    ValueList::setup_ext(extension);
    IntList::setup_ext(extension);
    Re8map::setup_ext(extension);


    Php::Class<Pcre8> preg(Pcre8::PHP_NAME);
    preg.method<&Pcre8::__construct>("__construct");
    preg.method<&Pcre8::setIdString> ("setIdString");
    preg.method<&Pcre8::getString> ("getString");
    preg.method<&Pcre8::getId> ("getId");
    preg.method<&Pcre8::isCompiled> ("isCompiled");
    preg.method<&Pcre8::match> ("match");

    extension.add(std::move(preg));


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

    Php::Class<PathTag> tag(PathTag::PHP_NAME);
    extension.add(std::move(tag));

    Php::Class<TomlBase> tbase(TomlBase::PHP_NAME);
    tbase.method<&TomlBase::setTag> ("setTag");
    tbase.method<&TomlBase::getTag> ("getTag");
    extension.add(std::move(tbase));




    Php::Class<KeyTable> keytab(KeyTable::PHP_NAME);
    keytab.implements(mergeable);
    keytab.extends(tbase);

    keytab.method<&KeyTable::setKV> ("setKV");
    keytab.method<&KeyTable::getV> ("getV");
    keytab.method<&KeyTable::get> ("get");
    keytab.method<&KeyTable::unsetK> ("unsetK");
    keytab.method<&KeyTable::hasK> ("hasK");
    keytab.method<&KeyTable::clear> ("clear");
    keytab.method<&KeyTable::merge> ("merge", {
        Php::ByVal("store", KeyTable::PHP_NAME)
    });
    //keytab.method<&KeyTable::merge> ("merge");
    keytab.method<&KeyTable::size> ("size");

    keytab.method<&KeyTable::toArray> ("toArray");
    keytab.method<&KeyTable::setTag> ("setTag");
    keytab.method<&KeyTable::getTag> ("getTag");



    extension.add(std::move(keytab));

    Php::Class<TomlReader> rdr(TomlReader::PHP_NAME);

    rdr.method<&TomlReader::parse> ("parse");
    rdr.method<&TomlReader::parseFile>("parseFile");
    rdr.method<&TomlReader::getUseVersion>("getUseVersion");
    rdr.method<&TomlReader::getTomlVersion>("getTomlVersion");
    extension.add(std::move(rdr));

    Php::Class<PunType> ptype(PunType::PHP_NAME);
    ptype.method<&PunType::fromValue> ("fromValue");
    ptype.method<&PunType::isMatch> ("isMatch");
    ptype.method<&PunType::type> ("type");
    ptype.method<&PunType::name> ("name");

    extension.add(std::move(ptype));

    return extension;
}

}
