#include "LatinSquare.h"
#include "lgraph/olap_base.h"
#include "./algo.h"

#include <random>
#include <iostream>
#include <unordered_set>
#include <vector>
#include <numeric>
#include <iostream>
#include <string>
#include <chrono>
#include <fstream>
#include <climits>

using namespace std;
using namespace lgraph_api;
using namespace lgraph_api::olap;

struct D_v{
    int no;
    int nodeColor = -1;
    // int cscore = 0;
    int state = 0;
    bool inList = false;
    unordered_set<int> D;
};

struct Pool_elment{
    int time = 0;
    int poolConflictNum;
    unordered_set<int> nodes_in_conf_list;
    vector<unordered_set<int>> V_n;
    vector<D_v> pool_d_v_structs;
};

void loadInput(istream& is, szx::LatinSquare& lsc) {
    is >> lsc.n;
    lsc.fixedNums.reserve(lsc.n * lsc.n);
    for (szx::Assignment a; is >> a.row >> a.col >> a.num; lsc.fixedNums.push_back(a)) {}
}

void saveOutput(ostream& os, szx::Table& assignments) {
    for (auto i = assignments.begin(); i != assignments.end(); ++i) {
        for (auto j = i->begin(); j != i->end(); ++j) { os << *j << '\t'; }
        os << endl;
    }
}

void test(istream& inputStream, ostream& outputStream, long long secTimeout, int randSeed, OlapBase<Empty>& graph) {
    cerr << "load input." << endl;
    szx::LatinSquare lsc;
    loadInput(inputStream, lsc);

    cerr << "solve." << endl;
    std::chrono::steady_clock::time_point endTime = std::chrono::steady_clock::now() + std::chrono::seconds(secTimeout);
    szx::Table assignments(lsc.n, std::vector<szx::Num>(lsc.n));
    solveLatinSquare(assignments, lsc, [&]() { return std::chrono::duration_cast<std::chrono::milliseconds>(endTime - std::chrono::steady_clock::now()).count(); }, randSeed,graph);

    cerr << "save output." << endl;
    saveOutput(outputStream, assignments);
}
//void test(istream& inputStream, ostream& outputStream, long long secTimeout) {
//    // return test(inputStream, outputStream, secTimeout, static_cast<int>(time(nullptr) + clock()));
//    return test(inputStream, outputStream, secTimeout, 20);
//}

void LSCCore(OlapBase<Empty>& graph) {
    cerr << "load environment." << endl;
    ifstream ifs("/tugraph-db/test_lsc/LSC.n50f750.00.txt"); // LSC.n60f2520.03.txt
    ofstream ofs("solution.txt");
    test(ifs, ofs, 300, 20, graph); // for self-test.
    // batchTest(ifs, 300, 10000);
}

namespace szx {

    class Solver {
        // random number generator.
        mt19937 pseudoRandNumGen;
        void initRand(int seed) { pseudoRandNumGen = mt19937(seed); }
        int fastRand(int lb, int ub) { return (pseudoRandNumGen() % (ub - lb)) + lb; }
        int fastRand(int ub) { return pseudoRandNumGen() % ub; }
        int rand(int lb, int ub) { return uniform_int_distribution<int>(lb, ub - 1)(pseudoRandNumGen); }
        int rand(int ub) { return uniform_int_distribution<int>(0, ub - 1)(pseudoRandNumGen); }
        double real_rand(int lb, int ub) { return uniform_real_distribution<double>(lb, ub)(pseudoRandNumGen);}

        int colorNum;
        int nodeNum;
        int fixedNums_size;

        vector<vector<int>> adjList;
        unordered_set<int> candSet;
        // vector<int> candSet;
        // unordered_set<int> V; // candSet := V;

        vector<D_v> d_v_structs;
        vector<unordered_set<int>> V_n;
        vector<D_v> best_d_v_structs;

        int conflictNum;
        int bestConflictNum;
        vector<vector<int>> neighborColorTable;
        int alpha;
        int iter;

        vector<vector<int>> tabuList;
        vector<pair<int, int>> tabu_pair;
        vector<pair<int, int>> non_tabu_pair;

        int depth;
        int maxScore;
        int tabu_maxScore;

        int d_v_one_list[1000];
        int pos[1000];
        int ls_num;

        int conf_list[10000];
        int conf_list_best[10000];
        int conf_pos[10000];
        int conf_ls_num;

        vector<Pool_elment> pool;
        const int pool_size = 20;
        int bestPoolConflictNum;
        const float theta = 0.2;
        vector<D_v> C;

    public:
        void solve(Table& output, LatinSquare& input, function<long long()> restMilliSec, int seed, OlapBase<Empty>& graph) {
            auto start = std::chrono::high_resolution_clock::now();

            initRand(seed);
            Construct(input, graph);
            FastLSC(restMilliSec);
            // int real_conflictNum = getNeighborColorTable();
            // cout << "real_conflictNum:" << real_conflictNum << endl;

            for (int node = 0; node < nodeNum; ++node) {
                output[node / colorNum][node % colorNum] = d_v_structs[node].nodeColor;
            }
            auto stop = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration<double>(stop - start);
            std::cout << "Time taken by function: " << duration.count() << " seconds" << std::endl;
            // cerr << input.n << endl;
            // for (Num i = 0; (restMilliSec() > 0) && (i < input.n); ++i) { cerr << i << '\t' << output[i][i] << endl; }
        }

        void graphtoadjlist(OlapBase<Empty>& graph) {
            size_t num = graph.NumVertices();
            // vector<vector<int>> adjList;
            adjList.resize(num);

            for (size_t i = 0; i < num; i++) {
                AdjList<Empty> src_list = graph.OutEdges(i);
                AdjUnit<Empty> *vertex = src_list.begin();
                while (vertex != src_list.end()) {
                    adjList[i].push_back(vertex -> neighbour);
                    vertex++;
                }
            }

        }

        void Construct(const LatinSquare& input, OlapBase<Empty>& graph){
            colorNum = input.n;
            nodeNum = colorNum * colorNum;
            fixedNums_size = input.fixedNums.size();
            ls_num = 0;
            conf_ls_num = 0;
            tabuList = neighborColorTable = vector<vector<int>>(nodeNum, vector<int>(colorNum, 0));
            conflictNum = 0;
            alpha = 100000;
            pool = vector<Pool_elment>(20);
            bestPoolConflictNum = INT_MAX;
            C = vector<D_v>(nodeNum);
            // candSet = vector<int>(nodeNum);

            adjList = vector<vector<int>>(nodeNum);
            graphtoadjlist(graph);
            // pre-allocate space for each node's adjacency list
//            for (int i = 0; i < colorNum; ++i) {
//                adjList[i].reserve(2*colorNum - 2);
//            }

            // Create a vector of D_v structures.
            d_v_structs.reserve(nodeNum);
            best_d_v_structs.reserve(nodeNum);

            vector<int> temp(colorNum);
            iota(temp.begin(), temp.end(), 0);
            unordered_set<int> standard_D(temp.begin(), temp.end());

            // line 2
            // V_n = vector<unordered_set<int>>(colorNum);

            for(int i = 0; i < colorNum; ++i) {
                for(int j = 0; j < colorNum; ++j){
                    int currentIdx = i * colorNum + j;

                    // line 1
                    D_v dv;
                    dv.no = currentIdx;
                    dv.D = standard_D;
                    d_v_structs.push_back(dv);

                    // line 3
                    candSet.insert(currentIdx);
                    // candSet.push_back(currentIdx);

//                    for (int k = 0; k < colorNum; ++k) {
//                        if (k != j) {
//                            adjList[currentIdx].emplace_back(i * colorNum + k);  // neighbor in same row
//                        }
//                        if (k != i) {
//                            adjList[currentIdx].emplace_back(k * colorNum + j);  // neighbor in same column
//                        }
//                    }
                }
            }

            // reduction rule 1
            for(int i = 0;i < fixedNums_size; ++i){
                int one_D_node_id = input.fixedNums[i].row * colorNum + input.fixedNums[i].col;
                int fixed_color = input.fixedNums[i].num;
                remove_d_v_one_nodes(one_D_node_id, fixed_color);
            }

            while(ls_num > 0){ // if ls_num != 0, enter the loop
                // try to print d_v_one_list
//            for(int i = 0; i < ls_num; i++){
//                cout << d_v_one_list[i] << " ";
//            }
//            cout << endl;
//            cout << ls_num << " ";
                int temp_num = ls_num;
                for(int i = 0; i < temp_num; ++i){
                    int del_node = d_v_one_list[i]; // If not assigned, this value may change
                    int color = *(d_v_structs[del_node].D).begin();
                    remove_d_v_one_nodes(d_v_one_list[i], color);
                    // delete_node(del_node);
                    // d_v_structs[del_node].inList = false;
                }
            }

            // V = candSet; // V means the uncertain nodes

            while(!candSet.empty()){ // random
                int nodeIndex = fastRand(candSet.size());
                auto it = candSet.begin();
                advance(it, nodeIndex);
                int selectedNode = *it;

                int color = fastRand(colorNum);
                d_v_structs[selectedNode].nodeColor = color;
                // V_n[color].insert(selectedNode);
                candSet.erase(selectedNode);
//            int nodeIndex = fastRand(candSet.size());
//            int selectedNode = candSet[nodeIndex];
//            int color = fastRand(colorNum);
//            d_v_structs[selectedNode].nodeColor = color;
//
//            swap(candSet[nodeIndex], candSet.back());
//            candSet.pop_back();
            }
        }

        void remove_d_v_one_nodes(int one_D_node_id, int fixed_color){
            // remove node from candSet
            // cout << one_D_node_id << " ";
            candSet.erase(one_D_node_id);
//        swap(candSet[one_D_node_id], candSet.back());
//        candSet.pop_back();
            // D = {}
            if(d_v_structs[one_D_node_id].inList){
                delete_node(one_D_node_id);
                d_v_structs[one_D_node_id].inList = false;
            }
            d_v_structs[one_D_node_id].D.clear();
            d_v_structs[one_D_node_id].nodeColor = fixed_color;
            // put the fixed color to V_n
            // V_n[fixed_color].insert(one_D_node_id);
            // Impact on neighbors
            for(auto &node : adjList[one_D_node_id]){
                auto& D = d_v_structs[node].D;
                int pre_d_v_size = D.size();
                D.erase(fixed_color);
                int current_d_v_size = D.size();

                if(current_d_v_size == 1 && pre_d_v_size == 2){
                    add_node(node);
                    d_v_structs[node].inList = true;
                } else if(current_d_v_size == 0 && pre_d_v_size == 1){
                    delete_node(node);
                    d_v_structs[node].inList = false;
                }
            }
        }

        void add_node(int node){
            d_v_one_list[ls_num] = node;
            pos[node]=ls_num++;
        }

        void delete_node(int node)
        {
            ls_num--; // Point to the last element
            d_v_one_list[pos[node]]=d_v_one_list[ls_num]; // In the list of elements, place the last element at the position where you want to delete the element
            pos[d_v_one_list[ls_num]]=pos[node]; // In the position list, update the position of the last element
        }

        int getNeighborColorTable(){
            conf_ls_num=0;
            conflictNum = 0;
            int flag;
            for (int i = 0; i < nodeNum; ++i) {
                flag = 0;
                int color = d_v_structs[i].nodeColor;
                for (auto j = 0u; j < adjList[i].size(); ++j) {
                    int neighbor = adjList[i][j];
                    int neighbor_color = d_v_structs[neighbor].nodeColor;
                    ++neighborColorTable[i][neighbor_color];
                    if(color == neighbor_color){ // conflict
                        ++conflictNum;
                        if (flag==0)
                        {
                            add_conf(i); // i has conflict with neighbor j , add the conflict i nodes in to conf
                            flag=1; // node i has been added
                        }
                    }
                }
            }
            // cout << "conf_ls_num_begin:" << conf_ls_num << endl;
            return conflictNum / 2;
        }

        void FastLSC(function<long long()> &restMilliSec){
            iter = 0;
            tabuList = neighborColorTable = vector<vector<int>>(nodeNum, vector<int>(colorNum, 0));
            best_d_v_structs.resize(nodeNum);
            while(restMilliSec() > 0){
                for (int i = 0; i < nodeNum; ++i) {
                    fill(tabuList[i].begin(), tabuList[i].end(), 0);
                    fill(neighborColorTable[i].begin(), neighborColorTable[i].end(), 0);
                    // d_v_structs[i].cscore = 0;
                }

                conflictNum = getNeighborColorTable();
                // cout << "conflictNum begin:" << conflictNum << endl;
                bestConflictNum = conflictNum;

                depth = 0;
                while(depth < alpha && conflictNum > 0){
                    findMove();
                    makeMove();
                    ++depth;
                    ++iter;
                    if(conflictNum < 100 && conflictNum <= bestConflictNum){
                        bestConflictNum = conflictNum;
                        for(int i = 0; i < conf_ls_num; ++i){
                            conf_list_best[i] = conf_list[i];
                        }
                        // best_d_v_structs = d_v_structs;
                        for(int i = 0; i < nodeNum; ++i){
                            best_d_v_structs[i].nodeColor = d_v_structs[i].nodeColor;
                            best_d_v_structs[i].state = d_v_structs[i].state;
                            // best_d_v_structs[i].inList = d_v_structs[i].inList;
                        }
                    }
                }

                // cout << "bestConflictNum:" << bestConflictNum << endl;
                if(conflictNum == 0){
                    break;
                }

                if(iter == alpha){
                    for(int i = 0; i < nodeNum; ++i){
                        best_d_v_structs[i].D = d_v_structs[i].D;
                    }
                }

                d_v_structs = Perturb();
            }
        }

        void findMove(){
            maxScore = INT_MIN; // bigger is better
            tabu_maxScore = INT_MIN;
            double count = 0;
            double tabu_count = 0;
            pair<int, int> point_and_color;

            non_tabu_pair.clear();
            tabu_pair.clear();

            // cout << conf_ls_num << " ";
            for (int p = 0; p < conf_ls_num; ++p) {
                int i = conf_list[p];
                // ++d_v_structs[i].cscore;
                const auto& nodeStruct = d_v_structs[i];
                int oldColor = nodeStruct.nodeColor;
                if(nodeStruct.D.empty()){ // D is empty means we cannot change the color
                    continue;
                }

                const vector<int>& tempColorTable = neighborColorTable[i];
                for(const auto &i1 : nodeStruct.D) {
                    // Consider changing the color of vertex i
                    if (oldColor == i1) {
                        continue;
                    } else {
                        vector<int>& tempTabuTable = tabuList[i];
                        // 5-2 means conflicts reduce 3, so bigger is better
                        int currentScore = tempColorTable[oldColor] - tempColorTable[i1];

                        point_and_color = make_pair(i,i1);

                        if (depth >= tempTabuTable[i1]) {
                            if (currentScore > maxScore) {
                                maxScore = currentScore;
                                count = 1;
                                non_tabu_pair.push_back(point_and_color);
                            } else if (currentScore == maxScore && real_rand(0, 1) < 1 / (++count)) {
                                non_tabu_pair.push_back(point_and_color);
                            }
                        } else {
                            if (currentScore > tabu_maxScore) {
                                tabu_maxScore = currentScore;
                                tabu_count = 1;
                                tabu_pair.push_back(point_and_color);
                            } else if (currentScore == tabu_maxScore && real_rand(0, 1) < 1 / (++tabu_count)) {
                                tabu_pair.push_back(point_and_color);
                            }
                        }
                    }
                }
            }
        }

        void makeMove(){
            int bestPoint = 0, bestColor = 0;
            if (tabu_pair.size() > 0 && tabu_maxScore > maxScore && conflictNum - tabu_maxScore < bestConflictNum){ // choose tabu
                maxScore = tabu_maxScore;
                bestPoint = tabu_pair.back().first;
                bestColor = tabu_pair.back().second;
            }else {
                bestPoint = non_tabu_pair.back().first;
                bestColor = non_tabu_pair.back().second;
            }

            auto& bestNode = d_v_structs[bestPoint];
            int oldColor = bestNode.nodeColor;
            tabuList[bestPoint][oldColor] = depth + conflictNum + rand(10) + 1; // cannot change the color back in the next beta iters

            // Neighboring nodes are also affected Original color -1, new color +1
            // neighborColorTable: Distribution of Neighborhood Colors
            //     color1  color2 ... colorNum-1
            // 0
            // 1
            // 2
            // 3
            int neighbor;
            int temp_size = adjList[bestPoint].size();
            for (int i = 0; i < temp_size; i++) {
                neighbor = adjList[bestPoint][i];
                --neighborColorTable[neighbor][oldColor];
                ++neighborColorTable[neighbor][bestColor];
                // for neighbor
                // if cause new conflict, add it
                // 1.if neighbor is bestColor, before change, it has no conflict, but change brings new conflict, then add it
                if(d_v_structs[neighbor].nodeColor == bestColor && neighborColorTable[neighbor][bestColor]==1){ // before was 0, because ++ change 1
                    add_conf(neighbor);
                }
                    // if reduce conflict to 0, delete it
                    // 2.if neighbor is oldColor, but with the change of i, no conflict, then delete it
                else if(d_v_structs[neighbor].nodeColor == oldColor && neighborColorTable[neighbor][oldColor]==0){ // before was 1, because -- change 0
                    delete_conf(neighbor);
                    // d_v_structs[neighbor].cscore = 0;
                }
            }

            // for bestPoint, if no conflict, delete it
            if(neighborColorTable[bestPoint][bestColor] == 0){
                delete_conf(bestPoint);
            }

            d_v_structs[bestPoint].nodeColor = bestColor;
            conflictNum -= maxScore;

            d_v_structs[bestPoint].state = iter;
        }

        void add_conf(int v)
        {
            conf_list[conf_ls_num]=v;
            conf_pos[v]=conf_ls_num++;
        }

        void delete_conf(int v)
        {
            conf_ls_num--;
            conf_list[conf_pos[v]]=conf_list[conf_ls_num];
            conf_pos[conf_list[conf_ls_num]]=conf_pos[v];
        }

        bool similar(Pool_elment pool_element1, Pool_elment pool_element2) {
            if(pool_element1.poolConflictNum != pool_element2.poolConflictNum) {
                return false;
            }

            unordered_set<int> set1;
            set1.reserve(colorNum);
            for(const auto &item : pool_element1.nodes_in_conf_list) {
                set1.insert(d_v_structs[item].nodeColor);
            }

            for(const auto &item : pool_element2.nodes_in_conf_list) {
                if(set1.find(d_v_structs[item].nodeColor) == set1.end()) {
                    return false;
                }
                set1.erase(d_v_structs[item].nodeColor);
            }

            return set1.empty();
        }

        vector<D_v> Perturb(){
            // init
            vector<D_v> d_v_structs_perturb;
            Pool_elment pool_element;
            pool_element.poolConflictNum = bestConflictNum;
            pool_element.pool_d_v_structs = best_d_v_structs;
            for(int i = 0;i < conf_ls_num;++i){
                pool_element.nodes_in_conf_list.insert(conf_list_best[i]);
            }

            bool similar_pair = false;

            // 1.updating phase of the solution pool
            if(bestConflictNum < bestPoolConflictNum) {
                pool.clear();
                bestPoolConflictNum = pool_element.poolConflictNum;
                pool.push_back(pool_element);
                d_v_structs_perturb = pool_element.pool_d_v_structs;
            } else {
                for(auto &p : pool) {
                    ++p.time;
                    if(similar(p, pool_element)) {
                        for(int j = 0; j < nodeNum; ++j) {
                            p.pool_d_v_structs[j].state = pool_element.pool_d_v_structs[j].state;
                        }
                        similar_pair = true;
                    }
                }

                if(similar_pair){
                    // cout << "similar!!!" << endl;
                    // random select
                    int r = rand(pool.size());
                    d_v_structs_perturb = pool[r].pool_d_v_structs;
                }else{
		    // int pool_size_2 = 20;
                    if(pool.size() < 20){
                        pool.push_back(pool_element);
                    }else{
                        int oldest_time = -1;
                        int oldest_no = 0;
                        for(int i = 0; i < pool_size; ++i){
                            if(pool[i].time > oldest_time){
                                oldest_time = pool[i].time;
                                oldest_no = i;
                            }
                        }
                        pool[oldest_no] = pool_element;
                    }
                    d_v_structs_perturb = pool_element.pool_d_v_structs;
                }
            }

//        for(int i = 0;i < pool.size();++i){
//            ++pool[i].time;
//        }

            // 2.the re-construction solution phase
            int maxIter = INT_MIN;
            int minIter = INT_MAX;
            for(auto &node:d_v_structs_perturb){
                if(node.state > maxIter){
                    maxIter = node.state;
                }
                if(node.state < minIter){
                    minIter = node.state;
                }
            }
            int RCL = (maxIter - minIter) * theta + minIter;
//        cout << "maxIter:" << maxIter << endl;
//        cout << "minIter:" << minIter << endl;
//        cout << "RCL:" << RCL << endl;
            C.clear();
            int pos_c = 0;
            for(auto &node:d_v_structs_perturb){
                if(node.state <= RCL){
                    // C[pos_c++] = node;
                    C.push_back(node);
                    pos_c++;
                }
            }

            // pos_c = C.size();
            int cnt = pos_c / 2;
            // cout << cnt << endl;
            while(cnt > 0){
                int rand_c = rand(pos_c);
                unordered_set<int> tempset = d_v_structs_perturb[C[rand_c].no].D;
                if(!tempset.empty()) {
                    int index = rand(tempset.size());
                    auto it = tempset.begin();
                    advance(it, index);
                    d_v_structs_perturb[C[rand_c].no].nodeColor = *it;
                    // cout << C[rand_c].no << " ";
                }
                C[rand_c] = C[--pos_c];
                --cnt;
            }
            // cout << "pool.size:" << pool.size() << endl;
            return d_v_structs_perturb;
        }
    };

// solver.
    void solveLatinSquare(Table& output, LatinSquare& input, function<long long()> restMilliSec, int seed, OlapBase<Empty>& graph) {
        Solver().solve(output, input, restMilliSec, seed, graph);
    }

}


