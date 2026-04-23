#include "file_manager/core/file_system.hpp"
#include "../test_main.hpp"

#include <filesystem>
#include <iostream>

namespace fs = std::filesystem;

bool run_test_file_system() {
	bool ok = true;
	test_support::TempDir temp;
	file_manager::FileSystem filesystem(temp.path.string());

	const fs::path docs = temp.path / "docs";
	const fs::path notes = temp.path / "notes.txt";
	const fs::path nested_file = temp.path / "abc" / "123";
	const fs::path nested_dir = temp.path / "deep";
	const fs::path nested_child = nested_dir / "inside.txt";

	const fs::path missing = temp.path / "missing.txt";

	// Test file / directory creation
	ok &= test_support::expect(filesystem.createDirectory(docs.string()) != nullptr, "createDirectory should succeed");
	ok &= test_support::expect(filesystem.createFile(notes.string()) != nullptr, "createFile should succeed");
	ok &= test_support::expect(filesystem.createFile(notes.string()) == nullptr, "createFile should fail on duplicate path");
	ok &= test_support::expect(filesystem.createDirectory(docs.string()) == nullptr, "createDirectory should fail on duplicate path");
	ok &= test_support::expect(filesystem.createFile(nested_file.string()) == nullptr, "createFile should fail when parent directories do not exist");

	// Test formatting
	const auto entries = filesystem.list(temp.path.string());
	ok &= test_support::expect(entries.size() == 2, "list should contain created directory and file");
	ok &= test_support::expect(entries.front()->isDirectory(), "directories should be listed before files");
	ok &= test_support::expect(entries.front()->getName() == "docs", "directory should sort before file by name");
	ok &= test_support::expect(entries.back()->getName() == "notes.txt", "file should appear in sorted results");

	// Test deletion
	ok &= test_support::expect(filesystem.deleteEntry(notes.string()), "deleteEntry should delete files");
	ok &= test_support::expect(!fs::exists(notes), "deleted file should not exist");
	ok &= test_support::expect(!filesystem.deleteEntry((missing).string()), "deleteEntry should fail for missing path");

	// Test nested logic
	ok &= test_support::expect(filesystem.createDirectory(nested_dir.string()) != nullptr, "createDirectory should create nested directory root");
	ok &= test_support::expect(filesystem.createFile(nested_child.string()) != nullptr, "createFile should work when parent already exists");
	ok &= test_support::expect(filesystem.deleteEntry(nested_dir.string()), "deleteEntry should delete directories recursively");
	ok &= test_support::expect(!fs::exists(nested_dir), "deleted directory should not exist");

	if (!ok) {
		return false;
	}

	std::cout << "All test_file_system cases passed.\n";
	return true;
}
