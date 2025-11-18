#include "obs/async_executor.hpp"
#include <nlohmann/json.hpp>
#include <fstream>

namespace obs::utils {

// Configuration loading utilities

class ConfigLoader {
public:
    static nlohmann::json load(const std::string& filename) {
        std::ifstream file(filename);
        if (!file.is_open()) {
            return nlohmann::json::object();
        }

        nlohmann::json config;
        file >> config;
        return config;
    }
};

} // namespace obs::utils
