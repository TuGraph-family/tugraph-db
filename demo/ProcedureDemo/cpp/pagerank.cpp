
#include "lgraph/olap_on_db.h"
#include "json.hpp"

using namespace lgraph_api;
using namespace lgraph_api::olap;
using json = nlohmann::json;

extern "C" ParallelVector<double> PageRank(Snapshot<Empty>& snapshot, int num_iterations) {
    auto all_vertices = snapshot.AllocVertexSubset();
    all_vertices.Fill();
    auto curr = snapshot.AllocVertexArray<double>();
    auto next = snapshot.AllocVertexArray<double>();
    size_t num_vertices = snapshot.NumVertices();

    double one_over_n = (double)1 / num_vertices;
    double delta = ForEachActiveVertex<double>(
        [&](size_t vi) {
            curr[vi] = one_over_n;
            if (snapshot.OutDegree(vi) > 0) {
                curr[vi] /= snapshot.OutDegree(vi);
            }
            return one_over_n;
        },
        all_vertices);

    double d = (double)0.85;
    for (int ii = 0; ii < num_iterations; ii++) {
        printf("delta(%d)=%lf\n", ii, delta);
        next.Fill((double)0);
        delta = ForEachActiveVertex<double>(
            [&](size_t vi) {
                double sum = 0;
                for (auto& edge : snapshot.InEdges(vi)) {
                    size_t src = edge.neighbour;
                    sum += curr[src];
                }
                next[vi] = sum;
                next[vi] = (1 - d) * one_over_n + d * next[vi];
                if (ii == num_iterations - 1) {
                    return (double)0;
                } else {
                    if (snapshot.OutDegree(vi) > 0) {
                        next[vi] /= snapshot.OutDegree(vi);
                        return fabs(next[vi] - curr[vi]) * snapshot.OutDegree(vi);
                    } else {
                        return fabs(next[vi] - curr[vi]);
                    }
                }
            },
            all_vertices);
        curr.Swap(next);
    }
    return curr;
}

extern "C" bool Process(GraphDB& db, const std::string& request, std::string& response) {
    auto start_time = get_time();

    int num_iterations = 20;
    try {
        json input = json::parse(request);
        if (input["num_iterations"].is_number()) {
            num_iterations = input["num_iterations"].get<int>();
        }
    } catch (std::exception & e) {
        throw std::runtime_error("json parse error");
    }

    auto txn = db.CreateReadTxn();
    Snapshot<Empty> snapshot(
        db,
        txn,
        SNAPSHOT_PARALLEL);

    auto preprocessing_time = get_time();
    std::printf("preprocessing_time = %.2lf(s)\n", preprocessing_time - start_time);

    auto pr = PageRank(snapshot, num_iterations);
    auto all_vertices = snapshot.AllocVertexSubset();
    all_vertices.Fill();
    size_t max_pr_vi = ForEachActiveVertex<size_t>(
        [&](size_t vi) {
            return vi;
        },
        all_vertices,
        0,
        [&](size_t a, size_t b) {
            return pr[a] > pr[b] ? a : b;
        });
    std::cout << "snapshot max_pr = pr[" << max_pr_vi << "] = " << pr[max_pr_vi] << std::endl;

    auto exec_time = get_time();
    std::printf("exec_time = %.2lf(s)\n", exec_time - preprocessing_time);

    auto end_time = get_time();
    std::printf("total_cost = %.2lf(s)\n", end_time - start_time);

    json output;
    output["preprocessing_time"] = std::to_string(preprocessing_time - start_time) + "(s)";
    output["exec_time"] = std::to_string(exec_time - preprocessing_time) + "(s)";
    output["total_cost"] = std::to_string(end_time - start_time) + "(s)";
    output["max_pr_id"] = snapshot.OriginalVid(max_pr_vi);
    output["max_pr_val"] = pr[max_pr_vi];
    response = output.dump();
    return true;
}
