# HTTP Load Balancer

## Build
```shell
$ cd ~
$ git clone https://github.com/Conzxy/lb
$ mkdir lb/build && cd lb/bin
$ ./build.sh load-balancer -m=release

# ./run-nginx.sh
$ ./load-balancer -c config-file -t $(nproc) -s &
```
