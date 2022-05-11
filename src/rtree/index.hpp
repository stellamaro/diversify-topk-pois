/**
 * @file
 * Defining of Rtree index algorithm, using the RTree and a hash-map for
 * the users on each MBR.
 */

#ifndef POPULAR_INDEX
#define POPULAR_INDEX

#include "../util/constants.hpp"
#include "RTree.h"

namespace popular
{
    using DataType = Point;
    using ElemType = float;
    int const NumDims = 2;
    using MyTree = RTree<DataType, ElemType, NumDims, float, Constants::RTREEMAXNODES>;

    using Checks = std::unordered_map< Point, std::vector< UserId >, boost::hash< Point > >;

    enum class Indexed_Variant
    {
        Naive, /**< Naive index-based */
        ReHeap, /**< Re-heap modification */
        Other
    };

    template < Indexed_Variant variant >
    class Index
    {
    public:
        Index() {}
        ~Index() {}

        void buildIndex(const PointsSet& places, const Checks& checkins);

        void print() const;

        void query(popular::ResultSet &results, Point const& q, float const& a, uint32_t const k,
                   double const& max_dist, uint32_t const& tot_users, uint32_t &prunes, uint32_t &reheaps);

    protected:

        void treeInsert(const ElemType a_min[NumDims], const ElemType a_max[NumDims], const DataType& a_dataId);
        void updateUsers(const Checks& checkins);
        void updateUsersRec(MyTree::Node* a_node, uint32_t* id, const Checks& checkins);

        /**
         * Calculates the score of an MBR
         * @param a_branch : the branch
         * @param q : the query point
         * @param a : the alpha parameter
         * @param k : the k parameter
         * @param max_dist : the maximum distance in the dataset
         * @param tot_users : the total number of users in the dataset
         * @param intermediateRes : the intermediate result set of POIs found so far
         * @return : the score of the MBR based on the inputs
         */
        double scoreMBR( MyTree::Branch* a_branch, Point const q, float const a, uint32_t const k,
                        double const max_dist, uint32_t const tot_users, IntermediateRes & intermediateRes );

        /**
         * Calculates the contribution of an MBR/Point based on f(q,p,P)
         * @param a_branch : the branch
         * @param q : the query point
         * @param a : the alpha parameter
         * @param k : the k parameter
         * @param max_dist : the maximum distance in the dataset
         * @param tot_users : the total number of users in the dataset
         * @param intermediateRes : the intermediate result set of POIs found so far
         * @return : the score of the MBR based on the inputs
         */
        double contributionMBR( MyTree::Branch* a_branch, Point const q, float const a, uint32_t const k,
                double const max_dist, uint32_t const tot_users, IntermediateRes & intermediateRes );

        /**
         * Finds the minimum distance point from an MBR to the query point
         * @param a_branch : the MBR
         * @param q : the query point
         * @return : the minDist point
         */
        Point minDistPoi(MyTree::Branch const& a_branch, Point const& q) const;

        /**
         * Checks if an MBR or POI can be pruned
         * @param a_branch : the MBR or POI
         * @param q : the query point
         * @param coverage : the users coverage of the intermediate results
         * @param pois : the POIs of the intermediate results
         * @return : true if MBR/POI can be pruned, false if not
         */
        bool prune(MyTree::Branch const& a_branch, Point const& q,
                std::vector< std::pair< uint32_t, Point > > const& pois, float const& a) const;

        MyTree rtree;
        std::vector< std::vector< UserId > > users;
    };

} // namespace popular

#endif