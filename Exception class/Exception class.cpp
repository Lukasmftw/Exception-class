#include <fstream>
#include <map>
#include <memory>
#include <iostream>
#include <sstream>

using namespace std;

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
    bool read_file(const char* filename);

    template <class FieldType>
    FieldType& get(const std::string& field_name) {
        auto it = m_fields.find(field_name);
        auto raw_field_ptr = it->second.get();
        auto field_ptr = static_cast<field_impl<FieldType>*>(raw_field_ptr);
        return *static_cast<FieldType*>(field_ptr->data());
    }

    friend std::ostream& operator << (std::ostream& os, const field_storage& fields);

private:
    void parse_line(const std::string& line);
    field_base* parse_value(const std::string& value);
};

field_storage::field_storage(const char* filename) {
    read_file(filename);
}

void field_storage::parse_line(const std::string& line) {
    auto colon_pos = line.find(':');
    auto field_name = line.substr(0, colon_pos);
    auto field_value = line.substr(colon_pos + 1);

    if (auto field = parse_value(field_value)) {
        m_fields[field_name] = std::unique_ptr<field_base>(field);
    }
}

field_base* field_storage::parse_value(const std::string& value) {
    if (value == "true") { return new field_impl<bool>(true); }
    if (value == "false") { return new field_impl<bool>(false); }

    auto last_char_pos = value.size() - 1;
    if (value[0] == '\"' && value[last_char_pos] == '\"') {
        auto string_value = value.substr(1, value.size() - 2);
        return new field_impl<std::string>(string_value);
    }

    {
        stringstream ss(value);
        int int_value;
        ss >> int_value;
        return new field_impl<int>(int_value);
    }
}

bool field_storage::read_file(const char* filename) {
    ifstream input_stream(filename);

    while (!input_stream.eof()) {
        string current_line;
        getline(input_stream, current_line);
        if (current_line.size() == 0)
            continue;
        parse_line(current_line);
    }

    return true;
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
    field_storage storage("fields.txt");

    cout << "fields:" << endl;
    cout << storage << endl;

    auto number1 = storage.get<int>("number1");
    cout << number1 << endl;

    auto number2 = storage.get<int>("number2");
    cout << number2 << endl;

    auto bool1 = storage.get<bool>("bool1");
    cout << bool1 << endl;

    auto word = storage.get<string>("word");
    cout << word << endl;

    //auto invalid = storage.get<int>("invalid");   //Tokio n�ra. Kaip elgtis?
    //cout << invalid << endl;
}
