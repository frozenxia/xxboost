#include "matrix.hpp"
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <string>
#include <iostream>
#include <algorithm>

using namespace std;

Matrix::Matrix(){
    
}

void Matrix::load(string filename,string &delimiter = string("\t"), bool use_column_labels=true,bool use_row_labels=true){
    ifstream file(filename.c_str());
    string line;
    int line_number = 0;
    while (getline(file,line)){
        vector<string> tokens = split_string(line,delimiter);
        if (tokens.size() == 0){
            cout << "with empty token " << endl;
            line_number ++;
            continue;
        }

        if (use_column_labels  && line_number == 0){
            this.column_labels = tokens;
            line_number ++;
            continue;
        }

        size_t index = 0;
        if (use_row_labels){
            this.row_labels.push_back(tokens[index]);
            index += 1;
        }
        vector<double> elements;
        for (int i = index;i < tokens.size();i ++){
            elements.push_back(atof(tokens[i]));
        }
        this.elements.push_back(elements);
        line_number ++;
    }
}


void Matrix::save(string filename){
    
}
int Matrix::columns(){
    if (this.elements.size() > 0){
        return this.elements[0].size();       
    }
    return 0;
}
int Matrix::rows(){
    return this.elements.size();
}

