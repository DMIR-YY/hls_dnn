/*
    Copyright (c) 2013, Taiga Nomi
    All rights reserved.
    
    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the <organization> nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY 
    EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE 
    DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY 
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES 
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; 
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND 
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS 
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#pragma once
#include "../util/util.h"
#include "layer.h"

namespace tiny_dnn {

/**
 * element-wise add N vectors
 **/
class elementwise_add_layer : public layer {
public:
    elementwise_add_layer(cnn_size_t num_args, cnn_size_t dim)
    : layer(std::vector<int32_t>(num_args, vector_type::data), {vector_type::data}), num_args_(num_args), dim_(dim) {}

    std::string layer_type() const {//Yao: deleted override
        return "elementwise-add";
    }

    std::vector<shape3d> in_shape() const {//Yao: deleted override
        return std::vector<shape3d>(num_args_, shape3d(dim_,1,1));
    }

    std::vector<shape3d> out_shape() const {//Yao: deleted override
        std::vector<shape3d> temp;
        temp.push_back(shape3d(dim_ ,1,1));
//    	return{ shape3d(dim_,1,1) };
        return temp;
    }

    void forward_propagation(const std::vector<tensor_t*>& in_data,
                             std::vector<tensor_t*>& out_data) {//Yao: deleted override
        const tensor_t& in1 = *in_data[0];
        tensor_t& out = *out_data[0];

        out = in1;

        // @todo parallelize
        for (cnn_size_t sample = 0, sample_count = in1.size(); sample < sample_count; ++sample) {
            for (cnn_size_t i = 1; i < num_args_; i++) {
//                std::transform((*in_data[i])[sample].begin(),
//                               (*in_data[i])[sample].end(),
//                               out[sample].begin(),
//                               out[sample].begin(),
//                               [](float_t x, float_t y){ return x + y; });
                for (cnn_size_t j = 0; j < in_data[i]->size(); j++){
                	out[sample][j] = ( (*in_data[i])[sample][j] + out[sample][j] );
                }
            }
        }
    }

    void back_propagation(const std::vector<tensor_t*>& in_data,
                          const std::vector<tensor_t*>& out_data,
                          std::vector<tensor_t*>&       out_grad,
                          std::vector<tensor_t*>&       in_grad) {//Yao: deleted override
        CNN_UNREFERENCED_PARAMETER(in_data);
        CNN_UNREFERENCED_PARAMETER(out_data);
        for (cnn_size_t i = 0; i < num_args_; i++)
            *in_grad[i] = *out_grad[0];
    }
private:
    cnn_size_t num_args_;
    cnn_size_t dim_;
};

} // namespace tiny_dnn
