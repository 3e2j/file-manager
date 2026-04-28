#include "core/file_system.hpp"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <system_error>

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

		// Keep directories grouped before files, then sort by name.
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
		std::error_code error;
		if (std::filesystem::exists(path, error) || error) {
			return nullptr;
		}

		std::ofstream stream(path);
		if (!stream.is_open()) {
			return nullptr;
		}
		return std::make_shared<File>(path);
	}

	std::shared_ptr<Directory> FileSystem::createDirectory(const std::string &path) {
		std::error_code error;
		if (std::filesystem::exists(path, error) || error) {
			return nullptr;
		}
		std::filesystem::create_directories(path, error);
		if (error) {
			return nullptr;
		}
		return std::make_shared<Directory>(path);
	}

	bool FileSystem::deleteEntry(const std::string &path) {
		std::error_code error;
		if (!std::filesystem::exists(path, error) || error) {
			return false;
		}

		if (std::filesystem::is_directory(path, error)) {
			if (error) {
				return false;
			}
			return std::filesystem::remove_all(path, error) > 0 && !error;
		}

		return std::filesystem::remove(path, error) && !error;
	}

}
