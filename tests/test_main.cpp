#include <exception>
#include <iostream>
void test_kinematics();
void test_pid();
void test_perception();
void test_safety();
void test_advanced_controllers();
void test_hinfinity();
int main() {
    try { test_kinematics(); test_perception(); test_pid(); test_safety(); test_advanced_controllers(); test_hinfinity(); std::cout << "All OBLIVION tests passed.\n"; return 0; }
    catch (const std::exception& e) { std::cerr << "TEST FAILURE: " << e.what() << '\n'; return 1; }
}
