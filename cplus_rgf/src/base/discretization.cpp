//
// Created by mi on 18-8-24.
//

#include <mr.h>
#include "discretization.h"

namespace _discretizationTrainerDense {
    struct Elem {
        float x = 0;
        float y = 0;
        float w = 0;

        bool operator<(const Elem &b) {
            return x < b.x;
        }

        Elem(float _x = 0, float _y = 0, float _w = 0) : x(_x), y(_y), w(_w) {}
    };

    /**
     * 没太明白这个地方使用的gain函数
     */
    class Bucket {
    public:
        size_t begin;
        size_t end;
        size_t cut;
        double lowest_value = 1e-10;
        double max_value = 1e10;
        double gain;


        Bucket(size_t _b, size_t _e, Elem *s_arr,
               double *y_sum_arr, double *w_sum_arr,
               double min_bucket_weights, float lamL2)
                : begin(_b), end(_e), gain(0.0) {
            if (min_bucket_weights < 1e-3) min_bucket_weights = 1e-3;
            if (lamL2 < lowest_value) lamL2 = lowest_value;

            for (size_t my_cut = begin; my_cut < end; my_cut++) {

                if (s_arr[my_cut].x >= s_arr[my_cut + 1].x) {
                    assert(s_arr[my_cut].x == s_arr[my_cut + 1].x);
                    continue;
                }
                double y_sum_left = y_sum_arr[my_cut + 1] - y_sum_arr[begin];
                double weight_sum_left = w_sum_arr[my_cut + 1] - w_sum_arr[begin];

                double y_sum_right = y_sum_arr[end + 1] - y_sum_arr[my_cut + 1];
                double weight_sum_right = w_sum_arr[end + 1] - w_sum_arr[my_cut + 1];

                if (weight_sum_left + lowest_value < min_bucket_weights ||
                    weight_sum_right + lowest_value < min_bucket_weights) {
                    continue;
                }

                double pred_left = y_sum_left / (weight_sum_left + lamL2);
                double pred_right = y_sum_right / (weight_sum_right + lamL2);

                double pred_total = (y_sum_left + y_sum_right) / (weight_sum_left + weight_sum_right + 2 * lamL2);

                double obj_left = (weight_sum_left + lamL2) * pred_left * pred_left - 2 * pred_left * y_sum_left;
                double obj_right = (weight_sum_right + lamL2) * pred_right * pred_right - 2 * pred_right * y_sum_right;

                double obj_tot = (weight_sum_left + weight_sum_right + 2 * lamL2) * pred_total * pred_total -
                                 2 * pred_total * (y_sum_left + y_sum_right);

                double my_gain = obj_tot - (obj_left + obj_right);
                if (my_gain > gain) {
                    cut = my_cut;
                    gain = my_gain;
                }
            }

        }

        const bool operator<(const Bucket &b) const {
            return (gain < b.gain);
        }
    };

    float
    train(UniqueArray<float> &boundaries, double min_bucket_weights, unsigned int max_buckets, float lamL2, Elem *s,
          size_t n);
}

float
_discretizationTrainerDense::train(UniqueArray<float> &boundaries, double min_bucket_weights, unsigned int max_buckets,
                                   float lamL2, _discretizationTrainerDense::Elem *s, size_t n) {

    if (min_bucket_weights < 1e-3) min_bucket_weights = 1e-3;
    sort(s, s + n);

    vector<double> y_sum_vec;
    vector<double> w_sum_vec;

    double y_sum = 0;
    double w_sum = 0;

    y_sum_vec.push_back(y_sum);
    w_sum_vec.push_back(w_sum);

    size_t i;
    for (i = 0; i < n; i++) {
        double w = s[i].w;
        y_sum += s[i].y * w;
        y_sum_vec.push_back(y_sum);
        w_sum += w;
        w_sum_vec.push_back(w_sum);
    }

    double total_gain = 0;
    priority_queue<Bucket, vector<Bucket> > qu;

    qu.push(Bucket(0, n - 1, s, y_sum_vec.data(), w_sum_vec.data(), min_bucket_weights, lamL2));

    int nbuckets = 1;
    vector<float> b_vec;

    while (nbuckets < max_buckets && qu.size() > 0) {
        Bucket b = qu.top();
        qu.pop();
        if (b.cut > n - 2 || b.gain <= 0 || (s[b.cut].x >= s[b.cut + 1].x)) continue;
        total_gain += b.gain;
        b_vec.push_back(0.5 * (s[b.cut].x + s[b.cut + 1].x));
        nbuckets++;
        qu.push(Bucket(b.begin, b.cut, s, y_sum_vec.data(), w_sum_vec.data(), min_bucket_weights, lamL2));
        qu.push(Bucket(b.cut + 1, b.end, s, y_sum_vec.data(), w_sum_vec.data(), min_bucket_weights, lamL2));
    }

    sort(b_vec.begin(), b_vec.end());
    boundaries.reset(b_vec.size());

    for (i = 0; i < b_vec.size(); i++) {
        boundaries[i] = b_vec[i];
    }
    return total_gain;
}


template<typename i_t>
void FeatureDiscretizationDense::train(DataSet<float, i_t, float> &ds, int j,
                                       FeatureDiscretizationDense::TrainParm &tr) {
    using namespace _discretizationTrainerDense;
    UniqueArray<Elem> s;
    s.reset(ds.size());

    double total_w = 1e-10;

    for (size_t i = 0; i < ds.size(); i++) {
        Elem tmp;
        tmp.x = ds.x_dense[i][j];
        tmp.y = ds.y[i];
        tmp.w = ds.row_weights.size() > 0 ? ds.row_weights[i] : 1.0;
        s[i] = tmp;
        total_w += tmp.w;
    }
    total_w = ds.size() / total_w;

    if (total_w < 1.0) total_w = 1.0;
    for (size_t i = 0; i < s.size(); i++) {
        s[i].w *= total_w;
    }
    _discretizationTrainerDense::train(boundaries, tr.min_bucket_weights.value, tr.max_buckets.value, tr.lamL2.value,
                                       s.get(), s.size());
}

void FeatureDiscretizationDense::read(istream &is) {
    int n;
    MyIO::read(is, n);
    boundaries.reset(n);
    for (int i = 0; i < n; i++) {
        MyIO::read<float>(is, boundaries[i]);
    }
}


void FeatureDiscretizationDense::write(ostream &os) {
    int n = boundaries.size();
    MyIO::write(os, n);
    for (size_t i = 0; i < n; i++) {
        MyIO::write<float>(os, boundaries[i]);
    }
}


template<typename src_i_t, typename dest_d_t, typename dest_i_t, typename dest_v_t>
void DataDiscretization<src_i_t, dest_d_t, dest_i_t, dest_v_t>::train(DataSet<float, src_i_t, float> &ds,
                                                                      FeatureDiscretizationDense::TrainParm &tr_dense,
                                                                      typename FeatureDiscretizationSparse<src_i_t, dest_i_t, dest_v_t>::TrainParam &tr_sparse,
                                                                      int nthread, int verbose) {

    if (tr_dense.max_buckets.value + 1 >= numeric_limits<dest_d_t>::max()) {
        cerr << "maximum dense discretization bucket size " << tr_dense.max_buckets.value
             << "is more than what's allowed" << endl;
        exit(-1);
    }

    if (tr_sparse.max_buckets.value + 1 >= numeric_limits<dest_v_t>::max()) {
        cerr << "maximum sparse discretization bucket size " << tr_dense.max_buckets.value
             << "is more than what's allowed" << endl;
        exit(-1);
    }

    MapReduceRunner runner(nthread, MapReduceRunner::INTERLEAVE);
    int j;
    disc_dense.reset(ds.dim_dense());

    if (ds.dim_dense() > 0) {
        class DenseDiscMR : public MapReduce {
        public:
            DataSet<float, src_i_t, float> *ds_ptr;
            FeatureDiscretizationDense *disc_dense_ptr;
            FeatureDiscretizationDense::TrainParm *tr_dense_ptr;

            void map(int tid, int j) {
                disc_dense_ptr[i].train(*ds_ptr, j, *tr_dense_ptr);
            }
        } mr;

        mr.disc_dense_ptr = disc_dense.get();
        mr.ds_ptr = &ds;
        mr.tr_dense_ptr = &tr_dense;
        runner.run(mr, 0, ds.dim_dense());
    }

    // sparse train and set

    offset_init();
    return;

}
