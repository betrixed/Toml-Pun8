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
	auto size = _view.size();
	if (offset < size) {
		char32_t result;
		auto ptr = _view.data() + offset;
		auto charSize = ucode8Fore(
			ptr,
			size - offset,
			result);
		if (charSize) {
			v = svx::string_view(ptr, charSize);
			return result;
		}
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

