# libRegen
String generation from a regex in C++

## Usage

The library is header only.
Include the main header which brings in everything else:

`#include "regen/regen.hpp"`

To generate a random string from a regex, just call the generate function:

`std::cout << regen::generate( "a{12}" ) << "\n"`

`aaaaaaaaaaaa`

It is possible to customize the maximum number of repetitions for `*` and `+`:

`std::cout << regen::generate( ".+", 20 ) << "\n"`

`dj1ofFSkUCiJqu`

It is also possible to customize the minimum number of repetitions for `*` and `+`, note that `+` will still have at least 1 repetition:

`std::cout << regen::generate( ".+", 42, 21 ) << "\n"`

`vA=N796t?hXwAJdTNQ!HE1_OLXmJ`

Finally, it is possible to restrict the set of characters used in the generated string:

`std::cout << regen::generate( ".+", 20, 0, "[A-Z]" ) << "\n"`

`LNUPLCHNHANDPR`

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
