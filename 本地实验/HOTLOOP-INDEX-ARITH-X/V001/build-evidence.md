# V001 build evidence

BUILD_ATTEMPT=local CMake exact-source attempt
SOURCE_SHA256=f847f4c794a4484403e7599acbb357517d97879f3bcea843a3bd25742e9ed121
CONFIGURE=PASS
COMPILE=FAIL
FAILURE=compiler invocation lacked ASCEND_HOME_PATH and then could not find kernel_operator.h
CORRECTNESS=NOT_RUN
SERVER3=BLOCKED_DNS_cann-server3_unresolvable
SINGLE_CHANGE_AUDIT=PASS
DIRECT_PARENT=R31B V011
ONLINE=FORBIDDEN
RAW_LOGS=configure.log, compile.log
