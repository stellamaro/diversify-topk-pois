/**
 * @file
 * Defining of Indexed algorithm, based on R-tree and Nearest Neighbours that solves
 * the Socially Diverse k-Nearest Neighbours query.
 */

#ifndef POPULAR_RTREE
#define POPULAR_RTREE

#include "../algorithm/algorithm.hpp"
#include "../util/constants.hpp"
#include "index.hpp"

namespace popular
{
    template < Indexed_Variant variant >
    class Indexed : public Algorithm
    {
    public:
        Indexed() {} /**< Empty constructor */
        ~Indexed() {} /**< Empty destructor */

        Indexed(Corpus const &corpus): Algorithm(corpus) {}

        void preprocess(Point const& q, uint32_t k, float const& a) override;
        void query(uint32_t k, Point const& q, float const& a, ResultSet &results, double &z_from_lp,
                uint32_t &prunes, uint32_t &reheaps) override;

        Index< variant > index;

    };

} // namespace popular

#endif