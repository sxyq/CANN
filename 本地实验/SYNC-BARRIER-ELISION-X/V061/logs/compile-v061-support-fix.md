# V061 Compile Attempt Record

- Candidate source was unchanged throughout these attempts; SHA256 remains `4efb391c70b8ef033a3b4675f32b39a282760e55f5222364773ea28d31c15977`.
- Initial `cmake --build .../V061/build --parallel 1` did not start: the expected in-tree build directory does not exist. This attempt is preserved in the session output; no existing evidence was overwritten.
- Fresh configure: `cmake -S 本地实验/SYNC-BARRIER-ELISION-X/V061 -B /tmp/sync-v061-build.30lOPF` completed successfully.
- Build without toolkit environment failed with `can not find ASCEND_HOME_PATH` and `kernel_operator.h` missing.
- Build with `ASCEND_HOME_PATH`/toolkit `set_env.sh` but without host include propagation failed in the generated registration compile with `<vector> file not found`.
- Successful support-only build command:

  ```sh
  bash -lc 'set -o pipefail && source /usr/local/Ascend/ascend-toolkit/set_env.sh && CPLUS_INCLUDE_PATH=/usr/include/c++/11:/usr/include/aarch64-linux-gnu/c++/11:/usr/include/c++/11/backward cmake --build /tmp/sync-v061-build.30lOPF --parallel 1'
  ```

- Result: PASS; both `sync_barrier_elision_v061` and `sync_barrier_elision_correctness` built. The include path matches the standard host C++ include directories already declared in V061 CMake; this environment-only fix did not change Candidate source or the runner source.
- Candidate executable SHA256: `db79236f011bfcff8786d48503a6bab38b9acb2c1d615793b94e6efc64b752ec`.
- Correctness runner SHA256: `d064ef681f4b624e2c91b61bccb3dd9e18b790174c1b89b465ebc041903e6da3`.
