#ifndef VARS_H
#define VARS_H

#include <map>
#include <string>

class sVars
{
public:
	const int Int(const std::string& name)
	{
		return integers[name];
	}
	void Int(const std::string& name, int value)
	{
		integers[name] = value;
	}
	const std::string& String(const std::string& name)
	{
		return strings[name];
	}
	void String(const std::string& name, const std::string& value)
	{
		strings[name] = value;
	}
	const double Double(const std::string& name)
	{
		return doubles[name];
	}
	void Double(const std::string& name, double value)
	{
		doubles[name] = value;
	}
	std::string& operator[] (const std::string& name)
	{
		return strings[name];
	}

protected:
	std::map<std::string, std::string> strings;
	std::map<std::string, int>         integers;
	std::map<std::string, double>      doubles;
};

extern sVars svars;

#endif