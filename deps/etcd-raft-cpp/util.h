#pragma once
#include <string>
#include <memory>
#include <sstream>
#include <random>
#include <cstdarg>
#include <cstring>
#include <iomanip>
#include <functional>
#include <utility>
#include <google/protobuf/util/message_differencer.h>
#include "raftpb/raft.pb.h"
namespace eraft {
class PanicException : public std::exception {
public:
    explicit PanicException(std::string msg) : msg_(std::move(msg)) {}
    [[nodiscard]] const char* what() const noexcept override {
        return msg_.c_str();
    }
private:
    std::string msg_;
};

void log_debug(const char *file, int line, const char *pattern, ...);
void log_info(const char *file, int line, const char *pattern, ...);
void log_warn(const char *file, int line, const char *pattern, ...);
void log_error(const char *file, int line, const char *pattern, ...);
void log_fatal(const char *file, int line, const char *pattern, ...);
#define ERAFT_DEBUG(msg, ...) log_debug(__FILE__, __LINE__, msg, ##__VA_ARGS__);
#define ERAFT_INFO(msg, ...)  log_info(__FILE__, __LINE__, msg, ##__VA_ARGS__);
#define ERAFT_WARN(msg, ...)  log_warn(__FILE__, __LINE__, msg, ##__VA_ARGS__);
#define ERAFT_ERROR(msg, ...) log_error(__FILE__, __LINE__, msg, ##__VA_ARGS__);
#define ERAFT_FATAL(msg, ...) log_fatal(__FILE__, __LINE__, msg, ##__VA_ARGS__);

#ifdef RAFT_LOG
enum class LOG_LEVEL {
    FATAL = 0,
    ERROR,
    WARN,
    INFO,
    DEBUG
};
const char* const log_level_to_str[] = {"FATAL", "ERROR", "WARN", "INFO", "DEBUG"};
inline void logger(LOG_LEVEL log_level, const char *file, int line, const char *pattern, va_list args) {
    char time[20] = {0};
    char final_content[1024] = {0};

    struct timespec ts{};
    struct tm       now{};
    clock_gettime(CLOCK_REALTIME, &ts);
    localtime_r(&ts.tv_sec, &now);
    std::snprintf(time, sizeof(time), "%.4d%.2d%.2d %.2d:%.2d:%.2d %.3ld",
                  now.tm_year + 1900, now.tm_mon + 1, now.tm_mday,
                  now.tm_hour, now.tm_min, now.tm_sec, ts.tv_nsec/1000000);

    const char *fn = std::strrchr(file, '/');
    fn = fn ? fn + 1 : file;

    uint32_t out_end = std::snprintf(final_content, sizeof(final_content), "%s [%s] %s:%u ", time, log_level_to_str[static_cast<uint64_t>(log_level)], fn, line);
    vsnprintf(final_content + out_end, sizeof(final_content) - out_end -1, pattern, args);

    std::cout << final_content << std::endl;
    if (log_level == LOG_LEVEL::FATAL) {
        throw PanicException("panic");
    }
}
inline void log_debug(const char *file, int line, const char *pattern, ...) {
    va_list args;
    va_start(args, pattern);
    logger(LOG_LEVEL::DEBUG, file, line, pattern, args);
    va_end(args);
}
inline void log_info(const char *file, int line, const char *pattern, ...) {
    va_list args;
    va_start(args, pattern);
    logger(LOG_LEVEL::INFO, file, line, pattern, args);
    va_end(args);
}
inline void log_warn(const char *file, int line, const char *pattern, ...) {
    va_list args;
    va_start(args, pattern);
    logger(LOG_LEVEL::WARN, file, line, pattern, args);
    va_end(args);
}
inline void log_error(const char *file, int line, const char *pattern, ...) {
    va_list args;
    va_start(args, pattern);
    logger(LOG_LEVEL::ERROR, file, line, pattern, args);
    va_end(args);
}
inline void log_fatal(const char *file, int line, const char *pattern, ...) {
    va_list args;
    va_start(args, pattern);
    logger(LOG_LEVEL::FATAL, file, line, pattern, args);
    va_end(args);
}
#endif

template<typename ... Args>
inline std::string format(const std::string &format, Args ... args) {
    int size_s = std::snprintf(nullptr, 0, format.c_str(), args ...) + 1; // Extra space for '\0'
    if (size_s <= 0) { throw std::runtime_error("Error during formatting."); }
    auto size = static_cast<size_t>( size_s );
    std::unique_ptr<char[]> buf(new char[size]);
    std::snprintf(buf.get(), size, format.c_str(), args ...);
    return {buf.get(), buf.get() + size - 1}; // We don't want the '\0' inside
}

inline std::string ToHex(const std::string& str) {
    std::stringstream ret;
    std::for_each(str.begin(), str.end(),[&ret](int byte) {
        if ((byte >= '!' && byte <= '~') || byte == ' ') {
          ret << (char)byte;
        } else {
          ret << "\\x" << std::setfill ('0') << std::setw(2) << std::hex << byte;
        }
    });
    return ret.str();
}

class Error {
public:
    enum Code : unsigned char {
        Ok = 0,
        Compacted,
        SnapOutOfDate,
        Unavailable,
        SnapshotTemporarilyUnavailable,
        Unknown,
    };

    Error() : code_(Ok) {}

    Error(std::nullptr_t) : code_(Ok) {}

    explicit Error(std::string msg)
            : code_(Unknown), msg_(std::move(msg)) {}

    explicit Error(Code code, std::string msg) : code_(code), msg_(std::move(msg)) {}

    Error(Error &&e) = default;

    Error &operator=(const Error &e) = default;

    Error &operator=(std::nullptr_t) {
        code_ = Ok;
        msg_.clear();
        return *this;
    }

    bool operator==(const Error &e) const {
        if (code_ == Unknown) {
            if (msg_ == e.msg_) {
                return true;
            } else {
                return false;
            }
        } else {
            if (code_ == e.code_) {
                return true;
            } else {
                return false;
            }
        }
    }

    bool operator!= (nullptr_t) const {
        return code_ != Ok;
    }

    bool operator!= (const Error& err) const {
        return code_ != err.code_;
    }

    bool operator== (nullptr_t) const {
        return code_ == Ok;
    }

    Error(const Error &e) = default;

    [[nodiscard]] bool ok() const { return code_ == Ok; }

    [[nodiscard]] std::string String() const {
        return msg_;
    }

private:
    Code code_;
    std::string msg_;
};

template <typename Range, typename Value = typename Range::value_type>
inline std::string Join(Range const& elements, const char *const delimiter) {
    std::ostringstream os;
    auto b = std::begin(elements), e = std::end(elements);

    if (b != e) {
        std::copy(b, std::prev(e), std::ostream_iterator<Value>(os, delimiter));
        b = std::prev(e);
    }
    if (b != e) {
        os << *b;
    }

    return os.str();
}

inline bool IsLocalMsg(raftpb::MessageType msgt) {
    return msgt == raftpb::MsgHup ||
           msgt == raftpb::MsgBeat ||
           msgt == raftpb::MsgUnreachable ||
           msgt == raftpb::MsgSnapStatus ||
           msgt == raftpb::MsgCheckQuorum ||
           msgt == raftpb::MsgStorageAppend ||
           msgt == raftpb::MsgStorageAppendResp ||
           msgt == raftpb::MsgStorageApply ||
           msgt == raftpb::MsgStorageApplyResp;
}

inline bool IsResponseMsg(raftpb::MessageType msgt) {
    return msgt == raftpb::MsgAppResp ||
           msgt == raftpb::MsgVoteResp ||
           msgt == raftpb::MsgHeartbeatResp ||
           msgt == raftpb::MsgUnreachable ||
           msgt == raftpb::MsgReadIndexResp ||
           msgt == raftpb::MsgPreVoteResp ||
           msgt == raftpb::MsgStorageAppendResp ||
           msgt == raftpb::MsgStorageApplyResp;
}

// None is a placeholder node ID used when there is no leader.
const uint64_t None = 0;
// LocalAppendThread is a reference to a local thread that saves unstable
// log entries and snapshots to stable storage. The identifier is used as a
// target for MsgStorageAppend messages when AsyncStorageWrites is enabled.
const uint64_t LocalAppendThread = std::numeric_limits<uint64_t>::max();
// LocalApplyThread is a reference to a local thread that applies committed
// log entries to the local state machine. The identifier is used as a
// target for MsgStorageApply messages when AsyncStorageWrites is enabled.
const uint64_t LocalApplyThread = std::numeric_limits<uint64_t>::max() - 1;

inline bool IsLocalMsgTarget(uint64_t id) {
    return id == LocalAppendThread || id == LocalApplyThread;
};

inline std::string describeTarget(uint64_t id) {
	switch (id) {
	case None:
		return "None";
	case LocalAppendThread:
		return "AppendThread";
	case LocalApplyThread:
		return "ApplyThread";
	default:
		return format("%x", id);
	}
}

// voteResponseType maps vote and prevote message types to their corresponding responses.
inline raftpb::MessageType voteRespMsgType(raftpb::MessageType msgt) {
	switch (msgt) {
        case raftpb::MsgVote:
		    return raftpb::MsgVoteResp;
        case raftpb::MsgPreVote:
		    return raftpb::MsgPreVoteResp;
	    default:
		    ERAFT_FATAL("not a vote message: %s", raftpb::MessageType_Name(msgt).c_str());
            return msgt;
	}
}

// trim from start (in place)
inline void ltrim(std::string &s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
        return !std::isspace(ch);
    }));
}

// trim from end (in place)
inline void rtrim(std::string &s) {
    s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
        return !std::isspace(ch);
    }).base(), s.end());
}

// trim from both ends (in place)
inline void trim(std::string &s) {
    ltrim(s);
    rtrim(s);
}

inline std::vector<std::string> split(const std::string &str, char delim) {
    std::vector<std::string> tokens;
    std::string token;
    std::stringstream ss(str);
    while (getline(ss, token, delim)) {
        tokens.push_back(std::move(token));
    }
    return tokens;
}

// EntryFormatter can be implemented by the application to provide human-readable formatting
// of entry data. Nil is a valid EntryFormatter and will use a default format.
typedef std::function<std::string(const std::string&)> EntryFormatter;

// IsEmptySnap returns true if the given Snapshot is empty.
inline bool IsEmptySnap(const raftpb::Snapshot& sp) {
    return sp.metadata().index() == 0;
}

// limitSize returns the longest prefix of the given entries slice, such that
// its total byte size does not exceed maxSize. Always returns a non-empty slice
// if the input is non-empty, so, as an exception, if the size of the first
// entry exceeds maxSize, a non-empty slice with just this entry is returned.
inline void limitSize(std::vector<raftpb::Entry> &ents, uint64_t maxSize) {
    if (ents.empty()) {
        return;
    }
    auto size = ents[0].ByteSizeLong();
    for (size_t limit = 1; limit < ents.size(); limit++) {
        size += ents[limit].ByteSizeLong();
        if (size > maxSize) {
            ents.erase(ents.begin() + (int64_t) limit, ents.end());
            return;
        }
    }
}

// PayloadSize is the size of the payload of this Entry. Notably, it does not
// depend on its Index or Term.
inline uint64_t payloadSize(const raftpb::Entry& e) {
    return e.data().size();
}

// payloadsSize is the size of the payloads of the provided entries.
template<class T>
inline uint64_t payloadsSize(const T& ents) {
	uint64_t s = 0;
    for (auto& e : ents) {
		s += payloadSize(e);
	}
	return s;
}

inline bool isHardStateEqual(const raftpb::HardState& a, const raftpb::HardState& b) {
    return a.term() == b.term() && a.vote() == b.vote() && a.commit() == b.commit();
}

// IsEmptyHardState returns true if the given HardState is empty.
inline bool IsEmptyHardState(const raftpb::HardState& st) {
    static raftpb::HardState empty;
    return isHardStateEqual(st, empty);
}

template<class T>
uint64_t entsSize(const T& ents) {
	uint64_t size = 0;
    for (auto& ent : ents) {
        size += ent.ByteSizeLong();
	}
	return size;
}

const uint64_t noLimit = std::numeric_limits<uint64_t>::max();

inline int64_t random(int64_t n) {
    std::random_device rd;
    std::mt19937 mt(rd());
    std::uniform_int_distribution<int64_t> dist(0, n - 1);
    return dist(mt);
}

struct EntryHelper {
    uint64_t index = 0;
    uint64_t term = 0;
    raftpb::EntryType type = raftpb::EntryType::EntryNormal;
    std::string data = {};
    [[nodiscard]] raftpb::Entry Done() const {
        raftpb::Entry entry;
        entry.set_index(index);
        entry.set_term(term);
        entry.set_type(type);
        entry.set_data(data);
        return entry;
    }
};

struct SnapshotHelper {
    std::string data = {};
    uint64_t  index = 0;
    uint64_t term = 0;
    raftpb::ConfState cs = {};
    raftpb::Snapshot Done() const {
        raftpb::Snapshot snap;
        snap.set_data(data);
        snap.mutable_metadata()->set_index(index);
        snap.mutable_metadata()->set_term(term);
        *snap.mutable_metadata()->mutable_conf_state() = cs;
        return snap;
    }
};

struct MessageHelper {
    uint64_t from = 0;
    uint64_t to = 0;
    uint64_t term = 0;
    raftpb::MessageType type = {};
    std::vector<raftpb::Entry> entries = {};
    [[nodiscard]] raftpb::Message Done() const {
        raftpb::Message m;
        m.set_from(from);
        m.set_to(to);
        m.set_term(term);
        m.set_type(type);
        *m.mutable_entries() = {entries.begin(), entries.end()};
        return m;
    }
};

template<class T>
inline bool VectorEquals(const std::vector<T>& left, const std::vector<T>& right) {
    if (left.size() != right.size()) {
        return false;
    }
    for (size_t i = 0; i < left.size(); i++) {
        if (!google::protobuf::util::MessageDifferencer::Equals(left[i], right[i])) {
            return false;
        }
    }
    return true;
}

}
