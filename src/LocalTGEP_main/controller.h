//
// Created by JSW on 2021/11/17.
//

#ifndef DLP_CONTROLLER_H
#define DLP_CONTROLLER_H

#include "global_graph.h"
#include "local_p.h"
#include "../util/memory_monitor.h"

class controller {
public:

    globalGraph *graph;

    Timer time_all, time_p;

    int ID_par;

    size_t max_part_e_num;

    size_t e_parted;
    vid_t v_parted;

    controller();

    ~controller();


    void graph_part();


    void write_results();

    void check_degree();

    void init_part();

    void pop_part();

    void push_part();

    void dec_graph(std::string p2_file_name);

    void dec_parts(std::vector<std::vector<vid_t>> *dec_data);
};


#endif //DLP_CONTROLLER_H
