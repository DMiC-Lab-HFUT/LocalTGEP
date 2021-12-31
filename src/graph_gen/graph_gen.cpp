//
// Created by JSW on 2021/1/20.
//

#include "../util/util.h"

DECLARE_bool(help);
DECLARE_bool(helpshort);
DEFINE_string(filename, "", "");
DEFINE_string(filetype, "edgelist","");
DEFINE_int64(n, 100, "");
DEFINE_int64(m, 10000,"");

int main(int argc, char *argv[]) {
    google::SetStderrLogging(google::GLOG_INFO);
    FLAGS_max_log_size = 5000;
    google::InitGoogleLogging(argv[0]);
    FLAGS_log_dir = "../logs/";
    google::SetStderrLogging(google::GLOG_INFO);
    FLAGS_max_log_size = 5000;


    std::string usage = "-filename "
                        "[-filetype <edgelist|adjlist>] "
                        "[-n ] "
                        "[-m ] ";
    google::SetUsageMessage(usage);
    google::ParseCommandLineNonHelpFlags(&argc, &argv, true);
    if (FLAGS_help) {
        FLAGS_help = false;
        FLAGS_helpshort = true;
    }
    google::HandleCommandLineHelpFlags();

    igraph_t graph;


    igraph_erdos_renyi_game(&graph, IGRAPH_ERDOS_RENYI_GNM, FLAGS_n, FLAGS_m, IGRAPH_UNDIRECTED, IGRAPH_NO_LOOPS);
    simplify_graph(&graph);


    std::stringstream ss;
    ss<<FLAGS_init_filename<<"n"<<FLAGS_n<<"m"<<FLAGS_m<<".txt";
    ss>>FLAGS_init_filename;

    if (FLAGS_filetype=="edgelist"){
        LOG(INFO) << FLAGS_init_filename << std::endl;
        char *file_path = (char *) FLAGS_init_filename.data();
        write_graph_edge(file_path, &graph);
    } else{
        write_graph_adj(FLAGS_init_filename, &graph);
    }

    int v_num = igraph_vcount(&graph);

    int e_num = igraph_ecount(&graph);

    google::ShutdownGoogleLogging();
}
