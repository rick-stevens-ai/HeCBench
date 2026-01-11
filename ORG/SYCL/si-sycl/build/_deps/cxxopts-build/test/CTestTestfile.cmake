# CMake generated Testfile for 
# Source directory: /home/stevens/HeCBench/ORG/SYCL/si-sycl/build/_deps/cxxopts-src/test
# Build directory: /home/stevens/HeCBench/ORG/SYCL/si-sycl/build/_deps/cxxopts-build/test
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test(options "options_test")
set_tests_properties(options PROPERTIES  _BACKTRACE_TRIPLES "/home/stevens/HeCBench/ORG/SYCL/si-sycl/build/_deps/cxxopts-src/test/CMakeLists.txt;4;add_test;/home/stevens/HeCBench/ORG/SYCL/si-sycl/build/_deps/cxxopts-src/test/CMakeLists.txt;0;")
add_test(find-package-test "/home/stevens/anaconda3/lib/python3.11/site-packages/cmake/data/bin/ctest" "-C" "--build-and-test" "/home/stevens/HeCBench/ORG/SYCL/si-sycl/build/_deps/cxxopts-src/test/find-package-test" "/home/stevens/HeCBench/ORG/SYCL/si-sycl/build/_deps/cxxopts-build/test/find-package-test" "--build-generator" "Unix Makefiles" "--build-makeprogram" "/usr/bin/gmake" "--build-options" "-DCMAKE_CXX_COMPILER=/opt/intel/oneapi/compiler/2025.3/bin/icpx" "-DCMAKE_BUILD_TYPE=" "-Dcxxopts_DIR=/home/stevens/HeCBench/ORG/SYCL/si-sycl/build/_deps/cxxopts-build")
set_tests_properties(find-package-test PROPERTIES  _BACKTRACE_TRIPLES "/home/stevens/HeCBench/ORG/SYCL/si-sycl/build/_deps/cxxopts-src/test/CMakeLists.txt;7;add_test;/home/stevens/HeCBench/ORG/SYCL/si-sycl/build/_deps/cxxopts-src/test/CMakeLists.txt;0;")
add_test(add-subdirectory-test "/home/stevens/anaconda3/lib/python3.11/site-packages/cmake/data/bin/ctest" "-C" "--build-and-test" "/home/stevens/HeCBench/ORG/SYCL/si-sycl/build/_deps/cxxopts-src/test/add-subdirectory-test" "/home/stevens/HeCBench/ORG/SYCL/si-sycl/build/_deps/cxxopts-build/test/add-subdirectory-test" "--build-generator" "Unix Makefiles" "--build-makeprogram" "/usr/bin/gmake" "--build-options" "-DCMAKE_CXX_COMPILER=/opt/intel/oneapi/compiler/2025.3/bin/icpx" "-DCMAKE_BUILD_TYPE=")
set_tests_properties(add-subdirectory-test PROPERTIES  _BACKTRACE_TRIPLES "/home/stevens/HeCBench/ORG/SYCL/si-sycl/build/_deps/cxxopts-src/test/CMakeLists.txt;21;add_test;/home/stevens/HeCBench/ORG/SYCL/si-sycl/build/_deps/cxxopts-src/test/CMakeLists.txt;0;")
