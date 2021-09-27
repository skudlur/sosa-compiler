#ifndef COMPILER_HPP
#define COMPILER_HPP


#include <list>
#include <iostream>
#include <algorithm>
#include <memory>

#include "layer.hpp"
#include "array.hpp"
#include "interconnect.hpp"
#include "post_processor.hpp"

#include <boost/log/trivial.hpp>

using namespace std;

class Compiler{
    public:
        Arrays* arrays;
        Banks* banks;
        Interconnects* interconnects;
        PostProcessors* post_processors;
        // mt19937* random_generator;


        Compiler(Arrays* arrays, Banks* banks, Interconnects* interconnects, PostProcessors* post_processors);
        void compile(Layers* layers);
        void compile_layer(Layer* layer, int init_round);
        void op_placement(int r, MultOp* op);
        void post_op_placement(int r, AggrOp* op);
        int no_main_rounds();
        int no_post_rounds();
        int get_cycles();
    private:
};

#endif /* COMPILER_HPP */
