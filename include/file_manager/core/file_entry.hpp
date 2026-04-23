#pragma once

#include <memory>
#include <string>
#include <vector>

namespace file_manager {

	class FileEntry {
	  public:
		explicit FileEntry(const std::string &path);
		virtual ~FileEntry() = default;

		std::string getName() const;
		std::string getPath() const;
		std::string getModifiedTime() const;
		virtual bool isDirectory() const = 0;

	  protected:
		std::string name_;
		std::string path_;
		std::string created_time_;
		std::string modified_time_;
		std::string accessed_time_;
	};

	class File : public FileEntry {
	  public:
		explicit File(const std::string &path);

		void open() const;
		std::string getType() const;
		bool isDirectory() const override;

	  private:
		int size_;
		std::string type_;
	};

	class Directory : public FileEntry {
	  public:
		explicit Directory(const std::string &path);

		void addEntry(const std::shared_ptr<FileEntry> &entry);
		void removeEntry(const std::string &path);
		std::vector<std::shared_ptr<FileEntry>> listEntries() const;
		void clearEntries();
		bool isDirectory() const override;

	  private:
		std::vector<std::shared_ptr<FileEntry>> entries_;
	};

}
