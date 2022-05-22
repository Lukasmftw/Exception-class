#pragma once
#ifndef EXCEPTION
#define EXCEPTION

#include <string>
#include <sstream>

using namespace std;


class field_file_missing : public exception {
public:
	string message;
public:
	field_file_missing() : message("No file found") {};
	field_file_missing(string str) : message(str) {};
	virtual const char* what() const noexcept override { return message.c_str(); }
};

class field_file_format_error : public exception {
public:
	string filename;
	int line;
	string message;
public:
	field_file_format_error(string filename, int line) : filename(filename), line(line) {
		stringstream ss;
		ss << filename << " line: " << line;
		message = ss.str();
	};
	virtual const char* what() const noexcept override
	{
		return message.c_str();
	}
};

class field_missing_error : public exception {
public:
	string message;
public:
	field_missing_error() : message("field_missing_error") {};
	field_missing_error(string str) : message(str) {};
	virtual const char* what() const noexcept override { return message.c_str(); }
};

class field_type_mismatch_error : public exception {
public:
	string message;
public:
	field_type_mismatch_error() : message("field_type_mismatch_error") {};
	field_type_mismatch_error(string str) : message(str) {};
	virtual const char* what() const noexcept override { return message.c_str(); }
};



#endif // !EXCEPTION
