#include "ustr_data.h"


#ifndef _H_UCODE8
#include "ucode8.h"
#endif
void UStrData::assign(std::string&& rh)
{
    _data = std::move(rh);
    reset_view();
}
void UStrData::reset_view()
{
    auto size = _data.size();
	if (size > 0) {
		_view = svx::string_view(_data.data(), size);
	}
	else {
		_view = svx::string_view();
	}
}
void UStrData::assign(const char* p, size_t slen)
{
	_data.assign(p, slen);
	reset_view();
}

void UStrData::ensureUTF8()
{
	auto offset = ::ensureUTF8(_data);
	auto size = _data.size();
	if (offset < size) {
		_view = svx::string_view(_data.data() + offset, size - offset);
	}
	else {
		_view = svx::string_view();
	}
}

std::string
UStrData::bomUTF16()
{
    uint16_t bom = 0xFEFF;
    return  std::string( (const char* )&bom, 2);
}

// return BOM for UTF8  as string
std::string
UStrData::bomUTF8()
{
    return std::string("\xEF\xBB\xBF");
}

char32_t
UStrData::fetch(uint64_t offset, svx::string_view& v)
{
	auto cptr = _view.data();
	auto endp = cptr + _view.size();
	cptr += offset;
    char32_t result;

	if (cptr < endp) {
		result = *((unsigned char*)cptr);
		if (result < 0x80) { // before 0100 0000
            v = svx::string_view(cptr, 1);
            return result;
		}
        else if (result < 0xC2) {
            // character 0x81 is
            // 1100 0010, 1000 0001, ie 0xC2 0x81
            // 0x81 to 0xBF are continuations, 0xC0, 0xC1 are illegal
            // This is a continuation byte if it was UTF-8
            // and should not be first byte of UTF-8 character.
            // but if it is some form of 8-bit ASCII
            // This is too lenient?

            v = svx::string_view(cptr, 1);
            return result;
        }
        else if (result < 0xE0) {
            result  &= 0x1F;
            if (++cptr >= endp) {
                return INVALID_CHAR;
            }
            v = svx::string_view(cptr,2);
            return (result << 6) + (*((unsigned char*)cptr) & 0x3F);
        }
        else if (result < 0xF0) {
            if (cptr + 2 >= endp) {
                v = svx::string_view(cptr, endp - cptr);
                return INVALID_CHAR;
            }
            v = svx::string_view(cptr, 3);
            result &= 0x0F;
            ++cptr;
            result = (result << 6) + (*((unsigned char*)cptr) & 0x3F);
            ++cptr;
            return (result << 6) + (*((unsigned char*)cptr) & 0x3F);
        }
        else if (result < 0xF5) {
            if (cptr + 3 >= endp) {
                v = svx::string_view(cptr, endp - cptr);
                return INVALID_CHAR;
            }
            v = svx::string_view(cptr, 4);
            result &= 0x07;
            ++cptr;
            result = (result << 6) + (*((unsigned char*)cptr) & 0x3F);
            ++cptr;
            result = (result << 6) + (*((unsigned char*)cptr) & 0x3F);
            ++cptr;
            return (result << 6) + (*((unsigned char*)cptr) & 0x3F);
        }
        else {
            return INVALID_CHAR;
        }
		/*
		auto charSize = ucode8Fore(
			ptr,
			size - offset,
			result);

		if (charSize) {
			v = svx::string_view(ptr, charSize);
			return result;
		}*/
	}
	return INVALID_CHAR;
}

svx::string_view
UStrData::substr(uint64_t begin, uint64_t len)
{
	return svx::string_view(_view.data() + begin, len);
}

std::string
UStrData::getBOMId()
{
	auto code = getBOMCode(_data.data(), _data.size());
	return getBOMName(code);
}

std::string
UStrData::asUTF16()
{
    std::string result;
    toUTF16(_data, result);
    return result;
}

