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
#include "../util/weight_init.h"
#include "../util/util.h"
#include "../layers/layer.h"
#include <algorithm>

using namespace tiny_dnn::weight_init;

namespace tiny_dnn {

/**
 * applies dropout to the input
 **/
class dropout_layer : public layer {
public:
    typedef activation::identity Activation;
    typedef layer Base;

    /**
     * @param in_dim       [in] number of elements of the input
     * @param dropout_rate [in] (0-1) fraction of the input units to be dropped
     * @param phase        [in] initial state of the dropout
     **/
    dropout_layer(cnn_size_t in_dim, float_t dropout_rate, net_phase phase = net_phase::train)
        : Base({vector_type::data}, {vector_type::data}),
          phase_(phase),
          dropout_rate_(dropout_rate),
          scale_(float_t(1) / (float_t(1) - dropout_rate_)),
          in_size_(in_dim)
    {
		mask_.resize(1, std::vector<uint8_t>(in_dim));
        clear_mask();
    }

    dropout_layer(const dropout_layer& obj) = default;
    virtual ~dropout_layer(){}

#ifdef CNN_USE_DEFAULT_MOVE_CONSTRUCTORS
    dropout_layer(dropout_layer&& obj) = default;
    dropout_layer& operator=(const dropout_layer& obj) = default;
    dropout_layer& operator=(dropout_layer&& obj) = default;
#endif

    void set_dropout_rate(float_t rate)
    {
        dropout_rate_ = rate;
        scale_ = float_t(1) / (float_t(1) - dropout_rate_);
    }

    ///< number of incoming connections for each output unit
    size_t fan_in_size() const //Yao: deleted override 
    {
        return 1;
    }

    ///< number of outgoing connections for each input unit
    size_t fan_out_size() const //Yao: deleted override
    {
        return 1;
    }

    std::vector<index3d<cnn_size_t>> in_shape() const {//Yao: deleted override
        return{ index3d<cnn_size_t>(in_size_, 1, 1) };
    }

    std::vector<index3d<cnn_size_t>> out_shape() const {//Yao: deleted override
        return{ index3d<cnn_size_t>(in_size_, 1, 1) };
    }

    void back_propagation(const std::vector<tensor_t*>& in_data,
                          const std::vector<tensor_t*>& out_data,
                          std::vector<tensor_t*>&       out_grad,
                          std::vector<tensor_t*>&       in_grad) {//Yao: deleted override
        tensor_t&       prev_delta = *in_grad[0];
        const tensor_t& curr_delta = *out_grad[0];

        CNN_UNREFERENCED_PARAMETER(in_data);
        CNN_UNREFERENCED_PARAMETER(out_data);

        for (cnn_size_t sample = 0, sample_count = prev_delta.size(); sample < sample_count; ++sample) {
            for (size_t i = 0; i < curr_delta.size(); i++) {
                prev_delta[sample][i] = mask_[sample][i] * curr_delta[sample][i];
            }
        }
    }

    void forward_propagation(const std::vector<tensor_t*>& in_data,
                             std::vector<tensor_t*>& out_data) {//Yao: deleted override
        const tensor_t& in  = *in_data[0];
        tensor_t&       out = *out_data[0];

        const cnn_size_t sample_count = in.size();

        if (mask_.size() < sample_count) {
            mask_.resize(sample_count, mask_[0]);
        }

        for (size_t sample = 0, sample_count = in.size(); sample < sample_count; ++sample) {

            std::vector<uint8_t>& mask = mask_[sample];

            const vec_t& in_vec = in[sample];
            vec_t& out_vec = out[sample];

            if (phase_ == net_phase::train) {
                for (size_t i = 0; i < in_vec.size(); i++)
                    mask[i] = my_bernoulli(dropout_rate_);
                    // mask[i] = (rand() < RAND_MAX * dropout_rate_);

                for (size_t i = 0; i < in_vec.size(); i++)
                    out_vec[i] = mask[i] * scale_ * in_vec[i];
            }
            else {
                for (size_t i = 0, end = in_vec.size(); i < end; i++)
                    out_vec[i] = in_vec[i];
            }
        }
    }

    /**
     * set dropout-context (training-phase or test-phase)
     **/
    void set_context(net_phase ctx) //Yao: deleted override 
    {
        phase_ = ctx;
    }

    std::string layer_type() const { return "dropout"; }//Yao: deleted override

    // currently used by tests only
    const std::vector<uint8_t>& get_mask(cnn_size_t sample_index) const {
        return mask_[sample_index];
    }

    void clear_mask() {
		for (cnn_size_t sample = 0, sample_count = mask_.size(); sample < sample_count; ++sample) {
			std::fill(mask_[sample].begin(), mask_[sample].end(), 0);
		}
    }

private:
    net_phase phase_;
    float_t dropout_rate_;
    float_t scale_;
    cnn_size_t in_size_;
	std::vector<std::vector<uint8_t> > mask_;
};

} // namespace tiny_dnn
