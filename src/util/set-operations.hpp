/**
 * @file
 * Common set operations
 */

#ifndef POPULAR_SET_OPERATIONS
#define POPULAR_SET_OPERATIONS

#include <vector>
#include <boost/function_output_iterator.hpp> // std::make_function_output_iterator()


namespace popular
{
    /**
     * Help method that calculates the union of 2 vectors and keeps the unique elements
     * @param a : the first set
     * @param b : the second set
     * @returns : the unique result set
     */
    template < typename T >
    std::vector< T > my_set_union( std::vector< T > const& a, std::vector< T > const& b )
    {
        std::vector< T > result;
        result.reserve( a.size() + b.size() );

        std::set_union  ( a.cbegin(), a.cend()
                        , b.cbegin(), b.cend()
                        , std::back_inserter( result ) );
        return result;
    }

    /**
     * Counts the number of elements in the union of two sets without physically materialising the set
     * @see https://stackoverflow.com/a/44348980/2769271
     */
    template < typename T >
    size_t set_union_size( std::vector< T > const& a, std::vector< T > const& b )
    {
        size_t count = 0u;
        std::set_union( a.cbegin(), a.cend()
                      , b.cbegin(), b.cend()
                      , boost::make_function_output_iterator( [ &count ]( T ){ ++count; } ) );
        return count;
    }

    /**
     * Counts the number of elements in set a that are not in set b without physically materialising the difference
     * @see https://stackoverflow.com/a/44348980/2769271
     */
    template < typename T >
    size_t set_difference_size( std::vector< T > const& a, std::vector< T > const& b )
    {
        size_t count = 0u;
        std::set_difference ( a.cbegin(), a.cend()
                            , b.cbegin(), b.cend()
                            , boost::make_function_output_iterator( [ &count ]( T ){ ++count; } ) );
        return count;
    }

} // namespace popular

#endif

