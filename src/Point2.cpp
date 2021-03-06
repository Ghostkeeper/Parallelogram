/*
 * Library for performing massively parallel computations on polygons.
 * Copyright (C) 2018 Ghostkeeper
 * This library is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
 * This library is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for details.
 * You should have received a copy of the GNU Affero General Public License along with this library. If not, see <https://gnu.org/licenses/>.
 */

#include "Point2.h"

namespace apex {

Point2::Point2(const coord_t x, const coord_t y) : x(x), y(y) {
    //Only copy the constructor parameters to the fields.
}

Point2 Point2::operator +(const Point2& other) const {
	return Point2(x + other.x, y + other.y);
}

Point2& Point2::operator +=(const Point2& other) {
	x += other.x;
	y += other.y;
	return *this;
}

Point2 Point2::operator -(const Point2& other) const {
	return Point2(x - other.x, y - other.y);
}

Point2& Point2::operator -=(const Point2& other) {
	x -= other.x;
	y -= other.y;
	return *this;
}

bool Point2::operator ==(const Point2& other) const {
	return x == other.x && y == other.y;
}

coord_t Point2::isLeftOfLineSegment(const Point2& start, const Point2& end) const {
	return (end.x - start.x) * (y - start.y) - (end.y - start.y) * (x - start.x);
}

}