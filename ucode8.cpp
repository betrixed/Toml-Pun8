
/** 
    For PHP
    Have to detect valid character, construct the unicode character number
    , and  buffer
    into character buffer during the process, with a terminal 0.
    Length of buffer also calculated from change in index
*/
#include "ucode8.h"

#include <sstream>

#include <phpcpp.h> // for the exception

const TestEndian MyEndian;

/**
 * char const*  cpt      pointer to first character to consume
 * unsigned int  slen    offset of EOS from cpt
 * char32_t&     uc      next unicode character number returned, or INVALID_CHAR
 * @return unsigned int  number of base character units consumed
 */

unsigned int
    ucode8Fore(
	char const* cpt, 
	unsigned int slen, 
	char32_t& uc)
{            
    if (!cpt || slen == 0) {
        uc = INVALID_CHAR;
        return 0;
    }
    const unsigned char* pa = reinterpret_cast<unsigned char const*>(cpt);

    unsigned int k = 0;
    if (k >= slen)
        return 0;
    unsigned char temp = pa[k++];
    char32_t test = temp;


    if (test < 0x80)
    {
        uc = test; //1
        return k;
    }
            
    if (test < 0xC2)
        return k;
                
    if (test < 0xE0)
    {
        if (k >= slen) {
            uc = INVALID_CHAR;
            return k;

        }
        temp = pa[k++];
        uc = ((test & 0x1F) << 6) + (temp & 0x3F);//2
        return k;
    }
    else if (test < 0xF0)
    {
        if (k >= slen) {
            uc = INVALID_CHAR;
            return k;
        }
        temp = pa[k++];
        test = ((test & 0x0F) << 6) + (temp & 0x3F);//2
        if (k >= slen)
        {
            uc = INVALID_CHAR;
            return k;
        }

        temp = pa[k++];
        uc = (test << 6) + (temp & 0x3F);//3
        return k;
    }
    else if (test < 0xF5) // reach to 140000
    {
        if (k >= slen) {
            uc = INVALID_CHAR;
            return k;
        }
        temp = pa[k++];

        test = ((test & 0x07) << 6) + (temp & 0x3F);//2
        if (k >= slen) {
            uc = INVALID_CHAR;
            return k;
        }
        temp = pa[k++];
        test = (test << 6) + (temp & 0x3F);//3
        if (k >= slen) {
            uc = INVALID_CHAR;
            return k;
        }
        temp = pa[k++];
        test = (test << 6) + (temp & 0x3F);//4
        if (test > 0x10FFFF) {
            uc = INVALID_CHAR;
            return k;
        }
        uc = test;
        return k;
    }
    else {
        uc = INVALID_CHAR;
        return k;
    }
}



unsigned int
EncodeUTF8::encode(char32_t d)
{
    if (d < 0x80)
    {
        result[0] = (char) d;
        result[1] = 0;
        return 1;
    }
    if (d < 0x800)
    {
        // encode in 11 bits, 2 bytes
        result[1] = (d & 0x3F) | 0x80;
        d >>= 6;
        result[0] = (char) (d | 0xC0);
        result[2] = 0;
        return 2;
    }
    else if (d < 0x10000)
    {
        // encode in 16 bits, 3 bytes
        result[2] = (d & 0x3F) | 0x80;
        d >>= 6;
        result[1] = (d & 0x3F) | 0x80;
        d >>= 6;
        result[0] = (char) (d | 0xE0);
        result[3] = 0;
        return 3;
    }
    else if (d > 0x10FFFF)
    {
        // not in current unicode range?
        return 0;
    }
    else {
        // encode in 21 bits, 4 bytes
        result[3] = (d & 0x3F) | 0x80;
        d >>= 6;
        result[2] = (d & 0x3F) | 0x80;
        d >>= 6;
        result[1] = (d & 0x3F) | 0x80;
        d >>= 6;
        result[0] = (char) (d | 0xF0);
        result[4] = 0;
        return 4;
    }
}


BOM_CODE getBOMCode(const char* sptr, unsigned int slen)
{
    if (slen == 0)
    {
        return BOM_CODE::NO_BOM;
    }
    auto raw = reinterpret_cast<const unsigned char*>(sptr);

    if (*raw == 0xEF || *raw == 0xFE || *raw == 0xFF || *raw == 0x00) 
    {
        if (*raw == 0xEF) {
            // only one handled here. Next 2 have to match

            if (slen < 3)
            {
                return BOM_CODE::BAD_BOM;
            }
            if (*(raw+1) == 0xBB && *(raw+2) == 0xBF) {
                return BOM_CODE::UTF_8;
            }
            return BOM_CODE::BAD_BOM;
        }
        else if (*raw == 0xFE) {
            if (slen < 2)
            {
                return BOM_CODE::BAD_BOM;
            }
            raw++;
            if (*raw == 0xFF) {
                return BOM_CODE::UTF_16BE;
            }
            return BOM_CODE::BAD_BOM;
        }
        else if (*raw == 0xFF) {
            if (slen < 4)
            {
                return BOM_CODE::BAD_BOM;
            }
            raw++;
            if (*raw == 0xFE) {
                if (*(raw+1) != 0x00) {
                    return BOM_CODE::UTF_16LE;
                }
                if (*(raw+2) == 0x00) {
                    return BOM_CODE::UTF_32LE;
                }
            }
            return BOM_CODE::BAD_BOM;
        }
        else if (*raw == 0x00) {
            if (slen < 4)
            {
                return BOM_CODE::BAD_BOM;
            }
            if (*(raw+1) == 0x00 && *(raw+2) == 0xFE && *(raw+3) == 0xFF) {
                return BOM_CODE::UTF_32BE;
            }
            return BOM_CODE::BAD_BOM;
        }
    }

    return BOM_CODE::NO_BOM;

}


const char* getBOMName(BOM_CODE code)
{
    switch(code) {
    case BOM_CODE::NO_BOM:
    case BOM_CODE::UTF_8:
        return "UTF-8";
    case BOM_CODE::UTF_16LE:
        return "UTF-16LE";
    case BOM_CODE::UTF_16BE:
        return "UTF-16BE";
    case BOM_CODE::UTF_32LE:
        return "UTF-32LE";
    case BOM_CODE::UTF_32BE:
        return "UTF-32BE";
    case BOM_CODE::BAD_BOM:
    default:
        return "Unknown BOM";
    }
}

/**
 * char16_t const* cp      pointer to first character to consume
 * unsigned int    slen    offset of EOS from cpt
 * char32_t&   uc           next unicode character number returned, or INVALID_CHAR
 * @return unsigned int      number of base character units consumed
 */
unsigned int
ucode16Fore(char16_t const* cp, unsigned int slen, char32_t& uc)
{

    if (!cp || slen == 0) {
        uc = INVALID_CHAR;
        return 0;
    }
    unsigned int k = 0;
    unsigned short const* pa = reinterpret_cast<unsigned short const*>(cp);
    const char32_t test = pa[k++];

    if (test >= 0xD800)
    {
        if (test >= 0xDC00)
        {
            if (test < 0xE000) {
                uc = INVALID_CHAR;
                return k; // second pair member first
            }
        }
        else {
            // first character of surrogate
            if (k >= slen) {
                uc = INVALID_CHAR;
                return k; // unpaired first member at end
            }
            const char32_t test2 = pa[k++];
            if (test2 < 0xDC00 || test2 > 0xDFFF) {
                // second pair member out of range
                uc = INVALID_CHAR;
                return k;
            }
            uc = ((test - 0xD7C0) << 10) + (test2 - 0xDC00);
            return k;
        }
    }
    uc = test;
    return k;
}

// convert same-endian UTF-16 to UTF8
// return false if a INVALID_CHAR error happens before buffer consumed
// All valid characters before INVALID_CHAR or EOS returned in 
// output
// 
bool convertUTF16(char16_t* cp, unsigned int slen, std::string& output)
{
    std::stringstream ss;

    char32_t     uc = INVALID_CHAR;
    char16_t*    pu16 = cp;
    EncodeUTF8   toUTF8;

    output.clear();
    bool    result = true;

    while (slen) {
        auto offset = ucode16Fore(pu16, slen, uc);
        if (uc != INVALID_CHAR) {
            toUTF8.encode(uc);
            ss << toUTF8.result;
        }
        else {
            result = false;
            break;
        }
        pu16 += offset;
        slen = (slen >= offset) ? slen - offset : 0;
    }
    output = std::move(ss.str());
    return result;
}

void swap16buffer(char16_t* cp, unsigned int wlen)
{
    auto wp = reinterpret_cast<uint16_t*> (cp);

    for(unsigned int i = 0 ; i < wlen; i++, wp++)
    {
        *wp = ((*wp & 0xff) << 8) | ((*wp & 0xff00) >> 8);
    }
}


unsigned int 
ensureUTF8(std::string& input)
{
    char* sptr = (char*)(input.data());
    auto slen = input.size();

    auto bomCode = getBOMCode(sptr,slen);
    if (bomCode == BOM_CODE::UTF_8) {
        return 3;
    }
    else if (bomCode == BOM_CODE::UTF_16LE) {
        auto cp = (char16_t*)(input.data());
        auto wlen = input.size() / 2;
        // no check for odd size?
        cp += 1; // skip BOM
        wlen -= 1;
        if (MyEndian.isBigEnd()) {
            // need to have even number of bytes, swap them 
            // als
            swap16buffer(cp, wlen);
        }
        std::string inPun8;
        bool result = convertUTF16(cp, wlen, inPun8);
        if (!result) {
            throw Php::Exception("Conversion failed from UTF-16LE to UTF-8");
        }
        input.clear();
        input = std::move(inPun8);
        return 0;
    }
    else if (bomCode == BOM_CODE::UTF_16BE) {
        auto cp =  (char16_t*)(input.data());
        auto wlen = input.size() / 2;
        // no check for odd size?
        cp += 1; // skip BOM
        wlen -= 1;
        if (!MyEndian.isBigEnd()) {
            // need to have even number of bytes, swap them 
            // als
            swap16buffer(cp, wlen);
        }
        std::string inPun8;
        bool result = convertUTF16(cp, wlen, inPun8);
        if (!result) {
            throw Php::Exception("Conversion failed from UTF-16BE to UTF-8");
        }
        input.clear();
        input = std::move(inPun8);
        return 0;
    }
    else if (bomCode != BOM_CODE::NO_BOM) {
        std::stringstream ss;
        ss << "Unhandled BOM type: " << getBOMName(bomCode);
        throw Php::Exception(ss.str()); 
    }
    return 0;
}


unsigned int 
EncodeUTF16::encode(const char32_t d)
{
    if (d < 0xD800)
    {
        result[0] = d;
        return 1;
    }
    if (d >= 0x10000)
    {
        // turn into pair ?
        const unsigned int paired = d - 0x10000;
        
        result[0] = (paired >> 10) + 0xD800;
        result[1] = (paired & 0x3FF) + 0xDC00;
        return 2;
    }
    else if (d >= 0xE000)
    {
        result[0] = d;
        return 1;
    }
    return 0; // TODO: exception, not a valid unicode character
}


int 
toUTF16(const std::string& input, std::string& output)
{
    const char* cp = input.data();
    unsigned int slen = input.size();

    unsigned int clen = 0;
    unsigned int wlen = 0;

    unsigned int i = 0;
    unsigned int wix = 0;
    char32_t    test;
    std::string result;

    EncodeUTF16 enc16;
    result.reserve(input.size()*2); // likely to double in size

    while (i < slen) {
        clen = ucode8Fore(cp + i, slen-i, test);
        if (test == INVALID_CHAR) {
            throw Php::Exception("Invalid character in UTF8 string"); 
        }
        wlen = enc16.encode(test);
        wix = 0;
        while (wix < wlen) {
            result.append((const char*) &enc16.result[wix], 2);
            wix++;
        }
        i += clen;
    }
    output = std::move(result);
    return output.size();
}