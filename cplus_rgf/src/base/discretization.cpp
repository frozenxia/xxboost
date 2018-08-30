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
                                       FeatureDiscretizationDense::TrainParam &tr) {
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
                                                                      FeatureDiscretizationDense::TrainParam &tr_dense,
                                                                      typename FeatureDiscretizationSparse<src_i_t, dest_i_t, dest_v_t>::TrainParam &tr_sparse,
                                                                      int nthread, int verbose) {

    if (tr_dense.max_buckets.value + 1 >= numeric_limits<dest_d_t>::max()) {
        cerr << "maximum dense discretization bucket size " << tr_dense.max_buckets.value
             << "is more than what's allowed" << endl;
        exit(-1);
    }

    if (tr_sparse.max_buckets.value + 1 >= numeric_limits<dest_v_t>::max()) {
        cerr << "maximum sparse discretization bucket size " << tr_sparse.max_buckets.value
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
            FeatureDiscretizationDense::TrainParam *tr_dense_ptr;

            void map(int tid, int j) {
//                cout << "j:" << j << endl;
                disc_dense_ptr[j].train(*ds_ptr, j, *tr_dense_ptr);
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

template<typename src_i_t, typename dest_d_t, typename dest_i_t, typename dest_v_t>
void DataDiscretization<src_i_t, dest_d_t, dest_i_t, dest_v_t>::offset_init() {
    _offset.clear();
    size_t v = disc_dense.size();
    _offset.push_back(v);
    for (int j = 0; j < disc_sparse.size(); j++) {
        v += disc_sparse[j].size();
        _offset.push_back(v);
    }

}

template<typename feat_t, typename id_t, typename disc_t>
void FeatureDiscretizationSparse<feat_t, id_t, disc_t>::train(DataSet<float, feat_t, float> &ds, int j,
                                                              TrainParam &tr,
                                                              int nthreads, int verbose) {
    bool use_omp = false;
#ifdef USE_OMP
    use_omp = true;
#endif
    class DataPartition {
    public:
        int nthreads;
        UniqueArray<size_t> data_offset;
        vector<size_t> data_index[256];

        bool valid() {
            return (nthreads > 1);
        }

        unsigned int feat2tid(size_t feat) {
            unsigned char r;
            char *s = (char *) &feat;
            for (int j = 0; j < sizeof(size_t); j++) {
                r = r * 97 + s[j];
            }
            return ((unsigned int) r) % (unsigned int) nthreads;
        }

        bool loop_init(size_t &i, size_t &k, size_t &pos, size_t tid) {
            i = 0;
            k = 0;
            pos = 0;

            return (pos < data_index[tid].size());
        }

        bool loop_next(size_t &i, size_t &k, size_t &pos, size_t tid) {
            if (pos > data_index[tid].size()) return false;
            size_t nitems = data_index[tid][pos];
            while (data_offset[i + 1] <= nitems) i++;
            k = nitems - data_offset[i];
            pos++;
            return true;
        }
    } th2data;

    th2data.nthreads = (nthreads <= 256) ? nthreads : 0;
//    if (th2data.valid() && use_omp) th2data.data_offset.reset(ds.size() + 1);
    if (th2data.valid() && use_omp) {
        th2data.data_offset.reset(ds.size() + 1);
    }

    using namespace _discretizationTrainerDense;
    Timer t;
    t = Timer("feature_id counting and filtering");
    t.start();

    size_t max_index = 0;

    if (th2data.valid() && use_omp) {
        size_t i;
        size_t nitems = 0;
        for (i = 0; i < ds.size(); i++) {
            th2data.data_offset[i] = nitems;
            nitems += ((ds.x_sparse[i])[j]).size();
        }

        th2data.data_offset[ds.size()] = nitems;
        UniqueArray<unsigned char> tid_arr(nitems);

#ifdef  USE_OMP
        omp_set_num_threads(th2data.nthreads);
#endif
#pragma omp parallel for
        for (i = 0; i < ds.size(); i++) {
            size_t nitems = th2data.data_offset[i];
            auto tmp = &((ds.x_sparse[i])[j]);
            for (size_t k = 0; k < tmp->size(); k++) {
                size_t feat = (*tmp)[k].index;
                if (max_index < feat) max_index = feat;
                tid_arr[nitems++] = th2data.feat2tid(feat);
            }
        }

        for (i = 0; i < tid_arr.size(); i++) {
            th2data.data_index[tid_arr[i]].push_back(i);
        }
    } else {
        for (size_t i = 0; i < ds.size(); i++) {
//            size_t nitems = th2data.data_offset[i];
            auto tmp = &((ds.x_sparse[i])[j]);
            for (size_t k = 0; k < tmp->size(); k++) {
                size_t feat = (*tmp)[k].index;
                if (max_index < feat) max_index = feat;
            }
        }
    }

    size_t id = 0;
    vector<size_t> id_counts;
    vector<feat_t> id2feat_vec;

    double min_counts = std::max(1, tr.min_occurrences.value);

    UniqueArray<int32_t> feat2id_count_arr;

    bool use_arr = (max_index < (numeric_limits<int32_t>::max() / 2 - 1));
    if (!use_arr) {
        unordered_map<feat_t, size_t> feat2id_count_hash;
        for (size_t i = 0; i < ds.size(); i++) {
            auto tmp = &((ds.x_sparse[i])[j]);
            for (size_t k = 0; k < tmp->size(); k++) {
                ++feat2id_count_hash[(*tmp)[k].index];
            }
        }

        for (auto it = feat2id_count_hash.begin(); it != feat2id_count_hash.end(); it++) {
            if (it->second >= min_counts) {
                id_counts.push_back(it->second);
                id2feat_vec.push_back(it->first);
                feat2id[it->first] = id++;
            }
        }
    } else {
        feat2id_count_arr.resize(max_index + 1);
        memset(feat2id_count_arr.get(), 0, sizeof(int32_t) * feat2id_count_arr.size());
        if (th2data.valid() && use_omp) {
#ifdef  USE_OMP
            auto mapper = [j, &th2data, &feat2id_count_arr, &ds](int tid) {
                size_t pi, i, k, pos;
                if (th2data.loop_init(i, k, pos, tid)) {
                    pi = i;
                    auto tmp = &((ds.x_sparse[i])[j]);
                    while (th2data.loop_next(i, k, pos, tid)) {
                        if (pi != i) {
                            tmp = &((ds.x_sparse[i])[j]);
                        }
                        ++feat2id_count_arr[(*tmp)[k].index];
                        pi = i;
                    }
                }
            };
            omp_set_num_threads(th2data.nthreads);
#pragma omp parallel for
            for (int tid = 0; tid < th2data.nthreads; tid++) {
                mapper(tid);
            }
#endif
        } else {

        }
    }
//    omp_set_num_threads(th2data.nthreads);
}


namespace rgf {
    template
    class DataDiscretization<src_index_t, int, int, int>;
}