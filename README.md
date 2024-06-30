# BWD

Bwd is an unofficial [Bitwarden](https://bitwarden.com/) cross-platform desktop client.

It is merely a proof-of-concept I made while reviewing Bitwarden's client [implementation](https://github.com/bitwarden/clients/).

![Bwd screenshot](./Resources/screenshot.jpg)

## Building

You need to have installed:

- cmake
- a working C++ compiler
- Qt libraries

You can compile it with the following commands:

```sh
cmake -Bbuild -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_BUILD_TYPE=Release
cmake --build build

# Run it
./build/bwd
```
