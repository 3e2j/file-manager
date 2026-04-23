#include <QApplication>

#include <filesystem>

#include "file_manager/app/file_manager.hpp"

int main(int argc, char *argv[]) {
	QApplication app(argc, argv);
	file_manager::FileManager manager(std::filesystem::current_path().string());
	manager.start();
	return app.exec();
}
