#ifndef _H_RE8MAP
#define _H_RE8MAP

#include <phpcpp.h>
#include "pcre8_imp.h"

namespace pun {
    class Re8map : public Php::Base {
    public:
        static const char* PHP_NAME;
        static void setup_ext(Php::Extension& ext);

    	Re8map();
    	~Re8map();

        // Install PCRE2 String, Return integer id, the first parameter.
    	Php::Value setIdRex(Php::Parameters& params);

        // Has key value in map
    	Php::Value hasIdRex(Php::Parameters& params) const;

    	// Remove key value and expression data from map
    	Php::Value unsetIdRex(Php::Parameters& params);
    	 // Return a new Pcre8 object by Id
        Php::Value getIdRex(Php::Parameters& params);

        // Two parameters, Array of integer keys, and
        // another Re8map object.
        // The compiled Re are shared with this map.
        // Key PCRE2 already in this map won't be re-copied over.
        Php::Value addMapIds(Php::Parameters& params);

        // Get unordered array of integer id's in map
    	Php::Value getIds() const;

      	Php::Value count() const;
        /*! function firstMatch(UStr8, Recap8, IntList|Array) : integer;
            Return number of captions
            Recap8 object for captures.
            List of Ids, to order and select match tries
        */

      	Php::Value firstMatch(Php::Parameters& params);

      	const Re8map_share& getImp() { return _remap;}

    public: // PHP cannot see this
        Re8map_share   _remap;
    };
};

#endif
