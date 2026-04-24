#include "file_manager/core/file_entry.hpp"
#include "../test_main.hpp"

#include <filesystem>
#include <fstream>
#include <iostream>

namespace fs = std::filesystem;

bool run_test_file_entry() {
	bool ok = true;
	test_support::TempDir temp("file-manager-tests-entry-");

	const fs::path file_with_ext = temp.path / "notes.txt";
	const fs::path file_no_ext = temp.path / "README";
	const fs::path folder = temp.path / "docs";
	const fs::path missing = temp.path / "missing.bin";

	std::ofstream(file_with_ext).close();
	std::ofstream(file_no_ext).close();
	fs::create_directory(folder);

	file_manager::File ext_file(file_with_ext.string());
	file_manager::File no_ext_file(file_no_ext.string());
	file_manager::Directory directory(folder.string());
	file_manager::File missing_file(missing.string());

	// Test File metadata and type inference
	ok &= test_support::expect(ext_file.getName() == "notes.txt", "File should expose filename as name");
	ok &= test_support::expect(ext_file.getPath() == file_with_ext.string(), "File path should be preserved");
	ok &= test_support::expect(ext_file.getType() == ".txt", "File extension should be exposed as type");
	ok &= test_support::expect(!ext_file.isDirectory(), "File should report isDirectory false");
	ok &= test_support::expect(ext_file.getCreatedTime() != "N/A", "Existing file should have created time");
	ok &= test_support::expect(ext_file.getModifiedTime() != "N/A", "Existing file should have modified time");
	ok &= test_support::expect(ext_file.getAccessedTime() != "N/A", "Existing file should have accessed time");

	// Test fallback values for unknown type / missing files
	ok &= test_support::expect(no_ext_file.getType() == "unknown", "File without extension should use unknown type");
	ok &= test_support::expect(missing_file.getCreatedTime() == "N/A", "Missing file should have N/A created time");
	ok &= test_support::expect(missing_file.getModifiedTime() == "N/A", "Missing file should have N/A modified time");
	ok &= test_support::expect(missing_file.getAccessedTime() == "N/A", "Missing file should have N/A accessed time");

	// Test Directory identity
	ok &= test_support::expect(directory.getName() == "docs", "Directory should expose folder name");
	ok &= test_support::expect(directory.getPath() == folder.string(), "Directory path should be preserved");
	ok &= test_support::expect(directory.isDirectory(), "Directory should report isDirectory true");

	// Test Directory entry collection operations
	directory.addEntry(std::make_shared<file_manager::File>(file_with_ext.string()));
	directory.addEntry(std::make_shared<file_manager::File>(file_no_ext.string()));
	ok &= test_support::expect(directory.listEntries().size() == 2, "Directory should track added entries");
	directory.removeEntry(file_with_ext.string());
	ok &= test_support::expect(directory.listEntries().size() == 1, "Directory removeEntry should remove matching path");
	directory.clearEntries();
	ok &= test_support::expect(directory.listEntries().empty(), "Directory clearEntries should remove all entries");

	if (!ok) {
		return false;
	}

	std::cout << "All test_file_entry cases passed.\n";
	return true;
}
