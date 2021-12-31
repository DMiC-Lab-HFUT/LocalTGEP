//
// Created by JSW on 2021/12/6.
//

#include "DivideG.h"

int main(int argc, char *argv[]) {
    DivideG DG("../graph_data/Wikipedia-Vote.txt.adjlist.txt", "v", 0.8);
    DG.divide_g();
}