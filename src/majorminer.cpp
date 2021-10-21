#include "majorminer.hpp"

using namespace majorminer;

bool majorminer::works() { return true; }


std::vector<int> majorminer::testTBB()
{
  std::vector<int> ret{54,2,6,1,45,1,3,9,7,6,45,77,32,45};
  tbb::parallel_sort(ret.begin(), ret.end());
  return ret;
}