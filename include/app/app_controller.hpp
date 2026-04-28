#pragma once

#include <filesystem>
#include <string>
#include <vector>

#include "core/file_system.hpp"
#include "ui/ui.hpp"

namespace file_manager {

	class FileManager {
	  public:
		explicit FileManager(std::string start_path);

		void start();

	  private:
		void refresh();
		void navigate(const std::string &path);
		void createEntry();
		void deleteEntry();
		void executeFile(const std::string &explicit_path = "");

		bool canGoBack() const;
		void goBack();
		bool canGoForward() const;
		void goForward();
		std::filesystem::path resolvePath(const std::string &raw_path) const;

		static constexpr size_t kMaxHistory = 50;

		std::string current_path_;
		std::vector<std::string> history_;
		size_t history_index_ = 0;
		UI ui_;
		FileSystem fs_;
	};

}
