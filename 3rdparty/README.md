3rdparty libraries
=========================

## boost

[boost](http://www.boost.org/) is a common c++ library

### compile

`./bootstrap.sh --prefix=INSTALDIR`

use `./bootstrap.sh --help` to get help

`./b2 -j12 --build-dir=./build toolset=gcc variant=release link=shared threading=multi cxxflags=-std=c++11 install`

use `./b2 --help` to get help

