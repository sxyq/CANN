# RFH V001 narrow build

`CMakeLists.txt` uses the route-local archived REDUCE-HIER-X ASC CMake/ASC
flags as reference, but defines only one target. `build_probe.asc` includes
the exact V001 `submission.asc`; it is a compile/link smoke probe and was not
executed. The build directory and executable are isolated under `/tmp`.

The established recipe also exports the HCC C++ include roots in the build
shell. This is required by the ASC plugin host phase (including `<vector>`).
See the committed configure/build logs and `build-result-retry-20261003.json`
for exact command outcomes and identities.
