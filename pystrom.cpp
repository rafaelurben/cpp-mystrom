#include <string>
#include <vector>
#include <iostream>
#include "pystrom-utils.cpp"

std::vector<MyStromDevice> devices;

void detect_devices() {
    devices.clear();
    std::cout << "Checking for MyStrom devices..." << std::endl;

}

int main() {
    winsock_startup();

    detect_devices();
}
