/**
 * @file
 * Implementation of greedy algorithm.
 */

#include "greedy.hpp"
#include "../util/commons.hpp"
#include "../util/constants.hpp"
#include "greedy_scores.hpp"

namespace // anonymous
{
    using namespace popular;

    /**
     * Function that decides which point to insert in the result set
     * @param candidates : the scored points
     * @param k : the number of points in the result
     * @param res : the result
     */
    std::pair< Point, double > greedy_deciding  ( Point const q
                                                , PointsSet const& corpus_pois
                                                , main_scoring const& scoring
                                                , IntermediateRes const& intermediateRes )
    {
        return score_with_function(q, corpus_pois, scoring, intermediateRes);
    }
} // namespace anonymous

namespace popular
{
    template < Greedy_Variant variant >
    void Greedy< variant >::query(uint32_t k, Point const& q, float const& a, ResultSet &results, double &z_from_lp,
            uint32_t &prunes, uint32_t &reheaps)
    {
        PointsSet corpus_pois(corpus_.places.begin(), corpus_.places.end());
        main_scoring scoring{ user_similarity{ corpus_ }, a, k};

        IntermediateRes intermediateRes { std::vector< UserId >(), 0.0 };

        for( auto i = 0u; i < k; ++i )
        {
            if( corpus_pois.empty() ) { continue; }

            auto const [ chosen_point, score ] = greedy_deciding( q, corpus_pois, scoring, intermediateRes );
            results.first.insert(chosen_point);

            addIntermediate(intermediateRes, q, chosen_point, corpus_.max_distance, corpus_.checkins.at(chosen_point));
            corpus_pois.erase(chosen_point);
        }

        results.second = scoring(q, results.first);
        z_from_lp = 0;
        prunes = 0;
        reheaps = 0;
    }

    template class Greedy< Greedy_Variant::Naive >;
} // namespace popular