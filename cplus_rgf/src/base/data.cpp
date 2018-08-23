#include "data.h"
#include "mr.hpp"

template<typename d_t,typename i_t,typename v_t>
void DataSet<d_t,i_t,v_t>::clear(){
    row_weights.clear();
    y.clear();
    assert(x_dense.size() == size() && x_sparse.size() == size());

    for (size_t i = 0;i < size();i ++){
        delete [] x_dense[i];
        x_dense[i] = nullptr;
        delete [] x_sparse[i];
        x_sparse[i] = nullptr;
    }
    x_dense.clear();
    x_sparse.clear();
    _nrows = 0;
    _dim_dense = -1;
    _dim_sparse = -1;
}



template<typename d_t,typename i_t, typename v_t>
int_t DataSet<d_t,i_t,v_t>::read_nextBatch(istream& is_x ,istream &is_y,
    istream & is_w,bool y_valid,bool w_valid,
    string is_x_format,size_t batch_size,int nthreads){
        if(is_x.eof()){
            return 0;
        }
        MyDataInputLineParserMR<d_t,i_t,v_t> parser(is_x,is_y,is_w,y_valid,w_valid,is_x_format,batch_size);
        MapReduceRunner runner(nthreads,MapReduceRunner::INTERLEAVE);
        runner.run(parser,0,runner.nthreads);
        size_t nl = parser.lines_read;

        int i ;
        for (i = 0;i < nl;i ++){
            if (_dim_dense < 0) {
                _dim_dense = parser.ps[i].feats_dense.size();
//                cout << "+dd" << endl;
//                cout << _dim_dense << endl;
//                cout << "dim_dense" << endl << parser.ps[i].feats_sparse[i].size()<< endl;
//                cout << parser.ps[i].feats_dense[0] << endl;
            }

            if (_dim_dense != parser.ps[i].feats_dense.size() ){
                throw MyDataInputException("number of dense feature is " +  to_string(parser.ps[i].feats_dense.size()) +
                    "but should be " + to_string(_dim_dense) ,i+1);
            }

            if (_dim_sparse < 0) _dim_sparse = parser.ps[i].feats_sparse.size();
            if (_dim_sparse != parser.ps[i].feats_sparse.size() ){
                throw MyDataInputException("number of dense feature is " +  to_string(parser.ps[i].feats_sparse.size()) +
                    "but should be " + to_string(_dim_sparse) ,i+1);
            }
            _nrows ++;

            if (!parser.use_uniform_weights){
                row_weights.push_back(parser.ps[i].w_val);
            }

            if (!parser.read_x_only){
                y.push_back(parser.ps[i].y_val);
            }

            int j;
            d_t *x_d = nullptr;
            if (_dim_dense > 0){
                x_d = new d_t[_dim_dense];
            }
            for (j = 0;j < _dim_dense; j ++){
                x_d[j] = parser.ps[i].feats_dense[j];
            }
            x_dense.push_back(x_d);
            
            SparseFeatureGroup<i_t,v_t> *x_s = nullptr;
            if(_dim_sparse > 0){
                x_s = new SparseFeatureGroup<i_t,v_t>[_dim_sparse];
            }

            for (j = 0; j < _dim_sparse ;j ++){
                x_s[j] = std::move(parser.ps[i].feats_sparse[j]);
            }
            x_sparse.push_back(x_s);
        }
        return nl;
    }


template<typename d_t,typename i_t,typename v_t>
size_t DataSet<d_t,i_t,v_t>::append(DataSet::IOParam &param){
    ifstream is_x(param.fn_x.value);
    ifstream is_y(param.fn_y.value);
    ifstream is_w(param.fn_w.value);

    bool w_valid = (param.fn_w.value.size() > 0);
    bool y_valid = (param.fn_y.value.size() > 0);

    cout << "y_valid"  << y_valid << endl;

    if( ! is_x.good()){
        cerr << "cannot open feature file <"  << param.fn_x.value << " >" << endl;
        return 0;
    }
    if( w_valid && ! is_w.good()){
        cerr << "cannot open weight file <"  << param.fn_w.value << " >" << endl;
        return 0;
    }
    if( y_valid  && ! is_y.good()){
        cerr << "cannot open label file <"  << param.fn_y.value << " >" << endl;
        return 0;
    }

    y_type = Target(param.y_type.value);
    int batch_size = 1000;
    int nthreads = param.nthreads.value;

    int_t nlines = 0;

    int nl = 0;
    int begin_ = is_sorted() ? size() : 0;
    while(true){
        try{
            nl = read_nextBatch(is_x,is_y,is_w,y_valid,w_valid,param.xfile_format.value,batch_size,nthreads);
        }catch (MyDataInputException &e){
            cerr << "error at reading file at line "  << (nlines + e.line_no) << " , " << e.error_message << endl;
            break;
        }
        if (nl == 0){
            break;
        }
        nlines += nl;
    }
    cout << nlines << endl;
    int end_ = size();

    for( int i = begin_; i < end_;i ++){
        (*this)[i].sort();
    }
    return nlines;
}

namespace rgf {
    template class DataSet<float,src_index_t,float>;
    template class DataSet<int,int,int>;
//    template class DataSet<DISC_TYPE_T>;
}