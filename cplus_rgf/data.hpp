#ifndef _RGF_DATA_H
#define _RGF_DATA_H

namespace rgf{
    template<typename d_t,typename i_t,typename v_t> class DataPoint{
        public:
            int dim_dense;
            d_t *x_dense;
            int dim_sparse;
            SparseFeatureGroup<i_t,v_t> *x_sparse;
        
    };
}



#endif
