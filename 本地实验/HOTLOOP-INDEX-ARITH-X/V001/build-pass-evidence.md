# V001 build pass evidence

BUILD=PASS
CONFIGURE=PASS
LINK=PASS
SOURCE_SHA256=f847f4c794a4484403e7599acbb357517d97879f3bcea843a3bd25742e9ed121
EXECUTABLE_SHA256=9ae220fe643300ccdc67a2f9f489c75b04f8e754c4c83846c4666901f7ea655
COMMAND=source /usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/aarch64-linux/script/set_env.sh; export CPLUS_INCLUDE_PATH and C_INCLUDE_PATH to toolkit C++ headers; cmake --build build-env -j2
EXACT_SOURCE=submission.asc
DIFF=diff.patch
CORRECTNESS=NOT_RUN
LOCAL=NOT_RUN
SERVER=hwnput3
DEVICE=4
RAW_LOG=compile-formal-env-confirm-20261006.log
COMPILE_TIMESTAMP=2026-10-06T16:00:00Z
