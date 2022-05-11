/**
 * @file
 * Implementation of methods (primarily stream overloading) for our common
 * data structures.
 */

#include "commons.hpp"

#include <algorithm> // std::for_each()

namespace popular
{
    std::ostream& operator << (std::ostream &o, const Point &p)
    {
        return o << "(" << p.first << "," << p.second << ")";
    }

    std::ostream& operator << (std::ostream &o, const Corpus &c)
    {
        o << " XMIN = " << c.xmin << " YMIN = " << c.ymin << std::endl;
        o << " XMAX = " << c.xmax << " YMAX = " << c.ymax << std::endl;
        o << " Max distance = " << c.max_distance << std::endl;
        for (auto const& check : c.checkins)
        {
            o << check.first << " -> ";
            for (auto const& us : check.second) { o << us << " "; }
            o << std::endl;
        }
        return o;
    }

    std::ostream& operator << (std::ostream &o, const ResultSet &res)
    {
        o << "RESULTS:" << std::endl;
        o << "Score = " << res.second << std::endl;
        o << "Points = ";
        for (auto const& p : res.first) { o << p << " "; }
        o << std::endl;
        return o;
    }

    std::ostream& operator << (std::ostream &o, Stats const& stats)
    {
        o << stats.input_file << "\t" << stats.algorithm << "\t" << stats.alg_index << "\t"
          << stats.query << "\t" << stats.query_index << "\t"
          << stats.k << "\t" << stats.a << "\t" << stats.microseconds_pre << "\t"
          << stats.microseconds_q << "\t" << stats.microseconds_retrieve << "\t"
          << stats.microseconds_all << "\t" << stats.peak_rss << "\t" << stats.num_points << "\t"
          << stats.num_users << "\t" << stats.num_checkins << "\t" << stats.z_from_lp << "\t" << stats.actual_score
          << "\t" << stats.prunes << "\t" << stats.reheaps;
        return o;
    }

    std::ostream& operator << (std::ostream &o, Headers const&)
    {
        o << "\033[95mDataset\tAlgorithm\tAlg index\tQuery\tQ index\tk\ta\tPreprocess time\tQuery time\tRetrieve time"
             "\tTotal time\tPeak RSS\tPoints\tUsers\tCheckins\tZ\tScore\tPrunes\tReheaps\033[00m";
        return o;
    }

    std::ostream& operator << (std::ostream &o, IntermediateRes const& intermediateRes)
    {
        o << "Coverage: " << std::endl;
        for(auto const& u : intermediateRes.coverage) { o << u << " "; }
        o << std::endl << "Distance = " << intermediateRes.distance << std::endl;
        return o;
    }

    double score( Point const q, Point const p, std::vector< UserId > const& users, double const max_dist,
                 uint32_t const k, uint32_t const tot_users, float const a_param, IntermediateRes const& intermediateRes)
    {
        double d, u;
        if(intermediateRes.coverage.size() == 0)
        {
            d = ( 1 - (distance(q, p)/max_dist) ) / k;
            u = (double) users.size() / tot_users;
        }
        else
        {
            d = ( 1 - distance( p, q ) / max_dist + intermediateRes.distance ) / k;

            u = set_union_size( intermediateRes.coverage, users )
              / static_cast< double >( tot_users );
        }

        return the_score(d, u, a_param);
    }

    double contribution( Point const q, Point const p, std::vector< UserId > const& users, double const max_dist,
            uint32_t const k, UserId const tot_users, float const a_param, IntermediateRes const& intermediateRes)
    {
        auto const d = ( 1 - distance( p, q ) / max_dist ) / k;
        auto const u = intermediateRes.coverage.size() == 0
                     ? users.size()
                     : set_difference_size( users, intermediateRes.coverage );

        return the_score( d, u / static_cast< double >( tot_users ), a_param );
    }

    void addIntermediate( IntermediateRes & intermediateRes
                        , Point const q
                        , Point const p
                        , double const max_dist
                        , std::vector< UserId > const& users )
    {
        intermediateRes.coverage  = my_set_union( users, intermediateRes.coverage );
        intermediateRes.distance += 1.0 - distance( p, q ) / max_dist;
    }

} // namespace popular