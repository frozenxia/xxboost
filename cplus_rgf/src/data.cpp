#include "data.hpp"


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
        
        

    }