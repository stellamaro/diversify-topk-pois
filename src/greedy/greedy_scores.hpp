/**
* @file
* Defining of the functions for the Greedy algorithm family.
*/

#ifndef GREEDY_SCORES
#define GREEDY_SCORES

#include <limits> // std::numeric_limits<>::max()

namespace popular
{
    /**
     * Score each point of the candidates_set together with the points of the chosen_set
     */
     std::pair< Point, double > score_with_function ( Point const q
                                                    , PointsSet const& candidates
                                                    , main_scoring const& scoring
                                                    , IntermediateRes const& intermediateRes )
    {
        Point best_point{ 0, 0 };
        auto max_score = std::numeric_limits< double >::max() * -1.0;

        for( auto const point : candidates )
        {
            double const score = scoring( q, point, intermediateRes );

            best_point = (score >= max_score) ? point : best_point;
            max_score  = (score >= max_score) ? score : max_score;
        }

        return std::make_pair( best_point, max_score );
    }
} // popular

#endif