/**
 * @file backtrack.h
 *
 */

#ifndef BACKTRACK_H_
#define BACKTRACK_H_

#include "candidate_set.h"
#include "common.h"
#include "graph.h"
#include <queue>

class Backtrack
{
public:
    Backtrack();
    ~Backtrack();

    void PrintAllMatches(const Graph &data, const Graph &query,
                         const CandidateSet &cs);

    void FindPartialEmbedding(const Graph &data, const Graph &query, const CandidateSet &cs,
                              std::vector<std::pair<int, int>> &searchTree, int u, int v, bool &backtrack);

    void FindPartialEmbedding(const Graph &data, const Graph &query, const CandidateSet &cs,
                              std::vector<std::pair<int, int>> &searchTree, int u, int v);

    void
    FindPartialEmbedding(const Graph &data, const Graph &query, const CandidateSet &cs,
                         std::vector<std::pair<int, int>> &M, int *visited);

    int ExtendableVertex(const Graph &query, std::vector<std::pair<int, int>> &M, const CandidateSet &cs);

    int IsInM(std::vector<std::pair<int, int>> &M, int vertex, bool first);
    /*
    std::vector<int> ExtendableCandidate(const Graph &data, const Graph &query, const CandidateSet &cs,
                                         std::vector<std::pair<int, int>> M,
                                         int u);*/

    std::priority_queue<std::pair<Vertex, int>,
                        std::vector<std::pair<Vertex, int>>, cmp>
    ExtendableCandidate(const Graph &data, const Graph &query, const CandidateSet &cs,
                        std::vector<std::pair<int, int>> &M,
                        int u);
};

#endif // BACKTRACK_H_
