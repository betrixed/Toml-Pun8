#ifndef _H_UCODE8
#define _H_UCODE8

bool ucode8Fore(
	char const* cpt, // buffer pointer
	unsigned int slen, // size
	unsigned int& ix, // index to start from, return next index.
	char32_t& d       // 32-bit unicode value
	);

#endif