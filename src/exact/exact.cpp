/**
 * @file
 * Implementation of exact algorithm.
 */

#include "exact.hpp"
#include "../util/commons.hpp"
#include "../util/constants.hpp"

#include <algorithm> // for_each, prev_permutation

namespace popular
{
    namespace
    {
        // https://stackoverflow.com/a/9430993
        void create_combinations(uint32_t k, std::vector< Point > const& points, Corpus const& corpus, float const& a,
                Point const& q, ResultSet &results, double &z_from_lp)
        {
            uint32_t n = points.size();

            double max_score = -DBL_MAX;
            user_similarity user_sim{corpus};
            main_scoring scoring{user_sim, a, k};

            std::vector<bool> v(n);
            if (k > n) { k = n; }
            std::fill(v.begin(), v.begin() + k, true);

            do {
                PointsSet one_comb;

                for (uint32_t i = 0; i < n; ++i)
                {
                    if (v[i]) { one_comb.insert( points[i] ); }
                }

                double score = scoring(q, one_comb);
                if (score >= max_score)
                {
                    max_score = score;
                    results.first = one_comb;
                    results.second = max_score;
                    z_from_lp = 0;
                }

            } while (std::prev_permutation(v.begin(), v.end()));
        }
    } // namespace anonymous

    void Exact::query(uint32_t k, Point const& q, float const& a, ResultSet &results, double &z_from_lp,
            uint32_t &prunes, uint32_t &reheaps)
    {
        std::vector< Point > points;
        points.assign(corpus_.places.cbegin(), corpus_.places.cend());

        create_combinations(k, points, corpus_, a, q, results, z_from_lp);
        prunes = 0;
        reheaps = 0;
    }

} // namespace popular