#include "utils.hpp"

bool ParameterParser::parse_and_assign(string token){
    size_t pos = token.find_first_of('=');
    if (pos == 0 || pos == string::npos){
        return false;
    }
    string key = token.substr(0,pos);
    string value=string("");
    if (pos + 1 < token.size()){
        value = token.substr(pos+1);
    }

    for (size_t i = 0; i < _kv_table.size() ;i ++){
        if (key.compare(_kv_table[i].first) == 0){
            _kv_table[i].second->parsed_value = value;
            _kv_table[i].second->set_value();

            return true;
        }
    }
    return false;
}


void ParameterParser::print_options(ostream & os,string indent){
    os << indent << _description <<endl;
    for (auto it = _kv_table.begin(); it != _kv_table.end();it ++ ){
        string key = it->first;
        os << indent << " " << key << "=value : " << it->second->description << " (default=" << it->second->default_value << ")" << endl; 
    }
}

void ParameterParser::print_parameters(ostream & os,string indent){
    // os << indent << _description <<endl;
    for (auto it = _kv_table.begin(); it != _kv_table.end();it ++ ){
        string key = it->first;
        string value = it->second->parsed_value;
        if (it->second->is_valid){
            os << indent << key << " = " << value << endl;
        }
    }
}





