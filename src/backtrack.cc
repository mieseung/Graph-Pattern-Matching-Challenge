/**
 * @file backtrack.cc
 *
 */

#include "backtrack.h"
#include <algorithm>

Backtrack::Backtrack()
{
    //    std::cout << "backtrack" << "\n";
}
Backtrack::~Backtrack() {}

/**
* Recursive function for Backtracking
*/
void Backtrack::PrintAllMatches(const Graph &data, const Graph &query,
                                const CandidateSet &cs)
{
    std::cout << "t " << query.GetNumVertices() << "\n";
    std::vector<std::pair<int, int>> searchTree;

    std::vector<std::pair<int, int>> M;
    int *visited = new int[data.GetNumVertices()];
    memset(visited, 0, sizeof(int) * data.GetNumVertices());
    FindPartialEmbedding(data, query, cs, M, visited);
}

void Backtrack::FindPartialEmbedding(const Graph &data, const Graph &query,
                                     const CandidateSet &cs, std::vector<std::pair<int, int>> &M, int *visited)
{

    if (M.size() == query.GetNumVertices())
    {
        /// stdout when a matching is found
        std::cout << "a ";
        for (int i = 0; i < query.GetNumVertices(); i++)
        {
            std::cout << IsInM(M, i, true) << " ";
        }
        std::cout << std::endl;
    }

    /// Initial state of the Partial embedding M
    else if (M.size() == 0)
    {
        for (int i = 0; i < cs.GetCandidateSize(query.root); i++)
        {
            M.push_back(std::pair<int, int>(query.root, cs.GetCandidate(query.root, i)));
            visited[cs.GetCandidate(query.root, i)] = 1;
            FindPartialEmbedding(data, query, cs, M, visited);
            visited[cs.GetCandidate(query.root, i)] = 0;
        }
    }

    else
    {
        int u = ExtendableVertex(query, M, cs);
        std::priority_queue<std::pair<Vertex, int>,
                            std::vector<std::pair<Vertex, int>>, cmp>
            CmU = ExtendableCandidate(data, query, cs, M, u);

        while (!CmU.empty())
        {
            int v = CmU.top().first;
            CmU.pop();
            M.push_back(std::pair<int, int>(u, v));
            visited[v] = 1;
            FindPartialEmbedding(data, query, cs, M, visited);
            visited[v] = 0;
        }
    }

    M.pop_back();
    return;
}

/**
 * Function for whether a vertex is in M or not
 */

int Backtrack::IsInM(std::vector<std::pair<int, int>> &M, int vertex, bool first)
{
    if (first)
    {
        for (int i = 0; i < M.size(); i++)
        {
            if (M.at(i).first == vertex)
            {
                return M.at(i).second;
            }
        }
        return -1;
    }
    else
    {
        for (int i = 0; i < M.size(); i++)
        {
            if (M.at(i).second == vertex)
            {
                return M.at(i).first;
            }
        }
        return -1;
    }
}

// Choose Extendable vertex u of Partial embedding M and use priority_queue to pop the minimum one
// If these is no extendable vertex of Partial embedding M, it returns -1.
// Otherwise, it returns the minimum candidate size extendable vertex

int Backtrack::ExtendableVertex(const Graph &query, std::vector<std::pair<int, int>> &M, const CandidateSet &cs)
{
    std::priority_queue<std::pair<Vertex, int>, std::vector<std::pair<Vertex, int>>, cmp> Ext;

    for (int i = 0; i < query.GetNumVertices(); i++)
    {
        if (IsInM(M, i, true) != -1)
        {
            continue;
        }
        bool parentInM = true;

        for (int j = 0; j < query.numParent[i]; j++)
        {
            if (IsInM(M, query.parentQuery[i][j], true) == -1)
            {
                parentInM = false;
                break;
            }
        }
        if (parentInM)
        {
            // use priority queue to find the minimum candidate size vertex
            Ext.push({i, cs.GetCandidateSize(i)});
        }
    }

    if (!Ext.empty())
    {
        return Ext.top().first;
    }

    return -1;
}

//Find Extendable Candidates of Extendable vertex u and use priority_queue to pop the minimum one

std::priority_queue<std::pair<Vertex, int>, std::vector<std::pair<Vertex, int>>, cmp> Backtrack::ExtendableCandidate(const Graph &data, const Graph &query, const CandidateSet &cs,
                                                                                                                     std::vector<std::pair<int, int>> &M, int u)
{
    // common.h에 정의한 comparator cmp 사용 : pair의 두번째 값을 기준으로 더 작은 값이 먼저 pop될 수 있게끔 구현.
    std::priority_queue<std::pair<Vertex, int>, std::vector<std::pair<Vertex, int>>, cmp> Cm;
    int parentNum = query.numParent[u];
    std::vector<int> parentVs;

    for (int i = 0; i < parentNum; i++)
    {
        int parentU = query.parentQuery[u][i];
        int parentV = IsInM(M, parentU, true);
        parentVs.push_back(parentV);
    }
    for (int i = 0; i < cs.GetCandidateSize(u); i++)
    {
        if (IsInM(M, cs.GetCandidate(u, i), false) != -1)
        {
            continue;
        }
        bool isConnected = true;
        for (int j = 0; j < parentNum; j++)
        {
            if (!data.IsNeighbor(parentVs.at(j), cs.GetCandidate(u, i)))
            {
                isConnected = false;
                break;
            }
        }
        if (isConnected)
        {
            // use degree of each extendable candidate vertex in data graph
            // 만약 data graph에서 해당 candidate vertex의 degree가 작다면,
            // 추후에 추가될 candidate vertex들과 연결되어 있지 않을 가능성이 높아져 따져야하는 경우의 수를 줄일 수 있음.

            Cm.push({cs.GetCandidate(u, i), data.GetDegree(cs.GetCandidate(u, i))});
        }
    }
    return Cm;
}