/**
 * @file
 * Implementation of exact algorithm.
 */

#include "lp.hpp"
#include "../util/commons.hpp"
#include "../util/constants.hpp"
extern "C" {
#include "lp_methods.h"
}
#include "../util/topkPriorityQueue.hpp"

#include <algorithm> // for_each, prev_permutation
#include <glpk.h>

namespace popular
{
    namespace
    {
        void populate_checkins(double *checks,
                std::unordered_map< Point, std::vector< UserId >, boost::hash< Point > > const& checkins,
                               std::vector< UserId > users, std::vector< Point > points)
        {
            for(uint32_t j = 0; j < points.size(); j++)
            {
                std::vector< UserId > poi_users = checkins.at(points[j]);
                for(uint32_t i = 0; i < users.size(); i++)
                {
                    if(std::find(poi_users.begin(), poi_users.end(), users[i]) != poi_users.end())
                    {
                        *((checks+i*points.size()) + j) = 1.0;
                    }
                    else
                    {
                        *((checks+i*points.size()) + j) = 0.0;
                    }
                }
            }
        }
    } // namespace anonymous

    template < LP_Variant variant >
    void Lp< variant >::preprocess(Point const& q, uint32_t k, float const& a)
    {
        points.assign(corpus_.places.cbegin(), corpus_.places.cend());
        dists_ = (double *)malloc(sizeof(double)*points.size());
        for(uint32_t i = 0; i< points.size(); i++)
        {
            dists_[i] = ( 1 - distance(q, points[i])/corpus_.max_distance ) / k;
        }

        users.assign(corpus_.users.cbegin(), corpus_.users.cend());

        checkins_ = (double *) malloc(sizeof(double)*users.size()*points.size());
        populate_checkins(checkins_, corpus_.checkins, users, points);

        unsigned long long size = (users.size() + points.size()) * (users.size() + 1);

        ia = (int*)malloc(sizeof(int)*(size+1));
        ja = (int*)malloc(sizeof(int)*(size+1));
        ar = (double*)malloc(sizeof(double)*(size+1));
        xjs = (double *)malloc(sizeof(double)*points.size());

        lp_ = glp_create_prob();
        size_ = 1;

        i_lp_setup(k, a, ia, ja, ar);
    }

    template < LP_Variant variant >
    void Lp< variant >::query(uint32_t /*k*/, Point const& /*q*/, float const& /*a*/,
            ResultSet &/*results*/, double &/*z_from_lp*/, uint32_t &prunes, uint32_t &reheaps)
    {
        i_lp(ia, ja, ar);
        prunes = 0;
        reheaps = 0;
    }

    template < LP_Variant variant >
    void Lp< variant >::retrieve_results(uint32_t k, Point const& q, float const& a,
            ResultSet &results, double &z_from_lp)
    {
        double z;
        double yis[users.size()];
        i_lp_retrieve(&z, yis);
        z_from_lp = z;

        PriorityQueue r = PriorityQueue(k);
        for(uint32_t i = 0; i< points.size(); i++)
        {
            r.add_to_queue(points[i], xjs[i]);
        }
        r.swap_queue();
        results.first.insert(r.return_best());
        while (results.first.size() < k && !r.isEmpty())
        {
            results.first.insert(r.return_best());
        }

        user_similarity user_sim{corpus_};
        main_scoring scoring{user_sim, a, k};
        results.second = scoring(q, results.first);
    }

    template <>
    void Lp< LP_Variant::LP >::i_lp_setup(uint32_t const& k, double const& a, int *ia, int *ja, double *ar)
    {
        lp_setup(lp_, static_cast<__uint32_t>(points.size()), static_cast<__uint32_t>(users.size()),
                k, a, dists_, checkins_, ia, ja, ar, &size_);
    }

    template <>
    void Lp< LP_Variant::Ilp >::i_lp_setup(uint32_t const& k, double const& a, int *ia, int *ja, double *ar)
    {
        ilp_setup(lp_, static_cast<__uint32_t>(points.size()), static_cast<__uint32_t>(users.size()),
                 k, a, dists_, checkins_, ia, ja, ar, &size_);
    }

    template <>
    void Lp< LP_Variant::LP >::i_lp(int *ia, int *ja, double *ar) const
    {
        lp_method(lp_, size_, ia, ja, ar);
    }

    template <>
    void Lp< LP_Variant::Ilp >::i_lp(int *ia, int *ja, double *ar) const
    {
        ilp_method(lp_, size_, ia, ja, ar);
    }

    template <>
    void Lp< LP_Variant::LP >::i_lp_retrieve(double *z, double *yis) const
    {
        lp_retrieve(lp_, static_cast<__uint32_t>(points.size()), static_cast<__uint32_t>(users.size()),
                z, xjs, yis);
    }

    template <>
    void Lp< LP_Variant::Ilp >::i_lp_retrieve(double *z, double *yis) const
    {
        ilp_retrieve(lp_, static_cast<__uint32_t>(points.size()), static_cast<__uint32_t>(users.size()),
                    z, xjs, yis);
    }


    template class Lp< LP_Variant::LP >;
    template class Lp< LP_Variant::Ilp >;

} // namespace popular