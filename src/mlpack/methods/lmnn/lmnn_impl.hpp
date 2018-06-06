/**
 * @file lmnn_impl.hpp
 * @author Manish Kumar
 *
 * Implementation of Large Margin Nearest Neighbor class.
 *
 * mlpack is free software; you may redistribute it and/or modify it under the
 * terms of the 3-clause BSD license.  You should have received a copy of the
 * 3-clause BSD license along with mlpack.  If not, see
 * http://www.opensource.org/licenses/BSD-3-Clause for more information.
 */
#ifndef MLPACK_METHODS_LMNN_LMNN_IMPL_HPP
#define MLPACK_METHODS_LMNN_LMNN_IMPL_HPP

// In case it was not already included.
#include "lmnn.hpp"

namespace mlpack {
namespace lmnn {

/**
 * Takes in a reference to the dataset. Copies the data, initializes
 * all of the member variables and constraint object and generate constraints.
 */
template<typename MetricType, typename OptimizerType>
LMNN<MetricType, OptimizerType>::LMNN(const arma::mat& dataset,
                       const arma::Row<size_t>& labels,
                       const size_t k,
                       const MetricType metric) :
    dataset(dataset),
    labels(labels),
    k(k),
    metric(metric),
    objFunction(dataset, labels, k, 0.5) // Default regularization value
                                         // set to 0.5.
{ /* nothing to do */ }

template<typename MetricType, typename OptimizerType>
void LMNN<MetricType, OptimizerType>::LearnDistance(arma::mat& outputMatrix)
{
  // See if we were passed an initialized matrix.
  if ((outputMatrix.n_rows != dataset.n_rows) ||
      (outputMatrix.n_cols != dataset.n_rows))
    outputMatrix.eye(dataset.n_rows, dataset.n_rows);

  Timer::Start("lmnn_sgd_optimization");

  optimizer.Optimize(objFunction, outputMatrix);

  Timer::Stop("lmnn_sgd_optimization");
}


} // namespace lmnn
} // namespace mlpack

#endif
