#include "ConfigHelp.h"
#include <fstream>
#include <string>
#include <vector>
#include "Utils.h"
#include "Macro.h"
#include <assert.h>

ConfigHelp::ConfigHelp(){
}

ConfigHelp::~ConfigHelp() {
}

int ConfigHelp::init(const char* configName) {
	std::ifstream in;
	in.open(configName);
	if (!in.is_open()) {
		LOG("Config:%s File Not Open\n", configName);
		return -1;
	}

	_record.clear();
	std::vector<std::string> res;
	std::string lineContent;
	for (; std::getline(in, lineContent); ) {
		strtrim(lineContent, " ");
		auto pos = lineContent.find("#");
		if (pos != lineContent.npos) {
			if (pos == 0)
				continue;
			lineContent = lineContent.substr(0, pos);
		}
		res = strsplit(lineContent, "=");
		if (res.size() < 2)
			continue;
		_record[res[0]] = res[1];
	}
	in.close();
	return 0;
}

std::string ConfigHelp::getString(const char* key) {
	assert(_record.find(key) != _record.end());
	return _record[key];
}

int ConfigHelp::getInt(const char* key) {
	std::string val = getString(key);
	return std::atoi(val.c_str());
}

void ConfigHelp::dump(){
	for (auto it : _record) {
		printf("key=%s,val=%s\n", it.first.c_str(), it.second.c_str());
	}
}
