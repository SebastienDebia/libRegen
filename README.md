# libRegen
String generation from a regex in C++

## Building the test binary

### On Linux

Install the dependencies:

`sudo apt install libboost-all-dev`

Clone the repository:

`git clone https://github.com/SebastienDebia/libRegen.git`

Once that's finished, navigate to the root directory. In this case it should be libRegen:

`cd libRegen`

Then run g++:

`g++ -std=c++14 main.cpp -o test_regen`

If everything went right, you should have a new binary test_regen. It contains a few test regex.
