/*
 * Library for performing massively parallel computations on polygons.
 * Copyright (C) 2018 Ghostkeeper
 * This library is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
 * This library is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for details.
 * You should have received a copy of the GNU Affero General Public License along with this library. If not, see <https://gnu.org/licenses/>.
 */

#ifndef TESTSIMPLEPOLYGONBATCH
#define TESTSIMPLEPOLYGONBATCH

#include <gtest/gtest.h>
#include <limits> //To simulate infinite memory.
#include "SimplePolygon.h" //Example polygons.
#include "SimplePolygonBatch.h"
#include "helpers/MockDevice.h" //To mock out cl::Device.
#include "helpers/SimplePolygonBatchGroper.h" //The class under test.

namespace apex {

/*
 * Some fixtures for the TestSimplePolygonBatch tests.
 */
class TestSimplePolygonBatch : public testing::Test {
protected:
	/*
	 * A polygon to test with. Put it in a batch, for instance.
	 *
	 * It's a triangle, so it'll have 3 vertices. It's also fairly fast to copy.
	 * This is a triangle of 20 by 10 units (so an area of 100).
	 */
	SimplePolygon triangle;

	/*
	 * A vector containing ten triangles.
	 */
	std::vector<SimplePolygon> ten_triangles;

	SimplePolygonBatch<std::vector<SimplePolygon>::iterator, MockDevice> ten_triangles_batch;

	/*
	 * Provides access to ``SimplePolygonBatch``'s private members in order to
	 * test them.
	 */
	SimplePolygonBatchGroper<std::vector<SimplePolygon>::iterator, MockDevice> groper;

	TestSimplePolygonBatch() :
		ten_triangles_batch(ten_triangles.begin(), ten_triangles.end()) {
	}

	virtual void SetUp() {
		triangle.emplace_back(0, 0);
		triangle.emplace_back(20, 0);
		triangle.emplace_back(10, 20);
		for(size_t i = 0; i < 10; i++) {
			ten_triangles.push_back(triangle);
		}
		ten_triangles_batch = SimplePolygonBatch<std::vector<SimplePolygon>::iterator, MockDevice>(ten_triangles.begin(), ten_triangles.end());
	}
};

/*
 * Tests the count and total vertices as generated by the constructor, with an
 * empty batch.
 */
TEST_F(TestSimplePolygonBatch, CountEmpty) {
	std::vector<SimplePolygon> empty;
	SimplePolygonBatch<std::vector<SimplePolygon>::iterator, MockDevice> batch(empty.begin(), empty.end());
	groper.tested_batch = &batch;
	EXPECT_EQ(0, groper.count());
	EXPECT_EQ(0, groper.total_vertices());
}

/*
 * Tests the count and total vertices as generated by the constructor, with a
 * batch containing one triangle.
 */
TEST_F(TestSimplePolygonBatch, CountOne) {
	std::vector<SimplePolygon> one_triangle;
	one_triangle.push_back(triangle);
	SimplePolygonBatch<std::vector<SimplePolygon>::iterator, MockDevice> batch(one_triangle.begin(), one_triangle.end());
	groper.tested_batch = &batch;

	EXPECT_EQ(1, groper.count());
	EXPECT_EQ(3, groper.total_vertices());
}

/*
 * Tests the count and total vertices as generated by the constructor, with a
 * batch containing ten triangles.
 */
TEST_F(TestSimplePolygonBatch, CountTen) {
	groper.tested_batch = &ten_triangles_batch;
	EXPECT_EQ(10, groper.count());
	EXPECT_EQ(30, groper.total_vertices());
}

/*
 * Tests ensure_fit on an empty batch.
 *
 * Whatever size you use, it should never do anything with the batch.
 */
TEST_F(TestSimplePolygonBatch, EnsureFitEmpty) {
	std::vector<SimplePolygon> empty;
	SimplePolygonBatch<std::vector<SimplePolygon>::iterator, MockDevice> batch(empty.begin(), empty.end());
	groper.tested_batch = &batch;

	bool result = groper.ensure_fit(100);
	EXPECT_TRUE(result);
	EXPECT_TRUE(groper.subbatches().empty());

	result = groper.ensure_fit(0);
	EXPECT_TRUE(result);
	EXPECT_TRUE(groper.subbatches().empty());
}

/*
 * Tests ensure_fit on a batch that is smaller than the maximum memory.
 *
 * The batch already fits, so it shouldn't create subbatches.
 */
TEST_F(TestSimplePolygonBatch, EnsureFitAlreadyFits) {
	groper.tested_batch = &ten_triangles_batch;

	constexpr cl_ulong vertex_size = sizeof(cl_ulong) * 2;
	const cl_ulong expected_memory_usage = 40 * vertex_size; //10 triangles, with one extra vertex_size per polygon.
	bool result = groper.ensure_fit(expected_memory_usage + 100); //Fits comfortably.
	EXPECT_TRUE(result);
	EXPECT_TRUE(groper.subbatches().empty());

	result = groper.ensure_fit(expected_memory_usage); //Fits exactly.
	EXPECT_TRUE(result);
	EXPECT_TRUE(groper.subbatches().empty());
}

/*
 * Tests ensure_fit on a batch that is too big for the maximum memory, and needs
 * to be split in two.
 */
TEST_F(TestSimplePolygonBatch, EnsureFitSplitInTwo) {
	groper.tested_batch = &ten_triangles_batch;

	constexpr cl_ulong vertex_size = sizeof(cl_ulong) * 2;
	const bool result = groper.ensure_fit(30 * vertex_size); //Requires 40 vertex_sizes to fit, so this is too little memory.
	EXPECT_TRUE(result);
	ASSERT_EQ(2, groper.subbatches().size());
	SimplePolygonBatchGroper<std::vector<SimplePolygon>::iterator, MockDevice> subbatch_groper;
	subbatch_groper.tested_batch = &groper.subbatches()[0];
	EXPECT_EQ(7, subbatch_groper.count()); //Each triangle requires 4 vertex_sizes. 7 * 4 = 28, which is the maximum that fits.
	EXPECT_EQ(21, subbatch_groper.total_vertices());
	subbatch_groper.tested_batch = &groper.subbatches()[1];
	EXPECT_EQ(3, subbatch_groper.count()); //Remaining 3 triangles.
	EXPECT_EQ(9, subbatch_groper.total_vertices());
}

/*
 * Tests ensure_fit on a batch that is too big for the maximum memory, and needs
 * to be split into many.
 */
TEST_F(TestSimplePolygonBatch, EnsureFitSplitInFive) {
	groper.tested_batch = &ten_triangles_batch;

	constexpr cl_ulong vertex_size = sizeof(cl_ulong) * 2;
	const bool result = groper.ensure_fit(8 * vertex_size); //Fits 2 triangles per batch, exactly.
	EXPECT_TRUE(result);
	EXPECT_EQ(5, groper.subbatches().size());
	SimplePolygonBatchGroper<std::vector<SimplePolygon>::iterator, MockDevice> subbatch_groper;

	for(SimplePolygonBatch<std::vector<SimplePolygon>::iterator, MockDevice>& subbatch : groper.subbatches()) {
		subbatch_groper.tested_batch = &subbatch;
		EXPECT_EQ(2, subbatch_groper.count());
	}
}

/*
 * Tests ensure_fit on a batch that contains polygons with different sizes.
 */
TEST_F(TestSimplePolygonBatch, EnsureFitUnevenSizes) {
	std::vector<SimplePolygon> uneven_sizes;
	uneven_sizes.emplace_back();
	for(size_t i = 0; i < 10; i++) { //First polygon gets 10 vertices.
		uneven_sizes.back().emplace_back(0, 0);
	}
	uneven_sizes.push_back(triangle); //Second polygon gets 3 vertices.
	uneven_sizes.push_back(triangle); //Third polygon gets 3 vertices too.
	uneven_sizes.emplace_back();
	for(size_t i = 0; i < 13; i++) { //Fourth polygon gets 13 vertices.
		uneven_sizes.back().emplace_back(0, 0);
	}
	SimplePolygonBatch<std::vector<SimplePolygon>::iterator, MockDevice> batch(uneven_sizes.begin(), uneven_sizes.end());
	groper.tested_batch = &batch;

	constexpr cl_ulong vertex_size = sizeof(cl_ulong) * 2;
	const bool result = groper.ensure_fit(14 * vertex_size); //Fits one large polygon or multiple triangles.
	EXPECT_TRUE(result);
	ASSERT_EQ(3, groper.subbatches().size());
	SimplePolygonBatchGroper<std::vector<SimplePolygon>::iterator, MockDevice> subbatch_groper;
	subbatch_groper.tested_batch = &groper.subbatches()[0];
	EXPECT_EQ(1, subbatch_groper.count()); //This batch contains just the first polygon. The second doesn't fit any more.
	EXPECT_EQ(10, subbatch_groper.total_vertices());
	subbatch_groper.tested_batch = &groper.subbatches()[1];
	EXPECT_EQ(2, subbatch_groper.count()); //This batch contains the two triangles.
	EXPECT_EQ(6, subbatch_groper.total_vertices());
	subbatch_groper.tested_batch = &groper.subbatches()[2];
	EXPECT_EQ(1, subbatch_groper.count()); //This batch contains just the last polygon.
	EXPECT_EQ(13, subbatch_groper.total_vertices());
}

/*
 * Tests the behaviour when ensure_fit is called multiple times with
 * consecutively smaller maximum memory.
 *
 * The second time, it has to re-batch everything, discarding the original
 * subbatches.
 */
TEST_F(TestSimplePolygonBatch, EnsureFitRebatch) {
	groper.tested_batch = &ten_triangles_batch;

	constexpr cl_ulong vertex_size = sizeof(cl_ulong) * 2;
	bool result = groper.ensure_fit(20 * vertex_size); //Fits 5 triangles per batch initially.
	EXPECT_TRUE(result);
	ASSERT_EQ(2, groper.subbatches().size());
	SimplePolygonBatchGroper<std::vector<SimplePolygon>::iterator, MockDevice> subbatch_groper;
	for(SimplePolygonBatch<std::vector<SimplePolygon>::iterator, MockDevice>& subbatch : groper.subbatches()) {
		subbatch_groper.tested_batch = &subbatch;
		EXPECT_EQ(5, subbatch_groper.count()); //All 2 subbatches have 5 triangles.
	}

	result = groper.ensure_fit(8 * vertex_size); //Fits only 2 triangles per batch now!
	EXPECT_TRUE(result);
	ASSERT_EQ(5, groper.subbatches().size());
	for(SimplePolygonBatch<std::vector<SimplePolygon>::iterator, MockDevice>& subbatch : groper.subbatches()) {
		subbatch_groper.tested_batch = &subbatch;
		EXPECT_EQ(2, subbatch_groper.count()); //All 5 subbatches have 2 triangles.
	}
}

/*
 * Tests ensure_fit when it can't find a way to fit the polygons in memory
 * because one polygon is too big.
 */
TEST_F(TestSimplePolygonBatch, EnsureFitTooBig) {
	std::vector<SimplePolygon> polygons;
	polygons.emplace_back();
	for(size_t i = 0; i < 10; i++) {
		polygons.back().emplace_back(0, 0);
	}
	SimplePolygonBatch<std::vector<SimplePolygon>::iterator, MockDevice> batch(polygons.begin(), polygons.end());
	groper.tested_batch = &batch;

	//First polygon doesn't fit.
	constexpr cl_ulong vertex_size = sizeof(cl_ulong) * 2;
	bool result = groper.ensure_fit(10 * vertex_size); //Needs 11 vertex_sizes (one for end marker), so it won't fit.
	EXPECT_FALSE(result);
	EXPECT_TRUE(groper.subbatches().empty());

	//Last polygon doesn't fit.
	polygons.emplace_back();
	for(size_t i = 0; i < 20; i++) {
		polygons.back().emplace_back(0, 0);
	}
	batch = SimplePolygonBatch<std::vector<SimplePolygon>::iterator, MockDevice>(polygons.begin(), polygons.end());
	groper.tested_batch = &batch;
	result = groper.ensure_fit(15 * vertex_size);
	EXPECT_FALSE(result);
	EXPECT_TRUE(groper.subbatches().empty());

	//Second polygon doesn't fit (but the last one does).
	polygons.push_back(triangle);
	batch = SimplePolygonBatch<std::vector<SimplePolygon>::iterator, MockDevice>(polygons.begin(), polygons.end());
	groper.tested_batch = &batch;
	result = groper.ensure_fit(15 * vertex_size);
	EXPECT_FALSE(result);
	EXPECT_TRUE(groper.subbatches().empty());
}

/*
 * Starts running the tests.
 *
 * This calls upon GoogleTest to start testing.
 * \param argc The number of arguments to read.
 * \param argv The arguments provided to this application via the command.
 * \return ``0`` if all tests ran successfully, or something else if any test
 * failed.
 */
int main(int argc, char* argv[]) {
	testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}

}

#endif //TESTSIMPLEPOLYGONBATCH