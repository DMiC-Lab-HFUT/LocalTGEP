//
// Created by JSW on 2020/12/5.
//

#include "util.h"



void create_empty_file(std::string file_name){
    std::fstream fout;
    fout.open(file_name, std::ios::out);
    fout.close();
}



void uniform_graph_file(char *filename){
    
    std::ifstream infile;
    infile.open(filename);
    
    char buf[1024];
    
    char attach_fname[]=".uniform.txt";
    std::strcat(filename,attach_fname);
    
    std::ofstream outfile;
    outfile.open(filename);
    int i=0;
    
    while(!infile.eof()){
        infile.getline(buf,1024,'\n');
        
        if (buf[0]!='#' and buf[0]!='/'){
            outfile<<buf<<std::endl;
        }
        std::cout<<++i<<std::endl;
    }
    
    infile.close();
    outfile.close();
}


void read_graph_edge(char *filename, igraph_t *graph) {
    


    FILE *file;
    file = fopen(filename, "r");

    igraph_read_graph_edgelist(graph, file, 0, false);
    delete_isolated_v(graph);
}


void write_graph_edge(char *filename, igraph_t *graph) {
    FILE *file;
    file = fopen(filename, "w");

    igraph_write_graph_edgelist(graph, file);
}


std::string write_graph_adj(const std::string &edge_filename, igraph_t *graph) {
    
    std::stringstream ss;
    ss << edge_filename << ".adjlist.txt";
    std::ofstream adj_file;
    adj_file.open(ss.str());

    adj_file<<igraph_vcount(graph)<<"\t"<<igraph_ecount(graph)<<std::endl;
    
    igraph_vit_t v_iterator;
    igraph_vit_create(graph, igraph_vss_all(), &v_iterator);
    
    int v_id = IGRAPH_VIT_GET(v_iterator);
    int v_num=0;
    
    while (!IGRAPH_VIT_END(v_iterator)) {
        
        adj_file<<get_v_degree(graph,v_id);
        
        igraph_vector_t v_neis;
        igraph_vector_init(&v_neis, 0);
        igraph_neighbors(graph, &v_neis, v_id, IGRAPH_ALL);
        
        for (int k = 0; k < igraph_vector_size(&v_neis); ++k) {
            
            int adj_v = VECTOR(v_neis)[k];
            
            adj_file<<"\t"<<adj_v;
        }
        adj_file<<std::endl;
        
        v_id = IGRAPH_VIT_NEXT(v_iterator);
        v_num++;
    }
    
    igraph_vit_destroy(&v_iterator);
    adj_file.close();
    LOG(WARNING)<<"转换成功"<< std::endl;
    return ss.str();
}


void simplify_graph(igraph_t *graph) {
    igraph_simplify(graph, true, true, 0);
}


int get_v_degree(igraph_t *graph, int v_id){
    
    igraph_vector_t v_degree;
    igraph_vector_init(&v_degree, 0);
    
    igraph_degree(graph, &v_degree, igraph_vss_1(v_id), IGRAPH_ALL, IGRAPH_NO_LOOPS);
    
    int degree=VECTOR(v_degree)[0];
    igraph_vector_destroy(&v_degree);
    return degree;
}




void del_1Dvec_item(vid_t item, std::vector<vid_t> *vec){
    auto iter = std::remove(vec->begin(), vec->end(), item);
    vec->erase(iter, vec->end());
}



void delete_isolated_v(igraph_t *graph){
    
    igraph_vit_t v_iterator;
    igraph_vit_create(graph, igraph_vss_all(), &v_iterator);
    
    igraph_vector_t zero_v;
    igraph_vector_init(&zero_v, 0);
    
    int v_id = IGRAPH_EIT_GET(v_iterator);
    
    while (!IGRAPH_VIT_END(v_iterator)) {
        
        if (get_v_degree(graph,v_id)==0) {
            
            igraph_vector_push_back(&zero_v, v_id);
        }
        
        v_id = IGRAPH_VIT_NEXT(v_iterator);
    }
    igraph_delete_vertices(graph,igraph_vss_vector(&zero_v));
    
    igraph_vit_destroy(&v_iterator);
    igraph_vector_destroy(&zero_v);
}



void print_igraph_vector(igraph_vector_t *i_vector){
    for (int i = 0; i < igraph_vector_size(i_vector); ++i) {
        std::cout<<"v_id: "<<i<<"\titem: "<<VECTOR(*i_vector)[i]<<std::endl;
    }
}



void fix_line(char *s) {
    int len = (int) strlen(s) - 1;
    if (s[len] == '\n'){
        s[len] = 0;
    }
    if (s[len-1] == '\r'){
        s[len-1] = 0;
    }
}



std::string convert_edgelist_to_adjlist(const std::string &edge_filename){

    char *file_path = (char *) edge_filename.data();
    igraph_t graph;
    read_graph_edge(file_path, &graph);
    simplify_graph(&graph);

    
    return write_graph_adj(edge_filename, &graph);
}





void process_bar(size_t done_num, size_t all_num, const std::string& tag) {

    int percent = (int)((double) done_num/all_num * 100);
    std::cout << "\33[1A";
    std::cout << "[" + std::string(percent, '=') + tag + std::string(100 - percent, ' ') << "]  "
              <<std::to_string(done_num)<<" / "<<std::to_string(all_num)<< std::endl;
    fflush(stdout);
}