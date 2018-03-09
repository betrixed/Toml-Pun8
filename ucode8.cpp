
/** 
    For PHP
    Have to detect valid character, construct the unicode character number
    , and  buffer
    into character buffer during the process, with a terminal 0.
    Length of buffer also calculated from change in index
*/
#include "ucode8.h"



bool ucode8Fore(
	char const* cpt, 
	unsigned int slen, 
	unsigned int& ix, 
	char32_t& d)
{            
    if (!cpt)
        return false;
    const unsigned char* pa = reinterpret_cast<unsigned char const*>(cpt);

    unsigned int k = ix;
    if (k >= slen)
        return false;
    unsigned char temp = pa[k++];
    char32_t test = temp;


    if (test < 0x80)
    {
        ix = k;
        d = test; //1
        return true;
    }
            
    if (test < 0xC2)
        return false;
                
    if (test < 0xE0)
    {
        if (k >= slen)
            return false;
        temp = pa[k++];
        d = ((test & 0x1F) << 6) + (temp & 0x3F);//2
        ix = k;

        return true;
    }
    else if (test < 0xF0)
    {
        if (k >= slen)
            return false;
        temp = pa[k++];
        test = ((test & 0x0F) << 6) + (temp & 0x3F);//2
        if (k >= slen)
        {
            return false;
        }

        temp = pa[k++];
        d = (test << 6) + (temp & 0x3F);//3
        ix = k;
        return true;
    }
    else if (test < 0xF5) // reach to 140000
    {
        if (k >= slen)
            return false;
        temp = pa[k++];

        test = ((test & 0x07) << 6) + (temp & 0x3F);//2
        if (k >= slen) {
            return false;
        }
        temp = pa[k++];
        test = (test << 6) + (temp & 0x3F);//3
        if (k >= slen) {
            return false;
        }
        temp = pa[k++];
        test = (test << 6) + (temp & 0x3F);//4
        if (test > 0x10FFFF) {
            return false;
        }
        d = test;
        ix = k;
        return true;
    }
    else
        return false;
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
