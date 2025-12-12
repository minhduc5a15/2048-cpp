add_executable(2048-train main_train.cpp trainer.cpp)
target_link_libraries(2048-train PRIVATE core)