/**
 * @file kde_rules.hpp
 * @author Roberto Hueso (robertohueso96@gmail.com)
 *
 * Rules Kernel Density estimation, so that it can be done with arbitrary tree
 * types.
 *
 * mlpack is free software; you may redistribute it and/or modify it under the
 * terms of the 3-clause BSD license.  You should have received a copy of the
 * 3-clause BSD license along with mlpack.  If not, see
 * http://www.opensource.org/licenses/BSD-3-Clause for more information.
 */

#ifndef MLPACK_METHODS_KDE_RULES_HPP
#define MLPACK_METHODS_KDE_RULES_HPP

#include <mlpack/core/tree/traversal_info.hpp>

namespace mlpack {
namespace kde {

template<typename MetricType, typename KernelType, typename TreeType>
class KDERules
{
 public:
  KDERules(const arma::mat& referenceSet,
           const arma::mat& querySet,
           arma::vec& densities,
           const double relError,
           const double absError,
           const std::vector<size_t>& oldFromNewQueries,
           MetricType& metric,
           const KernelType& kernel);

  double BaseCase(const size_t queryIndex, const size_t referenceIndex);

  // SingleTree
  double Score(const size_t queryIndex, TreeType& referenceNode);

  // SingleTree
  double Rescore(const size_t queryIndex,
                 TreeType& referenceNode,
                 const double oldScore) const;

  // DoubleTree
  double Score(TreeType& queryNode, TreeType& referenceNode);

  // DoubleTree
  double Rescore(TreeType& queryNode,
                 TreeType& referenceNode,
                 const double oldScore) const;

  typedef typename tree::TraversalInfo<TreeType> TraversalInfoType;

  const TraversalInfoType& TraversalInfo() const { return traversalInfo; }

  TraversalInfoType& TraversalInfo() { return traversalInfo; }

  //! Get the number of base cases.
  size_t BaseCases() const { return baseCases; }

  //! Get the number of scores.
  size_t Scores() const { return scores; }

 private:
  //! The reference set.
  const arma::mat& referenceSet;

  //! The query set.
  const arma::mat& querySet;

  //! Density values.
  arma::vec& densities;

  const double absError;

  const double relError;

  //! New query dataset order.
  const std::vector<size_t>& oldFromNewQueries;

  //! The instantiated metric.
  MetricType& metric;

  const KernelType& kernel;

  //! The last query index.
  size_t lastQueryIndex;

  //! The last reference index.
  size_t lastReferenceIndex;

  TraversalInfoType traversalInfo;

  //! The number of base cases.
  size_t baseCases;

  //! The number of scores.
  size_t scores;
};

} // namespace kde
} // namespace mlpack

// Include implementation.
#include "kde_rules_impl.hpp"

#endif
