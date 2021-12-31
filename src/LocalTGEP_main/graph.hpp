#ifndef DLP_GRAPH_HPP
#define DLP_GRAPH_HPP

#include "../util/util.h"


class Graph {
public:

    std::string filename;

    vid_t num_vertices;

    size_t num_edges;

    std::vector<vid_t> degrees_max;

    std::vector<vid_t> degrees_now;

    std::vector<size_t> offsets;

    std::fstream fout_g;

    vid_t head_index = 0;

    Graph(std::string filename);

    ~Graph();

    void write_neis(vid_t v, const std::vector<vid_t>& neis);

    std::vector<vid_t> read_neis(vid_t v);


    size_t get_file_size();


    void create_file();
};

#endif