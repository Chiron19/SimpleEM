# Simple Network Emulator

## Desciption
This project is forked from [Link](https://github.com/MirazSpecial/SimpleEM). The original project is a simple network emulator that can be used
to emulate a network topology and launch separate distributed terminals. It allows developers
to assign a matrix of delays of the network and distinct psuedo IP addresses within a subnet for
those distributed ends. It also provides various dummy examples to show case of the testing.
It supports IPv4 and UDP. It is written in C++ and fit for plug-and-play. The original project is licensed under MIT License.

This project is now reposited at [Link](.). The goal of this project is to add more features to the original
one and make it more practical. The features we added are listed below:

1. __Support for TCP__. We added support for TCP. We also added dummy examples with TCP
recurring messages and file transfer.
2. __Test Running on BFT-SMaRt__. We tested running TCP with verifying the performance of
BFT-SMaRt.
3. __Improvement on Configuration Parser and Debug Assistance__. We improved the configuration parser and added debug assistance.

## Installation
This installation guide is based on Ubuntu 18.04 (or higher version). It is assumed that the user has root access to the machine.

1. Clone the repository to your local directory on your machine, and cd to the root directory of the project.
```sh
cd /path/to/your/SimpleEM
```
2. Run cmake to generate the makefile. Please try clearing the `CMakeCache.txt` and
`cmake_install.cmake` if error occured.
```sh
cmake .
```
3. Run make to compile the project. Please make sure that you successfully make the project
by eliminating all the errors before running it.
```sh
make
```
4. Run the executable file tinyem to start the emulator. (notice that the default config file
path is set to `/configs/config.txt`, if you want to use other config file, please
specify it as the first argument of the executable file)
```sh
sudo ./tinyem
```

And you will see the following output on your terminal, which indicates the demo is running successfully:
```sh
[Server] 172.16.0.3, Listening on 5556
[Client] Socket Created!
[Client] Socket Connected!
[Client] Message Sent: Helloworld from client xx-xx-xx
[Server] 172.16.0.3:5556 Socket Accepted
[Server] Received: Helloworld from client xx-xx-xx
[Client] socket closed!
[Server] Children socket closed!
[Server] socket closed!
```

5. You can also run other executable files which the CMakeLists.txt specified.
For example, test_packet demo allows you to input a hexstream which represents a
TCP packet and test the result of the functions in `packet.hpp`;
```sh
./test_packet
## input the hex string e.g. 450000xxxx...
```

And dummy demo allows you to test the TCP recurring message and file transfer locally
(127.0.0.1), which would be introduced in details in the report.

---
Please read the report for detailed explaination and documentation.

Thank you for your reading!