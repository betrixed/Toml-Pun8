#ifndef _H_USTR8
#define _H_USTR8

#ifndef PHPCPP_H
#include <phpcpp.h>
#endif

#ifndef _H_USTR_DATA
#include "ustr_data.h"
#endif

namespace pun {
class UStr8;
class UStr_Iterator;


// Not going to do Countable, or ArrayAccess, or Serializable
// because of potential difficulties.
class UStr8 : public Php::Base, public Php::Traversable {
public:

		static const char* PHP_NAME;

		static void setup_ext(Php::Extension& ext);

		static UStr8* get_UStr8(Php::Value& val);

		UStr8();
        UStr8(UStr8& original);

        void __construct(Php::Parameters& params);

        // return another view of same shared string
        Php::Value share();

        // Set the string for iteration
        void setString(Php::Parameters& params);

		// return BOM as string for UTF16
        static Php::Value bomUTF16();

        // return BOM as string for UTF8
        static Php::Value bomUTF8();

        // Return PHP string converted to platform UTF16, no BOM
        Php::Value asUTF16() const;

        Php::Value getBOMId();

        // convert if none UTF8 contents to UTF8.
        // Return false if cannot convert
        // Regular expression and traversal functions only work with UTF8.
        // Return true if a UTF8 BOM was found.
        void ensureUTF8();

         // Get current byte offset into string
        Php::Value getBegin() const { return (intp_t) _index; }

        Php::Value  getEnd() const { return (intp_t) _size; }

        // First parameter is offset to first character,
        // Last parameter is offset to last character plus one.
        // Full range will be 0, size
        void setRange(Php::Parameters& param);

        void setEnd(Php::Parameters& param);

        // size of character buffer
        Php::Value  size() const { return (intp_t) _str.get()->_view.size(); }

        // For Traversing the current range
		virtual Php::Iterator *getIterator() override;

        // Get current artifical end position, within actual end.
        Php::Value value() const;

        // Fetch next character, update "Code" value and move the byte offset
        // if 1st param is byRef variable, place the unicode value in it
        Php::Value nextChar(Php::Parameters& params);

        // Fetch next character, update "Code" value but do not change the byte offset
        // if 1st param is byRef variable, place the unicode value in it
        Php::Value peekChar(Php::Parameters& params);

        /*! Return new object with replaced segments
            function replace_all(target, newstuff) : UStr8;
        */
        Php::Value replaceAll(Php::Parameters& params);
        /*! Return boolean if argument is last part of string */
        Php::Value endsWith(Php::Parameters& params);
        /*! Return boolean if argument is first part of string */
        Php::Value beginsWith(Php::Parameters& params);

        /*! Push the string argument onto the end. This resets the view range. */
        void pushBack(Php::Parameters& params);

        /*! Reduce length by so many characters.  This resets the view range. */
        void popBack(Php::Parameters& params);

        /*! return PHP string value */
        Php::Value __toString();

public:
		void fn_setBegin(Php::Value& val);
		void fn_setEnd(Php::Value& val);
		void fn_setString(const char* ptr, unsigned int len);
		void fn_setString(std::string&& m);

		svx::string_view fn_getView();
		svx::string_view param_getView(Php::Value& v);

public:
		Str_ptr				_str;
    	uint64_t			_index;
    	uint64_t			_size;
};


// Forward iterate through UStr8, produce offset, utf-8 string pair
class UStr_Iterator : public Php::Iterator {
	Str_ptr				_str;
	uint64_t			_startOffset;
	uint64_t			_offset;
	uint64_t			_size;
	svx::string_view	_seg;
public:
	UStr_Iterator(UStr8* sp, size_t start, size_t end) :
	Php::Iterator(sp),
	_str(sp->_str),
	_startOffset(start),
	_offset(start),
	_size(end)
	{}

	virtual bool valid() override
	{
		return _offset < _size;
	}

	virtual Php::Value current() override
	{
		return Php::Value(_seg.data(), _seg.size());
	}

	virtual Php::Value key() override
	{
		return (intp_t) _offset;
	}

	virtual void next() override
	{
		_offset += _seg.size();
		if (_offset < _size) {
			_str.get()->fetch(_offset, _seg);
		}
	}
	virtual void rewind() override
	{
		_offset = _startOffset;
		if (_offset < _size) {
			_str.get()->fetch(_offset, _seg);
		}
	}
};

};

#endif
