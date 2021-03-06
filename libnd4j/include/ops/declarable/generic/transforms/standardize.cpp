/*******************************************************************************
 * Copyright (c) 2015-2019 Skymind, Inc.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License, Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
 * License for the specific language governing permissions and limitations
 * under the License.
 *
 * SPDX-License-Identifier: Apache-2.0
 ******************************************************************************/

//
// @author Paul Dubs
//

#include <op_boilerplate.h>
#if NOT_EXCLUDED(OP_standardize)

#include <ops/declarable/CustomOperations.h>
#include <ops/declarable/helpers/reverse.h>


namespace nd4j {
namespace ops  {

    CONFIGURABLE_OP_IMPL(standardize, 1, 1, true, 0, -2) {
        
        auto input = INPUT_VARIABLE(0);
        auto output = OUTPUT_VARIABLE(0);
        
        std::vector<int> axis;        

        if (block.width() > 1)            
            axis = INPUT_VARIABLE(1)->template asVectorT<int>();
        else if (block.numI() > 0) 
            axis = *block.getIArguments();        

        REQUIRE_TRUE(!axis.empty(), 0, "STANDARDIZE OP: axis has to be non-empty")

        shape::checkDimensions(input->rankOf(), axis);

        auto means = input->reduceAlongDims(reduce::Mean, axis, true);
        auto stdev = *input->varianceAlongDimension(variance::SummaryStatsStandardDeviation, false, axis);
        stdev.reshapei(means.getShapeAsVector());

        input->applyTrueBroadcast(nd4j::BroadcastOpsTuple::Subtract(), &means, output, false);
        output->applyTrueBroadcast(nd4j::BroadcastOpsTuple::Divide(), &stdev, output, false);
        output->applyScalar(nd4j::scalar::ReplaceNans, 0, output, nullptr);

   
        return Status::OK();
    }


    DECLARE_TYPES(standardize) {
        getOpDescriptor()->setAllowedInputTypes(0, DataType::ANY);
        getOpDescriptor()->setAllowedInputTypes(1, {DataType::INT32, DataType::INT64});
        getOpDescriptor()->setAllowedOutputTypes(0, DataType::INHERIT);
    }

    CUSTOM_OP_IMPL(standardize_bp, 2, 1, false, 0, -2) {
        auto input = INPUT_VARIABLE(0);
        auto eps = block.width() == 3 ? INPUT_VARIABLE(2) : INPUT_VARIABLE(1);

        auto output = OUTPUT_VARIABLE(0);
        std::vector<int> axis;

        if (block.width() == 3)             
            axis = INPUT_VARIABLE(1)->template asVectorT<int>();
        else if (block.numI() > 0) 
            axis = *block.getIArguments();

        REQUIRE_TRUE(!axis.empty(), 0, "STANDARDIZE OP: axis has to be non-empty")


        shape::checkDimensions(input->rankOf(), axis);
        auto longAxis = ArrayUtils::toLongVector(axis);

        auto means = input->reduceAlongDims(reduce::Mean, axis, true);
        auto stdev = *input->varianceAlongDimension(variance::SummaryStatsStandardDeviation, false, axis);
        stdev.reshapei(means.getShapeAsVector());

        eps->applyTrueBroadcast(nd4j::BroadcastOpsTuple::Divide(), &stdev, output, false);

        auto dldu_sum = -output->reduceAlongDims(reduce::Sum, axis, true);
        nd4j::ops::reduce_mean_bp meanBp;
        auto dldx_u = *meanBp.execute({input, &dldu_sum}, {}, longAxis)->at(0);
        *output += dldx_u;

        // (eps * (means - input) / (stdev * stdev))
        NDArray tmp(eps);
        means.applyTrueBroadcast(nd4j::BroadcastOpsTuple::Subtract(), input, &tmp, false);
        tmp.applyPairwiseTransform(nd4j::pairwise::Multiply, eps, &tmp, nullptr);
        stdev.applyPairwiseTransform(nd4j::pairwise::Multiply, &stdev, &stdev, nullptr);
        tmp.applyTrueBroadcast(nd4j::BroadcastOpsTuple::Divide(), &stdev, &tmp, false);

        auto dlds_sum = tmp.reduceAlongDims(reduce::Sum, axis, true);
        nd4j::ops::reduce_stdev_bp stdevBp;
        auto dldx_s = *stdevBp.execute({input, &dlds_sum}, {}, longAxis)->at(0);
        *output += dldx_s;

        output->applyScalar(nd4j::scalar::ReplaceNans, 0, output, nullptr);


        return Status::OK();
    }

    DECLARE_TYPES(standardize_bp) {
        getOpDescriptor()
                ->setAllowedInputTypes(nd4j::DataType::ANY)
                ->setAllowedOutputTypes({ALL_FLOATS});
    }

    DECLARE_SHAPE_FN(standardize_bp) {
        auto in = inputShape->at(0);
        Nd4jLong *out;
        COPY_SHAPE(in, out);

        return SHAPELIST(out);
    }

}
}

#endif