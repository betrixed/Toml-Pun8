#include "text.h"

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



