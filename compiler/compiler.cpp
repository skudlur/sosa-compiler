

#include "compiler.hpp"


using namespace std;


Compiler::Compiler(Arrays* arrays, Banks* banks, Interconnects* interconnects, PostProcessors* post_processors){
    this->arrays = arrays;
    this->banks = banks;
    this->interconnects = interconnects;
    this->post_processors = post_processors;
}

void Compiler::compile(Layers* layers){
    int init_round = 0;

    while (!layers->all_layers_scheduled()){
        for(auto it = layers->begin(); it != layers->end(); it++){
            if (it->is_scheduled) continue;
            compile_layer(&(*it), init_round);
        }
    }
}

void Compiler::compile_layer(Layer* layer, int init_round){
    layer->create_main_ops();
    layer->init_banks(this->banks);

    for (auto it = layer->main_ops.begin(); it != layer->main_ops.end(); it++){
        MultOp* op = it->second;

        int r = init_round;
        while (!op->is_placed()){
            this->op_placement(r, op);

            r++;
        }
    }

    layer->create_post_ops(this->arrays, this->interconnects);

    for (int i = 0; i < get<0>(layer->no_tiles); i++){
        for (int k = 0; k < get<2>(layer->no_tiles); k++){
            
            list<AggrOp*>* post_op_list = &layer->post_ops[make_tuple(i,k)];

            for (auto it = post_op_list->begin(); it != post_op_list->end(); it++){
                AggrOp* post_op = *it;

                int r = post_op->max_round()+1;
                while(!post_op->is_placed()){
                    this->post_op_placement(r, post_op);
                    r++;
                }
            }
        }
    }

    layer->is_scheduled = true;
}

void Compiler::post_op_placement(int r, AggrOp* op){
    Op* in_op1 = op->get_op1();
    Op* in_op2 = op->get_op2();

    list<PostProcessor*> avail_pps = this->post_processors->available_pps(r);
    if (avail_pps.empty()){
        return;
    }

    map<Bank*, PostProcessor*> pin1_permute = this->post_processors->get_pin1_permute(r);
    if(this->post_processors->check_pin1_bank_conflict(r, in_op1->pout_tile)){
        return;
    }

    map<Bank*, PostProcessor*> pin2_permute = this->post_processors->get_pin2_permute(r);
    if(this->post_processors->check_pin2_bank_conflict(r, in_op2->pout_tile)){
        return;
    }

    map<Bank*, PostProcessor*> pout_permute = this->post_processors->get_pout_permute(r);
    list<Bank*> avail_pout_banks;
    if (op->pout_tile->bank != nullptr){
        if(this->post_processors->check_pout_bank_conflict(r, op->pout_tile)){
            return;
        }
        avail_pout_banks.push_back(op->pout_tile->bank);
    }
    else{
        avail_pout_banks = this->banks->get_p_banks();
        for (auto it = pout_permute.begin(); it != pout_permute.end(); it++){
            avail_pout_banks.remove(it->first);
        }
    }
    if(avail_pout_banks.empty()) return;

    this->interconnects->pp_in1_interconnect->apply_permute(pin1_permute);
    this->interconnects->pp_in2_interconnect->apply_permute(pin2_permute);
    this->interconnects->pp_out_interconnect->apply_permute(pout_permute);

    for (auto pp_it = avail_pps.begin(); pp_it != avail_pps.end(); pp_it++){
        if (!this->interconnects->pp_in1_interconnect->is_route_free(in_op1->pout_tile->bank, *pp_it)){
            continue;
        }
        if (!this->interconnects->pp_in2_interconnect->is_route_free(in_op2->pout_tile->bank, *pp_it)){
            continue;
        }

        for (auto pout_it = avail_pout_banks.begin(); pout_it != avail_pout_banks.end(); pout_it++){
            if (!this->interconnects->pp_out_interconnect->is_route_free(*pout_it, *pp_it)){
                continue;
            }

            op->pout_tile->assign_bank(*pout_it);
            (*pp_it)->assign_op(r, op);
            return;
        }
    }
}


void Compiler::op_placement(int r, MultOp* op){
    list<Array*> avail_arrays = this->arrays->available_arrays(r);
    if (avail_arrays.empty()){
        return;
    }

    map<Bank*, Array*> pout_permute = this->arrays->get_pout_permute(r);
    list<Bank*> avail_pout_banks;

    if (op->pout_tile->bank != nullptr){
        if (this->arrays->check_pout_bank_conflict(r, op->pout_tile)){
            return;
        }
        avail_pout_banks.push_back(op->pout_tile->bank);
    }
    else{
        avail_pout_banks = this->banks->get_p_banks();
        for (auto it = pout_permute.begin(); it != pout_permute.end(); it++){
            avail_pout_banks.remove(it->first);
        }
    }
    if(avail_pout_banks.empty()) return;
    
    map<Bank*, Array*> x_permute = this->arrays->get_x_permute(r);
    list<Bank*> avail_x_banks;
    if (op->x_tile->bank != nullptr){
        if (this->arrays->check_x_bank_conflict(r, op->x_tile)){
            return;
        }
        
        avail_x_banks.push_back(op->x_tile->bank);
    }
    else{
        avail_x_banks = this->banks->get_x_banks();
        for (auto it = x_permute.begin(); it != x_permute.end(); it++){
            avail_x_banks.remove(it->first);
        }
    }
    if(avail_x_banks.empty()) return;
    
    map<Bank*, Array*> w_permute = this->arrays->get_w_permute(r);
    list<Bank*> avail_w_banks;
    if (op->w_tile->bank != nullptr){
        if (this->arrays->check_w_bank_conflict(r, op->w_tile)){
            return;
        }

        avail_w_banks.push_back(op->w_tile->bank);
    }
    else{
        avail_w_banks = this->banks->get_w_banks();
        for (auto it = w_permute.begin(); it != w_permute.end(); it++){
            avail_w_banks.remove(it->first);
        }
    }
    if(avail_w_banks.empty()) return;

    this->interconnects->pout_interconnect->apply_permute(pout_permute);
    this->interconnects->x_interconnect->apply_permute(x_permute);    
    this->interconnects->w_interconnect->apply_permute(w_permute);    

    for(auto sa_it = avail_arrays.begin(); sa_it != avail_arrays.end(); sa_it++){
        for(auto x_bank_it = avail_x_banks.begin(); x_bank_it != avail_x_banks.end(); x_bank_it++){
            if (!this->interconnects->x_interconnect->is_route_free(*x_bank_it, *sa_it)){
                continue;
            }

            for(auto w_bank_it = avail_w_banks.begin(); w_bank_it != avail_w_banks.end(); w_bank_it++){
                if (!this->interconnects->w_interconnect->is_route_free(*w_bank_it, *sa_it)){
                    continue;
                }

                for(auto p_bank_it = avail_pout_banks.begin(); p_bank_it != avail_pout_banks.end(); p_bank_it++){
                    if (!this->interconnects->pout_interconnect->is_route_free(*p_bank_it, *sa_it)){
                        continue;
                    }

                    op->x_tile->assign_bank(*x_bank_it);
                    op->w_tile->assign_bank(*w_bank_it);
                    op->pout_tile->assign_bank(*p_bank_it);
                    (*sa_it)->assign_op(r, op);
                    return;

                }
            }
        }
    }

}


int Compiler::no_main_rounds(){
    int max_rounds = 0;
    for(auto it = this->arrays->array_map.begin(); it != this->arrays->array_map.end(); it++ ){
        if (it->second->last_no_round > max_rounds){
            max_rounds = it->second->last_no_round;
        }
    }
    return max_rounds;
}

int Compiler::no_post_rounds(){
    int max_rounds = 0;
    for(auto it = this->post_processors->pp_map.begin(); it != this->post_processors->pp_map.end(); it++ ){
        if (it->second->last_no_round > max_rounds){
            max_rounds = it->second->last_no_round;
        }
    }
    return max_rounds;
}