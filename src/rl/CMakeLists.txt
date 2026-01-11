add_library(rl
    NTupleNetwork.cpp
    NTupleNetwork.h
)

target_link_libraries(rl PUBLIC core)
