./dummy_duplex 0 1 ./examples/dummy/config/config0.txt & pid01=$!| \
./dummy_duplex 0 0 ./examples/dummy/config/config0.txt & pid00=$!| \
./dummy_duplex 1 1 ./examples/dummy/config/config0.txt & pid11=$!| \
./dummy_duplex 1 0 ./examples/dummy/config/config0.txt & pid10=$!

sleep 10
killall ./dummy_duplex
echo "[dummy has been killed]"