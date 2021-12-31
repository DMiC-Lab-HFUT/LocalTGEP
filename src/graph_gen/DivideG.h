//
// Created by JSW on 2021/12/6.
//

#ifndef DLP_DIVIDEG_H
#define DLP_DIVIDEG_H

#include "../util/util.h"
#include <fstream>

class DivideG {
public:

    std::string filename;

    std::string p1_filename;
    std::string p2_filename;

    std::string type;

    double ratio;

    double p1_max_e_num;

    std::ifstream graph_file;

    std::ofstream p1_file;
    std::ofstream p2_file;

    vid_t g_v_num;
    size_t g_e_num;
    std::vector<std::vector<vid_t>> graph;
    size_t p1_e_num;
    std::vector<std::vector<vid_t>> p1;
    size_t p2_e_num;
    std::vector<std::vector<vid_t>> p2;


    DivideG(std::string filename, std::string type, double ratio);

    ~DivideG();

    void load_graph();

    void load_line(char *buff, vid_t line_num);

    void divide_g();

    void self_check();

    void write();
};


#endif //DLP_DIVIDEG_H
