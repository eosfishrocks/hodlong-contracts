cmake_minimum_required(VERSION 3.5)
project(hodlong VERSION 1.0.0)

find_package(eosio.cdt)

### generate both the wasm and abi
add_contract( bpfish.hodlong bpfish.hodlong contracts/Hodlong.cpp )
