#include "gateway/fix_protocol.hpp"
#include <sstream>
#include <iomanip>

namespace hft {
namespace gateway {

const std::string FIXMessage::MSG_TYPE_NEW_ORDER = "D";
const std::string FIXMessage::MSG_TYPE_CANCEL = "F";
const std::string FIXMessage::MSG_TYPE_MODIFY = "G";
const std::string FIXMessage::MSG_TYPE_EXECUTION_REPORT = "8";

void FIXMessage::setField(int tag, const std::string& value) {
    fields_[tag] = value;
}

std::string FIXMessage::getField(int tag) const {
    auto it = fields_.find(tag);
    return (it != fields_.end()) ? it->second : "";
}

bool FIXMessage::hasField(int tag) const {
    return fields_.find(tag) != fields_.end();
}

std::string FIXMessage::encode() const {
    std::ostringstream oss;

    // Add begin string
    oss << TAG_BEGIN_STRING << "=FIX.4.4" << '\x01';

    // Add all fields
    for (const auto& [tag, value] : fields_) {
        if (tag != TAG_BEGIN_STRING) {
            oss << tag << "=" << value << '\x01';
        }
    }

    std::string message = oss.str();

    // Calculate and append checksum
    std::string checksum = calculateChecksum(message);
    message += "10=" + checksum + '\x01';

    return message;
}

bool FIXMessage::decode(const std::string& raw_message) {
    fields_.clear();

    size_t pos = 0;
    while (pos < raw_message.length()) {
        size_t equal_pos = raw_message.find('=', pos);
        if (equal_pos == std::string::npos) break;

        size_t soh_pos = raw_message.find('\x01', equal_pos);
        if (soh_pos == std::string::npos) break;

        int tag = std::stoi(raw_message.substr(pos, equal_pos - pos));
        std::string value = raw_message.substr(equal_pos + 1, soh_pos - equal_pos - 1);

        fields_[tag] = value;

        pos = soh_pos + 1;
    }

    return !fields_.empty();
}

std::string FIXMessage::calculateChecksum(const std::string& message) const {
    int sum = 0;
    for (char c : message) {
        sum += static_cast<unsigned char>(c);
    }

    std::ostringstream oss;
    oss << std::setfill('0') << std::setw(3) << (sum % 256);
    return oss.str();
}

} // namespace gateway
} // namespace hft
