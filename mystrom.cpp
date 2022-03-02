#include <string>
#include <vector>
#include <iostream>
#include "mystrom-utils.cpp"

void detect_devices() {
    devices.clear();
    printf("[mystrom] Checking for MyStrom devices...\n");

    socket_receive();
}

int main() {
    socket_setup();

    detect_devices();

    socket_cleanup();
}
