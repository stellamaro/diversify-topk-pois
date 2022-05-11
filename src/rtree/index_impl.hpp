/**
 * @file
 * Implementation of indexed algorithm.
 */

#include "index.hpp"
#include "../util/commons.hpp"
#include "../util/MBRPriorityQueue.hpp"

namespace popular
{
    template < Indexed_Variant variant >
    void Index< variant >::print() const
    {
        rtree.Print();

        std::cout << "  USERS : " << std::endl;
        for(uint32_t i = 0; i < users.size(); i++)
        {
            std::cout << "   mbr id = " << i << "  users.size = " << users[i].size() << std::endl;
        }
    }

    template < Indexed_Variant variant >
    void Index< variant >::buildIndex(const PointsSet& places, const Checks& checkins)
    {
        for(Point p:places)
        {
            float m[2];
            m[0] = p.first;
            m[1] = p.second;
            treeInsert(m, m, p);
        }

        updateUsers(checkins);
    }

    template < Indexed_Variant variant >
    void Index< variant >::treeInsert(const ElemType a_min[NumDims], const ElemType a_max[NumDims], const DataType& a_dataId)
    {
        rtree.Insert(a_min, a_max, a_dataId);
    }

    template < Indexed_Variant variant >
    void Index< variant >::updateUsers(const Checks& checkins)
    {
        MyTree::Node* root = rtree.GetRoot();
        uint32_t id = 0;
        updateUsersRec(root, &id, checkins);
    }

    template < Indexed_Variant variant >
    void Index< variant >::updateUsersRec(MyTree::Node* a_node, uint32_t* id, const Checks& checkins)
    {
        if(!(a_node->IsLeaf()))
        {
            for(int i = 0; i < a_node->m_count; ++i)
            {
                updateUsersRec(a_node->m_branch[i].m_child, id, checkins);
            }
            if (a_node)
            {
                for (int index = 0; index < a_node->m_count; ++index)
                {
                    a_node->m_branch[index].id = *id;
                    *id = *id + 1;
                    std::vector< UserId > u;
                    for(int i = 0; i < a_node->m_branch[index].m_child->m_count; i++)
                    {
                        auto const bid = a_node->m_branch[index].m_child->m_branch[i].id;
                        u = my_set_union( users.at( bid ), u);
                    }
                    std::sort( std::begin( u ), std::end( u ) );
                    users.push_back(u);
                }
            }
        }
        else
        {
            if (a_node)
            {
                for (int index = 0; index < a_node->m_count; ++index)
                {
                    a_node->m_branch[index].id = *id;
                    *id = *id + 1;
                    Point p = std::make_pair(a_node->m_branch[index].m_rect.m_min[0],
                            a_node->m_branch[index].m_rect.m_min[1]);
                    std::vector< UserId > u( std::cbegin( checkins.at( p ) ), std::cend( checkins.at( p ) ) );
                    std::sort( std::begin( u ), std::end( u ) );
                    users.push_back(u);
                }
            }
        }
    }

    template < Indexed_Variant variant >
    void Index< variant >::query(popular::ResultSet &results, Point const& q, float const& a, uint32_t const k,
            double const& max_dist, uint32_t const& tot_users, uint32_t &prunes, uint32_t &reheaps)
    {
        MBRPriorityQueue queue;
        reheaps = 0;
        IntermediateRes intermediateRes{ std::vector< UserId >(), 0.0 };
        std::vector< std::pair< uint32_t, Point > > temp_results;

        MyTree::Node* root = rtree.GetRoot();
        // open the root node
        for(int index = 0; index < root->m_count; ++index)
        {
            queue.add_to_queue(&root->m_branch[index], scoreMBR(&root->m_branch[index], q, a, k, max_dist, tot_users, intermediateRes));
        }

        while( (temp_results.size() < k ) && (!queue.isEmpty()) )
        {
            auto const [ branch, min_score ] = queue.return_best(); // dequeue the best scored element

            if(branch->m_child) // is internal node
            {
#ifndef NPRUNE
                // if cannot be pruned
                if(!prune(*branch, q, temp_results, a))
                {
#endif
                    // add all children to the queue
                    for(int index = 0; index < branch->m_child->m_count; ++index)
                    {
                        queue.add_to_queue(&branch->m_child->m_branch[index],
                                           contributionMBR(&branch->m_child->m_branch[index], q, a, k, max_dist,
                                                    tot_users, intermediateRes));
                    }
#ifndef NPRUNE
                }
                else
                {
                    prunes++;
                }
#endif
            }
            else // is point
            {
#ifndef NPRUNE
                // if cannot be pruned
                if(!prune(*branch, q, temp_results, a))
                {
#endif
                    // recompute the contribution of the point
                    double const contribution = ( variant == Indexed_Variant::Naive
                    									   ? min_score
                    									   : contributionMBR( branch, q, a, k, max_dist, tot_users, intermediateRes ) );

                    if( contribution == min_score || contribution > queue.peak_best_score() )
                    {
                        // add POI to result and intermediate
                        Point const p( branch->m_data );
                        temp_results.push_back(std::make_pair(branch->id, p ) );
                        results.first.insert( p );
                        addIntermediate(intermediateRes, q, p, max_dist, users.at(branch->id));
                    }
                    else if( variant == Indexed_Variant::ReHeap ) // reheap the point
                    {
                        queue.add_to_queue(branch, contribution);
                        reheaps++;
                    }
#ifndef NPRUNE
                }
                else
                {
                    prunes++;
                }
#endif
            }
        }
    }

    template < Indexed_Variant variant >
    double Index< variant >::scoreMBR( MyTree::Branch* a_branch, Point const q, float const a, uint32_t const k,
            double const max_dist, uint32_t const tot_users, IntermediateRes & intermediateRes )
    {
        Point const MBRpoi = minDistPoi(*a_branch, q);
        return score(q, MBRpoi, users.at(a_branch->id), max_dist, k, tot_users, a, intermediateRes);
    }

    template < Indexed_Variant variant >
    double Index< variant >::contributionMBR( MyTree::Branch* a_branch, Point const q, float const a, uint32_t const k,
                           double const max_dist, uint32_t const tot_users, IntermediateRes & intermediateRes )
    {
        Point MBRpoi = minDistPoi(*a_branch, q);
		return contribution(q, MBRpoi, users.at(a_branch->id), max_dist, k, tot_users, a, intermediateRes);
    }

    template < Indexed_Variant variant >
    Point Index< variant >::minDistPoi(MyTree::Branch const& a_branch, Point const& q) const
    {
        float px = q.first;
        float py = q.second;
        float rx, ry;

        if(px < a_branch.m_rect.m_min[0]) { rx = a_branch.m_rect.m_min[0]; }
        else if(px > a_branch.m_rect.m_max[0]) { rx = a_branch.m_rect.m_max[0]; }
        else { rx = px; }

        if(py < a_branch.m_rect.m_min[1]) { ry = a_branch.m_rect.m_min[1]; }
        else if(py > a_branch.m_rect.m_max[1]) { ry = a_branch.m_rect.m_max[1]; }
        else { ry = py; }

        return std::make_pair(rx, ry);
    }

    template < Indexed_Variant variant >
    bool Index< variant >::prune(MyTree::Branch const& a_branch, Point const& q,
            std::vector< std::pair< uint32_t, Point > > const& pois, float const& a) const
    {
        Point const p = a_branch.m_child // if internal node
        			  ? minDistPoi( a_branch, q )
        			  : a_branch.m_data;

        std::vector< UserId > const& u = users.at( a_branch.id );

        for( auto const [ point_id, point ] : pois) // overly selective; doesn't use union of already-chosen pois
        {
            std::vector< UserId > const& already_covered_users = users.at( point_id );

            if(a==0) // we care only about the users
            {
                if( std::includes( already_covered_users.begin(), already_covered_users.end(), u.cbegin(), u.cend()) )
                {
                    // return true;
                }
            }
            else
            {
                // Lemma 1 and 2 : if any chosen point can prune the point or branch
                if(  distance( point, q ) <= distance( p, q ) )
                {
                	if( std::includes( already_covered_users.cbegin(), already_covered_users.cend(), u.cbegin(), u.cend())  )
	                {
    	                // return true;
        	        }
        	    }
            }
        }
        return false;
    }

    template class Index< Indexed_Variant::Naive >;
    template class Index< Indexed_Variant::ReHeap >;
} // namespace popular