//
// Created by JSW on 2021/11/17.
//

#include "controller.h"



controller::controller(){
    this->graph = new globalGraph(FLAGS_init_filename);

    time_all.start();

    ID_par = 0;

    max_part_e_num = ceil((FLAGS_bf) * ((double) graph->num_edges / (FLAGS_pno)));

    e_parted = 0;
    v_parted = 0;
}



void controller::graph_part() {

    time_p.start();
    LOG(WARNING) << "LocalTGEP start\t" << FLAGS_init_filename << "\t" << FLAGS_pno;

    init_part();
    time_p.stop();

    if (FLAGS_dyna_type == "inc"){

        graph->inc_graph(FLAGS_dyna_filename);
    }

    if (FLAGS_dyna_type == "dec"){

        dec_graph(FLAGS_dyna_filename);
    }
    time_p.start();

    if (FLAGS_dyna_type != "static"){
        pop_part();
        push_part();
    }
    time_p.stop();
    time_all.stop();


}



void controller::write_results(){

    std::ofstream result_file;
    std::stringstream ss;
    ss << "../results/DLP-" << FLAGS_pno << "-";

    std::string mem_name = ss.str() + "mem-result.txt";
    result_file.open(mem_name, std::ios::app);
    double peakSize = (double) getPeakRSS() / (1024 * 1024);
    LOG(WARNING) << "peakSize: " << peakSize << std::endl;
    result_file << peakSize << ",";
    result_file.close();

    std::string RF_name = ss.str() + "RF-result.txt";
    result_file.open(RF_name, std::ios::app);
    double RF = (double) v_parted / graph->num_vertices;
    LOG(WARNING) << "RF：" << RF << std::endl;
    result_file << RF << ",";
    result_file.close();

    std::string time_name = ss.str() + "time-result.txt";
    result_file.open(time_name, std::ios::app);
    LOG(WARNING) << "time: " << time_p.get_time() << std::endl;
    result_file << time_p.get_time() << ",";
    result_file.close();
}


void controller::check_degree(){
    std::vector<vid_t> degree_check;
    degree_check.resize(graph->num_vertices);

    for (int p = 0; p < FLAGS_pno; ++p) {
        std::string deg_filename = FLAGS_init_filename + "-p-" + std::to_string(p) + "-degree";
        std::fstream fout_d;
        fout_d.open(binedgelist_name(deg_filename), std::ios::out|std::ios::in|std::ios::binary);

        vid_t deg;

        vid_t v = 0;

        while (fout_d.read((char *) &deg, sizeof(vid_t)) && v < graph->num_vertices){
            degree_check[v] = degree_check[v] + deg;
            v++;
        }
        fout_d.close();
    }
    for (int v = 0; v < graph->num_vertices; ++v) {
        CHECK_EQ(degree_check[v], graph->degrees_max[v]);
    }
}


controller::~controller(){
    delete graph;
}


void controller::init_part() {

    while (!graph->isend) {

        std::string filename_par = FLAGS_init_filename + "-p-" + std::to_string(ID_par);
        localPar* one_part = new localPar(filename_par, &max_part_e_num, &ID_par, graph);

        one_part->split();
        ID_par = ID_par + 1;
        e_parted = e_parted + one_part->num_edges;
        v_parted = v_parted + one_part->num_vertices;

        delete one_part;
    }
    LOG(WARNING) << "stage 1 over\t" << "edge num: " << e_parted << "\tvertex num" << v_parted << std::endl;
}


void controller::pop_part() {
    v_parted = 0;
    e_parted = 0;

    max_part_e_num = ceil((FLAGS_bf) * ((double) graph->num_edges / (FLAGS_pno)));
    for (int p = 0; p < FLAGS_pno; ++p) {

        std::string filename_par = FLAGS_init_filename + "-p-" + std::to_string(p);
        localPar* one_part = new localPar(filename_par, &max_part_e_num, &p, graph);
        one_part->pop(FLAGS_pop_ratio);
        v_parted = v_parted + one_part->num_vertices;
        e_parted = e_parted + one_part->num_edges;
        delete one_part;
    }
    graph->isend = false;
    LOG(WARNING) << "stage 2 over\t" << "edge num: " << e_parted << "\tvertex num" << v_parted << std::endl;
}


void controller::push_part() {
    v_parted = 0;
    e_parted = 0;

    max_part_e_num = ceil((FLAGS_bf) * ((double) graph->num_edges / (FLAGS_pno)));
    for (int p = 0; p < FLAGS_pno; ++p) {

        std::string filename_par = FLAGS_init_filename + "-p-" + std::to_string(p);
        localPar* one_part = new localPar(filename_par, &max_part_e_num, &p, graph);
        one_part->push();
        v_parted = v_parted + one_part->num_vertices;
        e_parted = e_parted + one_part->num_edges;
        delete one_part;
    }
    LOG(WARNING) << "stage 3 over\t" << "edge num: " << e_parted << "\tvertex num" << v_parted << std::endl;
}



void controller::dec_graph(std::string p2_file_name) {

    std::vector<std::vector<vid_t>> dec_g;
    dec_g.reserve(graph->num_vertices);

    vid_t dec_v_num;
    size_t dec_e_num;

    FILE *inf = fopen(p2_file_name.c_str(), "r");
    int maxlen = 1000000000;
    char *buff = (char *) malloc(maxlen);

    vid_t linenum = 0;

    while (fgets(buff, maxlen, inf) != nullptr) {

        char delims[] = " \t,";

        char *t;

        if (linenum == 0){

            t = strtok(buff, delims);
            dec_v_num = atoi(t);
            CHECK_EQ(dec_v_num, graph->num_vertices);

            t = strtok(nullptr, delims);
            dec_e_num = atoi(t);

            graph->num_edges = graph->num_edges - dec_e_num;
        } else{

            vid_t from = linenum-1;

            t = strtok(buff, delims);
            vid_t degree = atoi(t);

            std::vector<vid_t> adjs;
            adjs.reserve(degree);

            if (degree > 0){

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
            }
            dec_g.emplace_back(adjs);
        }
        linenum++;
    }
    free(buff);
    fclose(inf);
    dec_parts(&dec_g);
}



void controller::dec_parts(std::vector<std::vector<vid_t>> *dec_data) {
    size_t e_num_tmp = 0;
    for (int p = 0; p < FLAGS_pno; ++p) {

        std::string filename_par = FLAGS_init_filename + "-p-" + std::to_string(p);
        localPar* one_part = new localPar(filename_par, &max_part_e_num, &p, graph);

        one_part->dec_data(dec_data);
        e_num_tmp = e_num_tmp + one_part->num_edges;
        delete one_part;
    }
    CHECK_EQ(e_num_tmp, graph->num_edges);
}
