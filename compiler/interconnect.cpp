
#include "interconnect.hpp"

#include <cmath>

InterconnectBase *generate_interconnect(UnsignedInt n, InterconnectType type) {
#define XYZ(id, class) \
    if (((unsigned) type) & (unsigned) InterconnectType :: id) \
        return new class (n);
    
    XYZ(benes_vanilla, Benes)
    XYZ(benes_copy, BenesWithCopy)
    XYZ(crossbar, Crossbar)
#undef XYZ
    if (((unsigned) type) & (unsigned) InterconnectType::banyan) {
        bool expansion = (unsigned) type & 0x0Fu;
        auto banyan = new Banyan(n + expansion);
        banyan->set_expansion(expansion);
        return banyan;
    }

    // TODO add crossbar here
    return nullptr;
}

Interconnects::Interconnects() {
    x_interconnect = nullptr;
    w_interconnect = nullptr;
    pin_interconnect = nullptr;
    pout_interconnect = nullptr;
    pp_in1_interconnect = nullptr;
    pp_in2_interconnect = nullptr;
    pp_out_interconnect = nullptr;
}

Interconnects::Interconnects(int N, InterconnectType interconnect_type) {
    // TODO find a more efficient algo for this
    int n = std::ceil(std::log2(N));

    x_interconnect = generate_interconnect(n, interconnect_type);
    w_interconnect = generate_interconnect(n, interconnect_type);
    pin_interconnect = generate_interconnect(n, interconnect_type);
    pout_interconnect = generate_interconnect(n, interconnect_type);
    pp_in1_interconnect = generate_interconnect(n, interconnect_type);
    pp_in2_interconnect = generate_interconnect(n, interconnect_type);
    pp_out_interconnect = generate_interconnect(n, interconnect_type);
}

float Interconnects::tdp(int switch_width){
    float tdp = 0;
    tdp += this->x_interconnect->power(switch_width);
    tdp += this->w_interconnect->power(switch_width);
    tdp += this->pin_interconnect->power(switch_width);
    tdp += this->pout_interconnect->power(switch_width);
    tdp += this->pp_in1_interconnect->power(switch_width);
    tdp += this->pp_in2_interconnect->power(switch_width);
    tdp += this->pp_out_interconnect->power(switch_width);
    return tdp;
}




std::istream &operator>>(std::istream &in, InterconnectType &interconnect_type) {
    std::string token;
    in >> token;
#define XYZ(x) else if (token == #x) interconnect_type = InterconnectType :: x;
    if (false) ;
    XYZ(benes_copy)
    XYZ(benes_vanilla)
    XYZ(banyan)
    XYZ(banyan_exp_0)
    XYZ(banyan_exp_1)
    XYZ(banyan_exp_2)
    XYZ(banyan_exp_3)
    XYZ(banyan_exp_4)
    XYZ(crossbar)
    else {
        in.setstate(std::ios_base::failbit);
    }
#undef XYZ
    return in;
}

std::ostream &operator<<(std::ostream &out, InterconnectType interconnect_type) {
#define XYZ(x) else if ((unsigned) interconnect_type & (unsigned) InterconnectType :: x) out << #x;
    if (false) ;
    XYZ(benes_copy)
    XYZ(benes_vanilla)
    XYZ(banyan)
    XYZ(crossbar)
#undef XYZ
    else if ((unsigned) interconnect_type & (unsigned) InterconnectType::banyan) {
        out << "banyan_exp_" << ((unsigned) interconnect_type & 0x0Fu);
    }
    else {
        out.setstate(std::ios_base::failbit);
    }
    return out;
}

Interconnects *Interconnects::clone() const {
    Interconnects *r = new Interconnects;
#define CLONE(x) r->x = this->x->clone();
    CLONE(x_interconnect)
    CLONE(w_interconnect)
    CLONE(pin_interconnect)
    CLONE(pout_interconnect)
    CLONE(pp_in1_interconnect)
    CLONE(pp_in2_interconnect)
    CLONE(pp_out_interconnect)
#undef CLONE
    return r;
}

Interconnects::~Interconnects() {
#define DELETE(x) delete this->x;
    DELETE(x_interconnect)
    DELETE(w_interconnect)
    DELETE(pin_interconnect)
    DELETE(pout_interconnect)
    DELETE(pp_in1_interconnect)
    DELETE(pp_in2_interconnect)
    DELETE(pp_out_interconnect)
#undef DELETE
}
