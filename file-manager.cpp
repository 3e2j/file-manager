#include <QApplication>

#include <filesystem>

#include "app/app_controller.hpp"

int main(int argc, char *argv[]) {
	QApplication app(argc, argv);
	file_manager::FileManager manager(std::filesystem::current_path().string());
	manager.start();
	return app.exec();
}
