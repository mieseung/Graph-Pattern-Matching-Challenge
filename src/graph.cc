/**
 * @file graph.cc
 *
 */

#include <cfloat>
#include "graph.h"

namespace {
std::vector<Label> transferred_label;
void TransferLabel(const std::string &filename) {
  std::ifstream fin(filename);

  if (!fin.is_open()) {
    std::cout << "Graph file " << filename << " not found!\n";
    exit(EXIT_FAILURE);
  }

  char type;
  int32_t graph_id_;
  size_t num_vertices_;

  std::set<Label> label_set;

  fin >> type >> graph_id_ >> num_vertices_;

  // preprocessing
  while (fin >> type) {
    if (type == 'v') {
      Vertex id;
      Label l;
      fin >> id >> l;

      label_set.insert(l);
    } else if (type == 'e') {
      Vertex v1, v2;
      Label l;
      fin >> v1 >> v2 >> l;
    }
  }

  fin.close();

  transferred_label.resize(
      *std::max_element(label_set.begin(), label_set.end()) + 1, -1);

  Label new_label = 0;
  for (Label l : label_set) {
    transferred_label[l] = new_label;
    new_label += 1;
  }
}
}  // namespace

Graph::Graph(const std::string &filename, bool is_query) {
  if (!is_query) {
    TransferLabel(filename);
  }
  std::vector<std::vector<Vertex>> adj_list;

  // Load Graph
  std::ifstream fin(filename);
  std::set<Label> label_set;

  if (!fin.is_open()) {
    std::cout << "Graph file " << filename << " not found!\n";
    exit(EXIT_FAILURE);
  }

  char type;

  fin >> type >> graph_id_ >> num_vertices_;

  adj_list.resize(num_vertices_);

  start_offset_.resize(num_vertices_ + 1);
  label_.resize(num_vertices_);

  num_edges_ = 0;

  // preprocessing
  while (fin >> type) {
    if (type == 'v') {
      Vertex id;
      Label l;
      fin >> id >> l;

      if (static_cast<size_t>(l) >= transferred_label.size())
        l = -1;
      else
        l = transferred_label[l];

      label_[id] = l;
      label_set.insert(l);
    } else if (type == 'e') {
      Vertex v1, v2;
      Label l;
      fin >> v1 >> v2 >> l;

      adj_list[v1].push_back(v2);
      adj_list[v2].push_back(v1);

      num_edges_ += 1;
    }
  }

  fin.close();

  adj_array_.resize(num_edges_ * 2);

  num_labels_ = label_set.size();

  max_label_ = *std::max_element(label_set.begin(), label_set.end());

  label_frequency_.resize(max_label_ + 1);

  start_offset_by_label_.resize(num_vertices_ * (max_label_ + 1));

  start_offset_[0] = 0;
  for (size_t i = 0; i < adj_list.size(); ++i) {
    start_offset_[i + 1] = start_offset_[i] + adj_list[i].size();
  }

  for (size_t i = 0; i < adj_list.size(); ++i) {
    label_frequency_[GetLabel(i)] += 1;

    auto &neighbors = adj_list[i];

    if (neighbors.size() == 0) continue;

    // sort neighbors by ascending order of label first, and descending order of
    // degree second
    std::sort(neighbors.begin(), neighbors.end(), [this](Vertex u, Vertex v) {
      if (GetLabel(u) != GetLabel(v))
        return GetLabel(u) < GetLabel(v);
      else if (GetDegree(u) != GetDegree(v))
        return GetDegree(u) > GetDegree(v);
      else
        return u < v;
    });

    Vertex v = neighbors[0];
    Label l = GetLabel(v);

    start_offset_by_label_[i * (max_label_ + 1) + l].first = start_offset_[i];

    for (size_t j = 1; j < neighbors.size(); ++j) {
      v = neighbors[j];
      Label next_l = GetLabel(v);

      if (l != next_l) {
        start_offset_by_label_[i * (max_label_ + 1) + l].second =
            start_offset_[i] + j;
        start_offset_by_label_[i * (max_label_ + 1) + next_l].first =
            start_offset_[i] + j;
        l = next_l;
      }
    }

    start_offset_by_label_[i * (max_label_ + 1) + l].second =
        start_offset_[i + 1];

    std::copy(adj_list[i].begin(), adj_list[i].end(),
              adj_array_.begin() + start_offset_[i]);
  }
}

int Graph::findRoot(Graph G) {
    this->root = -1;
    double argmin = DBL_MAX;

    for (int i = 0; i < GetNumVertices(); i++){
        double C_ini_size = 0;
        for (int j = 0; j < G.GetNumVertices() ; j++){
            if (GetLabel(i) == G.GetLabel(j) && GetDegree(i) <= G.GetDegree(j)){
                C_ini_size++;
            }
        }
        if (C_ini_size/GetDegree(i) < argmin){
            argmin = C_ini_size/GetDegree(i);
            this->root = i;
        }
    }
    return this->root;
}

void Graph::buildDAG(Graph G) {
    if( this->childQuery == NULL ) {
        this->childQuery = new int*[num_vertices_];
        for(int i = 0; i < num_vertices_; ++i)
            this->childQuery[i] = NULL;
    }
    if( parentQuery == NULL ) {
        parentQuery = new int*[num_vertices_];
        for(int i = 0; i < num_vertices_; ++i)
            parentQuery[i] = NULL;
    }
    if( numChild == NULL )
        numChild = new int[num_vertices_];
    if( numParent == NULL )
        numParent = new int[num_vertices_];

    for (int i = 0; i < num_vertices_; i++){
        numChild[i] = 0;
        numParent[i] = 0;
    }

    for(int i = 0; i <num_vertices_; ++i) {
        if( childQuery[i] != NULL) {
            delete[] childQuery[i];
            childQuery[i] = NULL;
        }
        childQuery[i] = new int[GetDegree(i)];

        if( parentQuery[i] != NULL ) {
            delete[] parentQuery[i];
            parentQuery[i] = NULL;
        }
        parentQuery[i] = new int[GetDegree(i)];


        for (int j = 0 ; j < GetDegree(i); j++){
            childQuery[i][j] = -1;
            parentQuery[i][j] = -1;
        }
    }

    char* popped = new char[num_vertices_];
    memset(popped, 0, sizeof(char) * num_vertices_);
    char* visited = new char[num_vertices_];
    memset(visited, 0, sizeof(char) * num_vertices_);
    int* queue = new int[num_vertices_];
    int currQueueStart = 0;
    int currQueueEnd = 1;
    int nextQueueEnd = 1;

    root = findRoot(G);
    visited[root] = 1;
    popped[root] = 1;
    queue[0] = root;

    //BFS traversal using queue
    while(true) {
        int* sorted = new int[num_vertices_];
        merge_sort_by_degree(queue, currQueueStart, currQueueEnd-1, G, sorted);
        merge_sort_by_label_frequency(queue, currQueueStart, currQueueEnd-1, G, sorted);

//        std::cout << "sorted queue" << std::endl;
//        for (int i = currQueueStart; i < currQueueEnd; i++){
//            std::cout << queue[i];
//        }
//        std::cout << std::endl;

        while( currQueueStart != currQueueEnd ) {
            int currNode = queue[ currQueueStart ];
            ++currQueueStart;
            popped[currNode] = 1;
//            std::cout << currNode << " ";

            for(int i = GetNeighborStartOffset(currNode); i < GetNeighborEndOffset(currNode); ++i) {
                int childNode = GetNeighbor(i);
                if(popped[childNode] == 0) {
                    childQuery[currNode][ numChild[currNode] ] = childNode;
                    parentQuery[childNode][ numParent[childNode] ] = currNode;

                    ++numChild[currNode];
                    ++numParent[childNode];
                }
                if(visited[childNode] == 0) {
                    visited[childNode] = 1;
                    queue[nextQueueEnd] = childNode;
                    ++nextQueueEnd;
                }
            }
        }

        if(currQueueEnd == nextQueueEnd)
            break;

        currQueueStart = currQueueEnd;
        currQueueEnd = nextQueueEnd;
    }
    std::cout << std::endl;
    delete[] popped;
    delete[] visited;
    delete[] queue;

//    for (int i = 0; i < num_vertices_; i++){
//        std::cout << "vertex " << i << "'s child\n";
//        for (int j = 0; j < GetDegree(i); j++){
//            if (childQuery[i][j] != -1){
//                std::cout << childQuery[i][j] << " ";
//            }
//        }
//        std::cout << std::endl;

//        std::cout << "vertex " << i << "'s parent\n";
//        for (int j = 0; j < GetDegree(i); j++){
//            if (parentQuery[i][j] != -1){
//                std::cout << parentQuery[i][j] << " ";
//            }
//        }
//        std::cout << std::endl;
//    }
}

void Graph::merge_by_degree(int *data, int start, int mid, int end, Graph G, int* sorted){
    int i = start;
    int j = mid+1;
    int k = start;
    while(i <= mid && j <= end) {
        if(GetDegree(i) >=  GetDegree(j)){
            sorted[k] = data[i];
            i++;
        } else{
            // data[i] > data[j]
            sorted[k] = data[j];
            j++;
        }
        k++;
    }
    if(i > mid){
        for(int t = j; t<=end; t++){
            sorted[k] = data[t];
            k++;
        }
    }else{
        for(int t = i; t<=mid; t++){
            sorted[k] = data[t];
            k++;
        }
    }
    for(int t=start; t<=end; t++){
        data[t] = sorted[t];
    }
}

void Graph::merge_sort_by_degree(int *data, int start, int end, Graph G, int* sorted){
    if(start < end){
        int mid = (start+end)/2;
        merge_sort_by_degree(data, start, mid, G, (int* )sorted);
        merge_sort_by_degree(data, mid+1, end, G, (int* )sorted);
        merge_by_degree(data, start, mid, end, G, (int*) sorted);
    }
}

void Graph::merge_by_label_frequency(int *data, int start, int mid, int end, Graph G, int* sorted){
    int i = start;
    int j = mid+1;
    int k = start;
    while(i <= mid && j <= end) {
        if(G.GetLabelFrequency(GetLabel(i))<= G.GetLabelFrequency(GetLabel(j))){
            sorted[k] = data[i];
            i++;
        } else{
            // data[i] > data[j]
            sorted[k] = data[j];
            j++;
        }
        k++;
    }
    if(i > mid){
        for(int t = j; t<=end; t++){
            sorted[k] = data[t];
            k++;
        }
    }else{
        for(int t = i; t<=mid; t++){
            sorted[k] = data[t];
            k++;
        }
    }
    for(int t=start; t<=end; t++){
        data[t] = sorted[t];
    }
}

void Graph::merge_sort_by_label_frequency(int *data, int start, int end, Graph G, int* sorted){
    if(start < end){
        int mid = (start+end)/2;
        merge_sort_by_label_frequency(data, start, mid, G, (int*) sorted);
        merge_sort_by_label_frequency(data, mid+1, end, G, (int* )sorted);
        merge_by_label_frequency(data, start, mid, end, G, (int*) sorted);
    }
}

Graph::~Graph(){};