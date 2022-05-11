/**
 * @file
 * Definition of abstract base class for the various algorithms.
 */

#ifndef ALGORITHM
#define ALGORITHM

#include <cstdint>
#include <algorithm> // std::for_each()

#include "../util/commons.hpp"
#include "../util/constants.hpp"

namespace popular
{
    /**
     * A generic abstract class for the definition of the common behaviour of
     * Socially Diverse k-Nearest Neighbours query algorithms.
     */
     class Algorithm
     {
     public:
         Algorithm() {}
         virtual ~Algorithm() {}

         Algorithm(Corpus const& corpus): corpus_(corpus) {}

         /**
          * Preprocess (untimed)
          */
         virtual void preprocess(Point const& /*q*/, uint32_t /*k*/, float const& /*a*/) { }

         /**
          * The main (timed) work to answer a Socially Diverse k-Nearest Neighbours query.
          *
          * @param
          * @return The k points that maximise the SDNN score
          */
          virtual void query(uint32_t /*k*/, Point const& /*q*/, float const& /*a*/,
                  ResultSet &/*results*/, double &/*z_from_lp*/, uint32_t &/*prunes*/, uint32_t &/*reheaps*/) {}

          /**
           * Retrieve results method (untimed)
           */
          virtual void retrieve_results(uint32_t /*k*/, Point const& /*q*/, float const& /*a*/,
                  ResultSet &/*results*/, double &/*z_from_lp*/) {}

     protected:
         Corpus corpus_;
     private:
     };

} // namespace popular

#endif