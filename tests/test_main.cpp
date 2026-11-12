#include <exception>
#include <iostream>
void test_kinematics();
void test_pid();
void test_safety();
int main() {
    try { test_kinematics(); test_pid(); test_safety(); std::cout << "All OBLIVION tests passed.\n"; return 0; }
    catch (const std::exception& e) { std::cerr << "TEST FAILURE: " << e.what() << '\n'; return 1; }
}
