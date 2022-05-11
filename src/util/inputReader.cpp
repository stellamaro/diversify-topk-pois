/**
 * @file
 * Implementation of input handling methods.
 */

#include "inputReader.hpp"

#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <algorithm> // std::for_each()


namespace popular
{

/**
 * Assumes tab-separated file.
 * Iterates each line of the input, splits it on the tab character to produce
 * a userID a longitude and a latitude.
 */
int InputReader::readFile( std::string &filename, Corpus &corpus ) {
	
	std::ifstream infile( filename );
	if( infile ) { // check that file opened correctly.
        std::string line;
        while (std::getline(infile, line))
        {
            std::stringstream ss;
            ss.str(line);
            std::string item;
            if (std::getline(ss, item, '\t'))
            {
                UserId user = stoi(item);
                corpus.users.insert(user);
                Point point;
                if (std::getline(ss, item, '\t'))
                {
                    point.first = std::stof(item);
                    if (std::getline(ss, item, '\t'))
                    {
                        point.second = std::stof(item);
                        corpus.places.insert(point);
                        std::vector< UserId > a = corpus.checkins[point];
                        a.push_back(user);
                        corpus.checkins[point] = a;

                        corpus.xmin = std::min( corpus.xmin, point.first );
                        corpus.xmax = std::max( corpus.xmax, point.first );
                        corpus.ymin = std::min( corpus.ymin, point.second );
                        corpus.ymax = std::max( corpus.ymax, point.second );

                        corpus.num_checkins++;
                    }
                }
            }
        }

        std::for_each(corpus.checkins.begin(), corpus.checkins.end(),
                [](auto &checkin)
                {
                    std::sort(checkin.second.begin(), checkin.second.end());
                    auto it = std::unique(checkin.second.begin(), checkin.second.end());
                    checkin.second.resize(std::distance(checkin.second.begin(),it));
                });

        Point pmin = std::make_pair(corpus.xmin, corpus.ymin);
        Point pmax = std::make_pair(corpus.xmax, corpus.ymax);
        corpus.max_distance = distance(pmin, pmax);
	}
	else {
		std::cerr << "Could not open input file for reading: " << filename << std::endl;
		return 1;
	}
	
	return 0; // Success!
}

} // namespace popular
