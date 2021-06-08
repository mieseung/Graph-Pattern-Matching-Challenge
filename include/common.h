/**
 * @file common.h
 * @brief common headers and type definitions
 *
 */

#ifndef COMMON_H_
#define COMMON_H_

#include <algorithm>
#include <climits>
#include <fstream>
#include <iostream>
#include <set>
#include <string>
#include <utility>
#include <vector>

using Vertex = int32_t;
using Label = int32_t;

// compator for priority queue
// it makes priority queue pop the element in ascending order of second element of pair
struct cmp
{
    bool operator()(std::pair<Vertex, int> &a, std::pair<Vertex, int> &b)
    {
        return a.second >= b.second;
    }
};

#endif // COMMON_H_
