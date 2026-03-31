set(SRC_FILES
    "cpp/main.cpp"
)

add_executable(Sandbox ${SRC_FILES})

target_include_directories(Sandbox PRIVATE hpp/)

target_link_libraries(Sandbox PRIVATE
        IAVis
        glm::glm
        SDL3::SDL3
)
