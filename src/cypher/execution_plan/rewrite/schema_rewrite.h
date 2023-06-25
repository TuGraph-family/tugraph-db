#pragma once
#include <iostream>
#include <unordered_map>
#include <vector>
#include "node.h"
#include "graph.h"

#include "cypher/execution_plan/ops/op.h"
#include "graph/common.h"
#include "parser/data_typedef.h"
#include "state_info.h"
#include "execution_plan/execution_plan.h"

namespace rewrite_cypher {

class SchemaRewrite {
 public:
    std::map<std::string, int> label2idx;        // 每个label对应一个id
    int label_cnt = 0;
    std::vector<std::string> idx2label;          // id到label的对应

    std::map<size_t, cypher::NodeID> vidx2pidx;  // 顶点id到pattern graph中id对应
    std::map<cypher::NodeID, size_t> pidx2vidx;  // pattern graph中id到顶点id对应
    std::map<size_t, cypher::NodeID> eidx2pidx;  // 边id到pattern graph中id对应

    std::vector<std::set<size_t>> edge_core;  // 保存查询图中边id到目标图中边id的对应关系
    std::vector<cypher::SchemaGraphMap> sgm;
    cypher::SchemaNodeMap* m_schema_node_map;
    cypher::SchemaRelpMap* m_schema_relp_map;

    std::vector<bool> visited;
    std::vector<bool> edge_visited;

    Graph target_graph;
    Graph query_graph;
    std::vector<int> core_2;  // 保存查询图中点id到目标图中点id的对应关系
    size_t query_size;
    size_t target_size;
    size_t map_cnt = 0;  // 查询图中点已经匹配的个数
    size_t depth = 0;    // 遍历的深度

    SchemaRewrite(/* args */){};
    ~SchemaRewrite(){};

    std::vector<cypher::SchemaGraphMap> GetEffectivePath(const lgraph::SchemaInfo& schema_info,
                                                         cypher::SchemaNodeMap* schema_node_map,
                                                         cypher::SchemaRelpMap* schema_relp_map);
    int GetLabelNum(std::string label);
    void PrintLabel2Idx();
    void PrintMapping();
    void AddMapping();
    void Reset();
    bool CheckNodeLabel(size_t vid, size_t t_vid);
    // void Backtrack(size_t vid);
    std::map<size_t, std::set<size_t>> GetNextTVidByEdgeId(std::set<size_t>& edge_ids,
                                                           size_t t_vid);
    std::set<size_t> GetNextEdgeIds(Edge* edge, size_t t_vid, size_t direction);
    void MatchRecursive(size_t vid, size_t t_vid);
    std::vector<StateInfo> GenCandidateStateInfo();
};

};  // namespace rewrite_cypher