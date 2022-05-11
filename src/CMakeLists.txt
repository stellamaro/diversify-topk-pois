cmake_minimum_required( VERSION 3.4 )
project( diversify_pois )

set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++1z -Wall -fopenmp -Wextra -g ${NPRUNE}" )
set( CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Og" )
set( CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -DNDEBUG -O3 -march=native" )
set( basepath "${CMAKE_SOURCE_DIR}/.." )

# find boost
find_package( Boost 1.58.0 COMPONENTS program_options REQUIRED )

# for glpk
set( CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake )
find_package( GLPK )

# for my code
add_subdirectory( util )
add_subdirectory( algorithm )
add_subdirectory( greedy )
add_subdirectory( exact )
add_subdirectory( ilp )
add_subdirectory( heuristic )
add_subdirectory( rtree )

# link everything together
add_executable( diversify_pois main.cpp )
# link boost, glpk
target_link_libraries( diversify_pois ${Boost_LIBRARIES} ${GLPK_LIBRARIES} )
# link my code
target_link_libraries( diversify_pois util algorithm greedy exact ilp heuristic rtree )
