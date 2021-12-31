//
// Created by JSW on 2021/2/5.
//

#include "edge_parts.h"

edge_parts::edge_parts(std::string graph_name_para, std::string result_name_para, int part_num_para) {
    graph_name=std::move(graph_name_para);
    result_name=std::move(result_name_para);
    part_num=part_num_para;

    std::ifstream graph_file;
    graph_file.open(graph_name);
    graph_file>>v_num;
    graph_file>>e_num;
    graph_file.close();

    all_v_part.reserve(v_num);
    adjlist_all_parts.reserve(part_num);
    for (int k = 0; k < part_num; ++k) {
        std::vector<std::set<vid_t>> adjlist_one_part;
        adjlist_one_part.resize(v_num);
        adjlist_all_parts.emplace_back(adjlist_one_part);
    }
    read_v_part();
}

void edge_parts::read_v_part() {
    std::ifstream result_file;
    result_file.open(result_name);
    for (int k = 0; k < v_num; ++k) {
        int part;
        result_file>>part;
        all_v_part.emplace_back(part);
    }
    result_file.close();
}


void edge_parts::partition_edges() {

    std::ifstream graph_file;
    graph_file.open(graph_name);
    std::string line;
    getline(graph_file, line);
    vid_t v_id=0;
    while (getline(graph_file,line)){
        std::stringstream ss(line);
        vid_t adj_id;
        while (ss>>adj_id && v_id>=1){
            int part = judge_edge_part(v_id, adj_id);
            adjlist_all_parts[part][v_id].insert(adj_id);
            adjlist_all_parts[part][adj_id].insert(v_id);
        }
        v_id++;
    }
    graph_file.close();
}

int edge_parts::judge_edge_part(vid_t v_id, vid_t adj_id) {
    if (all_v_part[v_id]==all_v_part[adj_id]){
        return all_v_part[v_id];
    }
    int v_adjnum=adjlist_all_parts[all_v_part[v_id]][v_id].size();
    int adj_adjnum=adjlist_all_parts[all_v_part[adj_id]][adj_id].size();
    return all_v_part[(v_adjnum >= adj_adjnum ? v_id : adj_id)];
}

double edge_parts::count_RF() {
    vid_t replicate_num=0;
    for (const auto& one_part : adjlist_all_parts){
        for (const auto& adj : one_part){
            if (!adj.empty()){
                replicate_num++;
            }
        }
    }
    return (double) replicate_num/v_num;
}

