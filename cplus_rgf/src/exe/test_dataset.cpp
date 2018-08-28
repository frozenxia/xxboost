//
// Created by mi on 18-8-17.
//

#include "data.h"
#include "discretization.h"

//using namespace _discretizationTrainerDense;
int main(int argc, char *argv[]) {
    cout << "hello world" << endl;
    DataSetFlt::IOParam param_trnfile("trn.");
    FeatureDiscretizationDense::TrainParam param_disc_dense("discretize.dense.");
    FeatureDiscretizationSparseInt::TrainParam param_disc_sparse("discretize.sparse.");
    ParameterParserGroup ppg;
    ppg.add_parser(&param_trnfile);

    ppg.add_parser(&param_disc_dense);
    ppg.add_parser(&param_disc_sparse);

    param_trnfile.nthreads.set_value(1);
    ppg.command_line_parse(argc, argv);
    param_trnfile.print_options(cout);



//    param_trnfile.xfile_format.set_value();
//
    if (param_trnfile.fn_x.value.size() > 0) {
        DataSetFlt trn_orig;
        cerr << "loading training data ....." << endl;
        trn_orig.append(param_trnfile);

        cout << trn_orig.dim_sparse() << endl;
        cout << trn_orig.dim_dense() << endl;
        cout << trn_orig.size() << endl;
        cout << "yu" << endl;
        cout << trn_orig.y[0] << "\t" << trn_orig.y[4] << endl;
        cout << "end" << endl;
        DataDiscretizationInt disc;
        int nthreads = 1;
        int verbose = 2;
        disc.train(trn_orig, param_disc_dense, param_disc_sparse, nthreads, verbose);
//        disc.
//        cout << disc.disc_dense[0].boundaries.size();


    }


//    train it




//    DataSetFlt  trn_orig;
//    trn_orig.append(param_trnfile);



    return 0;
}
