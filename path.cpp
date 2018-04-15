#include "path.h"

std::string Path::OSPathSep;

const char* Path::PHP_NAME = "Pun\\Path";

void
Path::setup_ext(Php::Extension& ext)
{
    Php::Class<Path> cpath(Path::PHP_NAME);

    cpath.method<&Path::endSep> ("endSep");
    cpath.method<&Path::noEndSep> ("noEndSep");
    cpath.method<&Path::native> ("native");
}
