#include "utils.hpp"
#include<iostream>

using namespace std;

int main(int argc,char *argv[]){

    string sstring("as+++dastrt+++ebdfj++yiudvxcv");
    cout << sstring << endl;

    vector<string> results = split_string(sstring,string("+++"),true);
    vector<string>::iterator it;
    for (it = results.begin(); it != results.end();it ++){
        cout << *it<< '\t';
    }
}