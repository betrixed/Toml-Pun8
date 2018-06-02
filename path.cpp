#include "path.h"
#include "ustr8.h"
#include <ostream>

using namespace pun;



const char* Path::PHP_NAME = "Pun\\Path";
const char* WINSEP = "\\";
const char* UNIXSEP = "/";

#ifdef _WIN32
    const char* DS = "\\";
    const char* ODS = "/";
#else
    const char* DS = "/";
    const char* ODS = "\\";
#endif

std::string Path::OSPathSep(DS);
std::string Path::OtherSep(ODS);

void
Path::setup_ext(Php::Extension& ext)
{
    Php::Class<Path> cpath(Path::PHP_NAME);

    cpath.method<&Path::endSep> ("endSep");
    cpath.method<&Path::noEndSep> ("noEndSep");
    cpath.method<&Path::native> ("native");
    cpath.method<&Path::sep> ("sep");
    cpath.method<&Path::startsWith> ("startsWith");
    cpath.method<&Path::endsWith> ("endsWith");
    ext.add(std::move(cpath));

}

Php::Value
Path::sep()
{
    return Path::OSPathSep;
}

static bool arg0_view(svx::string_view& vw, Php::Value& arg, bool& isu8)
{
    if (arg.isString()) {
        vw = svx::string_view((const char*) arg, arg.size());
        isu8 = false;
        return true;
    }
    else {
        UStr8* u8 = UStr8::get_UStr8(arg);

        if (u8 != nullptr) {
            vw = u8->fn_getView();
            isu8 = true;
            return true;
        }
        else {
            return false;
        }
    }
}


bool
Path::fn_native(svx::string_view& src, std::string& result){

    svx::string_view wrong(ODS, 1);
    svx::string_view right(DS, 1);
    return ( pun::replaceAll(src,wrong,right,result) > 0);
}

Php::Value
Path::startsWith(Php::Parameters& param)
{
    if(param.size() < 2) {
        return false;
    }
    Php::Value& arg0 = param[0];
    Php::Value& arg1 = param[1];
    svx::string_view src;
    svx::string_view bit;
    bool isu8;
    if (arg0_view(src, arg0,isu8) && arg0_view( bit, arg1, isu8))
    {
        auto bsize = bit.size();
        return (src.substr(0,bsize) == bit) ? true : false;
    }
    return false;
}

Php::Value
Path::endsWith(Php::Parameters& param)
{
    if(param.size() < 2) {
        return false;
    }
    Php::Value& arg0 = param[0];
    Php::Value& arg1 = param[1];
    svx::string_view src;
    svx::string_view bit;
    bool isu8;
    if (arg0_view(src, arg0,isu8) && arg0_view( bit, arg1, isu8))
    {
        auto bsize = bit.size();
        auto slen = src.size();
        if (bsize > slen)
            return false;
        return (src.substr(slen-bsize,bsize) == bit) ? true : false;
    }
    return false;
}

Php::Value
Path::native(Php::Parameters& param)
{
    if (param.size() == 0)
        return Php::Value();
    svx::string_view src;
    bool isu8 = false;
    Php::Value& arg = param[0];
    if (!arg0_view(src, arg, isu8))
        return Php::Value();//null
    std::string result;
    if (fn_native(src, result))
    {
        if (isu8) {
            return UStr8::make_UStr8(result);
        }
        else {
            return result;
        }
    }
    return arg;
}

Php::Value
Path::endSep(Php::Parameters& param)
{
    if (param.size() == 0)
        return Php::Value();
    svx::string_view src;
    bool isu8 = false;
    Php::Value& arg = param[0];
    if (!arg0_view(src, arg, isu8))
        return Php::Value();

    if (src.size() > 0) {
        auto last = src.back();
        if (last == DS[0])
            return arg;
        std::string result;
        result.reserve(src.size()+1);
        result.assign(src.data(), src.size());
        result +=  DS[0];

        if (isu8)
            return UStr8::make_UStr8(result);
        else
            return result;
    }
    else {
        return Path::OSPathSep;
    }
}

Php::Value
Path::noEndSep(Php::Parameters& param)
{
    if (param.size() == 0)
        return Php::Value();
    svx::string_view src;
    bool isu8 = false;
    Php::Value& arg = param[0];
    if (!arg0_view(src, arg, isu8))
        return Php::Value();
    if (src.size() > 0) {
        auto last = src.back();
        if (last == DS[0]) {
            src.remove_suffix(1);
            std::string result(src.data(), src.size());
            if (isu8)
                return UStr8::make_UStr8(result);
            else
                return result;
        }
        return arg;
    }
    return Php::Value();
}

