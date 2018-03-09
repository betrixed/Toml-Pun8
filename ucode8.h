#ifndef _H_UCODE8
#define _H_UCODE8

bool ucode8Fore(
	char const* cpt, // buffer pointer
	unsigned int slen, // size
	unsigned int& ix, // index to start from, return next index.
	char32_t& d       // 32-bit unicode value
	);

// Encode char32_t into null terminated string of char_t, return length
struct EncodeUTF8 {
	char result[8];
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

const char* getBOMName(BOM_CODE code);

// byte by byte check
BOM_CODE getBOMCode(const char* sptr, unsigned int len);

#endif