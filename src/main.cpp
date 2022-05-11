/**
 * main.cpp
 */

#include <iostream>
#include <boost/program_options.hpp> // for handling input arguments
#include <chrono> // for timing
#include <sys/resource.h> // for reading mem usage
#include <fstream> // for ifstream

#include "util/commons.hpp"
#include "util/inputReader.hpp"
#include "algorithm/algorithm.hpp"
#include "greedy/greedy.hpp"
#include "heuristic/heuristic.hpp"
#include "util/outputwriter.hpp"
#include "exact/exact.hpp"
#include "ilp/lp.hpp"
#include "ilp/lp_methods.h" // I don't understand why this is needed here
#include "rtree/rtree.hpp"

namespace po = boost::program_options;

const char* ARG_HELP = "help";
const char* ARG_K = "k";
const char* ARG_Q = "query";
const char* ARG_INPUT = "input";
const char* ARG_ALGORITHM = "algorithm";
const char* ARG_A = "a";

namespace
{
    struct Parameters
    {
        uint32_t k;
        std::vector< popular::Point > query_points;
        std::string input_file;
        std::stringstream algorithms;
        float a;
    };

    struct Rule {};
    Rule const tag_rule;

    std::ostream& operator << (std::ostream& stream, Rule const&)
    {
        stream << "\033[95m";
        for (int i = 0; i < 195; i++) { stream << "\u2500"; }
        return stream << "\033[00m";
    }

    size_t peak_rss()
    {
        // From https://stackoverflow.com/a/14927379/2769271
        struct rusage rusage;
        getrusage( RUSAGE_SELF, &rusage );
        return rusage.ru_maxrss * 1024L;
    }

    long double median(std::vector< long double > microseconds)
    {
        const auto size = std::distance(microseconds.begin(), microseconds.end());
        std::nth_element(microseconds.begin(), microseconds.begin() + size / 2, microseconds.end());
        return *std::next(microseconds.begin(), size / 2);
    }

    long double sum(std::vector< long double > const& scores)
    {
        double s = std::accumulate (scores.cbegin(), scores.cend(), 0.0,
                []( auto i, auto p ){ return i + p; });
        return s;
    }
} // anonymous namespace

int main( int argc, char** argv ) {

    using namespace popular;

    Parameters parameters;
    Corpus corpus;
    Algorithm *alg;
    Stats stats;
    OutputWriter outWriter;

    try {
        po::options_description desc("Allowed options");
        desc.add_options()
                (ARG_HELP, "produce help message")
                (ARG_K, po::value< std::uint32_t >(), "number of POIs to report")
                (ARG_Q, po::value< std::vector< std::string > >()->multitoken(), "query point(s), multi token")
                (ARG_INPUT, po::value< std::string >(), "set input file")
                (ARG_ALGORITHM, po::value< std::string >(),
                 "choose algorithm(s), space separated; choices are:"
                 " exact naive dist user greedy lp ilp rtree re-heap")
                (ARG_A, po::value< float >(), "the parameter alpha");

        po::variables_map vm;
        po::store(po::parse_command_line(argc, argv, desc, po::command_line_style::unix_style ^ po::command_line_style::allow_short), vm);
        po::notify(vm);

        /* Help trumps everything */
        if (vm.count(ARG_HELP))
        {
            std::cout << desc << std::endl;
            return 0;
        }
        if (vm.count(ARG_INPUT))
        {
            parameters.input_file = vm[ARG_INPUT].as< std::string >();
            InputReader ir;
            if (ir.readFile(parameters.input_file, corpus) == 1)
            {
                return 1;
            }
            stats.input_file = parameters.input_file;
        }
        else
        {
            std::cout << "You must specify an input file" << std::endl;
            std::cout << desc << std::endl;
            return 0;
        }
        if (vm.count(ARG_K))
        {
            parameters.k = vm[ARG_K].as< std::uint32_t >();
        }
        else
        {
            std::cout << "You must specify a k" << std::endl;
            std::cout << desc << std::endl;
            return 0;
        }
        if (vm.count(ARG_Q))
        {
            std::vector< std::string > points = vm[ARG_Q].as< std::vector< std::string > >();
            std::for_each(points.begin(), points.end(),
                           [ &parameters ]( std::string p ){
                               std::size_t x_end = p.find(',');
                               float x = std::stof(p.substr(0, x_end));
                               float y = std::stof(p.substr(x_end+1));
                               parameters.query_points.push_back(Point(x,y)); } );
        }
        else
        {
            std::cout << "You must specify at least one query point" << std::endl;
            std::cout << desc << std::endl;
            return 0;
        }
        if (vm.count(ARG_A))
        {
            parameters.a = vm[ARG_A].as< float >();
        }
        else
        {
            std::cout << "You must specify a parameter alpha" << std::endl;
            std::cout << desc << std::endl;
            return 0;
        }
        if (vm.count(ARG_ALGORITHM))
        {
            parameters.algorithms.str(vm[ARG_ALGORITHM].as< std::string >());
            std::string next_algorithm;

            std::cout << tag_headers << std::endl << tag_rule << std::endl;
            while (parameters.algorithms >> next_algorithm)
            {
                uint32_t query_index = 1;
                std::unordered_map< std::string, std::vector< long double > > batches;
                for (auto const& q : parameters.query_points)
                {
                    if (next_algorithm.compare("exact") == 0)
                    {
                        alg = new Exact(corpus);
                        stats.algorithm = "exact";
                        stats.alg_index = 0;
                    }
                    else if (next_algorithm.compare("naive") == 0)
                    {
                        alg = new Heuristic< Heuristic_Variant::Naive >(corpus);
                        stats.algorithm = next_algorithm;
                        stats.alg_index = 1;
                    }
                    else if (next_algorithm.compare("dist") == 0)
                    {
                        alg = new Heuristic< Heuristic_Variant::Naive_dist >(corpus);
                        stats.algorithm = next_algorithm;
                        stats.alg_index = 2;
                    }
                    else if (next_algorithm.compare("user") == 0)
                    {
                        alg = new Heuristic< Heuristic_Variant::Naive_user >(corpus);
                        stats.algorithm = next_algorithm;
                        stats.alg_index = 3;
                    }
                    else if (next_algorithm.compare("lp") == 0)
                    {
                        alg = new Lp< LP_Variant::LP >(corpus);
                        stats.algorithm = next_algorithm;
                        stats.alg_index = 4;
                    }
                    else if (next_algorithm.compare("ilp") == 0)
                    {
                        alg = new Lp< LP_Variant::Ilp >(corpus);
                        stats.algorithm = next_algorithm;
                        stats.alg_index = 5;
                    }
                    else if (next_algorithm.compare("greedy") == 0)
                    {
                        alg = new Greedy< Greedy_Variant::Naive >(corpus);
                        stats.algorithm = next_algorithm;
                        stats.alg_index = 6;
                    }
                    else if (next_algorithm.compare("rtree") == 0)
                    {
#ifdef NPRUNE
                        std::cout << "\033[93mThe flag NPRUNE is set. There won't be any pruning checks on the tree.\033[00m" << std::endl;
#endif
                        alg = new Indexed< Indexed_Variant::Naive >(corpus);
                        stats.algorithm = next_algorithm;
                        stats.alg_index = 7;
                    }
                    else if (next_algorithm.compare("re-heap") == 0)
                    {
#ifdef NPRUNE
                        std::cout << "\033[93mThe flag NPRUNE is set. There won't be any pruning checks on the tree.\033[00m" << std::endl;
#endif
                        alg = new Indexed< Indexed_Variant::ReHeap >(corpus);
                        stats.algorithm = next_algorithm;
                        stats.alg_index = 8;
                    }
                    else
                    {
                        std::cout << "Algorithm " << next_algorithm << " unknown." << std::endl;
                        std::cout << desc << std::endl;
                        return 0;
                    }

                    stats.query = q;
                    stats.query_index = query_index;
                    stats.k = parameters.k;
                    stats.a = parameters.a;
                    stats.num_points = corpus.places.size();
                    stats.num_users = corpus.users.size();
                    stats.num_checkins = corpus.num_checkins;
                    ResultSet results;
                    double z_from_lp;
                    uint32_t prunes;
                    uint32_t reheaps;

                    uint32_t kk = (parameters.k >= corpus.places.size()) ? corpus.places.size() : parameters.k;
                    auto start_preprocess = std::chrono::high_resolution_clock::now();
                    alg->preprocess(q, kk, parameters.a);
                    auto const elapsed_preprocess = std::chrono::high_resolution_clock::now() - start_preprocess;

                    auto start_q = std::chrono::high_resolution_clock::now();
                    alg->query(kk, q, parameters.a, results, z_from_lp, prunes, reheaps);
                    auto const elapsed_q = std::chrono::high_resolution_clock::now() - start_q;

                    auto start_retrieve = std::chrono::high_resolution_clock::now();
                    alg->retrieve_results(kk, q, parameters.a, results, z_from_lp);
                    auto const elapsed_retrieve = std::chrono::high_resolution_clock::now() - start_retrieve;

                    size_t peak = peak_rss();

                    long long nanosec_preprocess =
                            std::chrono::duration_cast<std::chrono::nanoseconds>(elapsed_preprocess).count();
                    long long nanosec_q = std::chrono::duration_cast<std::chrono::nanoseconds>(elapsed_q).count();
                    long long nanosec_retrieve =
                            std::chrono::duration_cast<std::chrono::nanoseconds>(elapsed_retrieve).count();
                    stats.microseconds_pre = nanosec_preprocess / 1000;
                    stats.microseconds_q = nanosec_q / 1000;
                    stats.microseconds_retrieve = nanosec_retrieve / 1000;
                    stats.microseconds_all = (nanosec_preprocess + nanosec_q + nanosec_retrieve) / 1000;
                    stats.peak_rss = peak;
                    stats.z_from_lp = z_from_lp;
                    stats.actual_score = results.second;
                    stats.prunes = prunes;
                    stats.reheaps = reheaps;
                    std::cout << stats << std::endl;
                    query_index++;

                    // save the results for the batches
                    batches["preprocess"].push_back(stats.microseconds_pre);
                    batches["query"].push_back(stats.microseconds_q);
                    batches["retrieve"].push_back(stats.microseconds_retrieve);
                    batches["all"].push_back(stats.microseconds_all);
                    batches["rss"].push_back(stats.peak_rss);
                    batches["z"].push_back(stats.z_from_lp);
                    batches["score"].push_back(stats.actual_score);
                    batches["prunes"].push_back(stats.prunes);
                    batches["reheaps"].push_back(stats.reheaps);

                    delete alg;
                }

                // batched stats
                stats.query = std::make_pair(0.0,0.0);
                stats.query_index = 0;
                stats.microseconds_pre = median(batches["preprocess"]);
                stats.microseconds_q = median(batches["query"]);
                stats.microseconds_retrieve = median(batches["retrieve"]);
                stats.microseconds_all = median(batches["all"]);
                stats.peak_rss = median(batches["rss"]);
                stats.z_from_lp = sum(batches["z"]);
                stats.actual_score = sum(batches["score"]);
                stats.prunes = median(batches["prunes"]);
                stats.reheaps = median(batches["reheaps"]);
                std::cout << stats << std::endl;
                outWriter.writeResults(stats);
            }
        }
        else
        {
            std::cout << "You must specify at least one algorithm" << std::endl;
            std::cout << desc << std::endl;
            return 0;
        }
    }
    catch (std::exception const& e)
    {
        std::cerr << "error: " << e.what() << std::endl;
        return 1;
    }
    catch (...)
    {
        std::cerr << "Exception of unknown type!" << std::endl;
        return 1;
    }

    return 0;
}
