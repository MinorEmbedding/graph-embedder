#ifndef __MAJORMINER_CONFIG_HPP_
#define __MAJORMINER_CONFIG_HPP_

#define EIGEN_MPL2_ONLY
#define TBB_PREVIEW_CONCURRENT_LRU_CACHE 1
#define __DEBUG__ 1




#if __DEBUG__ == 1

#define DEBUG(...) __VA_ARGS__
#include <iostream>
#define OUT_S std::cout

#else
#define DEBUG(...)
#endif

#endif