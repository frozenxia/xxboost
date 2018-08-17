//
// Created by mi on 18-8-17.
//

#include "data.hpp"

int main(int argc,char*argv[]){
    cout << "hello world" << endl;
    DataSetFlt::IOParam param_trnfile("trn.");
    ParameterParserGroup ppg;
    ppg.add_parser(&param_trnfile);

    param_trnfile.nthreads.set_value(4);
    ppg.command_line_parse(argc,argv);
    param_trnfile.print_options(cout);

//    param_trnfile.xfile_format.set_value();

    if (param_trnfile.fn_x.value.size() > 0){
        DataSetFlt  trn_orig;
        cerr << "loading training data ....." << endl;
        trn_orig.append(param_trnfile);
    }



    return 0;
}
