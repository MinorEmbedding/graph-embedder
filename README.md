# majorminer
Testing a bunch of graph minor embedding heuristics and related techniques to improve the embedding of QUBOs.


| :warning:    | This is a research repo, so APIs and algorithms might change frequently as were are trying out different ideas. Moreover, this means that the code is not production-ready. |
|---------------|:------------------------|

| :point_up:    | We currently do not implement the same approaches in C++ and Python, but try out different ideas in different programming languages. Make sure to check out both the C++ and Python implementation. |
|---------------|:------------------------|



<details>
  <summary>C++</summary>
  
# Build C++ library
Note that in order to build, you have to clone the submodules as well. That is,
if you have already cloned this repository, you should run ```git submodule update --init --recursive``` and find the submodules in ```external/```.
In the case, you are about to clone the repository, just run ```git clone --recursive https://github.com/MinorEmbedding/majorminer.git```.

In order to build, you must then run the following commands
```
bash prepare.sh
mkdir build
cd build/
cmake ..
make
```

# Libraries used in the C++-Project
#### [oneTBB](https://github.com/oneapi-src/oneTBB) (License: [Apache 2.0](https://choosealicense.com/licenses/apache-2.0/))
#### [GoogleTest](https://github.com/google/googletest) (License: [BSD 3-Clause "New" or "Revised"](https://choosealicense.com/licenses/bsd-3-clause/))
#### [LEMON](https://lemon.cs.elte.hu/trac/lemon) (License: [Boost Software License 1.0](https://choosealicense.com/licenses/bsl-1.0/))
#### [Eigen](https://eigen.tuxfamily.org/index.php?title=Main_Page) (License: [MPL2](https://choosealicense.com/licenses/mpl-2.0/), disabled LGPL features.)
</details>



<details>
<summary>Python</summary>
  
All Python-related code and documentation can be found [in the Python folder](python/).
</details>
