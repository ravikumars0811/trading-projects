#pragma once

#include <string>
#include <unordered_map>
#include <cstdint>

namespace hft {
namespace gateway {

/**
 * Simplified FIX protocol implementation
 * For production use, consider using QuickFIX or similar library
 */
class FIXMessage {
public:
    FIXMessage() = default;

    void setField(int tag, const std::string& value);
    std::string getField(int tag) const;
    bool hasField(int tag) const;

    std::string encode() const;
    bool decode(const std::string& raw_message);

    // Common FIX tags
    static constexpr int TAG_BEGIN_STRING = 8;
    static constexpr int TAG_MSG_TYPE = 35;
    static constexpr int TAG_SENDER_COMP_ID = 49;
    static constexpr int TAG_TARGET_COMP_ID = 56;
    static constexpr int TAG_MSG_SEQ_NUM = 34;
    static constexpr int TAG_SENDING_TIME = 52;
    static constexpr int TAG_CL_ORD_ID = 11;
    static constexpr int TAG_SYMBOL = 55;
    static constexpr int TAG_SIDE = 54;
    static constexpr int TAG_ORDER_QTY = 38;
    static constexpr int TAG_PRICE = 44;
    static constexpr int TAG_ORD_TYPE = 40;

    // Message types
    static const std::string MSG_TYPE_NEW_ORDER;
    static const std::string MSG_TYPE_CANCEL;
    static const std::string MSG_TYPE_MODIFY;
    static const std::string MSG_TYPE_EXECUTION_REPORT;

private:
    std::string calculateChecksum(const std::string& message) const;

    std::unordered_map<int, std::string> fields_;
};

} // namespace gateway
} // namespace hft
