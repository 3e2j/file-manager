#include "test_main.hpp"

int main() {
    // Run tests
	const bool file_entry_ok = run_test_file_entry();
	const bool file_system_ok = run_test_file_system();
	if (file_entry_ok && file_system_ok) {
		return 0;
	}
	return 1;
}
