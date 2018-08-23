#ifndef _FASTRGF_DISCRETIZATION_H
#define _FASTRGF_DISCRETIZATION_H

#include "data.h"

namespace rgf {

    class FeatureDiscretizationDense {
    public:
        UniqueArray<float> boundaries;

        class TrainParm : public  ParameterParser{
        public:
            ParamValue<double> min_bucket_weights;
            ParamValue<int> max_buckets;
            ParamValue<double> lamL2;
            TrainParm(string prefix= "disc_dense."){
                min_bucket_weights.insert(prefix+"min_bucket_weights",
                        5,
                        "minimum number of effective samples for each discretization value",
                        this
                        );

                max_buckets.insert(prefix+"max_buckets",
                        65000,
                        "maximum number of discretized values",
                        this);
                lamL2.insert(prefix+"lamL2",
                        2,
                        "L2 regulation parameter",
                        this);

            }
        };

        FeatureDiscretizationDense(){}

        size_t  size(){
            return boundaries.size() +1;
        }

        void set(UniqueArray<float> &boundaries0){

        }
    };


};


#endif //FASTRGF_DISCRITIZATION_H
