/**
 * @file
 * Defining of Exact algorithm, a naive enumerating algorithm to solve
 * the Socially Diverse k-Nearest Neighbours query.
 * This algorithm doesn't use any preprocess.
 */

#ifndef POPULAR_LP
#define POPULAR_LP

#include "../algorithm/algorithm.hpp"

#include <glpk.h>

namespace popular
{
    enum class LP_Variant
    {
        Ilp, /**< Naive based on distance */
        LP, /**< Naive based on user */
        Other
    };

    template < LP_Variant variant >
    class Lp : public Algorithm
    {
    public:
        Lp() {} /**< Empty constructor */
        ~Lp() /**< Destructor */
        {
            free(dists_);
            free(checkins_);
            free(ia);
            free(ja);
            free(ar);
            free(xjs);
        }

        Lp(Corpus const& corpus): Algorithm(corpus) {}

        void preprocess(Point const& q, uint32_t k, float const& a) override;
        void query(uint32_t k, Point const& q, float const& a, ResultSet &results, double &z_from_lp,
                uint32_t &prunes, uint32_t &reheaps) override;
        void retrieve_results(uint32_t k, Point const& q, float const& a, ResultSet &results, double &z_from_lp) override;

    private:
        void i_lp_setup(uint32_t const& k, double const& a, int *ia, int *ja, double *ar);
        void i_lp(int *ia, int *ja, double *ar) const;
        void i_lp_retrieve(double *z, double *yis) const;

        std::vector< Point > points;
        std::vector< UserId > users;
        double *dists_;
        double *checkins_;
        glp_prob *lp_;
        unsigned long long size_;
        int *ia;
        int *ja;
        double *ar;
        double *xjs;
    };

} // namespace popular

#endif