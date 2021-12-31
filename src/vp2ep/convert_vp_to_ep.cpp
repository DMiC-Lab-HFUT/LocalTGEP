//
// Created by JSW on 2021/2/5.
//

#include <utility>

#include "edge_parts.h"

DEFINE_string(filetype, "adjlist", "图文件类型（值为：'edgelist'或'adjlist'）");


void convert(int p_num, std::string graph_filename, std::string vp_filename){

    if (FLAGS_filetype=="edgelist"){
        graph_filename= convert_edgelist_to_adjlist(graph_filename);
    }
    edge_parts parts(std::move(graph_filename),std::move(vp_filename),p_num);
    parts.partition_edges();
    double RF = parts.count_RF();
    LOG(WARNING)<<"RF is "<<RF;
}


int main(int argc, char *argv[]) {


    google::InitGoogleLogging(argv[0]);
    FLAGS_log_dir = "../logs/";
    google::SetStderrLogging(google::GLOG_INFO);
    FLAGS_max_log_size = 5000;

    std::string usage = "-filename <图文件路径> "
                        "[-filetype <edgelist|adjlist>] "
                        "[-p <分区数目>] ";
    google::SetUsageMessage(usage);
    google::ParseCommandLineNonHelpFlags(&argc, &argv, true);
    google::HandleCommandLineHelpFlags();

    int p_num=20;
    std::string graph_filename="/mnt/e/jsw/4-paper/5-code/graph_data/TIST_graph_data/Indian-Village.txt.adjlist.txt";
    std::string vp_filename="/mnt/e/jsw/4-paper/5-code/TIST实验结果/GWL-part-result/India_20.txt";
    convert(p_num, graph_filename, vp_filename);
}