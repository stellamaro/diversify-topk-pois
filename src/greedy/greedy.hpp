/**
 * @file
 * Defining of Greedy algorithm, a greedy straight forward algorithm to solve
 * the Socially Diverse k-Nearest Neighbours query.
 * This algorithm doesn't use any preprocess.
 */

#ifndef POPULAR_GREEDY
#define POPULAR_GREEDY

#include "../algorithm/algorithm.hpp"
#include "../util/topkPriorityQueue.hpp"

namespace popular
{
    enum class Greedy_Variant
    {
        Naive, /**< Naive greedy */
        Other
    };


    template < Greedy_Variant variant >
    class Greedy : public Algorithm
    {
    public:
        Greedy() {} /**< Empty constructor */
        ~Greedy() {} /**< Empty destructor */

        Greedy(Corpus const& corpus): Algorithm(corpus) {}

        void query(uint32_t k, Point const& q, float const& a, ResultSet &results, double &z_from_lp,
                uint32_t &prunes, uint32_t &reheaps) override;
    };
} // namespace popular

#endif