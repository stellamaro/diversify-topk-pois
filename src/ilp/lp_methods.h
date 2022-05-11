#ifndef LPMETHODS
#define LPMETHODS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <glpk.h>

void fix_matrix(__uint32_t dists_size, __uint32_t users_size, const double *checkins,
        int *ia, int *ja, double *ar, unsigned long long *c)
{
    __uint32_t i, j;
    // the k line (i.e. line 1)
    for(j = users_size+1; j <= users_size+dists_size; j++)
    {
        ia[*c] = 1;
        ja[*c] = j;
        ar[*c] = 1.0; // for k we need only xjs
        *c = *c + 1;
    }
    // for the rest of the lines
    __uint32_t l; // counts the lines in the matrix (after k)
    for(l = 2; l <= users_size+1; l++)
    {
        for(i = 1; i <= users_size; i++)
        {
            if( l == (i + 1) )
            {
                ia[*c] = l;
                ja[*c] = i;
                ar[*c] = -1.0;
                *c = *c + 1;
            }
        }
        for(j = users_size+1; j <= users_size+dists_size; j++)
        {
            if( *( (checkins+(l-1-1)*dists_size)+(j-users_size-1) ) == 1.0 )
            {
                ia[*c] = l;
                ja[*c] = j;
                ar[*c] = 1.0;
                *c = *c + 1;
            }
        }
    }
}

void lp_setup(glp_prob *lp, __uint32_t dists_size, __uint32_t users_size, __uint32_t k, double a,
              const double *dists, const double *checkins, int *ia, int *ja, double *ar, unsigned long long *c)
{
    __uint32_t i, j;

    glp_set_prob_name(lp, "lp formulation");
    glp_set_obj_dir(lp, GLP_MAX); // maximization problem

    // 1 + users_size lines for the equations
    glp_add_rows(lp, 1+users_size);
    glp_set_row_name(lp, 1, "k");
    glp_set_row_bnds(lp, 1, GLP_UP, 0.0, k);
    for(i = 2; i <= users_size+1; i++)
    {
        glp_set_row_name(lp, i, "ys");
        glp_set_row_bnds(lp, i, GLP_LO, 0.0, 0.0); // 0 <= p < +inf
    }

    // dists_size of columns (xj+yi j=|points| i=|users|)
    // z = (1-a)*sum{yi/max_users} + a*sum{(1-(dist/max_dist))*xj}
    glp_add_cols(lp, users_size + dists_size);
    for(i = 1; i <= users_size; i++)
    {
        glp_set_col_name(lp, i, "yi");
        glp_set_col_bnds(lp, i, GLP_DB, 0.0, 1.0);
        glp_set_obj_coef(lp, i, (1 - a)/users_size );
    }
    for(j = users_size+1; j <= users_size+dists_size; j++)
    {
        glp_set_col_name(lp, j, "xj");
        glp_set_col_bnds(lp, j, GLP_DB, 0.0, 1.0);
        glp_set_obj_coef(lp, j, (a * (dists[j-users_size-1]) ) );
    }

    // fix the constraints matrix
    fix_matrix(dists_size, users_size, checkins, ia, ja, ar,c);
    *c = *c - 1;
}

void lp_method(glp_prob *lp, unsigned long long size, int *ia, int *ja, double *ar)
{
    // begin the solving of the problem
    glp_load_matrix(lp, size, ia, ja, ar);
    glp_smcp smcp;
    glp_init_smcp(&smcp);
    smcp.msg_lev = GLP_MSG_OFF;
    glp_simplex(lp, &smcp);
}

void lp_retrieve(glp_prob *lp, __uint32_t dists_size, __uint32_t users_size,
        double *z, double *xjs, double *yis)
{
    __uint32_t i, j;

    // get the results
    *z = glp_get_obj_val(lp);
    for(i = 1; i <= users_size; i++)
    {
        yis[i-1] = glp_get_col_prim(lp, i);
    }
    for(j = users_size+1; j <= users_size+dists_size; j++)
    {
        xjs[j-users_size-1] = glp_get_col_prim(lp, j);
    }
    glp_delete_prob(lp);
}

void ilp_setup(glp_prob *lp, __uint32_t dists_size, __uint32_t users_size, __uint32_t k, double a,
               const double *dists, const double *checkins, int *ia, int *ja, double *ar, unsigned long long *c)
{
    __uint32_t i, j;

    glp_set_prob_name(lp, "ilp formulation");
    glp_set_obj_dir(lp, GLP_MAX); // maximization problem

    // 1 + users_size lines for the equations
    glp_add_rows(lp, 1+users_size);
    glp_set_row_name(lp, 1, "k");
    glp_set_row_bnds(lp, 1, GLP_UP, 0.0, k);
    for(i = 2; i <= users_size+1; i++)
    {
        glp_set_row_name(lp, i, "ys");
        glp_set_row_bnds(lp, i, GLP_LO, 0.0, 0.0); // 0 <= p < +inf
    }

    // dists_size of columns (xj+yi j=|points| i=|users|)
    // z = (1-a)*sum{yi/max_users} + a*sum{(1-(dist/max_dist))*xj}
    glp_add_cols(lp, users_size + dists_size);
    for(i = 1; i <= users_size; i++)
    {
        glp_set_col_name(lp, i, "yi");
        glp_set_col_kind(lp, i, GLP_BV);
        glp_set_obj_coef(lp, i, (1 - a)/users_size );
    }
    for(j = users_size+1; j <= users_size+dists_size; j++)
    {
        glp_set_col_name(lp, j, "xj");
        glp_set_col_kind(lp, j, GLP_BV);
        glp_set_obj_coef(lp, j, (a * (dists[j-users_size-1]) ) );
    }

    // fix the constraints matrix
    fix_matrix(dists_size, users_size, checkins, ia, ja, ar, c);
    *c = *c - 1;
}

void ilp_method(glp_prob *lp, unsigned long long size, int *ia, int *ja, double *ar)
{
    // begin the solving of the problem
    glp_load_matrix(lp, size, ia, ja, ar);
    glp_iocp iocp;
    glp_init_iocp(&iocp);
    iocp.presolve = GLP_ON;
    iocp.binarize = GLP_ON;
    iocp.msg_lev = GLP_MSG_OFF;
    glp_intopt(lp, &iocp);
}

void ilp_retrieve(glp_prob *lp, __uint32_t dists_size, __uint32_t users_size,
                 double *z, double *xjs, double *yis)
{
    __uint32_t i, j;

    // get the results
    *z = glp_mip_obj_val(lp);
    for(i = 1; i <= users_size; i++)
    {
        yis[i-1] = glp_mip_col_val(lp, i);
    }
    for(j = users_size+1; j <= users_size+dists_size; j++)
    {
        xjs[j-users_size-1] = glp_mip_col_val(lp, j);
    }

    glp_delete_prob(lp);
}

#endif