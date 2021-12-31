//
// Created by JSW on 2020/12/5.
//
#ifndef DLP_UTIL_H
#define DLP_UTIL_H

#include <igraph.h>
#include <iostream>
#include <cstdio>
#include <unistd.h>
#include <cstring>
#include <sstream>
#include <chrono>
#include <vector>
#include <algorithm>
#include <fstream>
#include <sys/stat.h>
#include <cmath>
#include <set>
#include <map>
#include <omp.h>
#include <unordered_map>
#include <unordered_set>
#include <glog/logging.h>
#include <gflags/gflags.h>

DECLARE_double(bf);
DECLARE_int32(pno);
DECLARE_string(init_filename);
DECLARE_string(dyna_filename);
DECLARE_string(dyna_type);
DECLARE_double(pop_ratio);
DECLARE_string(filetype);
DECLARE_int32(cache_ratio);


typedef uint32_t vid_t;




void create_empty_file(std::string file_name);

void uniform_graph_file(char *filename);

void read_graph_edge(char *filename, igraph_t *graph);

void write_graph_edge(char *filename, igraph_t *graph);

std::string write_graph_adj(const std::string &edge_filename, igraph_t *graph);

void simplify_graph(igraph_t *graph);

int get_v_degree(igraph_t *graph, int v_id);


class Timer {
private:
    std::chrono::system_clock::time_point t1, t2;
    double total;

public:
    Timer() : total(0) {}

    void reset() { total = 0; }

    void start() { t1 = std::chrono::system_clock::now(); }

    void stop() {
        t2 = std::chrono::system_clock::now();
        std::chrono::duration<double> diff = t2 - t1;
        total += diff.count();
    }

    double get_time() { return total; }
};


void del_1Dvec_item(vid_t item, std::vector<vid_t> *vec);



void delete_isolated_v(igraph_t *graph);



void print_igraph_vector(igraph_vector_t *i_vector);




inline std::string binedgelist_name(std::string &basefilename) {
    
    std::stringstream ss;
    
    ss << basefilename << ".binedgelist";
    
    return ss.str();
}




inline std::string degree_name(const std::string &basefilename) {
    std::stringstream ss;
    ss << basefilename << ".degree";
    return ss.str();
}




inline bool is_exists(const std::string &name) {
    
    struct stat buffer;
    return (stat(name.c_str(), &buffer) == 0);
}



void fix_line(char *s);



std::string convert_edgelist_to_adjlist(const std::string &edge_filename);





void process_bar(size_t done_num, size_t all_num, const std::string& tag);

#endif //DLP_UTIL_H