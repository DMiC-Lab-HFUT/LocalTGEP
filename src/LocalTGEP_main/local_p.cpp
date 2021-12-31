//
// Created by JSW on 2021/11/29.
//

#include "local_p.h"



localPar::localPar(std::string filename, size_t *max_part_e_num, int *id, globalGraph *graph)
: Graph(filename) {
    this->graph = graph;
    max_num_e = *max_part_e_num;
    p_id = *id;

    v_flag.resize(graph->num_vertices);

    cache_id2index.resize(graph->num_vertices, graph->num_vertices);

    cache_p.reserve(ceil((double) graph->num_vertices/FLAGS_cache_ratio));
    cache_index2id.reserve(ceil((double) graph->num_vertices/FLAGS_cache_ratio));

    B_out->reserve(graph->num_vertices);
    B_in->reserve(graph->num_vertices);



    offsets = graph->offsets;
    degrees_max = graph->degrees_max;

    degrees_now.resize(graph->num_vertices);
}


void localPar::split() {

    while (!is_full() && !graph->isend) {

        vid_t v_best = vbest_in_B();

        if (graph->isend) {
            break;
        }

        B_to_C(v_best);

        if (num_vertices > temp_flag && B_in->size() > 0) {

            vid_t v_worst = vworst_in_B();

            B_to_N(v_worst);
            temp_flag = 1.2*num_vertices;
        }
    }

    for (int k = 0; is_full(); ++k) {

        if (B_in->size() <= 0){
            break;
        }

        vid_t v_worst = vworst_in_B();

        B_to_N(v_worst);
    }

    graph->del_mem();
    del_mem();
    LOG(WARNING) << "p: " << p_id << "over. Edge num: " << num_edges << ".Vertex num: " << num_vertices << ".";
}




bool localPar::is_full() const {
    if (num_edges >= max_num_e) {
        return true;
    } else {
        return false;
    }
}



vid_t localPar::vbest_in_B() {

    CHECK_EQ(B_in->size(), B_out->size());
    vid_t v_best;
    size_t best_score;

    if (B_out->size() == 0) {

        v_best = free_N_to_B();
    } else {

        B_out->get_min(best_score, v_best);
    }
    return v_best;
}




vid_t localPar::free_N_to_B() {

    for (vid_t v: graph->unisolate_v) {

        CHECK_GT(graph->degrees_now[v], 0);
        if (v_flag[v] == 0) {

            CHECK_EQ(degrees_now[v], 0);
            N_to_B(v);
            return v;
        }
    }

    graph->isend = true;
    return 0;
}



void localPar::N_to_B(vid_t v) {
    CHECK_EQ(v_flag[v], 0);

    CHECK_EQ(cache_id2index[v], graph->num_vertices);
    CHECK_EQ(degrees_now[v], 0);
    CHECK_EQ(B_in->contains(v), false);
    CHECK_EQ(B_out->contains(v), false);

    v_flag[v] = 1;
    num_vertices++;

    std::vector<vid_t> v_neis = *graph->cache_get(v);
    for (vid_t adj_v: v_neis) {

        if (v_flag[adj_v] > 0) {

            add_cache_edge(v, adj_v);

            if (v_flag[adj_v] == 1) {

                B_out->decrease_value(adj_v);

                B_in->increase_value(adj_v);
            }
        }
    }

    B_out->insert(graph->degrees_now[v], v);
    B_in->insert(degrees_now[v], v);
}



void localPar::B_to_C(vid_t v) {



    B_out->remove(v);
    B_in->remove(v);
    CHECK_EQ(B_out->size(), B_in->size());
    v_flag[v] = 2;


    if (graph->degrees_now[v]>0){


        std::vector<vid_t> temp = *graph->cache_get(v);
        for (vid_t adj_out: temp) {

            if (v_flag[adj_out] == 1){

                add_cache_edge(v, adj_out);

                B_out->decrease_value(adj_out);

                B_in->increase_value(adj_out);
                continue;
            }

            CHECK_NE(v_flag[adj_out], 2);

            N_to_B(adj_out);
        }
    }
}



vid_t localPar::vworst_in_B() {
    vid_t v_worst;
    size_t worst_score;

    if (B_in->get_min(worst_score, v_worst)){
        return v_worst;
    } else{
        LOG(FATAL)<<"error";
    }
}



void localPar::B_to_N(vid_t v) {
    CHECK_EQ(B_in->contains(v), true);
    CHECK_EQ(B_out->contains(v), true);


    B_in->remove(v);
    B_out->remove(v);
    v_flag[v] = 0;
    num_vertices--;
    CHECK_EQ(B_in->size(), B_out->size());

    if (degrees_now[v] > 0){


        std::vector<vid_t> temp = *cache_get(v);

        CHECK_EQ(degrees_now[v], temp.size());
        for (vid_t adj_in: temp) {

            CHECK_NE(v_flag[adj_in], 0);

            del_cache_edge(v, adj_in);

            if (v_flag[adj_in] == 1) {

                B_out->increase_value(adj_in);
                B_in->decrease_value(adj_in);
            }

            if (v_flag[adj_in] == 2) {

                C_to_B(adj_in);
            }
        }
    }

    CHECK_EQ(degrees_now[v], 0);
    CHECK_EQ(cache_id2index[v], graph->num_vertices);
}



void localPar::C_to_B(vid_t v) {

    CHECK_EQ(B_in->contains(v), false);
    CHECK_EQ(B_out->contains(v), false);
    v_flag[v] = 1;

    B_in->insert(degrees_now[v], v);
    B_out->insert(graph->degrees_now[v], v);
}






std::vector<vid_t> * localPar::cache_get(vid_t v){

    CHECK_GT(degrees_now[v], 0);

    if (cache_id2index[v] == graph->num_vertices){

        cache_add(v, read_neis(v));
    }
    CHECK_EQ(degrees_now[v], cache_p[cache_id2index[v]].size());
    return &(cache_p[cache_id2index[v]]);
}





void localPar::cache_add(vid_t v, std::vector<vid_t> neis){

    CHECK_EQ(cache_id2index[v], graph->num_vertices);
    CHECK_EQ(cache_p.size(), cache_index2id.size());
    CHECK_EQ(cache_p.capacity(), ceil((double) graph->num_vertices/FLAGS_cache_ratio));


    if (cache_p.size() == cache_p.capacity()){


        vid_t head_v = cache_index2id[head_index];

        write_neis(head_v, cache_p[head_index]);

        cache_id2index[head_v] = graph->num_vertices;


        cache_p[head_index] = neis;
        cache_index2id[head_index] = v;
        cache_id2index[v] = head_index;



        if (head_index == cache_p.capacity() - 1){

            head_index = 0;
        }else{
            head_index++;
        }
    }

    else{

        cache_p.emplace_back(neis);
        cache_index2id.emplace_back(v);

        cache_id2index[v] = cache_p.size()-1;
    }

    CHECK_LT(cache_id2index[v], graph->num_vertices);
}




void localPar::cache_del(vid_t v){

    CHECK_LT(cache_id2index[v], graph->num_vertices);
    CHECK_EQ(cache_p.size(), cache_index2id.size());

    CHECK_EQ(degrees_now[v], 0);


    cache_p[cache_id2index[v]] = cache_p.back();
    cache_index2id[cache_id2index[v]] = cache_index2id.back();
    cache_id2index[cache_index2id.back()] = cache_id2index[v];


    cache_p.pop_back();
    cache_index2id.pop_back();
    cache_id2index[v] = graph->num_vertices;
}





void localPar::del_cache_edge(vid_t vfrom, vid_t vto){





    graph->add_cache_edge(vfrom, vto);

    del_cache_vto(vfrom, vto);
    del_cache_vto(vto, vfrom);

    num_edges--;
}





void localPar::del_cache_vto(vid_t vfrom, vid_t vto){

    del_1Dvec_item(vto, cache_get(vfrom));
    degrees_now[vfrom]--;

    CHECK_EQ(cache_p[cache_id2index[vfrom]].size(), degrees_now[vfrom]);

    if (degrees_now[vfrom] == 0){
        cache_del(vfrom);
    }
}





void localPar::add_cache_edge(vid_t vfrom, vid_t vto){

    graph->del_cache_edge(vfrom, vto);

    add_vto(vfrom, vto);
    add_vto(vto, vfrom);

    num_edges++;
}





void localPar::add_vto(vid_t vfrom, vid_t vto){

    if (degrees_now[vfrom] == 0){

        CHECK_EQ(cache_id2index[vfrom], graph->num_vertices);

        cache_add(vfrom, {vto});
        degrees_now[vfrom]++;
    }

    else{

        cache_get(vfrom)->emplace_back(vto);
        degrees_now[vfrom]++;

        CHECK_EQ(cache_p[cache_id2index[vfrom]].size(), degrees_now[vfrom]);
    }
}



void localPar::del_mem(){

    CHECK_EQ(cache_index2id.size(), cache_p.size());

    for (int k = 0; k < cache_index2id.size(); ++k) {

        CHECK_GT(cache_p[k].size(), 0);

        CHECK_EQ(cache_id2index[cache_index2id[k]], k);

        write_neis(cache_index2id[k], cache_p[k]);
    }

    fout_g.seekp(0);

    fout_g.write((char *) &num_vertices, sizeof(vid_t));
    fout_g.write((char *) &num_edges, sizeof(size_t));


    write_degree();
    write_offset();
    write_degree_max();




    std::vector<std::vector<vid_t>>().swap(cache_p);
    std::vector<vid_t>().swap(cache_index2id);
    std::vector<vid_t>().swap(cache_id2index);


    delete B_out;
    delete B_in;
}



void localPar::write_degree(){
    std::string deg_filename = filename + "-degree";

    if (!is_exists(binedgelist_name(deg_filename))){
        create_empty_file(binedgelist_name(deg_filename));
    }
    std::fstream fout_d;

    fout_d.open(binedgelist_name(deg_filename), std::ios::out|std::ios::in|std::ios::binary);
    fout_d.seekg(0);
    for (vid_t deg: degrees_now) {
        fout_d.write((char *) &deg, sizeof(vid_t));
    }
    fout_d.close();
}



void localPar::write_degree_max(){
    std::string deg_filename = filename + "-degreemax";

    if (!is_exists(binedgelist_name(deg_filename))){
        create_empty_file(binedgelist_name(deg_filename));
    }
    std::fstream fout_dm;

    fout_dm.open(binedgelist_name(deg_filename), std::ios::out|std::ios::in|std::ios::binary);
    fout_dm.seekg(0);
    for (vid_t deg: degrees_max) {
        fout_dm.write((char *) &deg, sizeof(vid_t));
    }
    fout_dm.close();
}



void localPar::write_offset(){
    std::string deg_filename = filename + "-offset";

    if (!is_exists(binedgelist_name(deg_filename))){
        create_empty_file(binedgelist_name(deg_filename));
    }
    std::fstream fout_o;

    fout_o.open(binedgelist_name(deg_filename), std::ios::out|std::ios::in|std::ios::binary);
    fout_o.seekg(0);
    for (size_t deg: offsets) {
        fout_o.write((char *) &deg, sizeof(size_t));
    }
    fout_o.close();
}



void localPar::self_check() {
    LOG(INFO) << "start";
    vid_t num_v_temp;
    size_t num_e_temp;

    fout_g.seekg(0);
    fout_g.read((char *) &num_v_temp, sizeof(vid_t));
    fout_g.read((char *) &num_e_temp, sizeof(vid_t));
    CHECK_EQ(num_v_temp, num_vertices);
    CHECK_EQ(num_e_temp, num_edges);
    LOG(INFO) << "pass";

    LOG(INFO) << "start";

    std::string offset_filename = FLAGS_init_filename + "-p-" + std::to_string(p_id) + "-offset";
    std::fstream fout_o;
    fout_o.open(binedgelist_name(offset_filename), std::ios::out | std::ios::in | std::ios::binary);

    size_t offset;

    vid_t v = 0;

    while (fout_o.read((char *) &offset, sizeof(size_t)) && v < graph->num_vertices) {

        if (degrees_now[v] > 0) {

            std::vector<vid_t> cache_neis = * cache_get(v);

            std::vector<vid_t> disk_neis;

            fout_g.seekg(offset);

            for (int k = 0; k < degrees_now[v]; ++k) {
                vid_t v_nei;
                fout_g.read((char *) &v_nei, sizeof(vid_t));
                disk_neis.emplace_back(v_nei);
            }

            CHECK_EQ(cache_neis.size(), disk_neis.size());

            for (int k = 0; k < cache_neis.size(); ++k) {
                CHECK_EQ(cache_neis[k], disk_neis[k]);
            }
        }
        v++;
    }
    fout_o.close();
    LOG(INFO) << "pass";
}


void localPar::reload_pop(){
    reload_meta();
    reload_Bin();
}



void localPar::reload_push(){
    reload_meta();
    reload_Bout();
    temp_flag = 1.2*num_vertices;
}



void localPar::reload_meta(){
    reload_degree();
    reload_degree_max();
    reload_offset();
    reload_e_v_num();
}



void localPar::reload_degree(){

    degrees_now.clear();

    degrees_now.reserve(graph->num_vertices);
    std::string deg_filename = filename + "-degree";

    if (!is_exists(binedgelist_name(deg_filename))){
        LOG(FATAL) << "error";
    }
    std::fstream fout_d;

    fout_d.open(binedgelist_name(deg_filename), std::ios::out|std::ios::in|std::ios::binary);

    vid_t v = 0;
    vid_t deg_tmp;
    while (fout_d.read((char *) &(deg_tmp), sizeof(vid_t))){
        degrees_now.emplace_back(deg_tmp);
        v++;
    }
    fout_d.close();
}



void localPar::reload_degree_max(){

    degrees_max.clear();

    degrees_max.reserve(graph->num_vertices);
    std::string deg_filename = filename + "-degreemax";

    if (!is_exists(binedgelist_name(deg_filename))){
        LOG(FATAL) << "error";
    }
    std::fstream fout_dm;

    fout_dm.open(binedgelist_name(deg_filename), std::ios::out|std::ios::in|std::ios::binary);

    vid_t v = 0;
    vid_t deg_max_tmp;
    while (fout_dm.read((char *) &(deg_max_tmp), sizeof(vid_t))){
        degrees_max.emplace_back(deg_max_tmp);
        v++;
    }
    fout_dm.close();
}



void localPar::reload_offset(){

    offsets.clear();

    offsets.reserve(graph->num_vertices);
    std::string deg_filename = filename + "-offset";

    if (!is_exists(binedgelist_name(deg_filename))){
        LOG(FATAL) << "error";
    }
    std::fstream fout_o;

    fout_o.open(binedgelist_name(deg_filename), std::ios::out|std::ios::in|std::ios::binary);

    vid_t v = 0;
    size_t off_tmp;
    while (fout_o.read((char *) &(off_tmp), sizeof(size_t))){
        offsets.emplace_back(off_tmp);
        v++;
    }
    fout_o.close();
}



void localPar::reload_e_v_num(){
    fout_g.seekg(0);
    fout_g.read((char *) &num_vertices, sizeof(vid_t));
    fout_g.read((char *) &num_edges, sizeof(size_t));
}




void localPar::reload_Bin(){
    for (int v = 0; v < degrees_now.size(); ++v) {

        if (degrees_now[v] > 0){
            if (degrees_now[v] == graph->degrees_max[v]){
                v_flag[v] = 2;
            }

            if (degrees_now[v] < graph->degrees_max[v]){
                v_flag[v] = 1;
                B_in->insert(degrees_now[v], v);
                B_out->insert(graph->degrees_now[v], v);
            }
            if (degrees_now[v] > graph->degrees_max[v]){
                LOG(FATAL) << "error";
            }
        }
    }
}



void localPar::reload_Bout(){
    for (int v = 0; v < degrees_now.size(); ++v) {

        if (degrees_now[v] > 0){

            if (graph->degrees_now[v] > 0){
                v_flag[v] = 1;
                B_in->insert(degrees_now[v], v);
                B_out->insert(graph->degrees_now[v], v);
            }

            else{
                v_flag[v] = 2;
            }
        }
    }
}




void localPar::pop(double pop_ratio){
    reload_pop();

    size_t min_e_num = ceil((double) max_num_e * (1-pop_ratio));

    while (num_edges > min_e_num){

        vid_t v_worst;

        if (B_in->size() <= 0){

            for (int v = 0; v < v_flag.size(); ++v) {
                if (v_flag[v] == 2){
                    C_to_B(v);
                    v_worst = v;
                    break;
                }
            }
        } else{

            v_worst = vworst_in_B();
        }

        B_to_N(v_worst);
    }
    del_mem();
    LOG(WARNING) << "part: " << p_id << " pop over. Edge num: " << num_edges << ". Vertices num: " << num_vertices << ".";
}




void localPar::push(){
    reload_push();
    split();
}




void localPar::dec_data(std::vector<std::vector<vid_t>> *dec_data){

    reload_meta();

    for (int v = 0; v < graph->num_vertices; ++v) {

        if (degrees_now[v] > 0){

            std::vector<vid_t> dec_adjs = dec_data->at(v);

            std::vector<vid_t> adjs = *cache_get(v);

            for (vid_t adj: adjs) {

                auto iter = std::find(dec_adjs.begin(), dec_adjs.end(), adj);
                if (iter != dec_adjs.end()){

                    del_cache_vto(v, adj);
                    del_cache_vto(adj, v);
                    num_edges--;
                }
            }
        }
    }
    del_mem();
}