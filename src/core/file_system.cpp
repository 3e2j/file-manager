#include "file_manager/core/file_system.hpp"

#include <algorithm>
#include <filesystem>
#include <fstream>

namespace file_manager {

	FileSystem::FileSystem(const std::string &root_path) : root_(root_path) {}

	std::vector<std::shared_ptr<FileEntry>> FileSystem::list(const std::string &path) {
		std::vector<std::shared_ptr<FileEntry>> result;
		if (!std::filesystem::exists(path) || !std::filesystem::is_directory(path)) {
			return result;
		}

		root_.clearEntries();
		for (const auto &item : std::filesystem::directory_iterator(path)) {
			std::shared_ptr<FileEntry> entry;
			if (item.is_directory()) {
				entry = std::make_shared<Directory>(item.path().string());
			} else {
				entry = std::make_shared<File>(item.path().string());
			}
			root_.addEntry(entry);
			result.push_back(entry);
		}

		std::sort(result.begin(), result.end(),
			[](const std::shared_ptr<FileEntry> &left, const std::shared_ptr<FileEntry> &right) {
				if (left->isDirectory() != right->isDirectory()) {
					return left->isDirectory();
				}
				return left->getName() < right->getName();
			});
		return result;
	}

	std::shared_ptr<File> FileSystem::createFile(const std::string &path) {
		std::ofstream(path).close();
		return std::make_shared<File>(path);
	}

	std::shared_ptr<Directory> FileSystem::createDirectory(const std::string &path) {
		std::filesystem::create_directories(path);
		return std::make_shared<Directory>(path);
	}

}
