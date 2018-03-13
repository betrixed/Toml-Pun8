# Toml Parser extension for PHP implemented with PHP-CPP
This was the main goal of creating all the other bitty classes, whose interfaces are described below. All parser processing is done in PHP-CPP for the expected order of magnitude of speed gain.

##Requirements 
  Linux version only.

  C++          C++17 - std::string_view STL gets used a little, which is marked C++17 "experimental".
  PHP          >= 7.1
  PHP-CPP      2.0.0  for compilation, and shared library installed.
  libpcre2-8   The latest version of the PCRE2 as shared library.

  libpre2-8    Is compiled for UTF-8 support.

  PHP uses PCRE2 as well, for preg_xxxx functions.

  Other configurations are possible. A UTF-16 version would be possible, and dependent classes named with "16" instead of "8"

## Pun\\TomlReader

```php
use Pun\\TomlReader;

$reader = new TomlReader();
$doc_model = $reader->parse($myinput);

$array = $doc_model->toArray();

```
OR
```php
use Pun\\TomlReader;

$doc_model = TomlReader::parseFile($file_path);

$array = $doc_model->toArray();

```

There are also two other static methods.
```php
        // Something about this build
        TomlReader::getUseVersion() : string;

        // Something about TOML version support
        TomlReader::getTomlVersion() : string;
```
The $doc_model returned by the Toml Parser, is a tree of "Pun\\KeyTable" and "Pun\\ValueList" objects.

### Pun\\KeyTable 
This Php class also supports the \\ArrayAccess interface, even though the methods are not directly available. Its accessible methods for getting, setting values use string keys, internal keys are strings only, and Php-numeric type is converted to string, whereas PHP ArrayAccess and array syntax converts string keys that are numeric to integer.

Currently internal storage is a C++ std::map.  The Traversable interface will iterate keys in alphabetical order.
```php
// Set a string key, and PHP value type
        setKV(string $key, $value);

// Get stored value using string key
        getV(string $key) : mixed;

// remove the key and value
        unsetK(string $key);

// check if key exists
        hasK(string $key);

// Clear out all keys and values
        clear();

// Recursive copy everything to Php Array, without KeyTable or ValueList objects.
        toArray() : array;

// Set a general use "Tag" property
        setTag($tag);
// Get the "Tag" property
        getTag() : mixed;
```
### Pun\ValueList
This class checks that everything that is set inside is of the same type, and for objects, the same class name.  It is implemented with C++ std::vector of PHP Value. Everything added must match the kind of value set with the first pushBack call. Traversable and Countable interfaces are implemented. ArrayAccess interface isn't implemented.
```php
// Add a value to the stack
      pushBack($value);
// Get the last value
      back() : mixed;
// Remove the end (back) value
      popBack();
// Get the number of values
      size() : int;
// Clear all values, size back to zero.
      clear();
// Set value at non-negative index, range checked
      setV(int $index, $value);
// Get value at non-negative index, range checked
      getV(int $index) : mixed;
// Set arbitrary tag value
      setTag($tag);
// Get previously set tag value
      getTag() : mixed;
// Return recursive contents as a PHP array,  without KeyTable or ValueList objects.
      toArray() : array;

```
The parser does utf-8 examples.  It will check and handle file Byte Order Marks - BOM

# Punicode - PHP extension for foreward iteration of unicode string with new PCRE2 interface.
This PHP extension, tentatively named "Punicode8", with namespace "Pun" for short, is created using the PHP-CPP toolkit.
The motivation arises from frustrations experienced in engineering the TOML parser projects, toml and toml-zephir, which centred around the interface limitations of preg_match, and the regretful need to use substr all the time.

Punicode is compiled with a shared link directly to the latest version of the libpcre2-8 library.

Of course utf-16 versions of this could be done but utf-8 was first priority.

Punicode currently being developed on a linux system, using the PHP-CPP source code.
This project is not likely to be modified, or even tried, on PHP versions earlier than 7.0

PHP-CPP does have a compatible "legacy version" for PHP 5.3+

The cause of going to so much development trouble is to create a TOML parser PHP-extension, using an existing design coded in PHP, which happens to make a lot of use of PCRE2. The current PHP interface for PCRE2 got in the way of trying various innovations.

A toml-PHP and toml-zephir versions have already been created and have set execution performance to be beaten.

## Lower Classes
  Various classes have been used to try out lower level functions in the TomlReader.
   Pun8, IdRex8, Re8map, Token8, Token8Stream, useful in trying out PHP versions of the TOML parser. 

The interface methods are close to the the minimum needed for a Toml parser, because of the time investment required. 
### Pun\\Pun8 
This was created first. It holds a reference to a PHP string, and maintains an absolute offset property. All regular expression matches start at the offset property.
It also has an internal shared IdRex8 map, and a list of key Ids to direct the firstMatch order, because the map is itself unordered.

```php
  class Pun8 {
  // method declarations (implmented in  C++)
  // Start with a utf-8 string to traverse
    __construct(string $input);

  // Reset with a new string, or just reset if no argument
    setString(string $input);

  // Get next character as one or more byte characters, advance the offset
    nextChar() : string

  // Get next character as one or more byte characters, no change in offset
    peekChar() : string

  // After nextChar() , or peekChar() retrive unicode code point
    getCode() : int;

  // The offset property, a byte position from 0
    getOffset() : int;
    setOffset(int $offset);
    addOffset(int $offset); 

  // Return the real buffer size
  size() : int;

  // The end of range of string marker, is independent of the internal buffer length,
  // and can be "moved" to end of a match operation range, from 0 to actual buffer size.
    getRangeEnd() : int;

    setRangeEnd(int $pos);

  // some primative encoding detection and management is available();

  // return a string describing BOM if it exists, such as after assigning file_get_contents();
    getBOMId() : string;  

    /* Detect BOM or not, convert to UTF8 if necessary.
     * Throw exception if unknown or unconvertible. 
     * If UTF8 BOM exists, return offset to first real character (maybe 3, else 0)
     * No end of line character management.
     */ 
    ensureUTF8() : int;  

    /*
     * Delete a block of characters within the string buffer, collapse the hole.
     * This resets the offset and rangeEnd properties, even if blockSize is zero.
     */
    erase(int $startPos, int $blockSize)

  /** 
   * Use the subset of key ids assigned by setIdList, return map index id of first matching PCRE2
   */


          firstMatch(Recap8 $captures) : int;

  /** Internal map of compiled regular expressions. 
   *  IdRex8 object can wrap 1 PCRE2, Re8map has an unordered_map of PCRE2 by integer key.
   *  std::shared_ptr is used to shared an object containing the integer key.
   * The regular expression, and its compiled version, and the Id are stuck together 
   *  and can't be seperated anyhow.
   */

  /**
   * Set map Id and Regular expression to be compiled
   */
          setIdRex(int $keyId, string $regex);

  /**
   * Get IdRex8 object to hold Id and shared compiled regular expression
   */
          getIdRex(int $keyId) : IdRex8;

 /**
  * Get a normal PHP array all of tthe integer keys (unordered) as a list
  */ 
          getIds() : array;

 /**
  * Assign a subset (ordered list of integer keys) selector 
  * for regular expression match (firstMatch())
  */
          setIdList(array $keyIdList);

  // Get the subset of keys
          getIdList() array;

  /* Using the internal map, try a match using a single Id key.
   * Return captures string list in $cap
   * Return value is number of captures
   */
          matchIdRex(int $key, Recap8 $cap) : int;
  
  /**
   * Try a single PCRE2 match from IdRex8 object.
   * Captures copied into $cap. 
   * Return value is number of captures.
   */
          matchIdRex8(IdRex8 $idrex, Recap8 $cap) : int;

  // Set the current internal map, from a sharing Map object
          setRe8map(Re8map $map);
/* 
 * This is such a big investment class, the Tag association property is now thrown in.
 */

  // Set a general use "Tag" property
        setTag($tag);
  // Get the "Tag" property
        getTag() : mixed;
 
```
### Pun\\IdRex8
This holds a single shareable IdRex8 internal object
```php
  // The Id and the Compiled Regular expression are stuck together
          __construct(int $id, string $regex);
          setIdString(int $id, string $regex);

  // Match results, or false, optional start offset
          match(string $target, int $offset = 0) : mixed;

  // Return some properties
          getString() : string;
          getId() : int;
          isCompiled() : bool;

```

### Pun\\Re8map
This object has more map management functions than the Pun8 target string interface.
It is convenient to set the shared Pun8 internal map from one of these.
```php
   // Add a Id, PCRE2 to the map
        setIdRex(int $keyId, string $regex) : int;

   // Map key Id exists
        hasIdRex(int $keyId) : bool;

   // return number of keys unset, 0 or 1
        unsetIdRex(int $keyId) : int;

   // Create a new IdRex8 object to hold a shared PCRE2 from the Map.
        getIdRex(int $keyId) : IdRex8;

   // Add any shared PCRE2, by keyID, from $source not already in this map, return count of new shares
   // Share individual PCRE2 by specifying Id list
   // If no Id list, try to share all
        addMapIds(Re8map $source, array $idList);

   // Get key Ids as PHP array list of integer
        getIds() : array;
   // Number of keys in map
        count() : int;
```


### Pun\\Recap8
Internally a vector of std::string stores the PCRE2 matches. 
This class is a wrapper to hold PCRE2 capture results created in method calls above.
```php
   // Return the indexed capture string
       getCap(int $keyId) : string;
   // Return how many captures.
       count() : int;
```

### Pun\\Token8
Has read only properties. Pass to some functions of TokenStream, to fill with summary match values.
```php
  // value of match PCRE2 or the next character from front of string
           getValue() : string;
  
  // match token id from map, or single character lookup table.
           getId() : int;

   // source line number, starting from 1
           getLine() : int;

   // character found in singles lookup table or is EOL or is EOS.
           isSingle() : bool;
```

### Pun\\Token8Stream
This is composed with an internal Pun8 member, plus a singles map,
line number counter, and id values to use for EOL, EOS (end of stream/string),
and a token for any un-mapped character, not in singles map.

Adding this as a C++ extension cut TOML parser execution time around 50%

```php
// Set some meaningful ids, usually taken from a kind-of enum list.
        setEOSId(int $id);
        setEOLId(int $id);
        setUnknownId(int $id);

// string to parse
        setInput(string $input);

// array of string => integer
        setSingles(array $smap);

// map of  id => sharable PCRE2
        setRe8map(Re8map $map);

// array of ordered ids, integer key subset of map.
        setExpSet(array $idList);
        getExpSet() : array;

// fetch the current line from offset, to before newline.
        beforeEOL() : string; 

// Read only property, for debugging purposes
        getOffset() : int;

// Get properties from last offset advance.
        getToken() : Token8;

// these correspond to Token8 properties
        getValue() : string;
        getId() : int;
        getLine() : int;

// Return a Token8 object, but first have to pass one as argument
// All the values come back in the Token8, no change in offset.
// This does no PCRE2 matching, just takes the front of the string.
        peekToken(Token8 $token) : Token8;

// Give the same token back to advance internal offset.
        acceptToken(Token8 $token);

/**
* Try in this order - any newline characters off front. 
* First match PCRE2 in the internal IdList, or 
* first character off front, with lookup in singles table.
* or return unknownId.  Value retrived with getToken(),
* of getValue(), getLine(), getId();
*/

        moveNextId() : int;

// Advanced the offset by the match argument, or failed to match
        moveRegex(IdRex8 $regex) : bool;

// Advanced the offset by the match argument from the integer key, or failed to match
        moveRegId(int $mapId) : bool;
```

This interface hasn't been used much yet, because it is just a wrapper around the enum value.
Pun extension and the Toml tree use a different list of enum values than PHP. For instance - True and False get to have the same Pun type enum, whereas in PHP they are different. There are values for integer, string, float, array (php-kind).

Php objects have an enum, excepting that KeyTable, ValueList and Php's DateTime are assigned individual type enum, as they are used by the TOML parser.

### Pun//Type
```php
/* DateTime has a type enum, and so does KeyTable and ValueList,
 * all other PHP objects are just an "Object". The type enums are used by Toml document tree.
 */
 // Set the enum from example Php value.
  fromValue($any);
 // return the type as a number
  type();
 // return the type as a string
  name();
 // return 1 if argument  matches the type, 0 if it doesn't
  isMatch($any);

```