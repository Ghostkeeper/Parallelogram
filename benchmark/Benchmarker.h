/*
 * Library for performing massively parallel computations on polygons.
 * Copyright (C) 2018 Ghostkeeper
 * This library is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
 * This library is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for details.
 * You should have received a copy of the GNU Affero General Public License along with this library. If not, see <https://gnu.org/licenses/>.
 */

#ifndef BENCHMARKER_H
#define BENCHMARKER_H

#include <string> //To return the device identifiers.

namespace cl {

class Device; //Forward declaration of device to associate benchmarker with.

}

namespace parallelogram {
namespace benchmarks {

/*
 * Performs benchmarks to determine the relative performance of various
 * algorithms on the current hardware.
 *
 * These benchmarks will then assist in making a better choice of strategy when
 * the algorithms are executed.
 *
 * The CPP file for this class also contains an entry point in order to perform
 * the benchmarks separately.
 */
class Benchmarker {
public:
	/*
	 * The device that this benchmarker is measuring.
	 *
	 * If measuring the host, this should be ``nullptr``.
	 */
	const cl::Device* device;

	/*
	 * Creates a new benchmarker for benchmarking the performance of a specific
	 * device.
	 * \param device The device to benchmark. If benchmarking the host, use
	 * ``nullptr``.
	 */
	Benchmarker(const cl::Device* device);

	/*
	 * Performs a benchmark on computing the area of a polygon.
	 */
	void benchmark_area() const;

	/*
	 * Prints the statistics of the device that can be detected via OpenGL.
	 *
	 * This will output computational statistics such as clock speed and number
	 * of cores. This is then used to approximate the performance for devices
	 * that are unknown but have similar statistics.
	 */
	void device_statistics() const;

	/*
	 * Returns a string identifying the device.
	 */
	std::string identifier() const;

	void compute_interpolation() const;

	/*
	 * Runs all benchmarks for the current device.
	 */
	void run() const;
private:
	/*
	 * Trims whitespace at the beginning and ending of a string.
	 *
	 * This is a helper function to canonicalise CPU and GPU names. The string
	 * is modified in-place.
	 * \return The input but with the whitespace at the start and end removed.
	 */
	inline void trim(std::string& input) const;
};

}
}

#endif //BENCHMARKER_H