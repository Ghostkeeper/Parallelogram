/*
 * Library for performing massively parallel computations on polygons.
 * Copyright (C) 2018 Ghostkeeper
 * This library is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
 * This library is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for details.
 * You should have received a copy of the GNU Affero General Public License along with this library. If not, see <https://gnu.org/licenses/>.
 */

#ifndef SIMPLEPOLYGONBENCHMARK_H
#define SIMPLEPOLYGONBENCHMARK_H

#include <time.h> //For high-resolution timers to measure benchmarks.
#include <vector> //To have multiple input sizes.
#include "SimplePolygon.h"

namespace parallelogram {

namespace benchmark {

/*
 * This class holds the parameters to run a benchmark to measure the performance
 * of a method of SimplePolygon.
 *
 * It functions as a factory for SimplePolygon instances, since the benchmark
 * will get run on polygons of different sizes. And it automates the running of
 * a benchmark.
 */
class SimplePolygonBenchmark {
public:
	/*
	 * The function to use to construct the input polygons.
	 *
	 * It is suggested that you use one of the static methods in this class.
	 */
	SimplePolygon(*construct_polygon)(const size_t) = regularNGon;

	/*
	 * The sizes to run the benchmark on, in order to test performance at
	 * various complexities of input.
	 *
	 * It is assumed that the algorithm runs in at most quadratic time to this
	 * input size. If it is more complex, then the polynomial that predicts the
	 * duration of a function call will not fit the data well.
	 */
	std::vector<size_t> input_sizes = {1, 10, 100, 1000, 10000, 20000, 40000, 80000, 160000, 320000, 640000, 1000000, 2000000, 4000000, 8000000};

	/*
	 * How many times to repeat the experiment.
	 *
	 * Repeating more often will result in more accurate measurements, but will
	 * take longer to benchmark.
	 */
	unsigned int repeats = 50;

	/*
	 * Constructs a regular n-gon.
	 *
	 * Use this as parameter for the constructor of `SimplePolygonBenchmark` to
	 * use regular n-gons as input for the benchmarks.
	 */
	static SimplePolygon regularNGon(const size_t size);

	/*
	 * Constructs a new benchmark.
	 *
	 * You can supply the parameters for the benchmark here.
	 * \param name The name of the benchmark. The benchmark data generated by
	 * this test is going to be stored under this name.
	 * \param run A function that executes the method you wish to benchmark. The
	 * function must accept one input `SimplePolygon` instance. Any other
	 * parameters must be generated before the function. Keep in mind that this
	 * is the function that we benchmark, so keep overhead to an absolute
	 * minimum.
	 */
	SimplePolygonBenchmark(const std::string name, const void(*run)(SimplePolygon));

	/*
	 * Starts benchmarking.
	 *
	 * The results of the benchmark are output to cout.
	 * \param device_identifier The device that the benchmarker is currently
	 * running on. This is what the results are keyed at in the output.
	 */
	void benchmark(const std::string device_identifier) const;

private:
	/*
	 * The name of the benchmark.
	 *
	 * This is where the results of the benchmark get stored in the dictionary
	 * of BenchmarkData.h. It is also the name by which the data is then
	 * retrieved during runtime when a call to a function needs to evaluate the
	 * benchmark data in order to determine which implementation to call.
	 */
	const std::string name;

	/*
	 * The function that runs one test with a pre-generated polygon.
	 */
	const void(*run)(SimplePolygon);
};

}

}

#endif //SIMPLEPOLYGONBENCHMARK_H