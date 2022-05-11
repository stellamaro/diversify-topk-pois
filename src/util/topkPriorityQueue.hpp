/**
 * @file
 * A basic Priority Queue to keep the results
 */

#ifndef TOPK_PRIORITY_QUEUE
#define TOPK_PRIORITY_QUEUE

#include <queue>

#include "commons.hpp"

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

    class PriorityQueue
    {
        using PQEntry = std::pair< Point, double >;
        using Q = std::priority_queue< PQEntry, std::vector< PQEntry >, entryIsGreater >;
        using Ql = std::priority_queue< PQEntry, std::vector< PQEntry >, entryIsLess >;

        Q q; /**< the candidates priority queue */
        Ql ql; /**< the swaped candidates priority queue */

    public:
        PriorityQueue() {} /**< Empty constructor */
        ~PriorityQueue() {} /**< Empty destructor */
        PriorityQueue(size_t const size)
        {
            std::vector< PQEntry > init_vals (size, {std::make_pair(FLT_MAX, FLT_MAX), -DBL_MAX});
            q = Q(init_vals.begin(), init_vals.end());

            init_vals = std::vector< PQEntry >(size, {std::make_pair(FLT_MAX, FLT_MAX), DBL_MAX});
            ql = Ql(init_vals.begin(), init_vals.end());
        }

        void add_to_queue(Point const& point, double score)
        {
            if (score > q.top().second)
            {
                q.pop();
                q.push({point, score});
            }
        }

        Point return_best()
        {
            PQEntry entry = ql.top();
            ql.pop();
            return entry.first;
        }

        void swap_queue()
        {
            while (!q.empty())
            {
                add_to_swaped(q.top());
                q.pop();
            }
        }

        void print()
        {
            while (!ql.empty())
            {
                std::cout << "point = " << ql.top().first << "  score = " << ql.top().second << std::endl;
                ql.pop();
            }
        }

        bool isEmpty()
        {
            return ql.empty();
        }

    private:
        void add_to_swaped(PQEntry entry)
        {
            if (entry.second < ql.top().second)
            {
                ql.pop();
                ql.push(entry);
            }
        }

    };

} // namespace popular

#endif