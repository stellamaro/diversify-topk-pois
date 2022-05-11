/**
 * @file
 * Definitions of common datastructures.
 */

#ifndef CHECKIN
#define CHECKIN

#include <string>
#include <vector>
#include <iostream> // for overloading stream operators
#include <unordered_set>
#include <unordered_map>
#include <boost/functional/hash.hpp> // for hash of std::pair
#include <cfloat> // for min and max values
#include <numeric> // std::accumulate
#include <limits> // std::numeric_limits

#include "constants.hpp"
#include "set-operations.hpp"

namespace popular
{
    using coordinate = float;
    using Point = std::pair< coordinate, coordinate >; /**< a point is a pair of <latitude, longitude> */
    using PointsSet = std::unordered_set< Point, boost::hash< Point > >; /**< unordered_set with Point */
    using UserId = uint32_t;
    using ResultSet = std::pair< PointsSet, double >; /**< the result is a pair of a set of points and a score. */

    typedef struct Corpus
    {
        Corpus() : xmin( std::numeric_limits< coordinate >::max() )
                 , xmax( std::numeric_limits< coordinate >::max() * -1 )
                 , ymin( std::numeric_limits< coordinate >::max() )
                 , ymax( std::numeric_limits< coordinate >::max() * -1 )
                 , max_distance( std::numeric_limits< coordinate >::max() * -1 )
                 , num_checkins(0) {};

        PointsSet places;
        std::unordered_set< UserId > users;
        std::unordered_map< Point, std::vector< UserId >, boost::hash< Point > > checkins;
        float xmin, xmax, ymin, ymax;
        double max_distance;
        uint32_t num_checkins;
    } Corpus;

    /**
     * Struct to keep the parameters and the results of each run
     */
    struct Stats
    {
        std::string input_file;
        std::string algorithm;
        uint32_t alg_index;
        popular::Point query;
        uint32_t query_index;
        uint32_t k;
        float a;
        long double microseconds_pre;
        long double microseconds_q;
        long double microseconds_retrieve;
        long double microseconds_all;
        size_t peak_rss;
        uint32_t num_points;
        uint32_t num_users;
        uint32_t num_checkins;
        long double z_from_lp;
        long double actual_score;
        uint32_t prunes;
        uint32_t reheaps;
    };

    /**
     * Struct to keep the intermediate results of the unique users
     * and the distances so far in order to not be recalculated
     */
    struct IntermediateRes
    {
        std::vector< UserId > coverage;
        double distance;
    };

    struct Headers {};
    Headers const tag_headers;

    std::ostream& operator << (std::ostream &o, const Point &p); /**< Defining << for Point */
    std::ostream& operator << (std::ostream &o, const Corpus &c); /**< Defining << for Corpus */
    std::ostream& operator << (std::ostream &o, const ResultSet &res); /**< Defining << for ResultSet */
    std::ostream& operator << (std::ostream &o, Stats const& stats); /**< Defining << for Stats */
    std::ostream& operator << (std::ostream &o, Headers const&); /**< Defining << for Headers */
    std::ostream& operator << (std::ostream &o, IntermediateRes const& intermediateRes); /**< Defining << for IntermediateRes */

    /**
     * Calculates the Euclidean distance between two points
     * @param q : the first point
     * @param p : the second point
     * @return : float the Euclidean distance between the points
     */
    double inline distance( Point const q, Point const p )
    {
        return sqrt( pow( p.first - q.first, 2 ) + pow( p.second - q.second, 2 ) );
    }

    /**
     * Calculated the distance between a point a set of points
     * @param q : the point
     * @param points : the set of points
     * @return : float the distance between the point and the sets
     */
    double inline distance( Point const q, PointsSet const& points, double const max_dist, uint32_t const k )
    {
        double dist = std::accumulate (points.cbegin(), points.cend(), 0.0,
                [ q, max_dist ]( auto i, auto p ){ return i + ( 1 - ( distance(q, p) / max_dist ) ); });
        return dist/k;
    }

    /**
    * The actual scoring function
    * @param distance : the summed normalised distance divided by k
    * @param user_sim : the user coverage divided by total number of users
    * @param a_param : the parameter alpha
    * @return : the scoring function
    */
    inline double the_score( double const distance, double const user_sim, float const a_param )
    {
        return a_param*(distance) + (1-a_param)*user_sim;
    }

    /**
     * Scoring function that scores on point taking into consideration the other chosen points
     * @param q : the query point
     * @param p : the point to be scored
     * @param users : the set of users of the point p
     * @param max_dist : the maximum distance of the dataset for normalising the distance
     * @param k : the parameter k
     * @param tot_users : the total number of users in the dataset
     * @param a_param : the parameter a
     * @param intermediateRes : the intermediate results
     * @return : the score of the point
     */
    double score( Point const q, Point const p, std::vector< UserId > const& users, double const max_dist,
            uint32_t const k, uint32_t const tot_users, float const a_param, IntermediateRes const& intermediateRes);

    /**
     * Contribution function that calculates a point's contribution to the score taking into consideration
     * the other chosen points
     * @param q : the query point
     * @param p : the point to be scored
     * @param users : the set of users of the point p
     * @param max_dist : the maximum distance of the dataset for normalising the distance
     * @param k : the parameter k
     * @param tot_users : the total number of users in the dataset
     * @param a_param : the parameter a
     * @param intermediateRes : the intermediate results
     * @return : the contribution of the point
     */
    double contribution( Point const q, Point const p, std::vector< UserId > const& users, double const max_dist,
            uint32_t const k, UserId const tot_users, float const a_param, IntermediateRes const& intermediateRes);

    /**
     * Adds a point to the intermediate results
     * @param intermediateRes : the intermediate results struct
     * @param q : the query point
     * @param p : the point to be added
     * @param max_dist : the maximum distance in the data set
     * @param users : the users of the point
     */
    void addIntermediate( IntermediateRes &intermediateRes
                        , Point const q
                        , Point const p
                        , double const max_dist
                        , std::vector< UserId > const& users );

    /**
    * Calculates the number of users checked in to a set of points
    * @param points : the set of points
    * @return : uint32_t the number of users
    */
    struct user_similarity
    {
        Corpus const& corpus_;

        double operator () (PointsSet const& points) const
        {
            std::vector< UserId > unique_users;

            std::for_each(points.cbegin(), points.cend(),
                    [this, &unique_users](Point const& p)
                    {
                        unique_users = my_set_union ( this->corpus_.checkins.at(p)
                                                    , unique_users );
                    });

            return ((double)unique_users.size() / corpus_.users.size());
        }
    };

    struct main_scoring
    {
        user_similarity const& users;
        float const a;
        uint32_t k;

        double operator () ( Point const q, Point const p, IntermediateRes const& intermediateRes ) const
        {
            return score(q, p, users.corpus_.checkins.at(p), users.corpus_.max_distance, k, users.corpus_.users.size(),
                         a, intermediateRes);
        }

        double operator () ( Point const q, PointsSet const& points ) const
        {
            double dist = distance(q, points, users.corpus_.max_distance, k);
            double us_n = users(points);
            return the_score(dist, us_n, a);
        }
    };

} // namespace popular

#endif

