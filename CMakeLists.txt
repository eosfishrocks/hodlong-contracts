cmake_minimum_required(VERSION 3.5)
project(bpfish.hodlong VERSION 1.0.0)

find_package(eosio.cdt)

### generate both the wasm and abi
add_contract( hodlong hodlong contracts/Hodlong.cpp )

add_contract( trackers trackers contracts/Trackers.cpp )