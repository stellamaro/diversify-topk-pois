/**
 * @file
 * A basic Priority Queue to keep the results
 */

#ifndef TOPK_PRIORITY_QUEUE
#define TOPK_PRIORITY_QUEUE

#include <queue>

#include "commons.hpp"
#include "../rtree/RTree.h"

namespace popular
{
    struct entryIsGreater
    {
        template < typename T >
        bool operator() ( T const l, T const r ) const
        {
            return l.second > r.second;
        }
    };

    struct entryIsLess
    {
        template < typename T >
        bool operator() ( T const l, T const r ) const
        {
            return l.second < r.second;
        }
    };

    class MBRPriorityQueue
    {
        using PQEntry = std::pair< MyTree::Branch*, double >;
        using Q = std::priority_queue< PQEntry, std::vector< PQEntry >, entryIsLess >;

        Q q; /**< the candidates priority queue */

    public:
        MBRPriorityQueue() {} /**< Empty constructor */
        ~MBRPriorityQueue() {} /**< Empty destructor */

        void add_to_queue(MyTree::Branch* const& branch, double score)
        {
            q.push({branch, score});
        }

        PQEntry return_best()
        {
            PQEntry entry = q.top();
            q.pop();
            return entry;
        }

        double peak_best_score() const
        {
            return q.top().second;
        }

        void print()
        {
            while (!q.empty())
            {
                std::cout << "id = " << q.top().first->id << "  score = " << q.top().second << std::endl;
                q.pop();
            }
        }

        bool isEmpty() const
        {
            return q.empty();
        }

        size_t size() const
        {
            return q.size();
        }

    };

} // namespace popular

#endif