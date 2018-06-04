/**
 * @file item_mean_normalization.hpp
 * @author Wenhao Huang
 *
 * This class performs item mean normalization on raw ratings. In another
 * word, this class is used to remove global effect of item mean.
 *
 * mlpack is free software; you may redistribute it and/or modify it under the
 * terms of the 3-clause BSD license.  You should have received a copy of the
 * 3-clause BSD license along with mlpack.  If not, see
 * http://www.opensource.org/licenses/BSD-3-Clause for more information.
 */
#ifndef MLPACK_METHODS_CF_NORMALIZATION_ITEM_MEAN_NORMALIZATION_HPP
#define MLPACK_METHODS_CF_NORMALIZATION_ITEM_MEAN_NORMALIZATION_HPP

#include <mlpack/prereqs.hpp>

namespace mlpack {
namespace cf {

/**
 * This normalization class performs item mean normalization on raw ratings.
 */
class ItemMeanNormalization
{
 public:
  // Empty constructor.
  ItemMeanNormalization() { }

  /**
   * Normalize the data by subtracting item mean from each of existing ratings.
   *
   * @param data Input dataset in the form of coordinate list.
   */
  void Normalize(arma::mat& data)
  {
    const size_t itemNum = arma::max(data.row(1)) + 1;
    itemMean = arma::vec(itemNum, arma::fill::zeros);
    // Number of ratings for each item.
    arma::Row<size_t> ratingNum(itemNum, arma::fill::zeros);

    // Sum ratings for each item.
    data.each_col([&](arma::vec& datapoint) {
      const size_t item = (size_t) datapoint(1);
      const double rating = datapoint(2);
      itemMean(item) += rating;
      ratingNum(item) += 1;
    });

    // Calculate item mean and subtract item mean from ratings.
    // Set item mean to 0 if the item has no rating.
    // Should we use mean of all item means if an item has no rating?
    for (size_t i = 0; i < itemNum; i++)
      if (ratingNum(i) != 0)
        itemMean(i) /= ratingNum(i);
    data.each_col([&](arma::vec& datapoint) {
      const size_t item = (size_t) datapoint(1);
      datapoint(2) -= itemMean(item);
    });
  }

  /**
   * Normalize the data by subtracting item mean from each of existing ratings.
   *
   * @param cleanedData Sparse matrix data.
   */
  void Normalize(arma::sp_mat& cleanedData)
  {
    itemMean = arma::vec(arma::mean(cleanedData, 1));

    arma::sp_mat::iterator it = cleanedData.begin();
    arma::sp_mat::iterator it_end = cleanedData.end();
    for (; it != it_end; it++)
      *it = *it - itemMean(it.row());
  }

  /**
   * Denormalize computed rating by adding item mean.
   *
   * @param user User ID.
   * @param item Item ID.
   * @param rating Computed rating before denormalization.
   */
  double Denormalize(const size_t /* user */,
                     const size_t item,
                     const double rating) const
  {
    return rating + itemMean(item);
  }

  /**
   * Denormalize computed rating by adding item mean.
   *
   * @param combinations User/Item combinations.
   * @param predictions Predicted ratings for each user/item combination.
   */
  void Denormalize(const arma::Mat<size_t>& combinations,
                   arma::vec& predictions) const
  {
    for (size_t i = 0; i < predictions.n_elem; i++)
    {
      const size_t item = combinations(1, i);
      predictions(i) += itemMean(item);
    }
  }

  /**
   * Return item mean.
   */
  arma::vec ItemMean() const
  {
    return itemMean;
  }

  /**
   * Serialization.
   */
  template<typename Archive>
  void serialize(Archive& ar, const unsigned int /* version */)
  {
    ar & BOOST_SERIALIZATION_NVP(itemMean);
  }

 private:
  //! item mean.
  arma::vec itemMean;
};

} // namespace cf
} // namespace mlpack

#endif
