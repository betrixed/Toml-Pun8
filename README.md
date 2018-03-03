
# Punicode - PHP extension for foreward iteration of unicode string with new PCRE2 interfaces.
This PHP extension, tentatively named "Punicode8", is created using the PHP-CPP toolkit.
The motivation arises from frustrations experienced in engineering the TOML parser projects, toml and toml-zephir, which centred around the interface limitations of preg_match. 
Punicode is compiled with a shared link directly to the latest version of the libpcre2-8 library.
It is currently being developed on a linux system.

## Classes
  The classes, so far, in this first version design, are labelled "Pun\\Pun8", "Pun\\IdRex8", "Pun\\Re8map".
The interface methods are so far, near the minimum needed for the envisioned usage, because of the time investment required.
### Pun\\Pun8 
This was created first. It holds a reference to a PHP string, and maintains an absolute offset property. 
It also has an internal shared IdRex8 map
```php
  class Pun8 {
  // method declarations (implmented in  C++)
  // Start with a utf-8 string to traverse
  public function __construct(string $input);

  // Reset with a new string, or just reset if no argument
  public function setString(string $input);

  // Get next character as one or more byte characters, advance the offset
  public function nextChar() : string

  // Get next character as one or more byte characters, no change in offset
  public function peekChar() : string

  // After nextChar() , or peekChar() retrive unicode code point
  public function getCode() : int;

  // The offset property, a byte position from 0
  public function getOffset() : int;
  public function setOffset(int $offset);
  public function addOffset(int $offset); 

  
  /** 
   * Use the subset of key ids assigned by setIdList, return id of first matching PCRE2
   */

  public function firstMatch(Recap8 $captures) : int;

  /** Internal map of compiled regular expressions. 
   *  IdRex8 object can wrap 1 PCRE2, Re8map has an unordered_map of PCRE2 by integer key.
   *  std::shared_ptr is used to shared an object containing the integer key, the regular expression,
   *  and its compiled version.
   */

  public function setIdRex(int $keyId, string $regex);
  public function getIdRex(int $keyId) : IdRex8;
  // Get a normal PHP array all of tthe integer keys (unordered) as a list
  public function getIds() : array;

  // Assign a subset (ordered list of integer keys) selector for regular expression match (firstMatch())
  public function setIdList(array $keyList);

  // Get the assigned ordered subset of keys
  public function getIdList() array;

  /* Using the internal map, try a match using a single Id key.
   * Return captures string list in $cap
   * Return value is number of captures
   */
  public function matchIdRex(int $key, Recap8 $cap) : int;
  
  /**
   * Try a single PCRE2 match from IdRex8 object.
   * Captures copied into $cap. 
   * Return value is number of captures.
   */
  public function matchIdRex8(IdRex8 $idrex, Recap8 $cap) : int;

  // Set the current internal map, from a sharing Map object
  public function setRe8map(Re8map $map);
 
```
### Pun\\IdRex8
This holds a single shareable IdRex8 internal object
```php
  // The Id and the Regular expression are stuck together
  public function __construct(int $id, string $regex);
  public function setIdString(int $id, string $regex);
  // Match results, or false, optional start offset
  public function match(string $target, int $offset = 0) : mixed;
  // Return some properties
  public function getString() : string;
  public function getId() : int;
  public function isCompiled() : bool;

```

### Pun\\Re8map
This object has just a few more map management functions than the Pun8 target string interface.
It is convenient to set the shared Pun8 internal map from one of these, assign a different map  as often as required.
```php
   public setIdRex(int $keyId, string $regex) : int;
   public hasIdRex(int $keyId) : bool;
   // return number of keys unset, 0 or 1
   public unsetIdRex(int $keyId) : int;
   // Create a new IdRex8 object to hold the shared PCRE2.
   public getIdRex(int $keyId) : IdRex8;
   // Add any shared PCRE2, by keyID, from $source not already in this map, return count of new shares
   // Share individual PCRE2 by specifying Id list
   // If no Id list, try to share all
   public addMapIds(Re8map $source, array $idList);

   // Get key Ids as PHP array list of integer
   public getIds() : array;
   // Number of keys in map
   public count() : int;
```


### Pun\\Recap8
Internally a vector of std::string stores the PCRE2 matches. 
This class is wrapper to pass results in method calls above.
```php
   // Return the indexed capture string
   public getCap(int $keyId) : string;
   // Return how many captures.
   public count() : int;
```

### Pun\\Token8
Has read only properties. Pass to some functions of TokenStream, to fill with summary match values.
```php
  // value of match PCRE2 or the next character from front of string
   public function getValue() : string;
  
  // match token id from map, or single character lookup table.
   public function getId() : int;

   // source line number, starting from 1
   public function getLine() : int;

   // character found in singles lookup table or is EOL or is EOS.
   public function isSingle() : bool;
```

### Pun\\Token8Stream
This is composed with an internal Pun8 member, plus a singles map,
line number counter, and id values to use for EOL, EOS (end of stream/string),
and a token for any un-mapped character, not in singles map.

Adding this as a C++ extension cut TOML parser execution time around 50%

```php
public function setEOSId(int $id);
public function setEOLId(int $id);
public function setUnknownId(int $id);

// string to parse
public function setInput(string $input);

// array of string => integer
public function setSingles(array $smap);

// map of  id => sharable PCRE2
public function setRe8map(Re8map $map);

// array of ordered ids, integer key subset of map.
public function setExpSet(array $idList);
public function getExpSet() : array;

// fetch the current line from offset, to before newline.
public function beforeEOL() : string; 

// Read only property, for debugging purposes
public function getOffset() : int;

// Get properties from last offset advance.
public function getToken() : Token8;

// these correspond to Token8 properties
public function getValue() : string;
public function getId() : int;
public function getLine() : int;

// Return a Token8 object, but first have to pass one as argument
// All the values come back in the Token8, no change in offset.
// This does no PCRE2 matching, just takes the front of the string.
public function peekToken(Token8 $token) : Token8;

// Give the same token back to advance internal offset.
public function acceptToken(Token8 $token);

/**
* Try in this order - any newline characters off front. 
* First match PCRE2 in the internal IdList, or 
* first character off front, with lookup in singles table.
* or return unknownId.  Value retrived with getToken(),
* of getValue(), getLine(), getId();
*/

public function moveNextId() : int;

// Advanced the offset by the match argument, or failed to match
public function moveRegex(IdRex8 $regex) : bool;

// Advanced the offset by the match argument from the integer key, or failed to match
public function moveRegId(int $mapId) : bool;
```
