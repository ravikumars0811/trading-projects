#pragma once

#include <string>
#include <unordered_map>
#include <memory>
#include <stdexcept>

namespace hft {
namespace core {

/**
 * Configuration manager
 * Loads and manages system configuration
 */
class Config {
public:
    static Config& instance() {
        static Config instance;
        return instance;
    }

    void load(const std::string& config_file);

    std::string getString(const std::string& key, const std::string& default_val = "") const;
    int getInt(const std::string& key, int default_val = 0) const;
    double getDouble(const std::string& key, double default_val = 0.0) const;
    bool getBool(const std::string& key, bool default_val = false) const;

private:
    Config() = default;
    std::unordered_map<std::string, std::string> config_map_;
};

} // namespace core
} // namespace hft
