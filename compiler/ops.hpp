#ifndef OPS_HPP
#define OPS_HPP


#include <string>
#include <tuple>
#include <cassert>

#include "tiles.hpp"
#include "post_processor.hpp"

using namespace std;

class Array;
class PostProcessor;

class Op{
    public:
        string layer_name;
        
        int round_placed;
        tuple<int, int, int> op_ind;

        bool is_multop;

        P_Tile* pout_tile;
        Op* pair_op;

        bool retired;
        
        bool is_placed();
    protected:
        bool is_placed_;
};

class MultOp: public Op{
    public:
        X_Tile* x_tile;
        W_Tile* w_tile;

        MultOp* pin_op;
        MultOp* aggregated_to;
        Array* array_placed;

        int exec_cycles;
        int weight_buffer_cycles;

        MultOp(string layer_name, tuple<int, int, int> op_ind, X_Tile* x_tile, W_Tile* w_tile, P_Tile* pout_tile);
        MultOp(const MultOp&);

        void assign_pin(MultOp* pin_op);
        void assign_to_array(int r, Array* array);

        void retire();
};

class AggrOp: public Op{
    public:
        
        P_Tile* pin1_tile;
        P_Tile* pin2_tile;

        Op* operand1;
        Op* operand2;

        int max_round();

        PostProcessor* pp_placed;

        bool flip; //this allows distinguishing same pairs of operations.

        AggrOp(string layer_name, tuple<int, int, int> op_ind, Op* operand1, Op* operand2, P_Tile* pout_tile, bool flip);
        AggrOp(const AggrOp&) = default;

        void assign_to_pp(int r, PostProcessor* pp);
        Op* get_op1();
        Op* get_op2();
        void set_pair(AggrOp* pair);
};

#endif /* OPS_HPP */
