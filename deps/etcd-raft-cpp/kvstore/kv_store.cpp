#include <iostream>
#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/dispatch.hpp>
#include <boost/asio/strand.hpp>
#include <algorithm>
#include <nlohmann/json.hpp>
#include <cstdlib>
#include <shared_mutex>
#include <memory>
#include <string>
#include <thread>
#include <vector>
#include <rocksdb/db.h>
#include <gflags/gflags.h>
#include "logger.h"
#include "io_service.h"
#include "connection.h"
#include "raft_log_store.h"
#include "raft_driver.h"

namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http = beast::http;           // from <boost/beast/http.hpp>
namespace net = boost::asio;            // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>

DEFINE_uint64(node_id, 0, "node id");
DEFINE_string(init_cluster, "", "init cluster");
DEFINE_int32(http_port, 0, "http port");
DEFINE_int32(raft_port, 0, "raft port");

#define ASSERT(cond) if (!(cond)) std::abort()

struct Generator {
    static const int tsLen  = 5 * 8;
    static const int cntLen  = 8;
    static const int suffixLen = tsLen + cntLen;

    Generator(uint64_t id, uint64_t time) {
        prefix = id << suffixLen;
        suffix = lowbit(time, tsLen) << cntLen;
    }
    uint64_t prefix = 0;
    std::atomic<uint64_t> suffix{0};
    uint64_t lowbit(uint64_t x, int n) {
        return x & (std::numeric_limits<uint64_t>::max() >> (64 - n));
    }
    uint64_t Next() {
        auto suf = suffix.fetch_add(1) + 1;
        auto id = prefix | lowbit(suf, suffixLen);
        return id;
    }
};

std::shared_mutex promise_mutex;
std::unordered_map<uint64_t, std::promise<std::string>> pending_promise;
rocksdb::DB* data_db = nullptr;

using namespace std::chrono;

std::unique_ptr<RaftDriver> driver;

void protobuf_handler(raftpb::Message rpc_msg) {
    driver->Message(std::move(rpc_msg));
}

Generator id_generator(FLAGS_node_id, duration_cast<milliseconds>(steady_clock::now().time_since_epoch()).count());

template<class Body, class Allocator, class Send>
void handle_request( http::request<Body, http::basic_fields<Allocator>>&& req, Send&& send) {
    // Returns a bad request response
    auto const bad_request =
    [&req](beast::string_view why) {
        http::response<http::string_body> res{http::status::bad_request, req.version()};
        res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
        res.set(http::field::content_type, "text/html");
        res.keep_alive(req.keep_alive());
        res.body() = std::string(why);
        res.prepare_payload();
        return res;
    };

    // Make sure we can handle the method
    if( req.method() != http::verb::post)
        return send(bad_request("Unknown HTTP-method"));

    if (req.target() == "/add") {
        const std::string& body = req.body();
        auto uid = id_generator.Next();
        auto obj = nlohmann::json::object();
        obj["uid"] = uid;
        obj["data"] = body;
        std::promise<std::string> promise;
        auto future = promise.get_future();
        {
            std::unique_lock lock(promise_mutex);
            pending_promise.emplace(uid, std::move(promise));
        }
        auto err = driver->Proposal(obj.dump());
        if (err != nullptr) {
            {
                std::unique_lock lock(promise_mutex);
                pending_promise.erase(uid);
            }
            http::response<http::string_body> res {http::status::internal_server_error, req.version()};
            res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
            res.set(http::field::content_type, "text/plain");
            res.keep_alive(req.keep_alive());
            res.body() = err.String();
            res.prepare_payload();
            return send(std::move(res));
        }

        http::response<http::string_body> res {http::status::ok, req.version()};
        res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
        res.set(http::field::content_type, "text/plain");
        res.keep_alive(req.keep_alive());
        res.body() = future.get();
        {
            std::unique_lock lock(promise_mutex);
            pending_promise.erase(uid);
        }
        res.prepare_payload();
        return send(std::move(res));
    }
    if (req.target() == "/add_member") {
        const std::string& body = req.body();
        auto uid = id_generator.Next();
        auto node_info = nlohmann::json::parse(body);

        raftpb::ConfChange cc;
        cc.set_type(raftpb::ConfChangeType::ConfChangeAddNode);
        cc.set_node_id(node_info["node_id"].get<uint64_t>());
        cc.set_id(uid);
        cc.set_context(body);

        std::promise<std::string> promise;
        auto future = promise.get_future();
        {
            std::unique_lock lock(promise_mutex);
            pending_promise.emplace(uid, std::move(promise));
        }
        auto err = driver->ProposeConfChange(cc);
        if (err != nullptr) {
            {
                std::unique_lock lock(promise_mutex);
                pending_promise.erase(uid);
            }
            http::response<http::string_body> res {http::status::internal_server_error, req.version()};
            res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
            res.set(http::field::content_type, "text/plain");
            res.keep_alive(req.keep_alive());
            res.body() = err.String();
            res.prepare_payload();
            return send(std::move(res));
        }

        http::response<http::string_body> res {http::status::ok, req.version()};
        res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
        res.set(http::field::content_type, "text/plain");
        res.keep_alive(req.keep_alive());
        res.body() = future.get();
        {
            std::unique_lock lock(promise_mutex);
            pending_promise.erase(uid);
        }
        res.prepare_payload();
        return send(std::move(res));
    }

    if (req.target() == "/add_learner") {
        const std::string& body = req.body();
        auto uid = id_generator.Next();
        auto node_info = nlohmann::json::parse(body);

        raftpb::ConfChange cc;
        cc.set_type(raftpb::ConfChangeType::ConfChangeAddLearnerNode);
        cc.set_node_id(node_info["node_id"].get<uint64_t>());
        cc.set_id(uid);
        cc.set_context(body);

        std::promise<std::string> promise;
        auto future = promise.get_future();
        {
            std::unique_lock lock(promise_mutex);
            pending_promise.emplace(uid, std::move(promise));
        }
        auto err = driver->ProposeConfChange(cc);
        if (err != nullptr) {
            {
                std::unique_lock lock(promise_mutex);
                pending_promise.erase(uid);
            }
            http::response<http::string_body> res {http::status::internal_server_error, req.version()};
            res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
            res.set(http::field::content_type, "text/plain");
            res.keep_alive(req.keep_alive());
            res.body() = err.String();
            res.prepare_payload();
            return send(std::move(res));
        }

        http::response<http::string_body> res {http::status::ok, req.version()};
        res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
        res.set(http::field::content_type, "text/plain");
        res.keep_alive(req.keep_alive());
        res.body() = future.get();
        {
            std::unique_lock lock(promise_mutex);
            pending_promise.erase(uid);
        }
        res.prepare_payload();
        return send(std::move(res));
    }

    if (req.target() == "/remove_member") {
        const std::string& body = req.body();
        auto uid = id_generator.Next();
        auto node_info = nlohmann::json::parse(body);

        raftpb::ConfChange cc;
        cc.set_type(raftpb::ConfChangeType::ConfChangeRemoveNode);
        cc.set_node_id(node_info["id"].get<uint64_t>());
        cc.set_id(uid);
        cc.set_context(body);

        std::promise<std::string> promise;
        auto future = promise.get_future();
        {
            std::unique_lock lock(promise_mutex);
            pending_promise.emplace(uid, std::move(promise));
        }
        auto err = driver->ProposeConfChange(cc);
        if (err != nullptr) {
            {
                std::unique_lock lock(promise_mutex);
                pending_promise.erase(uid);
            }
            http::response<http::string_body> res {http::status::internal_server_error, req.version()};
            res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
            res.set(http::field::content_type, "text/plain");
            res.keep_alive(req.keep_alive());
            res.body() = err.String();
            res.prepare_payload();
            return send(std::move(res));
        }

        http::response<http::string_body> res {http::status::ok, req.version()};
        res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
        res.set(http::field::content_type, "text/plain");
        res.keep_alive(req.keep_alive());
        res.body() = future.get();
        {
            std::unique_lock lock(promise_mutex);
            pending_promise.erase(uid);
        }
        res.prepare_payload();
        return send(std::move(res));
    }

    if (req.target() == "/get") {
        std::string val;
        auto s = data_db->Get({}, "count", &val);
        if (s.IsNotFound()) {
            val = "0";
        }
        http::response<http::string_body> res {http::status::ok, req.version()};
        res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
        res.set(http::field::content_type, "text/plain");
        res.keep_alive(req.keep_alive());
        res.body() = val;
        res.prepare_payload();
        return send(std::move(res));
    }

    return send(bad_request("Illegal request-target"));
}

//------------------------------------------------------------------------------

// Report a failure
void
fail(beast::error_code ec, char const* what) {
    std::cerr << what << ": " << ec.message() << "\n" << std::flush;
}

// Handles an HTTP server connection
class session : public std::enable_shared_from_this<session>
{
    // This is the C++11 equivalent of a generic lambda.
    // The function object is used to send an HTTP message.
    struct send_lambda {
        session& self_;

        explicit
        send_lambda(session& self)
            : self_(self) {
        }

        template<bool isRequest, class Body, class Fields>
        void
        operator()(http::message<isRequest, Body, Fields>&& msg) const {
            // The lifetime of the message has to extend
            // for the duration of the async operation so
            // we use a shared_ptr to manage it.
            auto sp = std::make_shared<
                http::message<isRequest, Body, Fields>>(std::move(msg));

            // Store a type-erased version of the shared
            // pointer in the class to keep it alive.
            self_.res_ = sp;

            // Write the response
            http::async_write(
                self_.stream_,
                *sp,
                beast::bind_front_handler(
                    &session::on_write,
                    self_.shared_from_this(),
                    sp->need_eof()));
        }
    };

    beast::tcp_stream stream_;
    beast::flat_buffer buffer_;
    http::request<http::string_body> req_;
    std::shared_ptr<void> res_;
    send_lambda lambda_;

public:
    // Take ownership of the stream
    session(
        tcp::socket&& socket)
        : stream_(std::move(socket))
        , lambda_(*this) {
    }

    // Start the asynchronous operation
    void
    run() {
        // We need to be executing within a strand to perform async operations
        // on the I/O objects in this session. Although not strictly necessary
        // for single-threaded contexts, this example code is written to be
        // thread-safe by default.
        net::dispatch(stream_.get_executor(),
                      beast::bind_front_handler(
                          &session::do_read,
                          shared_from_this()));
    }

    void
    do_read() {
        // Make the request empty before reading,
        // otherwise the operation behavior is undefined.
        req_ = {};

        // Set the timeout.
        stream_.expires_after(std::chrono::seconds(30));

        // Read a request
        http::async_read(stream_, buffer_, req_,
            beast::bind_front_handler(
                &session::on_read,
                shared_from_this()));
    }

    void
    on_read(
        beast::error_code ec,
        std::size_t bytes_transferred) {
        boost::ignore_unused(bytes_transferred);
        // This means they closed the connection
        if(ec == http::error::end_of_stream)
            return do_close();
        if(ec)
            return fail(ec, "read");
        // Send the response
        handle_request(std::move(req_), lambda_);
    }

    void
    on_write(
        bool close,
        beast::error_code ec,
        std::size_t bytes_transferred) {
        boost::ignore_unused(bytes_transferred);

        if(ec)
            return fail(ec, "write");

        if(close) {
            // This means we should close the connection, usually because
            // the response indicated the "Connection: close" semantic.
            return do_close();
        }

        // We're done with the response so delete it
        res_ = nullptr;

        // Read another request
        do_read();
    }

    void
    do_close() {
        // Send a TCP shutdown
        beast::error_code ec;
        stream_.socket().shutdown(tcp::socket::shutdown_send, ec);

        // At this point the connection is closed gracefully
    }
};

//------------------------------------------------------------------------------

// Accepts incoming connections and launches the sessions
class listener : public std::enable_shared_from_this<listener>
{
    net::io_context& ioc_;
    tcp::acceptor acceptor_;

public:
    listener(
        net::io_context& ioc,
        tcp::endpoint endpoint)
        : ioc_(ioc)
        , acceptor_(net::make_strand(ioc)) {
        beast::error_code ec;

        // Open the acceptor
        acceptor_.open(endpoint.protocol(), ec);
        if(ec) {
            fail(ec, "open");
            return;
        }

        // Allow address reuse
        acceptor_.set_option(net::socket_base::reuse_address(true), ec);
        if(ec) {
            fail(ec, "set_option");
            return;
        }

        // Bind to the server address
        acceptor_.bind(endpoint, ec);
        if(ec) {
            fail(ec, "bind");
            return;
        }

        // Start listening for connections
        acceptor_.listen(
            net::socket_base::max_listen_connections, ec);
        if(ec) {
            fail(ec, "listen");
            return;
        }
    }

    // Start accepting incoming connections
    void
    run() {
        do_accept();
    }

private:
    void
    do_accept() {
        // The new connection gets its own strand
        acceptor_.async_accept(
            net::make_strand(ioc_),
            beast::bind_front_handler(
                &listener::on_accept,
                shared_from_this()));
    }

    void
    on_accept(beast::error_code ec, tcp::socket socket) {
        if(ec) {
            fail(ec, "accept");
        } else {
            // Create the session and run it
            std::make_shared<session>(
                std::move(socket))->run();
        }

        // Accept another connection
        do_accept();
    }
};

void runServer() {
    auto thread_num = 4;
    net::io_context ioc{thread_num};
    std::make_shared<listener>(ioc,
        tcp::endpoint{net::ip::make_address("127.0.0.1"),
            static_cast<unsigned short>(FLAGS_http_port)})->run();
    std::vector<std::thread> threads;
    threads.reserve(thread_num);
    for(auto i = 0; i < thread_num; i++) {
        threads.emplace_back([&ioc]{
            pthread_setname_np(pthread_self(), "http_service");
            try {
                ioc.run();
            } catch (std::exception& e) {
                std::cout << "ioc.run exist: " << e.what() << std::endl << std::flush;
                throw;
            }
            std::cout << "http_service exist" << std::endl << std::flush;
        });
    }
    for (auto& th : threads) {
        th.join();
    }
    std::cout << "runServer exist" << std::endl << std::flush;
}

void apply(rocksdb::DB* db, uint64_t index, const std::string& log) {
    int64_t num = std::stoi(log);
    int64_t new_num = 0;
    std::string val;
    auto s = db->Get(rocksdb::ReadOptions(), "count", &val);
    if (s.ok()) {
        new_num = num + std::stoi(val);
    } else if (s.IsNotFound()) {
        new_num = num;
    } else {
        ASSERT(false);
    }
    rocksdb::WriteBatch batch;
    batch.Put("index", std::to_string(index));
    batch.Put("count", std::to_string(new_num));
    s = db->Write({}, &batch);
    ASSERT(s.ok());
}

int main(int argc, char **argv) {
    gflags::ParseCommandLineFlags(&argc, &argv, true);
    SetupLogger();
    std::vector<std::thread> threads;
    std::unordered_map<std::string, std::shared_ptr<boost::asio::io_service>> services = {
        {"raft_service", std::make_shared<boost::asio::io_service>()},
        {"tick_service", std::make_shared<boost::asio::io_service>()},
    {"ready_service", std::make_shared<boost::asio::io_service>()},
        {"client_service", std::make_shared<boost::asio::io_service>()},
        {"raft_listener", std::make_shared<boost::asio::io_service>()}
    };
    threads.reserve(services.size());
    for (auto& [name, service]: services) {
        threads.emplace_back([name, &service]() {
            pthread_setname_np(pthread_self(), name.c_str());
            boost::asio::io_service::work holder(*service);
            service->run();
        });
    }

    std::vector<eraft::Peer> init_peers;
    if (!FLAGS_init_cluster.empty()) {
        auto cluster = nlohmann::json::parse(FLAGS_init_cluster);
        for (const auto& item : cluster) {
            eraft::Peer peer;
            peer.id_ = item["node_id"].get<int64_t>();
            peer.context_ = item.dump();
            init_peers.emplace_back(std::move(peer));
        }
    }
    rocksdb::Options options;
    options.create_if_missing = true;
    options.create_missing_column_families = true;
    auto s = rocksdb::DB::Open(options, "data", &data_db);
    ASSERT(s.ok());
    uint64_t apply_id = 0;
    std::string val;
    s = data_db->Get({}, "index", &val);
    if (s.ok()) {
        apply_id = std::stoi(val);
    }

    driver = std::make_unique<RaftDriver> (*services["raft_service"],
        *services["tick_service"],
        *services["ready_service"],
        *services["client_service"],
        [](uint64_t index, const std::string& log){apply(data_db, index, log);},
        apply_id,
        FLAGS_node_id,
        init_peers,
        "raftlog");
    auto err = driver->Run();
    if (err != nullptr) {
        LOG_ERROR("driver run error: {}", err.String());
        for (auto& [_,service]: services) {
            service->stop();
        }
        for (auto& t : threads) {
            t.join();
        }
        return -1;
    }

    std::function handler(protobuf_handler);
    kvstore::IOService<kvstore::ProtobufConnection, decltype(handler)> protobuf_io_service(
        *services["raft_listener"], FLAGS_http_port - 1, 1, handler);

    try {
        runServer();
    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl << std::flush;
    }
    for (auto& t : threads) {
        t.join();
    }
    return 0;
}