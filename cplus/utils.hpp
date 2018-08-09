#include <vector>
#include <string>
#include <sstream>
#include <iostream>
#include <cstdlib>
using std::vector;
using std::string;
using std::cout;
using std::endl;

vector<string> split_string(const string &source,const string& delimiter,bool keepEmpty = false){
    vector<string> results;
    size_t prev = 0;
    size_t next = 0;
    size_t delimiterLength = delimiter.size();
    while((next = source.find(delimiter,prev)) != string::npos){
        if (keepEmpty || (next - prev) != 0){
            results.push_back(source.substr(prev,next-prev));
        }
        // cout << prev <<',' << next << endl;
        prev = next+delimiterLength;
    }
    if (prev < source.size()){
        results.push_back(source.substr(prev));
    }
    return results;
}