/**
 * @file load_csv.hpp
 * @author ThamNgapWei
 *
 * This is a csv parsers which use to parse the csv file format
 *
 * mlpack is free software; you may redistribute it and/or modify it under the
 * terms of the 3-clause BSD license.  You should have received a copy of the
 * 3-clause BSD license along with mlpack.  If not, see
 * http://www.opensource.org/licenses/BSD-3-Clause for more information.
 */
#ifndef MLPACK_CORE_DATA_LOAD_CSV_HPP
#define MLPACK_CORE_DATA_LOAD_CSV_HPP

#include <boost/spirit/include/qi.hpp>

#include <mlpack/core.hpp>
#include <mlpack/core/util/log.hpp>

#include <set>
#include <string>

#include "extension.hpp"
#include "format.hpp"
#include "dataset_mapper.hpp"

namespace mlpack {
namespace data {

/**
 *Load the csv file.This class use boost::spirit
 *to implement the parser, please refer to following link
 *http://theboostcpplibraries.com/boost.spirit for quick review.
 */
class LoadCSV
{
public:
  explicit LoadCSV(std::string file, bool fatal = false);

  template<typename T, typename PolicyType>
  void Load(arma::Mat<T> &inout, DatasetMapper<PolicyType> &infoSet, bool transpose = true)
  {
    if(!CanOpen())
    {
      return;
    }

    if(transpose)
    {
      TranposeParse(inout, infoSet);
    }
    else
    {
      NonTranposeParse(inout, infoSet);
    }
  }

  size_t ColSize();
  size_t RowSize();

private:
  using iter_type = boost::iterator_range<std::string::iterator>;

  struct ElemParser
  {
    //return int_parser if the type of T is_integral
    template<typename T>
    static typename std::enable_if<std::is_integral<T>::value,
    boost::spirit::qi::int_parser<T>>::type
    Parser()
    {
      return boost::spirit::qi::int_parser<T>();
    }

    //return real_parser if T is floating_point
    template<typename T>
    static typename std::enable_if<std::is_floating_point<T>::value,
    boost::spirit::qi::real_parser<T>>::type
    Parser()
    {
      return boost::spirit::qi::real_parser<T>();
    }
  };

  bool CanOpen();

  template<typename T, typename PolicyType>
  void NonTranposeParse(arma::Mat<T> &inout, DatasetMapper<PolicyType> &infoSet)
  {
    using namespace boost::spirit;

    size_t row = 0;
    size_t col = 0;
    infoSet = DatasetMapper<PolicyType>(RowSize());
    std::string line;
    inout.set_size(infoSet.Dimensionality(), ColSize());
    inFile.clear();
    inFile.seekg(0, std::ios::beg);

    auto setNum = [&](T val)
    {
      inout(row, col++) = val;
    };
    auto setCharClass = [&](iter_type const &iter)
    {
      std::string str(iter.begin(), iter.end());
      if(str == "\t")
      {
        str.clear();
      }
      inout(row, col++) =
          static_cast<T>(infoSet.MapString(std::move(str),
                                           row));
    };

    auto numRule = CreateNumRule<T>();
    auto charRule = CreateCharRule();
    while(std::getline(inFile, line))
    {
      auto begin = line.begin();
      //parse the numbers from a line(ex : 1,2,3,4), if the parser find the number
      //it will execute the setNum function
      qi::phrase_parse(begin, line.end(), numRule[setNum] % ",", ascii::space);
      if(col != inout.n_cols)
      {
        begin = line.begin();
        col = 0;
        const bool canParse = qi::phrase_parse(begin, line.end(),
                                               charRule[setCharClass] % ",",
                                               ascii::space);
        if(!canParse)
        {
          throw std::runtime_error("LoadCSV cannot parse categories");
          break;
        }
      }
      ++row; col = 0;
    }
  }

  template<typename T, typename PolicyType>
  void TranposeParse(arma::Mat<T> &inout, DatasetMapper<PolicyType> &infoSet)
  {
    infoSet = DatasetMapper<PolicyType>(ColSize());
    inout.set_size(infoSet.Dimensionality(), RowSize());
    size_t parseTime = 0;
    std::set<size_t> mapCols;
    while(!TranposeParseImpl(inout, infoSet, mapCols))
    {      
      ++parseTime; //avoid infinite loop
      infoSet = DatasetMapper<PolicyType>(inout.n_rows);
      if(parseTime == inout.n_rows)
      {
        return;
      }
    }
  }

  template<typename T, typename PolicyType>
  bool TranposeParseImpl(arma::Mat<T> &inout, DatasetMapper<PolicyType> &infoSet,
                         std::set<size_t> &mapCols)
  {
    using namespace boost::spirit;

    size_t row = 0;
    size_t col = 0;
    size_t progress = 0;
    std::string line;
    inFile.clear();
    inFile.seekg(0, std::ios::beg);
    auto setNum = [&](T val)
    {
      if(mapCols.find(progress) != std::end(mapCols))
      {
        inout(row, col) =
            static_cast<T>(infoSet.MapString(std::to_string(val),
                                             progress));
      }
      else
      {
        inout(row, col) = val;
      }
      ++progress; ++row;
    };
    auto setCharClass = [&](iter_type const &iter)
    {
      if(mapCols.find(progress) != std::end(mapCols))
      {
        std::string str(iter.begin(), iter.end());
        if(str == "\t")
        {
          str.clear();
        }
        inout(row, col) =
            static_cast<T>(infoSet.MapString(std::move(str),
                                             progress));
      }
      else
      {
        mapCols.insert(progress);
      }
      ++progress; ++row;
    };

    auto numRule = CreateNumRule<T>();
    auto charRule = CreateCharRule();
    while(std::getline(inFile, line))
    {
      auto begin = line.begin();
      row = 0;
      progress = 0;
      const size_t oldSize = mapCols.size();
      //parse number of characters from a line, it will execute setNum if it is number,
      //else execute setCharClass, "|" means "if not a, then b"
      const bool canParse = qi::phrase_parse(begin, line.end(),
                                             (numRule[setNum] | charRule[setCharClass]) % ",",
                                             ascii::space);
      if(!canParse)
      {
        throw std::runtime_error("LoadCSV cannot parse categories");
      }
      if(mapCols.size() > oldSize)
      {
        return false;
      }
      ++col;
    }

    return true;
  }

  template<typename T>
  boost::spirit::qi::rule<std::string::iterator, T(), boost::spirit::ascii::space_type>
  CreateNumRule() const
  {
    using namespace boost::spirit;

    //elemParser will generate integer or real parser based on T
    auto elemParser = ElemParser::Parser<T>();
    //qi::skip can specify which characters you want to skip,
    //in this example, elemParser will parse int or double value,
    //we use qi::skip to skip space

    //qi::omit can omit the attributes of spirit, every parser of spirit
    //has attribute(the type will pass into actions(functor))
    //if you do not omit it, the attribute combine with attribute may
    //change the attribute

    //input like 2-200 or 2DM will make the parser fail,
    //so we use "look ahead parser--&" to make sure next
    //character is "," or end of line(eof) or end of file(eoi)
    //looks ahead parser will not consume any input or generate
    //any attribute
    if(extension == "csv" || extension == "txt")
    {
      return elemParser >> &(qi::lit(",") | qi::eol | qi::eoi);
    }
    else
    {
      return elemParser >> &(qi::lit("\t") | qi::eol | qi::eoi);
    }
  }

  boost::spirit::qi::rule<std::string::iterator, iter_type(), boost::spirit::ascii::space_type>
  CreateCharRule() const;

  std::string extension;
  bool fatalIfOpenFail;
  std::string fileName;
  std::ifstream inFile;
};

} // namespace data
} // namespace mlpack

#endif
