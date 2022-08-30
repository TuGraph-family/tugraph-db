
#include "tools/json.hpp"
#include "cypher_sender.h"
#include <boost/algorithm/string/trim_all.hpp>
#include <fstream>
#include <unordered_map>

const static float PRECIISION = 1000 * 1000;

void cypher_thread_func(multithread_client::ClientThread* thread) {
    while (true) {
        std::string param = thread->fetch();
        if (param == "") {
            return;
        }
        nlohmann::json obj = nlohmann::json::parse(param);
        std::string res;
        auto start = std::chrono::steady_clock::now();
        bool ret = thread->get_channel()->CallCypher(res,  obj["Cypher"].get<std::string>(),
                obj["Graph"].get<std::string>());
        auto end = std::chrono::steady_clock::now();
        multithread_client::PerformanceIndicator& indicator = thread->get_indicator()[param];
        indicator.time_used +=
                std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
        if (ret) {
            if (indicator.result.empty()) indicator.result = std::move(res);
            ++indicator.success_query;
        }
        ++indicator.total_query;
    }
}

namespace multithread_client {

    CypherSender::CypherSender(const Config& conf)
            : config(conf)
            , pool(conf) {}

    bool CypherSender::parse_line(std::string& line, uint32_t& times) {
        boost::trim_all(line);
        nlohmann::json obj = nlohmann::json::parse(line);
        if (!obj.contains("Cypher") || !obj.contains("Graph") || !obj.contains("Times")) {
            return false;
        }
        times = obj["Times"].get<uint32_t>();
        return true;
    }

    void CypherSender::file_reader() {
        std::fstream ifs(config.input, std::fstream::in);
        std::string line;
        while (std::getline(ifs, line)) {
            if (line.empty()) continue;
            size_t idx = 0;
            if ((idx = line.find("##"), idx != std::string::npos)) {
                if (idx == 0) continue;
                line = line.substr(0, idx);
            }
            uint32_t times;
            if (!parse_line(line, times)) {
                if (config.continue_on_error) continue;
                return;
            }
            pool.assign(line, times);
        }
    }

    void CypherSender::process() {
        auto start = std::chrono::steady_clock::now();
        pool.start(std::bind(&cypher_thread_func, std::placeholders::_1));
        std::thread reader(&CypherSender::file_reader, this);
        reader.join();
        pool.stop();
        pool.join();
        auto end = std::chrono::steady_clock::now();
        uint64_t all_time_used =
                std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
        calculate(all_time_used);
    }

    void CypherSender::calculate(uint64_t time_used) {
        std::fstream ofs(config.output, std::fstream::out);
        std::unordered_map<std::string, PerformanceIndicator> mutil_result;
        std::vector<std::shared_ptr<ClientThread>> threads = pool.get_threads();
        for (std::shared_ptr<ClientThread> thread : threads) {
            std::unordered_map<std::string, PerformanceIndicator>& indicator = thread->get_indicator();
            for(auto in : indicator) {
                PerformanceIndicator& all = mutil_result[in.first];
                if (all.result.empty()) all.result = std::move(in.second.result);
                all.success_query += in.second.success_query;
                all.total_query += in.second.total_query;
                all.time_used = all.time_used > in.second.time_used ? all.time_used : in.second.time_used;
            }
        }
        uint64_t all_query = 0;
        uint64_t all_success = 0;
        for (auto& p : mutil_result) {
            float total_time_used = p.second.time_used / PRECIISION;
            float avg_time_used = total_time_used / p.second.total_query;
            float qps = p.second.total_query / total_time_used;
            ofs << "---------" << p.first << "---------\n";
            ofs << p.second.result << "\n";
            ofs << "total query " << p.second.total_query << "\n";
            ofs << "success query " << p.second.success_query << "\n";
            ofs << "total time used " << total_time_used << " second\n";
            ofs << "average time used " << avg_time_used << " second\n";
            ofs << "QPS " << qps << " per second\n";
            all_query += p.second.total_query;
            all_success += p.second.success_query;
        }
        float all_time_used = time_used / PRECIISION;
        float all_avg_time_used = all_time_used / all_query;
        float all_qps = all_query / all_time_used;
        ofs << "-------------" << "ALL" << "--------------\n";
        ofs << "all query " << all_query << "\n";
        ofs << "all success query " << all_success << "\n";
        ofs << "all time used " << all_time_used << " second\n";
        ofs << "all average time used " << all_avg_time_used << " second\n";
        ofs << "all QPS " << all_qps << " per second\n";
    }

} // end of namespace multithread_client