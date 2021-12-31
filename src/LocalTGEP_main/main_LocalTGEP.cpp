//
// Created by JSW on 2020/12/6.
//
#include "controller.h"

DECLARE_bool(help);
DECLARE_bool(helpshort);
DEFINE_double(bf, 1.01, "bf");
DEFINE_int32(pno, 10, "number of parts");
DEFINE_string(init_filename, "../graph_data/Email-Eu-Core.txt", "initial filename");
DEFINE_string(dyna_filename, "../graph_data/Email-Eu-Core.txt.inc.txt", "update filename");
DEFINE_string(dyna_type, "inc", "inc/dec/static");
DEFINE_double(pop_ratio, 0, "");
DEFINE_string(filetype, "adjlist", "edgelist/adjlist");
DEFINE_int32(cache_ratio, FLAGS_pno, "cache_ratio");


int main(int argc, char *argv[]) {

    google::InitGoogleLogging(argv[0]);
    FLAGS_log_dir = "../logs/";
    google::SetStderrLogging(google::GLOG_INFO);
    FLAGS_max_log_size = 5000;


    std::string usage = "-filename"
                        "[-filetype <edgelist|adjlist>] "
                        "[-p] ";
    google::SetUsageMessage(usage);
    google::ParseCommandLineNonHelpFlags(&argc, &argv, true);
    if (FLAGS_help) {
        FLAGS_help = false;
        FLAGS_helpshort = true;
    }
    google::HandleCommandLineHelpFlags();


    controller co;
    co.graph_part();
    co.write_results();

    google::ShutdownGoogleLogging();
}