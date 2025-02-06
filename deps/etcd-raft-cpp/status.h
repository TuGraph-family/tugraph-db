#pragma once
#include "util.h"
#include "raftpb/raft.pb.h"
#include "tracker/tracker.h"
#include "raft.h"
namespace eraft {

// BasicStatus contains basic information about the Raft peer. It does not allocate.
struct BasicStatus {
	uint64_t id_ = 0;

	raftpb::HardState hardState_;
	SoftState softState_;

    uint64_t applied_ = 0;

	 uint64_t leadTransferee_ = 0;
};

// Status contains information about this Raft peer and its view of the system.
// The Progress is only populated on the leader.
struct Status {
	BasicStatus basicStatus_;
    tracker::Config config_;
    std::unordered_map<uint64_t, tracker::Progress> progress_;

    // MarshalJSON translates the raft status into JSON.
    // TODO: try to simplify this by introducing ID type into raft
    std::pair<std::string, Error> MarshalJSON() {
        auto j = format(R"({"id":"%x","term":%d,"vote":"%x","commit":%d,"lead":"%x","raftState":"%s","applied":%d,"progress":{)",
            basicStatus_.id_,
            basicStatus_.hardState_.term(),
            basicStatus_.hardState_.vote(),
            basicStatus_.hardState_.commit(),
            basicStatus_.softState_.lead_,
            ToString(basicStatus_.softState_.raftState_).c_str(),
            basicStatus_.applied_);

        if (progress_.empty()) {
            j += "},";
        } else {
            for (auto& pair : progress_) {
                auto subj = format(R"("%x":{"match":%d,"next":%d,"state":"%s"},)",
                                   pair.first,
                                   pair.second.match_,
                                   pair.second.next_,
                ToString(pair.second.state_).c_str());
                j += subj;
            }
            // remove the trailing ","
            j = j.substr(0, j.size()-1) + "},";
        }

        j += format(R"("leadtransferee":"%x"})", basicStatus_.leadTransferee_);
        return {j, {}};
    }
    std::string String() {
        std::string b;
        Error err;
        std::tie(b, err) = MarshalJSON();
        if (err != nullptr) {
            ERAFT_FATAL("unexpected error: %s", err.String().c_str());
        }
        return b;
    }
};
namespace detail {
inline std::unordered_map<uint64_t, tracker::Progress> getProgressCopy(raft& r) {
    std::unordered_map<uint64_t, tracker::Progress> m;
	r.trk_.Visit([&m](uint64_t id, std::shared_ptr<tracker::Progress>& pr) {
		auto p = *pr;
        p.inflights_ = pr->inflights_->Clone();
		m[id] = p;
	});
	return m;
}

inline BasicStatus getBasicStatus(raft& r) {
    BasicStatus s;
    s.id_ = r.id_;
    s.leadTransferee_ = r.leadTransferee_;
	s.hardState_ = r.hardState();
	s.softState_ = r.softState();
	s.applied_ = r.raftLog_->applied_;
	return s;
}

// getStatus gets a copy of the current raft status.
inline Status getStatus(raft& r) {
	Status s;
	s.basicStatus_ = getBasicStatus(r);
	if (s.basicStatus_.softState_.raftState_ == StateLeader) {
		s.progress_ = getProgressCopy(r);
	}
	s.config_ = r.trk_.config_.Clone();
	return s;
}
}

};