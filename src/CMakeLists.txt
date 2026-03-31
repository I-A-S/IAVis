add_library(IAVis STATIC
        cpp/iavis.cpp

        cpp/vulkan/device.cpp
        cpp/vulkan/instance.cpp
        cpp/vulkan/swapchain.cpp
)

target_include_directories(IAVis PUBLIC
    $<BUILD_INTERFACE:${IAVis_ROOT}/include>
    $<INSTALL_INTERFACE:include>
)
target_include_directories(IAVis PRIVATE hpp)

target_link_libraries(IAVis PUBLIC
        libauxid
)

target_link_libraries(IAVis PRIVATE
        volk::volk
        GPUOpen::VulkanMemoryAllocator
)
