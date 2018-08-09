#include<vector>
using std::vector

#include <string>

using std::string;


class Matrix{
    private:
        vector<vector<double>> elements;
        vector<string> column_labels;
        vector<string> row_labels;

    public:
        Matrix();
        void load(string filename,bool use_column_labels=true,bool use_row_labels=true);
        void save(string filename);
        int columns();
        int rows();
        vector<double> column(int index);
        Matrix submatrix(vector<int> rows,vector<int>columns);
        void split(int column_index,double value,Matrix&m1,Matrix&m2);
        Matrix shuffled();
        void append_column(vecotr<double> &col,string name="");
        void merge_rows(Matrix&other);
        //bracket operator overload
        void<double>& operator[](int i);
}