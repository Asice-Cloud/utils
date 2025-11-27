//
// Created by asice-cloud on 10/5/25.
//

#include "reflect.h"
#include <simdjson.h>
#include <sstream>

struct json_show : public reflected_object {
    json_show() {
        REGISTER_MEMBER(example);
        REGISTER_FUNCTIONS(FUNCTION(parse_and_show));
    }

    std::string example;

    void parse_and_show(const std::string &json_str) {
        simdjson::dom::parser parser;
        simdjson::dom::element doc = parser.parse(json_str);
        std::cout << "Parsed JSON: " << doc << std::endl;
    }

    void show_json_by_reflection() const {
        std::ostringstream oss;
        oss << "{";
        bool first = true;
        this->visit_all_members(
            [&](const std::string &name, const std::any &value,
                const std::string_view) {
                if (!first)
                    oss << ", ";
                first = false;
                oss << '\"' << name << "\":";
                if (value.type() == typeid(std::string)) {
                    oss << '\"' << std::any_cast<std::string>(value) << '\"';
                } else if (value.type() == typeid(int)) {
                    oss << std::any_cast<int>(value);
                } else if (value.type() == typeid(double)) {
                    oss << std::any_cast<double>(value);
                } else if (value.type() == typeid(bool)) {
                    oss << (std::any_cast<bool>(value) ? "true" : "false");
                } else {
                    oss << '"' << "<unsupported type>" << '"';
                }
            },
            Nothing_TODO_With_Function);
        oss << "}";
        std::string json_str = oss.str();
        simdjson::dom::parser parser;
        auto doc = parser.parse(json_str);
        std::cout << "Reflected JSON: " << doc << std::endl;
    }
};

int main() {
    json_show js;
    js.example = "test value";
    js.show_json_by_reflection();
    return 0;
}
