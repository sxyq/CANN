# V001 build pass evidence

BUILD=PASS
CONFIGURE=PASS
LINK=PASS
SOURCE_SHA256=f847f4c794a4484403e7599acbb357517d97879f3bcea843a3bd25742e9ed121
EXECUTABLE_SHA256=9ae220fe643300ccdc67a2f9f489c75b04f8e754c4c83846c4666901f7ea655
COMMAND=source toolkit aarch64-linux/script/set_env.sh; inject toolkit C++ include paths; cmake --build build-env -j2
EXACT_SOURCE=submission.asc
DIFF=diff.patch
CORRECTNESS=NOT_RUN
LOCAL=NOT_RUN
SERVER3=DNS_UNRESOLVED
RAW_LOG=compile-env-cxxpath.log
