install required libraries:
```sh
sudo apt-get install libboost-all-dev libwebsocketpp-dev libcpprest-dev libmysqlcppconn-dev
```

to build run
```sh
g++ -std=c++11 -I ./websocketpp -o build/ami_ws_server ami_ws_server.cpp -lboost_system -lboost_thread -lcpprest -lmysqlcppconn -lpthread
```

Running the Program
Run the compiled program:

```sh
./build/ami_ws_server
```