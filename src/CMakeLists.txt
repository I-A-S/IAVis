add_library(IAVis STATIC cpp/IAVis.cpp)

target_include_directories(IAVis PUBLIC
    $<BUILD_INTERFACE:${IAVis_ROOT}/include>
    $<INSTALL_INTERFACE:include>
)
target_include_directories(IAVis PRIVATE hpp)

target_link_libraries(IAVis PUBLIC
        libauxid
        glm::glm
)

target_link_libraries(IAVis PRIVATE
        volk::volk
        GPUOpen::VulkanMemoryAllocator
)
