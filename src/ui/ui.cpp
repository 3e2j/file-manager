#include "ui/ui.hpp"

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

#include "core/file_entry.hpp"

namespace file_manager {
	constexpr std::size_t kDisplayLineLength = 18;
	constexpr std::size_t kSecondLineVisibleLength = kDisplayLineLength - 3;

	// Avoids weird formatting when adding newlines, tries to keep naming readible
	std::size_t FindPreferredSplit(const std::string &name) {
		const std::size_t space_pos = name.rfind(' ', kDisplayLineLength - 1);
		const std::size_t underscore_pos = name.rfind('_', kDisplayLineLength - 1);
		if (space_pos == std::string::npos) {
			return underscore_pos;
		}
		if (underscore_pos == std::string::npos) {
			return space_pos;
		}
		if (space_pos > underscore_pos) {
			return space_pos;
		}
		return underscore_pos;
	}

	// Formats name over a max of two lines, truncates if too long.
	std::string FormatDisplayName(const std::string &name) {
		if (name.size() <= kDisplayLineLength) {
			return name;
		}

		std::size_t split_pos = kDisplayLineLength;
		std::size_t second_start = kDisplayLineLength;
		const std::size_t preferred_split = FindPreferredSplit(name);
		if (preferred_split != std::string::npos && preferred_split > 0) {
			const std::size_t candidate_second_start = preferred_split + 1;
			if (name.size() - candidate_second_start <= kDisplayLineLength) {
				split_pos = preferred_split;
				second_start = candidate_second_start;
			}
		}

		if (name.size() <= kDisplayLineLength * 2) {
			return name.substr(0, split_pos) + "\n" + name.substr(second_start);
		}
		return name.substr(0, split_pos) + "\n" +
			name.substr(second_start, kSecondLineVisibleLength) + "...";
	}


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
		entry_list_->setIconSize(QSize(70, 70));
		entry_list_->setResizeMode(QListView::Adjust);
		entry_list_->setMovement(QListView::Static);
		entry_list_->setGridSize(QSize(150, 110));
		entry_list_->setSpacing(12);
		entry_list_->setWordWrap(true);
		entry_list_->setTextElideMode(Qt::ElideNone);

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

		// Entry related display
		for (const auto &entry : entries) {
			auto *item = new QListWidgetItem();
			item->setText(QString::fromStdString(FormatDisplayName(entry->getName())));

			const std::string tooltip =
				"Name: " + entry->getName() +
				"\nCreated: " + entry->getCreatedTime() +
				"\nModified: " + entry->getModifiedTime() +
				"\nAccessed: " + entry->getAccessedTime();
			item->setToolTip(QString::fromStdString(tooltip));
			item->setData(kPathRole, QString::fromStdString(entry->getPath()));
			item->setData(kDirRole, entry->isDirectory());

			// This is to be moved out if more type-specific file icons get added
			QStyle::StandardPixmap icon_type = QStyle::SP_FileIcon;
			if (entry->isDirectory()) {
				icon_type = QStyle::SP_DirIcon;
			}
			item->setIcon(style()->standardIcon(icon_type));

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
		std::filesystem::path absolute;
		if (std::filesystem::path(path).is_absolute()) {
			absolute = std::filesystem::path(path);
		} else {
			absolute = std::filesystem::absolute(path);
		}
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
