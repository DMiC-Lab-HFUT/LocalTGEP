//
// Created by JSW on 2021/2/5.
//

#ifndef METIS_EDGE_PARTS_H
#define METIS_EDGE_PARTS_H

#include "../util/util.h"


class edge_parts{
    vid_t v_num{};
    size_t e_num{};
    std::string graph_name,result_name;
    int part_num;
    
    std::vector<vid_t> all_v_part;

    std::vector<std::vector<std::set<vid_t>>> adjlist_all_parts;

public:
    edge_parts(std::string graph_name_para, std::string result_name_para, int part_num_para);

    void read_v_part();

    void partition_edges();

    int judge_edge_part(vid_t v_id, vid_t adj_id);

    double count_RF();
};


#endif //METIS_EDGE_PARTS_H
