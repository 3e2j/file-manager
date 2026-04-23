#pragma once

#include <chrono>
#include <filesystem>
#include <iostream>
#include <string>
#include <system_error>

// Helper for quick test setup & expect logic
namespace test_support {
    namespace fs = std::filesystem;

    struct TempDir {
    	fs::path path;

    	explicit TempDir(const std::string &prefix = "file-manager-tests-") {
    		const auto stamp = std::chrono::steady_clock::now().time_since_epoch().count();
    		path = fs::temp_directory_path() / (prefix + std::to_string(stamp));
    		fs::create_directories(path);
    	}

    	~TempDir() {
    		std::error_code error;
    		fs::remove_all(path, error);
    	}
    };

    inline bool expect(bool condition, const std::string &message) {
    	if (!condition) {
    		std::cerr << "FAIL: " << message << '\n';
    		return false;
    	}
    	return true;
    }
}

// Tests to be ran
bool run_test_file_entry();
bool run_test_file_system();
