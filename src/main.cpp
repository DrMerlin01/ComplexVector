#include "../inc/tests.h"

#include <iostream>
#include <stdexcept>

using namespace std;

int main() {
    try {
        Test1();
        Test2();
        Test3();
        Test4();
        Test5();
        Test6();
        Benchmark();
    } catch (const exception& e) {
        cerr << e.what() << endl;
    }

    return 0;
}