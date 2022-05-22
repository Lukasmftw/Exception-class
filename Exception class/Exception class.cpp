#include <fstream>
#include <map>
#include <memory>
#include <iostream>
#include <sstream>
#include "exception.h"

using namespace std;

void check(string line, string filename);

class field_base {
public:
    virtual ~field_base() { }
    virtual void* data() = 0;
    virtual void display(ostream& os) const = 0;
};

template <class FieldType>
class field_impl : public field_base {
    FieldType m_value;

public:
    field_impl(const FieldType& value) :
        m_value(value) { }

    ~field_impl() { }

    void* data() {
        return static_cast<void*>(&m_value);
    }

    void display(ostream& os) const {
        os << m_value;
    }
};

class field_storage {
    std::map<std::string, std::unique_ptr<field_base>> m_fields;

public:
    field_storage() { }
    field_storage(const char* filename);
    void read_file(const char* filename);

    template <class FieldType>
    FieldType& get(const std::string& field_name) {

        auto it = m_fields.find(field_name);
        if (it == m_fields.end())
            throw field_missing_error(field_name); //2.5
        auto raw_field_ptr = it->second.get();
        auto field_ptr = static_cast<field_impl<FieldType>*>(raw_field_ptr);
        //How to check if string is string, int is int and so on, based off of template?
        return *static_cast<FieldType*>(field_ptr->data());
    }

    friend std::ostream& operator << (std::ostream& os, const field_storage& fields);

private:
    void parse_line(const std::string& line, const std::string& filename, const int& where);
    field_base* parse_value(const std::string& value, const std::string& filename, const int& where);
};

field_storage::field_storage(const char* filename) {
    ifstream check(filename);
    if (!check)
        throw field_file_missing(filename); //2.1

    read_file(filename);
}

void field_storage::parse_line(const std::string& line, const std::string& filename, const int& where) {
    auto colon_pos = line.find(':');

    if (colon_pos == string::npos)
        throw field_file_format_error(filename, where); // 2.2


    auto field_name = line.substr(0, colon_pos);
    auto field_value = line.substr(colon_pos + 1);

    if (auto field = parse_value(field_value, filename, where)) {
        m_fields[field_name] = std::unique_ptr<field_base>(field);
    }
}

field_base* field_storage::parse_value(const std::string& value, const std::string& filename, const int& where) {
    if (value == "true") { return new field_impl<bool>(true); }
    if (value == "false") { return new field_impl<bool>(false); }

    auto last_char_pos = value.size() - 1;
    if (value[0] == '\"' && value[last_char_pos] == '\"') {
        auto string_value = value.substr(1, value.size() - 2);
        return new field_impl<std::string>(string_value);
    }
    else if((int)value[0]>47 && (int)value[0]<58)
    {
        stringstream ss(value);
        int int_value;
        ss >> int_value;
        return new field_impl<int>(int_value);
    }
    else
    {
        throw field_file_format_error(filename, where); //2.3
    }
}

void field_storage::read_file(const char* filename) {
    ifstream input_stream(filename);
    int where = 0;

    while (!input_stream.eof()) {
        string current_line;
        getline(input_stream, current_line);
        check(current_line, filename);
        if (current_line.size() == 0)
            continue;
        ++where;
        parse_line(current_line, filename, where);
    }

}

//2.4
void check(string line, string filename)
{
    ifstream checkFile(filename);
    string temp;
    int k = 0;
    int where = 0;
    while (!checkFile.eof())
    {
        getline(checkFile, temp);
        if (temp.size() == 0)
        {
            break;
        }
        ++where;
        if (temp == line)
        {
            ++k;
        }
        if (k == 2)
        {
            throw field_file_format_error(filename, where);
        }
    }

}

std::ostream& operator << (std::ostream& os, const field_storage& fields) {
    for (const auto& pair : fields.m_fields) {
        os << pair.first << ": ";
        pair.second->display(os);
        os << endl;
    }
    return os;
}


int main(int argc, char** argv) {

    try
    {
        field_storage storage("fields1.txt");
    }
    catch (field_file_missing& a)
    {
        cout << "Field file missing: " << a.what() << endl;
        cout << "Enter new file name.\n\n";
    }

    try
    {
        field_storage storage("fields.txt");

        cout << "fields:" << endl;
        cout << storage << endl;

        auto word = storage.get<string>("word");
        cout << word << endl;

        auto number1 = storage.get<int>("number1");
        cout << number1 << endl;

        auto number2 = storage.get<int>("number2");
        cout << number2 << endl;

        auto bool1 = storage.get<bool>("bool1");
        cout << bool1 << endl;
    }
    catch (field_file_missing& a)
    {
        cout << "Field file missing: " << a.what() << endl;
        cout << "Enter new file name." << endl;
    }
    catch (field_file_format_error& a)
    {
        cout << "Field file format error: " << a.what() << endl;
    }
    catch (field_missing_error& a)
    {
        cout << "Field missing error: " << a.what();
    }


    //auto invalid = storage.get<int>("invalid");   //Tokio nëra. Kaip elgtis?
    //cout << invalid << endl;
}
