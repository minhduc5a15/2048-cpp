add_library(ai
    expectimax_agent.cpp
    expectimax_agent.h
    agent.h
)

target_link_libraries(ai PUBLIC core)