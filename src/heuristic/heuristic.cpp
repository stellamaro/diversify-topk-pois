/**
 * @file
 * Implementation of heuristic algorithm.
 */

#include "heuristic.hpp"
#include "../util/commons.hpp"
#include "../util/constants.hpp"

namespace { //anonymous

    /**
     * Converts a PriorityQueue into a PointsSet
     */
    popular::PointsSet PQ_to_PS( popular::PriorityQueue && candidates )
    {
        popular::PointsSet res;

        while ( ! candidates.isEmpty() )
        {
            res.insert( candidates.return_best() );
        }

        return res;
    }

} // namespace anonymous

namespace popular
{
    template < Heuristic_Variant variant >
    void Heuristic< variant >::query(uint32_t k, Point const& q, float const& alpha, ResultSet &results,
            double &z_from_lp, uint32_t &prunes, uint32_t &reheaps)
    {
        z_from_lp = prunes = reheaps = 0;
        assert( "Dataset contains at least k possible answers" && corpus_.places.size() >= k );
        
        results.first = PQ_to_PS( scoring( PriorityQueue( k )
                                , q
                                , alpha ) );

        results.second = main_scoring{ user_similarity{ corpus_ }, alpha, k }( q, results.first );
    }

    /**
     * Implementation of the scoring for the Naive heuristic algorithm.
     * Scores each point as if k = 1
     */
    template < Heuristic_Variant variant >
    PriorityQueue Heuristic< variant >::scoring( PriorityQueue && candidates
                                               , Point const q
                                               , float const alpha ) const
    {
        for( Point const p : corpus_.places )
        {
            candidates.add_to_queue( p
                                   , score( p, q, alpha ) );
        }

        candidates.swap_queue();
        return candidates;
    }

    template <>
    double Heuristic< Heuristic_Variant::Naive_user >::score( Point const p, Point const, float const ) const
    {
        return user_sim( p );
    }

    template <>
    double Heuristic< Heuristic_Variant::Naive_dist >::score( Point const p, Point const q, float const ) const
    {
        return spatial_sim( p, q );
    }

    template <>
    double Heuristic< Heuristic_Variant::Naive >::score( Point const p, Point const q, float const alpha ) const
    {
        return alpha * spatial_sim( p, q ) + ( 1.0 - alpha ) * user_sim( p );
    }


    template class Heuristic< Heuristic_Variant::Naive >;
    template class Heuristic< Heuristic_Variant::Naive_dist >;
    template class Heuristic< Heuristic_Variant::Naive_user >;
} // namespace popular