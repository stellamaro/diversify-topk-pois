/**
 * @file
 * Defining of Heuristic algorithm, a heuristic straight forward algorithm to solve
 * the Socially Diverse k-Nearest Neighbours query.
 * This algorithm doesn't use any preprocess.
 */

#ifndef POPULAR_HEURISTIC
#define POPULAR_HEURISTIC

#include "../algorithm/algorithm.hpp"
#include "../util/topkPriorityQueue.hpp"

namespace popular
{
    enum class Heuristic_Variant
    {
        Naive_dist, /**< Naive based on distance */
        Naive_user, /**< Naive based on user */
        Naive, /**< Naive heuristic */
        Other
    };


    template < Heuristic_Variant variant >
    class Heuristic : public Algorithm
    {
    public:
        Heuristic() {} /**< Empty constructor */
        ~Heuristic() {} /**< Empty destructor */

        Heuristic(Corpus const& corpus): Algorithm(corpus) {}

        void query(uint32_t k, Point const& q, float const& a, ResultSet &results, double &z_from_lp,
                uint32_t &prunes, uint32_t &reheaps) override;

    private:
        PriorityQueue scoring( PriorityQueue && pq, Point const q, float const alpha ) const;

        double score( Point const p, Point const q, float const alpha ) const;

        double user_sim( Point const p ) const
        {
            return corpus_.checkins.at( p ).size() / static_cast< double >( corpus_.users.size() );
        }

        double spatial_sim( Point const p, Point const q ) const
        {
            return 1.0 - distance( p, q ) / corpus_.max_distance;
        }
    };

} // namespace popular

#endif