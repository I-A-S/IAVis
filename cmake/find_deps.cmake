include(FetchContent)

FetchContent_Declare(
        IAGHI
        GIT_REPOSITORY https://github.com/I-A-S/IAGHI
        GIT_TAG        main
        OVERRIDE_FIND_PACKAGE
)

FetchContent_Declare(
        glm
        GIT_REPOSITORY https://github.com/g-truc/glm
        GIT_TAG        master
        OVERRIDE_FIND_PACKAGE
)

FetchContent_Declare(
        DearImGui_CMake
        GIT_REPOSITORY https://github.com/I-A-S/imgui-cmake
        GIT_TAG        main
        OVERRIDE_FIND_PACKAGE
)

FetchContent_MakeAvailable(IAGHI glm DearImGui_CMake)

if(IAVis_BUILD_SANDBOX)
    FetchContent_Declare(
            SDL
            GIT_REPOSITORY https://github.com/libsdl-org/SDL
            GIT_TAG        main
            OVERRIDE_FIND_PACKAGE
    )

    FetchContent_MakeAvailable(SDL)
endif()
