#pragma once

#include <QHBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QPushButton>
#include <QWidget>

#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace file_manager {

	class FileEntry;

	class UI : public QWidget {
	  public:
		UI();

		void setOnCreate(const std::function<void()> &callback);
		void setOnDelete(const std::function<void()> &callback);
		void setOnRefresh(const std::function<void()> &callback);
		void setOnBack(const std::function<void()> &callback);
		void setOnPathPartClicked(const std::function<void(const std::string &)> &callback);
		void setOnEntryActivated(const std::function<void(const std::string &, bool)> &callback);

		void displayMenu();
		std::string getUserInput(const std::string &prompt = "Input");
		void showDirectory(const std::vector<std::shared_ptr<FileEntry>> &entries,
			const std::string &path, bool can_go_back);
		void showMessage(const std::string &message);
		std::string selectedEntryPath(bool *is_directory = nullptr) const;
		bool moveToTrash(const std::string &path) const;
		bool openPath(const std::string &path) const;

	  private:
		void clearBreadcrumbs();
		void renderBreadcrumbs(const std::string &path);

		static constexpr int kPathRole = Qt::UserRole + 1;
		static constexpr int kDirRole = Qt::UserRole + 2;

		std::string input_;
		std::string output_;
		QPushButton *back_button_ = nullptr;
		QHBoxLayout *breadcrumb_layout_ = nullptr;
		QListWidget *entry_list_ = nullptr;
		QLabel *status_label_ = nullptr;
		std::function<void()> on_create_;
		std::function<void()> on_delete_;
		std::function<void()> on_refresh_;
		std::function<void()> on_back_;
		std::function<void(const std::string &)> on_path_part_clicked_;
		std::function<void(const std::string &, bool)> on_entry_activated_;
	};

} // namespace file_manager
