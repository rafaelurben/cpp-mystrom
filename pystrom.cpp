#include <string>
#include <vector>
#include <iostream>
#include "pystrom-utils.cpp"

void detect_devices() {
    devices.clear();
    std::cout << "Checking for MyStrom devices..." << std::endl;

    winsock_listen();
}

int main() {
    winsock_startup();
    winsock_create();

    detect_devices();
}
