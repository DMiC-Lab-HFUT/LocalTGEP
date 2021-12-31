//
// Created by JSW on 2021/11/28.
//

#include "global_graph.h"



globalGraph::globalGraph(std::string filename): Graph(filename) {

    isend= false;

    convert_adj_file();

    cache_id2index.resize(num_vertices, num_vertices);

    cache_graph.reserve(ceil((double) num_vertices/FLAGS_cache_ratio));
    cache_index2id.reserve(ceil((double) num_vertices/FLAGS_cache_ratio));
}



void globalGraph::convert_adj_file() {

    if (FLAGS_filetype=="edgelist"){
        filename= convert_edgelist_to_adjlist(filename);
    }

    FILE *inf = fopen(filename.c_str(), "r");
    if (inf == nullptr) {
        LOG(FATAL) << "error：" << filename<< std::endl;
    }
    LOG(WARNING) << "start：" << filename << std::endl;
    int maxlen = 1000000000;
    char *buff = (char *) malloc(maxlen);
    size_t bytesread = 0;
    size_t lastlog = 0;

    vid_t linenum = 0;

    size_t offset_now=0;
    while (fgets(buff, maxlen, inf) != nullptr) {
        fix_line(buff);
        if (buff[0] == '#')
            continue;
        if (buff[0] == '%')
            continue;
        if (buff[0] == '/')
            continue;

        add_adj_line(buff, linenum++, &offset_now);

        if (bytesread - lastlog >= 500000000) {
            LOG(INFO) << "read" << linenum << "lines，"
                      << (double) bytesread / 1024 / 1024. << " MB" << std::endl;
            lastlog = bytesread;
        }
        bytesread += strlen(buff);
    }
    free(buff);
    fclose(inf);
    LOG(WARNING)<<"over, "<<"edges "<<num_edges<<", vertices"<<num_vertices<<std::endl;
}





void globalGraph::add_adj_line(char *buffer, vid_t linenum, size_t *offset_now){

    char delims[] = " \t,";

    char *t;

    if (linenum==0){

        t = strtok(buffer, delims);
        num_vertices=atoi(t);

        degrees_max.reserve(num_vertices);
        degrees_now.reserve(num_vertices);
        offsets.reserve(num_vertices);

        t = strtok(nullptr, delims);
        num_edges=atoi(t);


        fout_g.write((char *) &num_vertices, sizeof(vid_t));
        (*offset_now)=(*offset_now) + sizeof(vid_t);
        fout_g.write((char *) &num_edges, sizeof(size_t));
        (*offset_now)=(*offset_now) + sizeof(size_t);
    } else{

        vid_t from=linenum-1;

        offsets.emplace_back(*offset_now);

        t = strtok(buffer, delims);
        vid_t degree_now = atoi(t);

        degrees_max.emplace_back(degree_now);
        degrees_now.emplace_back(degree_now);

        (*offset_now)=(*offset_now)+degree_now* sizeof(vid_t);

        if (degree_now > 0){

            unisolate_v.insert(from);

            if (t != nullptr) {

                vid_t i = 0;
                while ((t = strtok(nullptr, delims)) != nullptr) {

                    vid_t to = atoi(t);

                    if (from != to) {

                        fout_g.write((char *) &to, sizeof(vid_t));
                    } else{
                        LOG(FATAL)<<"line num: "<<linenum;
                    }
                    i++;
                }
                LOG_IF(FATAL, degree_now!=i)<<"error";
            }
        }
    }
}





void globalGraph::cache_add(vid_t v, std::vector<vid_t> neis){

    CHECK_EQ(cache_id2index[v], num_vertices);
    CHECK_EQ(cache_graph.size(), cache_index2id.size());
    CHECK_EQ(cache_graph.capacity(), ceil((double) num_vertices/FLAGS_cache_ratio));


    if (cache_graph.size() == cache_graph.capacity()){


        vid_t head_v = cache_index2id[head_index];

        write_neis(head_v, cache_graph[head_index]);

        cache_id2index[head_v] = num_vertices;


        cache_graph[head_index] = neis;
        cache_index2id[head_index] = v;
        cache_id2index[v] = head_index;



        if (head_index == cache_graph.capacity() - 1){

            head_index = 0;
        }else{
            head_index++;
        }
    }

    else{

        cache_graph.emplace_back(neis);
        cache_index2id.emplace_back(v);

        cache_id2index[v] = cache_graph.size()-1;
    }

    CHECK_LT(cache_id2index[v], num_vertices);
}





std::vector<vid_t> * globalGraph::cache_get(vid_t v){

    CHECK_GT(degrees_now[v], 0);

    if (cache_id2index[v] == num_vertices){

        cache_add(v, read_neis(v));
    }
    CHECK_EQ(degrees_now[v], cache_graph[cache_id2index[v]].size());
    return &(cache_graph[cache_id2index[v]]);
}




void globalGraph::cache_del(vid_t v){

    CHECK_LT(cache_id2index[v], num_vertices);
    CHECK_EQ(cache_graph.size(), cache_index2id.size());

    CHECK_EQ(degrees_now[v], 0);


    cache_graph[cache_id2index[v]] = cache_graph.back();
    cache_index2id[cache_id2index[v]] = cache_index2id.back();
    cache_id2index[cache_index2id.back()] = cache_id2index[v];


    cache_graph.pop_back();
    cache_index2id.pop_back();
    cache_id2index[v] = num_vertices;
}





void globalGraph::del_cache_edge(vid_t vfrom, vid_t vto){





    del_cache_vto(vfrom, vto);
    del_cache_vto(vto, vfrom);
}





void globalGraph::del_cache_vto(vid_t vfrom, vid_t vto){

    del_1Dvec_item(vto, cache_get(vfrom));
    degrees_now[vfrom]--;

    CHECK_EQ(cache_graph[cache_id2index[vfrom]].size(), degrees_now[vfrom]);

    if (degrees_now[vfrom] == 0){
        cache_del(vfrom);

        unisolate_v.erase(vfrom);

        if (unisolate_v.empty()){

            isend = true;
        }
    }
}





void globalGraph::add_cache_edge(vid_t vfrom, vid_t vto){

    add_vto(vfrom, vto);
    add_vto(vto, vfrom);
}





void globalGraph::add_vto(vid_t vfrom, vid_t vto){

    if (degrees_now[vfrom] == 0){

        CHECK_EQ(cache_id2index[vfrom], num_vertices);

        cache_add(vfrom, {vto});
        degrees_now[vfrom]++;
        unisolate_v.insert(vfrom);
    }

    else{

        cache_get(vfrom)->emplace_back(vto);
        degrees_now[vfrom]++;

        CHECK_EQ(cache_graph[cache_id2index[vfrom]].size(), degrees_now[vfrom]);
    }
}



void globalGraph::del_mem(){

    CHECK_EQ(cache_index2id.size(), cache_graph.size());

    for (int k = 0; k < cache_index2id.size(); ++k) {

        CHECK_GT(cache_graph[k].size(), 0);

        CHECK_EQ(cache_id2index[cache_index2id[k]], k);

        write_neis(cache_index2id[k], cache_graph[k]);
    }


    cache_graph.clear();
    cache_index2id.clear();
    cache_id2index.clear();
    cache_id2index.resize(num_vertices, num_vertices);
}




void globalGraph::inc_graph(std::string p2_file_name) {

    vid_t inc_v_num;
    size_t inc_e_num;

    FILE *inf = fopen(p2_file_name.c_str(), "r");
    LOG(WARNING) << "start: " << p2_file_name << std::endl;
    int maxlen = 1000000000;
    char *buff = (char *) malloc(maxlen);

    vid_t linenum = 0;

    while (fgets(buff, maxlen, inf) != nullptr) {

        char delims[] = " \t,";

        char *t;

        if (linenum == 0){

            t = strtok(buff, delims);
            inc_v_num = atoi(t);
            CHECK_EQ(inc_v_num, num_vertices);

            t = strtok(nullptr, delims);
            inc_e_num = atoi(t);

            num_edges = num_edges + inc_e_num;
        } else{

            vid_t from = linenum-1;

            t = strtok(buff, delims);
            vid_t degree = atoi(t);

            if (degree > 0){

                unisolate_v.insert(from);
                std::vector<vid_t> adjs;
                adjs.reserve(degree);

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
                    LOG_IF(FATAL, degree!=i)<<"error";
                }

                write_neis(from, adjs);
                degrees_now[from] = degree;
            }
        }
        linenum++;
    }
    free(buff);
    fclose(inf);
}




void globalGraph::dec_graph(std::string file_name) {

}
