#include "file_manager/app/app_controller.hpp"

#include <filesystem>

namespace file_manager {

	FileManager::FileManager(std::string start_path)
		: current_path_(std::move(start_path)), fs_(current_path_) {
		ui_.setOnCreate([this] { createEntry(); });
		ui_.setOnDelete([this] { deleteEntry(); });
		ui_.setOnRefresh([this] { refresh(); });
		ui_.setOnBack([this] { goBack(); });
		ui_.setOnPathPartClicked([this](const std::string &path) { navigate(path); });
		ui_.setOnEntryActivated([this](const std::string &path, bool is_directory) {
			if (is_directory) {
				navigate(path);
				return;
			}
			executeFile(path);
		});
	}

	void FileManager::start() {
		// Initialize navigation state from the canonical startup path.
		current_path_ =
			std::filesystem::weakly_canonical(std::filesystem::path(current_path_)).string();
		history_.push_back(current_path_);
		history_index_ = 0;
		refresh();
		ui_.show();
	}

	void FileManager::refresh() {
		ui_.showDirectory(fs_.list(current_path_), current_path_, canGoBack());
	}

	void FileManager::navigate(const std::string &path) {
		const std::filesystem::path candidate = resolvePath(path);
		if (!std::filesystem::exists(candidate) || !std::filesystem::is_directory(candidate)) {
			ui_.showMessage("Directory does not exist.");
			return;
		}
		const std::string next_path = std::filesystem::weakly_canonical(candidate).string();
		if (next_path == current_path_) {
			return;
		}
		current_path_ = next_path;
		// Trim forward history when navigating from a previous location.
		if (history_index_ + 1 < history_.size()) {
			history_.erase(
				history_.begin() + static_cast<long>(history_index_ + 1), history_.end());
		}
		history_.push_back(current_path_);

		// Prevent prolonged sessions from infinitely increasing memory usage
		if (history_.size() > kMaxHistory) {
            const size_t to_drop = history_.size() - kMaxHistory;
            history_.erase(history_.begin(), history_.begin() + static_cast<long>(to_drop));
        }

		history_index_ = history_.size() - 1;
		refresh();
	}

	void FileManager::createEntry() {
		const std::string name = ui_.getUserInput("Entry name (append / for directory)");
		if (name.empty()) {
			ui_.showMessage("Entry name cannot be empty.");
			return;
		}

		std::string normalized = name;
		bool create_directory = false;
		while (!normalized.empty() && (normalized.back() == '/' || normalized.back() == '\\')) {
			create_directory = true;
			normalized.pop_back();
		}

		if (normalized.empty()) {
			ui_.showMessage("Entry name cannot be empty.");
			return;
		}

		const std::filesystem::path target = std::filesystem::path(current_path_) / normalized;
		if (std::filesystem::exists(target)) {
			ui_.showMessage("Entry already exists.");
			return;
		}

		if (create_directory) {
			if (!fs_.createDirectory(target.string())) {
				ui_.showMessage("Failed to create directory.");
				return;
			}
		} else {
			if (!fs_.createFile(target.string())) {
				ui_.showMessage("Failed to create file.");
				return;
			}
		}
		refresh();
	}

	void FileManager::deleteEntry() {
		bool unused = false;
		std::string selected = ui_.selectedEntryPath(&unused);
		if (selected.empty()) {
			selected = ui_.getUserInput("Delete entry name/path");
		}
		if (selected.empty()) {
			ui_.showMessage("Entry name cannot be empty.");
			return;
		}

		const std::filesystem::path target = resolvePath(selected);
		if (!std::filesystem::exists(target)) {
			ui_.showMessage("Entry does not exist.");
			return;
		}

		if (!ui_.moveToTrash(target.string())) {
			ui_.showMessage("Failed to move entry to trash.");
			return;
		}
		refresh();
	}

	void FileManager::executeFile(const std::string &explicit_path) {
		std::string chosen_path = explicit_path;
		bool is_directory = false;
		if (chosen_path.empty()) {
			chosen_path = ui_.selectedEntryPath(&is_directory);
			if (chosen_path.empty()) {
				chosen_path = ui_.getUserInput("Execute file name/path");
			}
		}
		if (chosen_path.empty()) {
			ui_.showMessage("File name cannot be empty.");
			return;
		}

		const std::filesystem::path target = resolvePath(chosen_path);
		if (!std::filesystem::exists(target) || std::filesystem::is_directory(target)) {
			ui_.showMessage("Target is not a file.");
			return;
		}

		if (!ui_.openPath(target.string())) {
			ui_.showMessage("Failed to open file.");
			return;
		}
	}

	bool FileManager::canGoBack() const { return history_index_ > 0; }

	void FileManager::goBack() {
		if (!canGoBack()) {
			return;
		}
		--history_index_;
		current_path_ = history_[history_index_];
		refresh();
	}

	std::filesystem::path FileManager::resolvePath(const std::string &raw_path) const {
		std::filesystem::path candidate(raw_path);
		if (candidate.is_relative()) {
			candidate = std::filesystem::path(current_path_) / candidate;
		}
		return candidate;
	}
}
