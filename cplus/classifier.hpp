#pragma once

#include "matrix.hpp"
#include <iostream>


class Classifier{
    public:
        virtual void train(Matrix&m) = 0;
        virtual double classify(vector<double> row) = 0;
}