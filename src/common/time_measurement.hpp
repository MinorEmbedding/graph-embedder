#ifndef __MAJORMINER_TIME_MEASUREMENT_HPP_
#define __MAJORMINER_TIME_MEASUREMENT_HPP_

#include <chrono>

#define CHRONO_STUFF(t1, t2, diff, var, ...)  \
  auto t1 = std::chrono::high_resolution_clock::now();    \
   __VA_ARGS__                                            \
  auto t2 = std::chrono::high_resolution_clock::now();    \
  std::chrono::duration<double> diff = t2 - t1;           \
  var+=diff.count();


#define PRINT_TIME(name) std::cout << "Time: " << #name << ": " << name << std::endl;

namespace majorminer
{
  extern double TIME_MUTATION, TIME_REDUCE;
  extern double GENERATE_POP, OPTIMIZE;

  extern double NETWORK_SIMPLEX_BUILDTIME, NETWORK_SIMPLEX_SOLVETIME;
}


#endif
