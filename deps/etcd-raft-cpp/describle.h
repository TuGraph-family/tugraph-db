#pragma once
#include "util.h"
#include "raftpb/confchange.h"

namespace eraft {

inline std::string DescribeConfState(const raftpb::ConfState& state) {
	return format(
		"Voters:[%s] VotersOutgoing:[%s] Learners:[%s] LearnersNext:[%s] AutoLeave:%s",
        eraft::Join(state.voters(), ",").c_str(),
        eraft::Join(state.voters_outgoing(), ",").c_str(),
        eraft::Join(state.learners(),",").c_str(),
        eraft::Join(state.learners_next(),",").c_str(),
        state.auto_leave() ? "true" : "false"
	);
}

inline std::string DescribeSnapshot(const raftpb::Snapshot& snap) {
	auto& m = snap.metadata();
	return format("Index:%d Term:%d ConfState:%s", m.index(), m.term(), DescribeConfState(m.conf_state()).c_str());
}

// DescribeEntry returns a concise human-readable description of an
// Entry for debugging.
inline std::string DescribeEntry(const raftpb::Entry& e, EntryFormatter f) {
    if (f == nullptr) {
        f = [](const std::string& data){ return format("\"%s\"", ToHex(data).c_str()); };
    }

    auto formatConfChange = [](raftpb::ConfChangeI& cc) {
        // TODO(tbg): give the EntryFormatter a type argument so that it gets
        // a chance to expose the Context.
        auto& changes = cc.AsV2().changes();
        return raftpb::ConfChangesToString({changes.begin(), changes.end()});
    };

    std::string formatted;
    switch (e.type()) {
        case raftpb::EntryNormal: {
            formatted = f(e.data());
            break;
        }
        case raftpb::EntryConfChange: {
            raftpb::ConfChange cc;
            cc.ParseFromString(e.data());
            raftpb::ConfChangeWrap ccWarp(std::move(cc));
            formatted = formatConfChange(ccWarp);
            break;
        }
        case raftpb::EntryConfChangeV2: {
            raftpb::ConfChangeV2 cc;
            cc.ParseFromString(e.data());
            raftpb::ConfChangeV2Wrap ccWarp(std::move(cc));
            formatted = formatConfChange(ccWarp);
            break;
        }
        default: {
            break;
        }
    }
    if (!formatted.empty()) {
        formatted = " " + formatted;
    }
    return format("%d/%d %s%s", e.term(), e.index(), raftpb::EntryType_Name(e.type()).c_str(), formatted.c_str());
}

// DescribeMessage returns a concise human-readable description of a
// Message for debugging.
inline std::string DescribeMessage(const raftpb::Message& m, const EntryFormatter& f) {
	std::string buf;
	buf.append(format("%s->%s %v Term:%d Log:%d/%d", describeTarget(m.from()).c_str(), describeTarget(m.to()).c_str(), m.type(), m.term(), m.logterm(), m.index()));
	if (m.reject()) {
		buf.append(format(" Rejected (Hint: %d)", m.rejecthint()));
	}
	if (m.commit() != 0) {
		buf.append(format(" Commit:%d", m.commit()));
	}
    if (m.vote() != 0) {
        buf.append(format(" Vote:%d", m.vote()));
    }
	if (!m.entries().empty()) {
		buf.append(" Entries:[");
        for (int i = 0; i < m.entries().size(); i++) {
			if (i != 0) {
				buf.append(", ");
			}
			buf.append(DescribeEntry(m.entries(i), f));
		}
        buf.append("]");
	}
	if (!IsEmptySnap(m.snapshot())) {
        buf.append(format(" Snapshot: %s", DescribeSnapshot(m.snapshot()).c_str()));
	}

    if (!m.responses().empty()) {
        buf.append(" Responses:[");
        for (int i = 0; i < m.responses().size(); i++) {
            if (i != 0) {
                buf.append(", ");
            }
            buf.append(DescribeMessage(m.responses(i), f));
        }
        buf.append("]");
    }

	return buf;
}

};