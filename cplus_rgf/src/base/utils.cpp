#include "utils.h"

bool ParameterParser::parse_and_assign(string token) {
    size_t pos = token.find_first_of('=');
    if (pos == 0 || pos == string::npos) {
        return false;
    }
    string key = token.substr(0, pos);
    string value = string("");
    if (pos + 1 < token.size()) {
        value = token.substr(pos + 1);
    }

    for (size_t i = 0; i < _kv_table.size(); i++) {
        if (key.compare(_kv_table[i].first) == 0) {
            _kv_table[i].second->parsed_value = value;
            _kv_table[i].second->set_value();

            return true;
        }
    }
    return false;
}


void ParameterParser::print_options(ostream &os, string indent) {
    os << indent << _description << endl;
    for (auto it = _kv_table.begin(); it != _kv_table.end(); it++) {
        string key = it->first;
        os << indent << " " << key << "=value : " << it->second->parsed_value << " (default="
           << it->second->default_value << ")" << endl;
    }
}

void ParameterParser::print_parameters(ostream &os, string indent) {
    // os << indent << _description <<endl;
    for (auto it = _kv_table.begin(); it != _kv_table.end(); it++) {
        string key = it->first;
        string value = it->second->parsed_value;
        if (it->second->is_valid) {
            os << indent << key << " = " << value << endl;
        }
    }
}


void ParameterParserGroup::command_line_parse(int_t argc, char *argv[]) {
    unparsed_tokens.clear();

    for (int_t c = 1; c < argc; c++) {
        string token(argv[c]);
        cout << token << endl;
        int_t nn = parse(token);
        if (nn <= 0) {
            unparsed_tokens.push_back(token);
        }
        if (nn >= 2) {
            cerr << "ambiguous command line option " << token << endl;
        }

    }
}


int_t ParameterParserGroup::parse(string token) {
    int_t num_parsed = 0;
    for (int_t i = 0; i < pp_vec.size(); i++) {
        if (pp_vec[i]->parse_and_assign(token)) num_parsed++;
    }
    return num_parsed;
}

// not implement
void ParameterParserGroup::config_file_parse(string filename) {

}

//not implement
void ParameterParserGroup::print_options(ostream &os, string indent, int_t line_skip) {

}


