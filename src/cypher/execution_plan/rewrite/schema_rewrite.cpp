
#include "schema_rewrite.h"

// #define DEBUG

namespace rewrite_cypher {

// 根据schema信息和cypher信息获取有效路径
std::vector<cypher::SchemaGraphMap> SchemaRewrite::GetEffectivePath(
    const lgraph::SchemaInfo& schema_info, cypher::SchemaNodeMap* schema_node_map,
    cypher::SchemaRelpMap* schema_relp_map) {
    // 根据schema构建目标图
    m_schema_node_map = schema_node_map;
    m_schema_relp_map = schema_relp_map;
    label2idx.insert({"", -2});
    std::vector<std::string> v_labels = schema_info.v_schema_manager.GetAllLabels();
    int i = 0;
    for (; i < v_labels.size(); i++) {
        label2idx.insert({v_labels[i], i});
        idx2label.push_back(v_labels[i]);
        target_graph.AddNode((size_t)i, GetLabelNum(v_labels[i]));
    }
    lgraph::SchemaManager e_schema_manager = schema_info.e_schema_manager;
    size_t label_num = 0;
    const lgraph::Schema* schema;
    size_t e_cnt = 0;
    while (schema = e_schema_manager.GetSchema(label_num)) {
        // std::cout<<"schema label:"<<schema->GetLabel()<<std::endl;
        label2idx.insert({schema->GetLabel(), i});
        idx2label.push_back(schema->GetLabel());
        i++;
        const lgraph::EdgeConstraints& ec = schema->GetEdgeConstraints();
        for (auto pair : ec) {
            // std::cout<<"src:"<<pair.first<<",dst:"<<pair.second<<std::endl;
            std::set<int> label_nums;
            label_nums.insert(GetLabelNum(schema->GetLabel()));
            target_graph.AddEdge(e_cnt, (size_t)GetLabelNum(pair.first),
                                 (size_t)GetLabelNum(pair.second), label_nums,
                                 parser::LinkDirection::LEFT_TO_RIGHT);
            // std::cout<<"edge id :"<<e_cnt<<",label num:"<<GetLabelNum(schema->GetLabel())<<",src
            // id:"<<(size_t)GetLabelNum(pair.first)<<",dst
            // id:"<<(size_t)GetLabelNum(pair.second)<<std::endl;
            e_cnt++;
        }
        label_num++;
    }

    // cypher语句构建查询图
    size_t cnt = 0;
    for (auto it = schema_node_map->begin(); it != schema_node_map->end(); it++) {
        vidx2pidx.insert({cnt, it->first});
        pidx2vidx.insert({it->first, cnt});
        query_graph.AddNode(cnt, GetLabelNum(it->second));
        // std::cout<<"node id :"<<cnt<<",label num:"<<GetLabelNum(it->second)<<std::endl;
        cnt++;
    }

    e_cnt = 0;
    for (auto it = schema_relp_map->begin(); it != schema_relp_map->end(); it++) {
        eidx2pidx.insert({e_cnt, it->first});
        auto p_it = pidx2vidx.find(std::get<0>(it->second));
        auto src_idx = p_it->second;
        p_it = pidx2vidx.find(std::get<1>(it->second));
        auto dst_idx = p_it->second;
        std::set<std::string> labels = std::get<2>(it->second);
        parser::LinkDirection direction = std::get<3>(it->second);
        std::set<int> label_nums;
        if (labels.empty()) {
            query_graph.AddEdge(e_cnt, src_idx, dst_idx, label_nums, direction);
        } else {
            for (auto lit = labels.begin(); lit != labels.end(); lit++) {
                label_nums.insert(GetLabelNum(*lit));
            }
            query_graph.AddEdge(e_cnt, src_idx, dst_idx, label_nums, direction);
        }
        e_cnt++;
    }

    // 初始化相关参数
    target_size = target_graph.m_nodes.size();
    query_size = query_graph.m_nodes.size();
    edge_core.resize(query_graph.m_edges.size());
    visited.resize(query_size);
    edge_visited.resize(target_graph.m_edges.size());
    core_2.resize(query_size);

#ifdef DEBUG
    PrintLabel2Idx();
    std::cout << "目标图:" << std::endl;
    target_graph.PrintGraph();
    std::cout << "查询图:" << std::endl;
    query_graph.PrintGraph();
#endif

    // 回溯
    for (size_t i = 0; i < target_size; i++) {
        Reset();
        if (CheckNodeLabel(0, i)) {
            core_2[0] = i;
            depth++;
            MatchRecursive(0, i);
        }
    }

    return sgm;
}

// 重置所有状态
void SchemaRewrite::Reset() {
    for (size_t i = 0; i < query_size; i++) {
        core_2[i] = -1;
        visited[i] = false;
    }
    map_cnt = 0;
    depth = 0;
}

void SchemaRewrite::MatchRecursive(size_t vid, size_t t_vid) {
    if (depth == query_size) {
#ifdef DEBUG
        PrintMapping();
#endif
        AddMapping();
    } else {
        std::vector<StateInfo> candidate_state_infos = GenCandidateStateInfo();

#ifdef DEBUG
        std::cout << "num of stateinfos:" << candidate_state_infos.size() << ",depth:" << depth
                  << std::endl;
        for (StateInfo si : candidate_state_infos) {
            std::cout << "vid:" << si.m_vid << std::endl;
            std::cout << "next vid:" << si.m_next_vid << std::endl;
            std::cout << "eid:" << si.m_eid << std::endl;
        }
#endif

        for (StateInfo si : candidate_state_infos) {
            for (auto it = si.m_id_map.begin(); it != si.m_id_map.end(); it++) {
#ifdef DEBUG
                std::cout << "check:query:" << si.m_next_vid << ",target:" << it->first
                          << std::endl;
#endif
                if (CheckNodeLabel(si.m_next_vid, it->first)) {
                    core_2[si.m_next_vid] = it->first;
                    depth++;
                    edge_core[si.m_eid] = si.m_id_map[it->first];
                    MatchRecursive(si.m_next_vid, it->first);
                    core_2[si.m_next_vid] = -1;
                    depth--;
                }
            }
        }
    }
}

std::vector<StateInfo> SchemaRewrite::GenCandidateStateInfo() {
    std::vector<StateInfo> candidate_state_infos;
    for (size_t i = 0; i < query_size; i++) {
        if (core_2[i] > -1) {
            Node node = query_graph.m_nodes[i];
            for (size_t eid : node.m_outedges) {
                Edge edge = query_graph.m_edges[eid];
                if (core_2[edge.m_target_id] == -1) {
                    std::set<size_t> edge_ids = GetNextEdgeIds(&edge, core_2[i], 0);
                    std::map<size_t, std::set<size_t>> id_map =
                        GetNextTVidByEdgeId(edge_ids, core_2[i]);
                    StateInfo si(i, edge.m_target_id, eid, 0, &id_map);
                    candidate_state_infos.push_back(si);
                }
            }
            if (!candidate_state_infos.empty()) {
                return candidate_state_infos;
            }
            for (size_t eid : node.m_inedges) {
                Edge edge = query_graph.m_edges[eid];
                if (core_2[edge.m_source_id] == -1) {
                    std::set<size_t> edge_ids = GetNextEdgeIds(&edge, core_2[i], 1);
                    std::map<size_t, std::set<size_t>> id_map =
                        GetNextTVidByEdgeId(edge_ids, core_2[i]);
                    StateInfo si(i, edge.m_source_id, eid, 1, &id_map);
                    candidate_state_infos.push_back(si);
                }
            }
            if (!candidate_state_infos.empty()) {
                return candidate_state_infos;
            }
            for (size_t eid : node.m_undirectededges) {
                Edge edge = query_graph.m_edges[eid];
                size_t next_vid = edge.m_source_id == i ? edge.m_target_id : edge.m_source_id;
                if (core_2[next_vid] == -1) {
                    std::set<size_t> edge_ids = GetNextEdgeIds(&edge, core_2[i], 2);
                    std::map<size_t, std::set<size_t>> id_map =
                        GetNextTVidByEdgeId(edge_ids, core_2[i]);
                    StateInfo si(i, next_vid, eid, 2, &id_map);
                    candidate_state_infos.push_back(si);
                }
            }
            if (!candidate_state_infos.empty()) {
                return candidate_state_infos;
            }
        }
    }
    return candidate_state_infos;
}

// //回溯
// void SchemaRewrite::Backtrack(size_t vid){
//     map_cnt--;
//     visited[vid]=false;
//     core_2[vid]=-1;
// }

// 检查目标图和查询图上当前点的label是否一致
bool SchemaRewrite::CheckNodeLabel(size_t vid, size_t t_vid) {
    Node* query_node = &query_graph.m_nodes[vid];
    Node* target_node = &target_graph.m_nodes[t_vid];
    if (query_node->m_label == -2 || query_node->m_label == target_node->m_label) {
        return true;
    }
    return false;
}
// 根据目标图上的边id集合获取点id集合
std::map<size_t, std::set<size_t>> SchemaRewrite::GetNextTVidByEdgeId(std::set<size_t>& edge_ids,
                                                                      size_t t_vid) {
    std::map<size_t, std::set<size_t>> id2eidset;
    for (auto mit = edge_ids.begin(); mit != edge_ids.end(); mit++) {
        Edge& edge = target_graph.m_edges[*mit];
        size_t next_tvid = edge.m_target_id;
        if (next_tvid == t_vid) {
            next_tvid = edge.m_source_id;
        }
        auto sit = id2eidset.find(next_tvid);
        if (sit != id2eidset.end()) {
            sit->second.insert(*mit);
        } else {
            std::set<size_t> ids;
            ids.insert(*mit);
            id2eidset[next_tvid] = ids;
        }
    }
    return id2eidset;
}
// 根据查询图上query_edge获取目标图上可行的边id集合
std::set<size_t> SchemaRewrite::GetNextEdgeIds(Edge* query_edge, size_t t_vid, size_t direction) {
    std::set<size_t> edge_ids;
    Node& target_node = target_graph.m_nodes[t_vid];
    if (direction == 0) {
        for (size_t out_edge_id : target_node.m_outedges) {
            Edge& out_edge = target_graph.m_edges[out_edge_id];
            auto out_edge_label = out_edge.m_labels.begin();
            auto it = query_edge->m_labels.find(*out_edge_label);
            if (query_edge->m_labels.size() == 0 || it != query_edge->m_labels.end()) {
                edge_ids.insert(out_edge_id);
            }
        }
    } else if (direction == 1) {
        for (size_t in_edge_id : target_node.m_inedges) {
            Edge& in_edge = target_graph.m_edges[in_edge_id];
            auto in_edge_label = in_edge.m_labels.begin();
            auto it = query_edge->m_labels.find(*in_edge_label);
            if (query_edge->m_labels.size() == 0 || it != query_edge->m_labels.end()) {
                edge_ids.insert(in_edge_id);
            }
        }
    } else if (direction == 2) {
        for (size_t out_edge_id : target_node.m_outedges) {
            Edge& out_edge = target_graph.m_edges[out_edge_id];
            auto out_edge_label = out_edge.m_labels.begin();
            auto it = query_edge->m_labels.find(*out_edge_label);
            if (query_edge->m_labels.size() == 0 || it != query_edge->m_labels.end()) {
                edge_ids.insert(out_edge_id);
            }
        }
        for (size_t in_edge_id : target_node.m_inedges) {
            Edge& in_edge = target_graph.m_edges[in_edge_id];
            auto in_edge_label = in_edge.m_labels.begin();
            auto it = query_edge->m_labels.find(*in_edge_label);
            if (query_edge->m_labels.size() == 0 || it != query_edge->m_labels.end()) {
                edge_ids.insert(in_edge_id);
            }
        }
    }
    return edge_ids;
}
// 根据label获取其唯一的id
int SchemaRewrite::GetLabelNum(std::string label) {
    auto it = label2idx.find(label);
    int label_num = -1;
    if (it == label2idx.end()) {
        label2idx.insert({label, label_cnt});
        label_num = label_cnt;
        label_cnt++;
    } else {
        label_num = it->second;
    }
    return label_num;
}
// 将匹配的路径加入到schema graph map中
void SchemaRewrite::AddMapping() {
    cypher::SchemaNodeMap snm(*m_schema_node_map);
    cypher::SchemaRelpMap srm(*m_schema_relp_map);
    for (size_t j = 0; j < core_2.size(); j++) {
        int core_id = core_2[j];
        std::string label = idx2label[core_id];
        auto p_id_it = vidx2pidx.find(j);
        auto p_id = p_id_it->second;
        snm[p_id] = label;
    }
    for (Edge e : query_graph.m_edges) {
        size_t src_id = e.m_source_id, tar_id = e.m_target_id;
        size_t core_src_id = core_2[src_id];
        size_t core_tar_id = core_2[tar_id];

        auto p_id_it = eidx2pidx.find(e.m_id);
        auto p_id = p_id_it->second;
        auto srm_it = srm.find(p_id);

        parser::LinkDirection direction = std::get<3>(srm_it->second);
        std::set<size_t> edge_ids = edge_core[e.m_id];
        std::set<std::string> edge_labels;

        for (auto it = edge_ids.begin(); it != edge_ids.end(); it++) {
            Edge& edge = target_graph.m_edges[*it];
            auto lit = edge.m_labels.begin();
            edge_labels.insert(idx2label[*lit]);
        }
        if (srm_it != srm.end()) {
            auto value = srm_it->second;
            srm_it->second = std::tuple<cypher::NodeID, cypher::NodeID, std::set<std::string>,
                                        parser::LinkDirection>(
                std::get<0>(value), std::get<1>(value), edge_labels, std::get<3>(value));
        }
    }
    sgm.push_back(std::make_pair(snm, srm));
}
// 打印当前匹配的路径
void SchemaRewrite::PrintMapping() {
    std::cout << "Node Mapping:" << std::endl;
    for (size_t j = 0; j < query_size; j++) {
        int core_id = core_2[j];
        std::string label = idx2label[core_id];
        std::cout << "(" << core_id << "[" << label << "]-" << j << ")";
    }
    std::cout << std::endl;
    std::cout << "Edge Mapping:" << std::endl;
    for (Edge e : query_graph.m_edges) {
        size_t src_id = e.m_source_id, tar_id = e.m_target_id;
        size_t core_src_id = core_2[src_id], core_tar_id = core_2[tar_id];
        std::set<size_t> edge_ids = edge_core[e.m_id];
        std::string label_str = "";
        for (auto i = edge_ids.begin(); i != edge_ids.end(); i++) {
            Edge& edge = target_graph.m_edges[*i];
            int label_id = 0;
            for (auto it = edge.m_labels.begin(); it != edge.m_labels.end(); it++) {
                label_id = *it;
                break;
            }
            label_str += idx2label[label_id];
            label_str += ".";
        }
        std::string src_label = idx2label[core_src_id];
        std::string tar_label = idx2label[core_tar_id];
        std::cout << "(" << src_id << "[" << src_label << "])-[" << label_str << "]-(" << tar_id
                  << "[" << tar_label << "]) " << std::endl;
    }
}

void SchemaRewrite::PrintLabel2Idx() {
    for (auto it : label2idx) {
        std::cout << it.first << " " << it.second << std::endl;
    }
}

};  // namespace rewrite_cypher