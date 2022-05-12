# Diversifying Top-k Point-of-Interest Queries via Collective Social Reach

This repository contains the C++ source code of our paper
`Diversifying Top-k Point-of-Interest Queries via Collective Social Reach`
that was published on CIKM 2020 and can be found 
[here](https://dl.acm.org/doi/abs/10.1145/3340531.3412097).

If you use our code, please cite:
> Stella Maropaki, Sean Chester, Christos Doulkeridis, and Kjetil Nørvåg. 
> "Diversifying Top-k Point-of-Interest Queries via Collective Social Reach". 
> In Proceedings of the 29th ACM International Conference on Information & Knowledge Management 
> (CIKM '20).
> pp. 2149-2152. 2020.
> https://doi.org/10.1145/3340531.3412097


## Build

This project has been set up to use `CMake` for an out-of-source build.

The code is using:
* the Boost C++ libraries: https://www.boost.org/
* the GLPK (GNU Linear Programming Kit) package:
  https://www.gnu.org/software/glpk/
* the [src/rtree/RTree.h](/src/rtree/RTree.h) file is from https://github.com/nushoin/RTree

To build the code in _debug_ mode, execute the following instructions:
> mkdir bin-debug
>
> cd bin-debug
>
> cmake ../src/ -DCMAKE_BUILD_TYPE=Debug
>
> make

To build the code in _release_ mode, execute the following instructions:
> mkdir bin-release
>
> cd bin-release
>
> cmake ../src/ -DCMAKE_BUILD_TYPE=Release
>
> make


## Run

By running the executable `diversify_pois` with no arguments gives
a list of allowed options:
> ./diversify_pois

| Argument        | Description                                                                                          |
|-----------------|------------------------------------------------------------------------------------------------------|
| --help          | produce help message                                                                                 |
| --k arg         | number of POIs to report                                                                             |
| --query arg     | query point(s), multi token                                                                          |
| --input arg     | set input file                                                                                       |
| --algorithm arg | choose algorithm(s), space separated; choices are: exact naive dist user lp ilp greedy rtree re-heap |
| --a             | the parameter alpha for the scoring function                                                         |

An example execution can be the following:
> ./diversify_pois --input "../workloads/test.tsv" --k 2 --query "6,4" "4,6"
        --algorithm "exact naive" --a 0.5

In this example, the algorithms `exact` and `naive` are executed for query points `(6,4)` and `(4,6)`, and alpha parameter `0.5`.
Each setting is performed once, and the running times are shown in the console and appended in the
`results_log.txt` file.

For each algorithm, the median of the running times of all the queries, and the sum of the scores from all the queries, are calculated and shown in the results with query point `(0,0)` and query index `0`.


## Input Data

The input data are assumed to be tab-separated file with the format of
`userID \t longtitude \t latitude`.

Example workloads can be found in the _workloads_ folder.


## License

This code is published under the MIT License.


## Contribution

Contributions are gladly welcomed for this project.

If you would like to make suggestions, please open an issue.
If you would like to make changes yourself, please open a new branch, make your edits on that branch,
and then create a pull request for @stellamaro or @sean-chester to review and merge.


## Contact

Feel free to submit any questions, comments, or corrections to @stellamaro or
@sean-chester

