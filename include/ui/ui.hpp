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

	// Callbacks assigned by app_controller to respond to internal events
	struct EventHandlers {
		std::function<void()> on_create;
		std::function<void()> on_delete;
		std::function<void()> on_refresh;
		std::function<void()> on_back;
		std::function<void()> on_forward;
		std::function<void(const std::string &)> on_path_part_clicked;
		std::function<void(const std::string &, bool)> on_entry_activated;
	};

	class UI : public QWidget {
	  public:
		UI();

		void setEventHandlers(const EventHandlers &handlers);

		void displayMenu();
		std::string getUserInput(const std::string &prompt = "Input");
		void showDirectory(const std::vector<std::shared_ptr<FileEntry>> &entries,
			const std::string &path, bool can_go_back, bool can_go_forward);
		void showMessage(const std::string &message);
		std::string selectedEntryPath(bool *is_directory = nullptr) const;
		bool openPath(const std::string &path) const;

	  private:
		void clearBreadcrumbs();
		void renderBreadcrumbs(const std::string &path);

		static constexpr int kPathRole = Qt::UserRole + 1;
		static constexpr int kDirRole = Qt::UserRole + 2;

		std::string input_;
		std::string output_;
		QPushButton *back_button_ = nullptr;
		QPushButton *forward_button_ = nullptr;
		QHBoxLayout *breadcrumb_layout_ = nullptr;
		QListWidget *entry_list_ = nullptr;
		QLabel *status_label_ = nullptr;
		EventHandlers handlers_;
	};

}
