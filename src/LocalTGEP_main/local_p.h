//
// Created by JSW on 2021/11/29.
//

#ifndef DLP_LOCAL_P_H
#define DLP_LOCAL_P_H

#include "graph.hpp"
#include "global_graph.h"
#include "../util/min_heap.hpp"

class localPar : public Graph{
public:

    globalGraph *graph;

    size_t max_num_e;

    int p_id;



    std::vector<vid_t> v_flag;

    std::vector<std::vector<vid_t>> cache_p;

    std::vector<vid_t> cache_index2id;

    std::vector<vid_t> cache_id2index;


    MinHeap <size_t, vid_t> *B_out = new MinHeap <size_t, vid_t>;

    MinHeap <size_t, vid_t> *B_in = new MinHeap <size_t, vid_t>;
    vid_t temp_flag = 20;

    localPar(std::string filename, size_t *max_part_e_num, int *id, globalGraph *graph);

    void cache_add(vid_t v, std::vector<vid_t> neis);
    std::vector<vid_t> * cache_get(vid_t v);
    void cache_del(vid_t v);
    void del_cache_edge(vid_t vfrom, vid_t vto);
    void del_cache_vto(vid_t vfrom, vid_t vto);
    void add_cache_edge(vid_t vfrom, vid_t vto);
    void add_vto(vid_t vfrom, vid_t vto);
    void del_mem();


    void split();

    bool is_full() const;

    vid_t vbest_in_B();

    vid_t free_N_to_B();

    void N_to_B(vid_t v);

    void B_to_C(vid_t v);

    vid_t vworst_in_B();

    void B_to_N(vid_t v);

    void C_to_B(vid_t v);

    void write_degree();

    void write_degree_max();

    void write_offset();

    void self_check();

    void reload_meta();

    void reload_degree();

    void reload_degree_max();

    void reload_offset();

    void reload_e_v_num();

    void reload_Bin();

    void reload_Bout();

    void reload_pop();

    void reload_push();

    void pop(double pop_ratio);

    void push();

    void dec_data(std::vector<std::vector<vid_t>> *dec_data);
};


#endif
