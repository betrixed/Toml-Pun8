#include <phpcpp.h>
#include "ucode8.h"
//#include "pun8.h"
#include "path.h"
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
 // interface methods
    mergeable.method("merge", {
        Php::ByVal("store", KeyTable::PHP_NAME)
    } );
    mergeable.method("toArray");
    extension.add(std::move(mergeable));

    Php::Interface tagable("Tagable");

    tagable.method("setTag", {
        Php::ByVal("tag")
    } );
    tagable.method("getTag");

    extension.add(std::move(tagable));

// registration methods
    UStr8::setup_ext(extension);
    TomlBase::setup_ext(extension);
    KeyTable::setup_ext(extension, mergeable, tagable);
    ValueList::setup_ext(extension, tagable);

    IntList::setup_ext(extension);
    Re8map::setup_ext(extension);
    Recap8::setup_ext(extension);
    Pcre8::setup_ext(extension);

    Token8Stream::setup_ext(extension);
    Token8::setup_ext(extension);
    TomlReader::setup_ext(extension);
    
    PunType::setup_ext(extension);
    Path::setup_ext(extension);
    return extension;
}

}
