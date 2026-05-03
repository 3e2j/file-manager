#include "core/file_entry.hpp"

#include <chrono>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <sys/stat.h>

namespace file_manager {

	struct FileTimestamps {
		std::string created = "N/A";
		std::string modified = "N/A";
		std::string accessed = "N/A";
	};

	// Convert to local (zoned) time and return formatted string
	static std::string TimeTToString(std::time_t raw_time) {
        auto tp = std::chrono::system_clock::from_time_t(raw_time);
        auto zt = std::chrono::zoned_time{std::chrono::current_zone(), std::chrono::floor<std::chrono::seconds>(tp)};
        return std::format("{:%Y-%m-%d %H:%M:%S}", zt);
    }

	static FileTimestamps ReadFileTimestamps(const std::filesystem::path &fs_path) {
		FileTimestamps timestamps;
		if (!std::filesystem::exists(fs_path)) {
			return timestamps;
		}

		struct stat file_stat {};
		if (stat(fs_path.c_str(), &file_stat) == 0) {
			timestamps.created = TimeTToString(file_stat.st_ctime);
			timestamps.accessed = TimeTToString(file_stat.st_atime);
			timestamps.modified = TimeTToString(file_stat.st_mtime);
		}
		return timestamps;
	}

	FileEntry::FileEntry(const std::string &path) : path_(path) {
		const std::filesystem::path fs_path(path_);
		name_ = fs_path.filename().string();
		if (name_.empty()) {
			name_ = path_;
		}

		const FileTimestamps times = ReadFileTimestamps(fs_path);
		created_time_ = times.created;
		modified_time_ = times.modified;
		accessed_time_ = times.accessed;
	}

	std::string FileEntry::getName() const { return name_; }
	std::string FileEntry::getPath() const { return path_; }
	std::string FileEntry::getCreatedTime() const { return created_time_; }
	std::string FileEntry::getModifiedTime() const { return modified_time_; }
	std::string FileEntry::getAccessedTime() const { return accessed_time_; }

	File::File(const std::string &path) : FileEntry(path) {
		if (std::filesystem::exists(path_)) {
			size_ = static_cast<int>(std::filesystem::file_size(path_));
		} else {
			size_ = 0;
		}
		type_ = std::filesystem::path(path_).extension().string();
		if (type_.empty()) {
			type_ = "unknown";
		}
	}

	void File::open() const {
		std::ifstream stream(path_);
		if (!stream) {
			return;
		}
	}

	std::string File::getType() const { return type_; }
	bool File::isDirectory() const { return false; }

	Directory::Directory(const std::string &path) : FileEntry(path) {}

	void Directory::addEntry(const std::shared_ptr<FileEntry> &entry) { entries_.push_back(entry); }

	void Directory::removeEntry(const std::string &path) {
		// Find and remove the entry with matching path
		for (size_t i = 0; i < entries_.size(); ++i) {
			if (entries_[i]->getPath() == path) {
				entries_.erase(entries_.begin() + i);
				return;
			}
		}
	}

	std::vector<std::shared_ptr<FileEntry>> Directory::listEntries() const { return entries_; }
	void Directory::clearEntries() { entries_.clear(); }
	bool Directory::isDirectory() const { return true; }

}
