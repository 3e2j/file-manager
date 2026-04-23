#pragma once

#include <memory>
#include <string>
#include <vector>

#include "file_manager/core/file_entry.hpp"

namespace file_manager {

	class FileSystem {
	  public:
		explicit FileSystem(const std::string &root_path);

		std::vector<std::shared_ptr<FileEntry>> list(const std::string &path);
		std::shared_ptr<File> createFile(const std::string &path);
		std::shared_ptr<Directory> createDirectory(const std::string &path);
		bool deleteEntry(const std::string &path);

	  private:
		Directory root_;
	};

}
