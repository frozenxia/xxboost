#ifndef _FASTRGF_DISCRETIZATION_H
#define _FASTRGF_DISCRETIZATION_H

#include "data.h"

namespace rgf {

    class FeatureDiscretizationDense {
    public:
        UniqueArray<float> boundaries;

        class TrainParm : public ParameterParser {
        public:
            ParamValue<double> min_bucket_weights;
            ParamValue<int> max_buckets;
            ParamValue<double> lamL2;

            TrainParm(string prefix = "disc_dense.") {
                min_bucket_weights.insert(prefix + "min_bucket_weights",
                                          5,
                                          "minimum number of effective samples for each discretization value",
                                          this
                );

                max_buckets.insert(prefix + "max_buckets",
                                   65000,
                                   "maximum number of discretized values",
                                   this);
                lamL2.insert(prefix + "lamL2",
                             2,
                             "L2 regulation parameter",
                             this);

            }
        };

        FeatureDiscretizationDense() {}

        size_t size() {
            return boundaries.size() + 1;
        }

        void set(UniqueArray<float> &boundaries0) {
            boundaries = std::move(boundaries0);
        }

        pair<float, float> operator[](const int v) {
            float low, high;
            if (v <= 0) {
                low = 1e-10;
            } else {
                low = boundaries[v - 1];
            }

            if (v + 1 >= boundaries.size()) {
                high = 1e20;
            } else {
                high = boundaries[v];
            }
            return pair<float, float>(low, high);
        }

        int apply(float x);

        void clear() {
            boundaries.clear();
        }

        template<typename i_t>
        void train(DataSet<float, i_t, float> &ds, int j, TrainParm &tr);

        void read(istream &is);

        void write(ostream &os);
    };


};


#endif //FASTRGF_DISCRITIZATION_H
