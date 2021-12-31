//
// Created by JSW on 2021/11/28.
//

#ifndef DLP_GLOBAL_GRAPH_H
#define DLP_GLOBAL_GRAPH_H

#include "graph.hpp"

class globalGraph : public Graph{
public:

    std::unordered_set<vid_t> unisolate_v;

    bool isend;

    std::vector<std::vector<vid_t>> cache_graph;

    std::vector<vid_t> cache_index2id;

    std::vector<vid_t> cache_id2index;

    globalGraph(std::string filename);

    void convert_adj_file();

    void add_adj_line(char *buffer, vid_t linenum, size_t *offset_now);

    std::vector<vid_t> * cache_get(vid_t v);

    void cache_add(vid_t v, std::vector<vid_t> neis);

    void cache_del(vid_t v);

    void del_cache_edge(vid_t vfrom, vid_t vto);

    void del_cache_vto(vid_t vfrom, vid_t vto);

    void add_cache_edge(vid_t vfrom, vid_t vto);

    void add_vto(vid_t vfrom, vid_t vto);

    void del_mem();

    void inc_graph(std::string file_name);

    void dec_graph(std::string file_name);
};


#endif
