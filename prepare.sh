cd external/
git clone https://github.com/oneapi-src/oneTBB.git
git clone https://gitlab.com/libeigen/eigen.git
git clone https://github.com/splintersu/NetworkSimplex.git

curl http://lemon.cs.elte.hu/pub/sources/lemon-1.3.1.tar.gz --output lemon.tar.gz
tar -xvzf lemon.tar.gz
mv lemon-1.3.1/ lemon
