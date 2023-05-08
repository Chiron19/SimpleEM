#pragma once

#include <stddef.h>
#include <string>

const std::string CONFIG_PATH("../configs/config4.txt");
const int STEPS = 100; /* Number of times emulator awakes a process */

const char PROGRAM_PATH[] = "dummy"; 
const char PROGRAM_CONFIG_PATH[] = "../examples/dummy/config/config4.txt";

const std::string TUN_DEV_NAME("tun0");
const std::string TUN_ADDR("172.16.0.1");
const std::string TUN_MASK("255.240.0.0");
