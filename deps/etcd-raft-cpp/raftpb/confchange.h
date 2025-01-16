#pragma once

#include <google/protobuf/util/message_differencer.h>
#include "../util.h"
#include "../raftpb/raft.pb.h"

namespace raftpb {
using namespace eraft;
// ConfChangeI abstracts over ConfChangeV2 and (legacy) ConfChange to allow
// treating them in a unified manner.
struct ConfChangeI {
    [[nodiscard]] virtual ConfChangeV2 AsV2() const = 0;
    [[nodiscard]] virtual std::pair<ConfChange, bool> AsV1() const = 0;
    virtual ~ConfChangeI() = default;
    [[nodiscard]] virtual std::string String() const = 0;
};

struct ConfChangeWrap : ConfChangeI {
    ConfChange data_;
    explicit ConfChangeWrap(ConfChange cf) : data_(std::move(cf)) {}
    // AsV2 returns a V2 configuration change carrying out the same operation.
    ConfChangeV2 AsV2() const override {
        ConfChangeV2 ret;
        ret.set_context(data_.context());
        auto ccs = ret.mutable_changes()->Add();
        ccs->set_type(data_.type());
        ccs->set_node_id(data_.node_id());
        return ret;
    };
    // AsV1 returns the ConfChange and true.
    std::pair<ConfChange, bool> AsV1() const override {
        return {data_, true};
    };
    std::string String() const override {
        return data_.SerializeAsString();
    }
};

struct ConfChangeV2Wrap : ConfChangeI {
    ConfChangeV2 data_;
    ConfChangeV2& data() {return data_;}
    explicit ConfChangeV2Wrap(ConfChangeV2 cf) : data_(std::move(cf)) {}
    // AsV2 is the identity.
    ConfChangeV2 AsV2() const override {return data_;};
    // AsV1 returns ConfChange{} and false.
    std::pair<ConfChange, bool> AsV1() const override {
        return {{}, false};
    };

    // EnterJoint returns two bools. The second bool is true if and only if this
    // config change will use Joint Consensus, which is the case if it contains more
    // than one change or if the use of Joint Consensus was requested explicitly.
    // The first bool can only be true if second one is, and indicates whether the
    // Joint State will be left automatically.
    std::pair<bool,bool> EnterJoint() {
        // NB: in theory, more config changes could qualify for the "simple"
        // protocol but it depends on the config on top of which the changes apply.
        // For example, adding two learners is not OK if both nodes are part of the
        // base config (i.e. two voters are turned into learners in the process of
        // applying the conf change). In practice, these distinctions should not
        // matter, so we keep it simple and use Joint Consensus liberally.
        if (data_.transition() != ConfChangeTransitionAuto || data_.changes().size() > 1) {
            // Use Joint Consensus.
            bool autoLeave = false;
            switch (data_.transition()) {
                case ConfChangeTransitionAuto:
                case ConfChangeTransitionJointImplicit:
                    autoLeave = true;
                    break;
                case ConfChangeTransitionJointExplicit:
                    break;
                default:
                    ERAFT_FATAL("unknown transition: %s", data_.ShortDebugString().c_str());
            }
            return {autoLeave, true};
        }
        return {false, false};
    }

    // LeaveJoint is true if the configuration change leaves a joint configuration.
    // This is the case if the ConfChangeV2 is zero, with the possible exception of
    // the Context field.
    bool LeaveJoint() {
        // TODO(wangzhiyong)
        data_.clear_context();
        return google::protobuf::util::MessageDifferencer::Equals(data_, ConfChangeV2());
    }

    std::string String() const override {
        return data_.SerializeAsString();
    }
};

// MarshalConfChange calls Marshal on the underlying ConfChange or ConfChangeV2
// and returns the result along with the corresponding EntryType.
inline std::tuple<EntryType, std::string, Error> MarshalConfChange(ConfChangeI* c) {
	EntryType typ;
	std::string ccdata;
	Error err;
    if (c == nullptr) {
        // A nil data unmarshals into an empty ConfChangeV2 and has the benefit
        // that appendEntry can never refuse it based on its size (which
        // registers as zero).
        typ = EntryConfChangeV2;
    } else {
        ConfChange ccv1;
        bool ok = false;
        std::tie(ccv1, ok) = c->AsV1();
        if (ok) {
            typ = EntryConfChange;
            if (!ccv1.SerializeToString(&ccdata)) {
                err = Error("ConfChange.SerializeToString error");
            }
        } else {
            auto ccv2 = c->AsV2();
            typ = EntryConfChangeV2;
            if (!ccv2.SerializeToString(&ccdata)) {
                err = Error("ConfChangeV2.SerializeToString error");
            }
        }
    }
	return std::make_tuple(typ, ccdata, err);
}

// ConfChangesFromString parses a Space-delimited sequence of operations into a
// slice of ConfChangeSingle. The supported operations are:
// - vn: make n a voter,
// - ln: make n a learner,
// - rn: remove n, and
// - un: update n.
inline std::pair<std::vector<ConfChangeSingle>, Error> ConfChangesFromString(std::string s) {
    std::vector<ConfChangeSingle> ccs;
    trim(s);
    auto toks = split(s, ' ');
	if (toks[0].empty()) {
        toks.clear();
	}
    for (auto& tok : toks) {
		if (tok.size() < 2) {
			return {{}, Error(format("unknown token %s", tok.c_str()))};
		}
		ConfChangeSingle cc;
		switch (tok[0]) {
		case 'v':
			cc.set_type(ConfChangeAddNode);
            break;
		case 'l':
            cc.set_type(ConfChangeAddLearnerNode);
            break;
		case 'r':
            cc.set_type(ConfChangeRemoveNode);
            break;
		case 'u':
            cc.set_type(ConfChangeUpdateNode);
            break;
		default:
			return {{}, Error(format("unknown input: %s", tok.c_str()))};
		}
        uint64_t id;
        try {
            id = std::stoull(tok.substr(1));
        } catch(...) {
            return {{}, Error("stoull error")};
        }
        cc.set_node_id(id);
        ccs.push_back(std::move(cc));
	}
	return {ccs, Error()};
}

// ConfChangesToString is the inverse to ConfChangesFromString.
inline std::string ConfChangesToString(std::vector<ConfChangeSingle> ccs) {
	std::string buf;
    for (size_t i = 0; i < ccs.size(); i++) {
		if (i > 0) {
            buf.push_back(' ');
		}
		switch (ccs[i].type()) {
            case ConfChangeAddNode:
                buf.push_back('v');
                break;
            case ConfChangeAddLearnerNode:
                buf.push_back('l');
                break;
            case ConfChangeRemoveNode:
                buf.push_back('r');
                break;
            case ConfChangeUpdateNode:
                buf.push_back('u');
                break;
            default:
                buf.append("unknown");
		}
		buf.append(format("%d", ccs[i].node_id()));
	}
	return buf;
}

namespace detail {
inline std::pair<raftpb::Message, Error> confChangeToMsg(ConfChangeI* c) {
    EntryType typ;
    std::string data;
    Error err;
    std::tie(typ, data, err) = MarshalConfChange(c);
    if (err != nullptr) {
        return {{}, err};
    }
    raftpb::Message ret;
    ret.set_type(raftpb::MsgProp);
    auto entry = ret.mutable_entries()->Add();
    entry->set_type(typ);
    entry->set_data(data);
    return {std::move(ret), {}};
}
}

}