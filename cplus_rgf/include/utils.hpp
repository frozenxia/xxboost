#ifndef _RGF_UTILS_H
#define _RGF_UTILS_H
#include "header.hpp"

template <typename T>
class UniqueArray
{
    UniqueArray(const UniqueArray &) = delete;
    UniqueArray &operator=(const UniqueArray &) = delete;
    size_t _num;
    unique_ptr<T[]> _data;

  public:
    UniqueArray() : _num(0), _data(nullptr) {}

    UniqueArray(size_t n):_num(0),_data(nullptr){
        reset(n);
    }
    //&& rvalue reference, just like & 
    // =default means use system generate function
    UniqueArray(UniqueArray&&)=default; 
    UniqueArray & operator = (UniqueArray &&) = default;

    size_t size(){return _num;}

    T* get(){return _data.get();}

    T* begin(){
        return get();
    }

    T* end(){
        return get() + size();
    }
    void reset(size_t n){
        _num = n;
        if (n < 0){
            _data.reset(nullptr);
        }
        else{
            _data.reset(new T[n]);
        }
    }

    void resize(size_t n){
        if (n <= _num){
            _num = n;
            return ;
        }

        T * ptr = new T[n];

        memcpy(ptr,get(),sizeof(T)*_num);
        _num = n;
        _data.reset(ptr);
    }

    void clear(){
        _num = 0;
        _data.reset(nullptr);
    }

    T & operator [] (size_t i) {return _data[i];}
};


class ParameterParser{
    public:
        class ParamValueBase{
            public:
                string default_value;
                string description;
                string parsed_value;
                bool is_valid;
                virtual void set_value()=0;
        };
    
    private:
        static string to_string(string str){return str;}
        static string to_string(bool value){return value? "true":"false";}

        template<typename T> static string to_string(T value){return std::to_string(value);}

        vector<pair<string,ParamValueBase* > > _kv_table;

        string _description;

    public:
        template<typename T> 
            class ParamValue: public ParamValueBase{
                public:
                    T value;
                    T default_value_T;
                    ParamValue(){}

                    void insert(string key,T _default,string description,ParameterParser *pp,bool is_valid=true){
                        value = default_value_T = _default;
                        default_value = to_string(_default);
                        parsed_value = default_value;
                        description = description;
                        pp->init_insert(key,this);
                    }

                    virtual void set_value(){
                        if (parsed_value != ""){
                            stringstream convert(parsed_value);
                            convert >> value;
                        }else{
                            value = default_value_T;
                        }
                        is_valid=true;
                    }

                    void set_value(T v){
                        value = v;
                        parsed_value = to_string(v);
                        is_valid = true;
                    }
            };

        
        void init_insert(string key,ParamValueBase *value){
            _kv_table.push_back(pair<string,ParamValueBase*>(key,value));
        }

        bool parse_and_assign(string token);

        void print_parameters(ostream & os ,string indent= "  ");

        void print_options(ostream & os, string indent = "  ");

        void set_description(string desc){
            _description =  desc;
        }

        void clear(){
            _kv_table.clear();
        }
};



#endif