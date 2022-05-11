/**
 * @file
 * Classes and methods for handling and parsing input files.
 */

#ifndef INPUTREADER
#define INPUTREADER

#include <string>

#include "commons.hpp"

namespace popular
{

/**
 * Wrapper class for our file parsing methods.
 */
class InputReader {

public:
	InputReader() {} /**< Empty constructor. */
	~InputReader() {} /**< Empty destructor. */
	
	/**
	 * Reads a tab-separated file into a new corpus object.
	 *
	 * @param filename The file path for in the input file (as a string)
	 * @param corpus The object into which the input file should be loaded.
	 * @return 0 if successful; 1 if the input file could not be opened for reading.
	 * @post All contents of corpus are erased and replaced with the data in the input file.
	 */
	int readFile( std::string &filename, Corpus &corpus );

};

} // namespace popular

#endif
