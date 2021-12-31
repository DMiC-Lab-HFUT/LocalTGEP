# LocalTGEP 

## Environment
Ubuntu 16.04.7 LTS

## Setup

### 1.install `glog`, `gflags`

```sh
sudo apt-get install libgoogle-glog-dev libgflags-dev libboost-all-dev
```

### 2.install igraph（https://igraph.org/）

```sh
./configure
make
make check_degree
make install
```

### 3.make

```sh
cd ALP_C
mkdir release && cd release
cmake ..
make -j8
```

## Usage

```shell
./LocalTGEP
```

Graph file

```shell
vertex_num \t edge_num \n
degree \t adj1 \t adj2 \t...\t adjn \n
...
degree \t adj1 \t adj2 \t...\t adjn \n
```