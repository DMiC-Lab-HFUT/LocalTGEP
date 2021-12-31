//
// Created by JSW on 2021/12/6.
//

#include "DivideG.h"


DivideG::DivideG(std::string filename, std::string type, double ratio) {
    this->filename = filename;
    this->type = type;
    this->ratio = ratio;

    std::stringstream ss1;
    ss1 << filename << ".p1.txt";
    p1_filename = ss1.str();
    std::stringstream ss2;
    ss2 << filename << ".p2.txt";
    p2_filename = ss2.str();

    graph_file.open(filename);
    p1_file.open(p1_filename);
    p2_file.open(p2_filename);
    p1_e_num = 0;
    p2_e_num = 0;
}


void DivideG::load_graph() {
    int maxlen = 1000000000;
    char *buff = (char *) malloc(maxlen);
    vid_t line_num = 0;
    while (graph_file.getline(buff, maxlen)) {
        fix_line(buff);
        load_line(buff, line_num);
        line_num++;
    }
    free(buff);
}



void DivideG::load_line(char *buff, vid_t line_num) {

    char delims[] = " \t,";

    char *t;

    if (line_num == 0){

        t = strtok(buff, delims);
        g_v_num=atoi(t);
        graph.reserve(g_v_num);
        p1.reserve(g_v_num);
        p2.reserve(g_v_num);

        t = strtok(nullptr, delims);
        g_e_num=atoi(t);
    } else{

        vid_t from=line_num-1;

        t = strtok(buff, delims);
        vid_t degree_now = atoi(t);

        std::vector<vid_t> adjs;
        adjs.reserve(degree_now);

        if (t != nullptr) {

            vid_t i = 0;
            while ((t = strtok(nullptr, delims)) != nullptr) {

                vid_t to = atoi(t);

                if (from != to) {

                    adjs.emplace_back(to);
                } else{
                    LOG(FATAL)<<"error";
                }
                i++;
            }

        }

        graph.emplace_back(adjs);
    }
}



void DivideG::divide_g() {

    load_graph();
    p1_max_e_num = ceil((double) g_e_num * ratio);
    p1.reserve(g_v_num);
    p2.resize(g_v_num);


    p1 = graph;
    p1_e_num = g_e_num;


    vid_t v = 0;

    while (p1_e_num > p1_max_e_num){

        p2[v] = graph[v];

        p2_e_num = p2_e_num + p1[v].size();
        p1_e_num = p1_e_num - p1[v].size();

        p1[v].clear();

        for (vid_t adj : p2[v]) {

            auto iter1 = std::find(p1[adj].begin(), p1[adj].end(), v);

            if (iter1 != p1[adj].end()) {
                del_1Dvec_item(v, &(p1[adj]));
            }
            auto iter2 = std::find(p2[adj].begin(), p2[adj].end(), v);

            if (iter2 == p2[adj].end()){

                p2[adj].emplace_back(v);
            }
        }
        v++;
    }
    self_check();
    write();
}


void DivideG::self_check() {
    CHECK_EQ(p1.size(), g_v_num);
    CHECK_EQ(p2.size(), g_v_num);
    CHECK_EQ(p1_e_num + p2_e_num, g_e_num);
    size_t p1_tmp = 0;
    size_t p2_tmp = 0;
    for (int v = 0; v < g_v_num; ++v) {
        CHECK_EQ(p1[v].size() + p2[v].size(), graph[v].size());
        p1_tmp = p1_tmp + p1[v].size();
        p2_tmp = p2_tmp + p2[v].size();
    }
    CHECK_EQ(p1_tmp/2, p1_e_num);
    CHECK_EQ(p2_tmp/2, p2_e_num);
}



void DivideG::write() {


    p1_file << g_v_num << "\t" << p1_e_num << std::endl;

    for (int v = 0; v < g_v_num; ++v) {

        p1_file << p1[v].size();

        if (p1[v].size() > 0){

            for (vid_t adj: p1[v]) {
                p1_file << "\t" << adj;
            }
        }
        p1_file << std::endl;
    }



    p2_file << g_v_num << "\t" << p2_e_num << std::endl;

    for (int v = 0; v < g_v_num; ++v) {

        p2_file << p2[v].size();

        if (p2[v].size() > 0){

            for (vid_t adj: p2[v]) {
                p2_file << "\t" << adj;
            }
        }
        p2_file << std::endl;
    }
}

DivideG::~DivideG() {
    graph_file.close();
    p1_file.close();
    p2_file.close();
}
