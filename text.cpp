#include "text.h"
#include "pcre8.h"
#include "valuelist.h"
#include "keytable.h"

unsigned int
pun::replaceAll(svx::string_view src,
                const svx::string_view& out,
                const svx::string_view& in,
                    std::string& result) {
    std::string build;
    //Php::out << "replaceAll:arg: " << src << " | " <<  out << " | " << in << std::endl;
    svx::string_view::size_type fpos;
    unsigned int replaced = 0;
    for(;;) {
        fpos = src.find(out);
        if (fpos==svx::string_view::npos) {
            if (replaced > 0) {
                build += src;
            }
            break;
        }
        else {
            //
            replaced += 1;
            build += src.substr(0,fpos);
            build += in;
            //Php::out << fpos << ": replaceAll " << src << " | " <<  build << std::endl;
            src.remove_prefix(fpos + out.size());

        }
    }
    if (replaced > 0) {
        result = std::move(build);
        //Php::out << "replaceAll:done: " << result << std::endl;
    }
    return replaced;
}

std::string
pun::str_replace(
				const std::string& from,
				const std::string& to,
				const std::string& subject)
{
	if (from.empty()) {
		return subject;
	}
	std::string result = subject;
	size_t spos = 0;
	while((spos = result.find(from, spos)) != std::string::npos) {
		result.replace(spos, from.size(), to);
		spos += to.size();
	}
	return result;
}

std::string
pun::uncamelize(const std::string& s, char sep) {
	std::string result;

	for(size_t i = 0; i < s.size(); i++) {
		auto test = s[i];
		if (isupper(test)) {
			result += sep;
			result += (char) tolower(test);
		}
		else {
			result += (char) test;
		}
	}
	return result;
}

StringList
pun::explode(const std::string& sep, const std::string& toSplit)
{
	StringList slist;

	auto fpos = 0;
	auto ipos = toSplit.find(sep);
	auto sepSize = sep.size();

	while (ipos != std::string::npos)
	{
		slist.push_back(std::move(toSplit.substr(fpos, ipos - fpos)));
		fpos = ipos + sepSize;
	}
	slist.push_back(std::move(toSplit.substr(fpos, toSplit.size() - fpos)));
	return slist;
}

static Pcre8_imp gVarRegexp;

Pcre8_imp* getVarRegexp() {
    //TODO: need a mutex here
    if (!gVarRegexp.isCompiled()) {
        std::string msg;
        gVarRegexp._id = 1000;
        gVarRegexp._eStr = "\\${(\\w+)}";
        if (!gVarRegexp.compile(msg)) {
            throw Php::Exception(msg);
        }
    }
    return &gVarRegexp;
}

void
pun::replaceVar_object(Php::Value& obj, Php::Value& lookup)
{
    if (obj.instanceOf(ValueList::PHP_NAME)) {
        auto vlist = (ValueList*) obj.implementation();
        replaceVar_ValueArray(vlist->fn_store(), lookup);
    }
    else if (obj.instanceOf(KeyTable::PHP_NAME)) {
        auto vmap = (KeyTable*) obj.implementation();
        replaceVar_ValueMap(vmap->fn_store(), lookup);
    }
}

bool
pun::replaceVar_str(
    const svx::string_view& src,
    std::string& result,
    Php::Value& lookup)
{
    Pcre8_matchAll mlist;
    bool changed = false;
    auto rct = getVarRegexp()->doMatchAll(src,mlist);
    if (rct > 0) {
        result = src;
        auto mend = mlist.end();
        for( auto mit = mlist.begin(); mit != mend; mit++) {
            const StringList& sl = mit->_slist;
            const std::string& var_name = sl[1];
            const std::string& var_spec = sl[0];
            // lookup
            //Php::out << "Before " << var_name << " and " << var_spec << std::endl;
            Php::Value value = lookup[var_name];
            if (value.isString()) {
                changed = (0 < pun::replaceAll(svx::string_view(result.data(), result.size()),
                                svx::string_view(var_spec.data(), var_spec.size()),
                                svx::string_view(value, value.size()),
                                result
                                ));
                //Php::out << "After " << result << std::endl;
            }
            else {
                //Php::out << "Not found: " << var_name << std::endl;
            }
        }
    }
    return changed;
}

void
pun::replaceVar_ValueArray(ValueArray& items, Php::Value& lookup)
{
    auto vend = items.end();
    for( auto vit = items.begin(); vit != vend; vit++) {
        Php::Value& sval = *vit;
        if (sval.isString()) {
            std::string result;
            if (0 < replaceVar_str(svx::string_view(sval, sval.size()), result, lookup)) {
                sval = result;
            }
        }
        else if (sval.isObject()) {
            replaceVar_object(sval, lookup);
        }
    }
}

void
pun::replaceVar_ValueMap(ValueMap& items, Php::Value& lookup)
{
    auto mend = items.end();
    for(auto ait = items.begin(); ait != mend; ait++)
	{
		Php::Value& sval = ait->second;

		if (sval.isObject()) {
            //Php::out << "Do object" << std::endl;
			replaceVar_object(sval,lookup);
			continue;
		}
		else if (sval.isString()) {

            std::string result;
            if (replaceVar_str(svx::string_view(sval, sval.size()), result, lookup))
            {
                sval = result;
            }
		}
	}
}


