#ifndef _H_UCODE8
#define _H_UCODE8

#include <cstdint>
#include <string>

// return number of character units consumed, and unicode code number
unsigned int ucode8Fore(
	char const* cpt, // buffer pointer
	unsigned int slen, // size
	char32_t& d       // 32-bit unicode value
	);

// return number of character units consumed, and unicode code number
unsigned int
ucode16Fore(
	char16_t const* cp, 
	unsigned int slen, 
	char32_t& uc);

// Encode char32_t into null terminated string of char_t, return length
struct EncodeUTF8 {
	char result[8];
	unsigned int encode(const char32_t d);
};

// Encode char32_t , return number of char16_t units
struct EncodeUTF16 {
	char16_t result[2];
	unsigned int encode(const char32_t d);
};

enum BOM_CODE {
	BAD_BOM = -1,
	NO_BOM = 0,
	UTF_8 = 1,
	UTF_16LE = 2,
	UTF_16BE = 3,
	UTF_32LE = 4,
	UTF_32BE = 5
};


class TestEndian {
private:
	union {
        uint32_t i;
        char c[4];
    } bint = {0x01020304};
public:
	bool isBigEnd() const {
		return bint.c[0] == 1;
	}
};

const char32_t INVALID_CHAR = (char32_t) -1;
const char* getBOMName(BOM_CODE code);

// byte by byte check
BOM_CODE getBOMCode(const char* sptr, unsigned int len);

// 16-bit character buffer from UTF-16 on this platform, to UTF-8
bool convertUTF16(char16_t const* cp, unsigned int wlen, std::string& output);

// Result is returned in std::string, because PHP uses string for buffer storage.
int toUTF16(const std::string& input, std::string& output);

void swap16buffer(char16_t* cp, unsigned int wlen);

/* 
 * Check the value of input for encoding, and make sure it is UTF-8
 * Convert if possible.
 * Return an offset to end of BOM , if any remains
 */
unsigned int ensureUTF8(std::string& input);
#endif