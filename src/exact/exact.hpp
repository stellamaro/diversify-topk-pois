/**
 * @file
 * Defining of Exact algorithm, a naive enumerating algorithm to solve
 * the Socially Diverse k-Nearest Neighbours query.
 * This algorithm doesn't use any preprocess.
 */

#ifndef POPULAR_EXACT
#define POPULAR_EXACT

#include "../algorithm/algorithm.hpp"

namespace popular
{
    class Exact : public Algorithm
    {
    public:
        Exact() {} /**< Empty constructor */
        ~Exact() {} /**< Empty destructor */

        Exact(Corpus const& corpus): Algorithm(corpus) {}

        void query(uint32_t k, Point const& q, float const& a, ResultSet &results, double &z_from_lp,
                uint32_t &prunes, uint32_t &reheaps) override;
    };

} // namespace popular

#endif