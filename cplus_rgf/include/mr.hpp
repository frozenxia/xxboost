#ifndef _RGF_MR_H
#define _RGF_MR_H
#include "header.hpp"
#include "data.hpp"
class MapReduce{
    public:
        void map(int tid,int j){}
        void map_range(int tid,int begin,int end){}
        void reduce(int tid){}
        void master(){}
};


class MapReduceRunner{
    private:
        vector<thread> _th;
    public: 
        enum par_t{
            DYNAMIC = 0,
            INTERLEAVE = 1,
            BLOCK =2
        } parallel_mode;

        int nthreads;

        MapReduceRunner(int nthrds= 0 , enum par_t par_mode = INTERLEAVE){
            set(nthrds,par_mode);
        }
        static unsigned int max_nthreads(){
            int results = std::thread::hardware_concurrency();
            return results <1 ? 1: results;
        }

        static unsigned int num_threads(int nthrds){
            int results = nthrds;
            int _max_threads  = max_nthreads();
            if (results <= 0 || results > _max_threads ){
                results = _max_threads;
            }
            return results;
        }

        void set(int nthrds = 0, enum par_t par_mode = INTERLEAVE){
            nthreads = num_threads(nthrds);
            _th.resize(nthreads);
            parallel_mode = par_mode;
        }

        template<typename T> void single_thread_map_reduce(T &mr,int begin,int end,int tid,int nthreads,bool run_range){
            int j ;
            if (run_range){
                int block_size = 1 + (int)((end-1-begin)/nthreads);
                int my_begin = begin + tid * block_size;
                int my_end = min(end,(tid+1)*block_size);
                mr.map_range(tid,my_begin,my_end);
                return;
            }

            switch (parallel_mode){
                case INTERLEAVE:
                    for (j = begin + tid; j < end;j += nthreads){
                        mr.map(tid,j);
                    }
                    break;
                default:
                    {
                        int block_size = 1 + (int)((end-1-begin)/nthreads);
                        int my_begin = begin + tid * block_size;
                        int my_end = min(end,(tid+1)*block_size);

                        for (j = my_begin ; j < my_end;j ++){
                            mr.map(tid,j);
                        }
                    }
            }
        }


        template<typename T> 
        void run_threads(T &mr,int begin,int end,bool run_range){
            int tid;

            if (nthreads <= 1){
                mr.master();
                single_thread_map_reduce(mr,begin,end,0,1,run_range);
                mr.reduce(0);
                return;
            }

            static const bool use_omp = true;

            #ifndef USE_OMP
                for (tid = 0; tid < nthreads;tid ++){
                    _th[tid] = thread(&MapReduceRunner::single_thread_map_reduce<T>,this,std::ref(mr),begin,end,tid,nthreads,run_range);
                }
            #else
                omp_set_num_threads(nthreads);
                #pragma omp parallel for 

                for (tid = 0; tid < nthreads; tid ++){
                    single_thread_map_reduce<T>(std::ref(mr),begin,end,tid,nthreads,run_range);
                }
            #endif

            mr.master(); 
            for (tid = 0; tid < nthreads; tid ++){
                #ifndef USE_OMP
                    _th[tid].join();
                #else
                    mr.reduce(tid);
                #endif
            }
        }

        template<typename T> void run(T &mr,int begin,int end){
            run_threads(mr,begin,end,false);
        }

        template<typename T> void run_range(T &mr, int begin,int end){
            run_threads(mr,begin,end,true);
        }

};

class MyDataInputException : public std::exception{
    public:
        string error_message;
        int line_no;

        MyDataInputException(string m,int l):error_message(m),line_no(l){}
};

template<typename d_t,typename i_t,typename v_t>
class MyDataInputLineParseResult{
    public:
        double w_val;
        double y_val;
        vector<d_t> feats_dense;
        vector<SparseFeatureGroup<i_t,v_t> > feats_sparse;
        vector<SparseFeatureElement<i_t,v_t> > sparse_elem_vec;
        string line;

        static void parse_sparse_element(char* token_str,SparseFeatureElement<i_t,v_t> &result,int lno){
            size_t step = 0;
            for (step = 0; token_str[step] != 0 && token_str[step] != ':'; step ++);

            if (token_str[step] == 0){
                throw MyDataInputException(": not in the format of index:value",lno);
            }
            token_str[step] = 0;
            long tmp = atol(token_str);

            if (tmp >= numeric_limits<i_t>::max() || tmp < numeric_limits<i_t>::max() ){
                throw MyDataInputException("index out of range",lno);
            }
            result.index = tmp;

            if (is_same<v_t,float>::value ||is_same<v_t,double>::value){
                double tmp = atof(token_str + (step+1));
                if (tmp >= numeric_limits<v_t>::max()) tmp = numeric_limits<v_t>::max();
                if (tmp <= numeric_limits<v_t>::lowest()) tmp = numeric_limits<v_t>::lowest();
                if (!(tmp == tmp)) tmp = numeric_limits<v_t>::lowest();
                result.value = (float)tmp;
            }else{
                long tmp = atol(token_str + (step+1));
                if (tmp >= numeric_limits<v_t>::max() || tmp <= numeric_limits<v_t>::lowest()){
                    throw MyDataInputException(": value out of range",lno);
                }
                result.value = tmp;
            }
            return;
        }

        void parse_x(bool sparse_format,int lno){
            const char* str = line.c_str();
            vector<char> char_arr;
            do{
                const char * begin = str;
                while (!isspace(*str) && *str) str ++;
                if (str == begin) continue;

                string token(begin,str);

               try{
                    if (! sparse_format && token.find_first_of('!') == string::npos ){
                        double tmp = stod(token);
                        if (tmp >= numeric_limits<float>::max()) tmp = numeric_limits<float>::max();
                        if (tmp <= numeric_limits<float>::lowest()) tmp = numeric_limits<float>::lowest();
                        if (!(tmp == tmp)) tmp = numeric_limits<float>::lowest();
                        feats_dense.push_back(tmp);
                        continue;
                    }

                    SparseFeatureElement<i_t,v_t> result;
                    if (sparse_format){
                        char_arr.resize(token.size()+1);
                        memcpy(char_arr.data(),token.c_str(),token.size()+1);
                        char *token_str = char_arr.data();
                        parse_sparse_element(token_str,result,lno);
                        sparse_elem_vec.push_back(result);
                        continue;
                    }

                    sparse_elem_vec.clear();
                    char_arr.resize(token.size()+1);
                    memcpy(char_arr.data(),token.c_str(),token.size()+1);
                    char *token_str = char_arr.data();

                    size_t pos ;
                    bool is_end = false;
                    while(true){
                        for(pos = 0; token_str[pos] != 0 && token_str[pos] != '|' ; pos ++);
                        if (pos == 0) break;

                        if (token_str[pos] == 0){
                            is_end = true;
                        }

                        parse_sparse_element(token_str,result,lno);
                        sparse_elem_vec.push_back(result);

                        if (is_end) break;
                        token_str = token_str + (pos+1);
                        
                    }
               }catch(MyDataInputException &e){
                   e.error_message = "cannot parse token " + token + e.error_message ;
                   throw(e);
               }

               SparseFeatureGroup<i_t,v_t> tmp (sparse_elem_vec.size());
               for (size_t ii = 0;ii < tmp.size(); ii ++){
                   tmp[ii]  = sparse_elem_vec[ii];
               }
               feats_sparse.push_back(std::move(tmp));
            }while(0!= *str++);

            if (sparse_format){
                SparseFeatureGroup<i_t,v_t> tmp(sparse_elem_vec.size());
                for (size_t ii = 0;ii < tmp.size(); ii ++){
                   tmp[ii]  = sparse_elem_vec[ii];
                }
                feats_sparse.push_back(std::move(tmp));
            }

            return;
        }
};



template<typename d_t,typename i_t,typename v_t> 
class MyDataInputLineParserMR: public MapReduce{
    private:
        istream *is_x_ptr;
        istream *is_y_ptr;
        istream *is_w_ptr;

        bool w_format;
        bool y_format;
        bool sparse_format;
        mutex io_mutex;


    public:
        bool read_x_only;
        bool use_uniform_weights;
        bool is_eof;
        int lines_read;
        vector<MyDataInputLineParseResult<d_t,i_t,v_t> > ps;

        MyDataInputLineParserMR(istream & is_x,istream & is_y,istream &is_w,bool y_valid, bool w_valid,string is_x_format,int batch_size){
            is_x_ptr = &is_x;
            is_y_ptr = y_valid ? (&is_y): nullptr;
            is_w_ptr = w_valid ? (&is_w): nullptr;

            w_format = (is_x_format.find('w') != string::npos);
            y_format = (is_x_format.find('y') != string::npos);
            sparse_format = (is_x_format.find("sparse") != string::npos);

            read_x_only = (!y_valid) && (!w_valid);
            use_uniform_weights = (!w_format) && (!w_valid);

            is_eof = false;
            lines_read = 0;
            ps.resize(batch_size);
        }

        bool read_line(int &j){
            lock_guard<mutex> guard(io_mutex);

            if (is_eof){
                return false;
            }

            if (is_x_ptr == nullptr || is_x_ptr->eof()){
                is_eof = false;
                return false;
            }

            if (!is_x_ptr->good()){ 
                throw MyDataInputException("invalid feature file",0);
                return false;
            }

            if (lines_read >= ps.size()){
                
                return false;
            }

            j = lines_read;

            ps[j].w_val = 1.0;
            if (w_format) (*is_x_ptr) >> ps[j].w_val;
            if (is_w_ptr != nullptr){
                (*is_w_ptr) >> ps[j].w_val;
            }

            ps[j].y_val = 0;

            if(y_format) (*is_x_ptr) >> ps[j].y_val;
            if (is_y_ptr != nullptr){
                (*is_y_ptr) >> ps[j].y_val;
            }
            getline(*is_x_ptr,ps[j].line);
            is_eof = is_x_ptr->eof();

            if (is_w_ptr != nullptr && is_w_ptr->eof() != is_eof){
                throw MyDataInputException(
                    "number of lines in weight file does not match that of feature file", lines_read
                );
            }

            if (is_y_ptr != nullptr && is_y_ptr->eof() != is_eof){
                throw MyDataInputException(
                    "number of lines in label file does not match that of feature file", lines_read
                );
            }
            if (is_eof){
                return false;
            }
            lines_read ++;
            return true;
        }

        void map(int tid,int j){
            int jj ;
            while (read_line(jj)){
                ps[jj].parse_x(sparse_format,jj);
            }
        }
};

#endif