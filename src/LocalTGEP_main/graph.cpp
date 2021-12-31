#include "graph.hpp"




Graph::Graph(std::string filename){
    this->filename=filename;
    num_vertices = 0;
    num_edges = 0;
    create_file();
}


void Graph::create_file(){

    if (!is_exists(binedgelist_name(filename))){
        create_empty_file(binedgelist_name(filename));
    }

    fout_g.open(binedgelist_name(filename), std::ios::out|std::ios::in|std::ios::binary);
}





void Graph::write_neis(vid_t v, const std::vector<vid_t> &neis) {

    if (degrees_max[v] < neis.size()) {

        size_t file_size = get_file_size();
        offsets[v] = file_size;

        degrees_max[v] = neis.size();

    }

    fout_g.seekp(offsets[v]);

    for(vid_t nei : neis) fout_g.write((char *) &nei, sizeof(vid_t));
}





std::vector<vid_t> Graph::read_neis(vid_t v){

    CHECK_GT(degrees_now[v], 0);

    vid_t deg = degrees_now[v];

    std::vector<vid_t> neis;
    neis.reserve(deg);

    fout_g.seekg(offsets[v]);
    for (int k = 0; k < deg; ++k) {
        vid_t v_nei;
        fout_g.read((char *) &v_nei, sizeof(vid_t));
        neis.emplace_back(v_nei);
    }
    return neis;
}


size_t Graph::get_file_size(){

    fout_g.seekg(0, std::ios::end);

    return fout_g.tellg();
}


Graph::~Graph() {
    fout_g.close();
}