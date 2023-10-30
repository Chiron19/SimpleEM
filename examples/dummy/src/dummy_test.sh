./dummy_duplex 0 1 ./examples/dummy/config/config.txt & pid01=$! | \
./dummy_duplex 0 0 ./examples/dummy/config/config.txt & pid00=$!

sleep 6
killall ./dummy_duplex
echo "[dummy has been killed]"