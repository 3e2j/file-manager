#include "file_manager/ui/ui.hpp"

#include <QDesktopServices>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QLabel>
#include <QListView>
#include <QListWidget>
#include <QMessageBox>
#include <QPushButton>
#include <QShortcut>
#include <QStyle>
#include <QUrl>
#include <QVBoxLayout>

#include <filesystem>

#include "file_manager/core/file_entry.hpp"

namespace file_manager {

	UI::UI() {
		setWindowTitle("File Manager");
		resize(900, 550);

		// Build primary layout and top navigation row.
		auto *root_layout = new QVBoxLayout();
		entry_list_ = new QListWidget();
		status_label_ = new QLabel();
		displayMenu();

		auto *top_row = new QHBoxLayout();
		back_button_ = new QPushButton("< Back");
		breadcrumb_layout_ = new QHBoxLayout();
		breadcrumb_layout_->setContentsMargins(0, 0, 0, 0);
		breadcrumb_layout_->setSpacing(4);

		auto *breadcrumb_container = new QWidget();
		breadcrumb_container->setLayout(breadcrumb_layout_);
		top_row->addWidget(back_button_);
		top_row->addWidget(breadcrumb_container, 1);

		entry_list_->setViewMode(QListView::IconMode);
		entry_list_->setIconSize(QSize(66, 66));
		entry_list_->setResizeMode(QListView::Adjust);
		entry_list_->setMovement(QListView::Static);
		entry_list_->setWordWrap(true);
		entry_list_->setSpacing(12);

		auto *button_row = new QHBoxLayout();
		auto *create_button = new QPushButton("Create");
		auto *delete_button = new QPushButton("Delete");
		auto *refresh_button = new QPushButton("Refresh");

		button_row->addWidget(create_button);
		button_row->addWidget(delete_button);
		button_row->addWidget(refresh_button);

		root_layout->addLayout(top_row);
		root_layout->addWidget(entry_list_);
		root_layout->addLayout(button_row);
		root_layout->addWidget(status_label_);
		setLayout(root_layout);

		// Wire UI actions to controller callbacks.
		connect(back_button_, &QPushButton::clicked, [this] {
			if (on_back_) {
				on_back_();
			}
		});
		connect(create_button, &QPushButton::clicked, [this] {
			if (on_create_) {
				on_create_();
			}
		});
		connect(delete_button, &QPushButton::clicked, [this] {
			if (on_delete_) {
				on_delete_();
			}
		});
		auto *delete_shortcut = new QShortcut(QKeySequence::Delete, this);
		delete_shortcut->setContext(Qt::WidgetWithChildrenShortcut);
		connect(delete_shortcut, &QShortcut::activated, [this] {
			if (on_delete_) {
				on_delete_();
			}
		});
		connect(refresh_button, &QPushButton::clicked, [this] {
			if (on_refresh_) {
				on_refresh_();
			}
		});
		connect(entry_list_, &QListWidget::itemDoubleClicked, [this](QListWidgetItem *item) {
			if (!item || !on_entry_activated_) {
				return;
			}
			const std::string path = item->data(kPathRole).toString().toStdString();
			const bool is_directory = item->data(kDirRole).toBool();
			if (!path.empty()) {
				on_entry_activated_(path, is_directory);
			}
		});
	}

	void UI::setOnCreate(const std::function<void()> &callback) { on_create_ = callback; }
	void UI::setOnDelete(const std::function<void()> &callback) { on_delete_ = callback; }
	void UI::setOnRefresh(const std::function<void()> &callback) { on_refresh_ = callback; }
	void UI::setOnBack(const std::function<void()> &callback) { on_back_ = callback; }
	void UI::setOnPathPartClicked(const std::function<void(const std::string &)> &callback) {
		on_path_part_clicked_ = callback;
	}
	void UI::setOnEntryActivated(const std::function<void(const std::string &, bool)> &callback) {
		on_entry_activated_ = callback;
	}

	void UI::displayMenu() {
		output_ = "Actions: Back, Create, Delete, Refresh\n"
				  "Double-click: open file / enter directory\n"
				  "Create: append '/' to create a directory";
		status_label_->setText(QString::fromStdString(output_));
	}

	std::string UI::getUserInput(const std::string &prompt) {
		bool ok = false;
		const QString value = QInputDialog::getText(
			this, "File Manager", QString::fromStdString(prompt), {}, "", &ok);
		if (!ok) {
			return "";
		}
		input_ = value.toStdString();
		return input_;
	}

	void UI::showDirectory(const std::vector<std::shared_ptr<FileEntry>> &entries,
		const std::string &path, bool can_go_back) {
		back_button_->setEnabled(can_go_back);
		renderBreadcrumbs(path);
		entry_list_->clear();

		for (const auto &entry : entries) {
			auto *item = new QListWidgetItem();
			item->setText(QString::fromStdString(entry->getName()));
			item->setToolTip(QString::fromStdString(entry->getModifiedTime()));
			item->setData(kPathRole, QString::fromStdString(entry->getPath()));
			item->setData(kDirRole, entry->isDirectory());
			item->setIcon(style()->standardIcon(
				entry->isDirectory() ? QStyle::SP_DirIcon : QStyle::SP_FileIcon));
			entry_list_->addItem(item);
		}
	}

	void UI::showMessage(const std::string &message) {
		QMessageBox::information(this, "File Manager", QString::fromStdString(message));
	}

	std::string UI::selectedEntryPath(bool *is_directory) const {
		auto *item = entry_list_->currentItem();
		if (!item) {
			return "";
		}
		if (is_directory) {
			*is_directory = item->data(kDirRole).toBool();
		}
		return item->data(kPathRole).toString().toStdString();
	}

	bool UI::openPath(const std::string &path) const {
		return QDesktopServices::openUrl(QUrl::fromLocalFile(QString::fromStdString(path)));
	}

	void UI::clearBreadcrumbs() {
		while (auto *item = breadcrumb_layout_->takeAt(0)) {
			if (item->widget()) {
				delete item->widget();
			}
			delete item;
		}
	}

	void UI::renderBreadcrumbs(const std::string &path) {
		clearBreadcrumbs();
		// Normalize path so each clickable segment resolves correctly.
		const std::filesystem::path absolute = std::filesystem::path(path).is_absolute()
												   ? std::filesystem::path(path)
												   : std::filesystem::absolute(path);
		std::filesystem::path cumulative;
		bool first = true;
		for (const auto &part : absolute) {
			const std::string segment = part.string();
			if (segment.empty()) {
				continue;
			}

			if (!first) {
				breadcrumb_layout_->addWidget(new QLabel(">"));
			}
			first = false;

			if (cumulative.empty()) {
				cumulative = part;
			} else {
				cumulative /= part;
			}

			auto *button = new QPushButton(QString::fromStdString(segment));
			button->setFlat(true);
			const std::string target_path = cumulative.string();
			connect(button, &QPushButton::clicked, [this, target_path] {
				if (on_path_part_clicked_) {
					on_path_part_clicked_(target_path);
				}
			});
			breadcrumb_layout_->addWidget(button);
		}
		breadcrumb_layout_->addStretch();
	}

}
