/**
 * @file
 * Implementation of indexed algorithm.
 */

#include "rtree.hpp"
#include "../util/commons.hpp"
#include "../util/constants.hpp"
#include "index_impl.hpp"

#include <algorithm> // for_each, prev_permutation

namespace popular
{
    template < Indexed_Variant variant >
    void Indexed< variant >::preprocess(const popular::Point &/*q*/, uint32_t /*k*/, float const &/*a*/)
    {
        index.buildIndex(corpus_.places, corpus_.checkins);
    }

    template < Indexed_Variant variant >
    void Indexed< variant >::query(uint32_t k, Point const& q, float const& a, ResultSet &results,
            double &z_from_lp, uint32_t &prunes, uint32_t &reheaps)
    {
        z_from_lp = prunes = reheaps = 0;

        index.query(results, q, a, k, corpus_.max_distance, corpus_.users.size(), prunes, reheaps);

        results.second = main_scoring{ user_similarity{ corpus_ }, a, k }( q, results.first );
    }

    template class Indexed< Indexed_Variant::Naive >;
    template class Indexed< Indexed_Variant::ReHeap >;
} // namespace popular