#ifndef _H_IDLIST
#define _H_IDLIST

#include "pcre8_imp.h"

//! Simple but specialized :
// Pack Php array of integers into C++ vector

namespace pun {

	class IntList : public Php::Base, Php::Countable  {
	public:
		static const char* PHP_NAME;
        static void setup_ext(Php::Extension& ext);
        static IntList* get_IntList(Php::Value& val);

		void __construct(Php::Parameters& param);
		// set list from PHP array of integers
		void setArray(Php::Parameters& param);
		// return list as PHP array of integers
		Php::Value toArray();

		void pushBack(Php::Parameters& param);
		void popBack();
		Php::Value getV(Php::Parameters& params) const;
		void setV(Php::Parameters& param);

		Php::Value back() const;
		virtual long  count() override { return (long) _store.size(); }

		Php::Value size() const;
		void clear() { _store.clear(); }

	public:

		IdList   _store; // access by PHP-CPP objects

 		void fn_copyIdList(Php::Value& v);
 		void fn_pushBack(Php::Value& val);
	};

};

#endif
