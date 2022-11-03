/*
   dpg_graphics.h
*/

/*
Index of this file:
// [SECTION] header mess
// [SECTION] forward declarations
// [SECTION] public api
// [SECTION] structs
*/

#ifndef DPG_GRAPHICS_VULKAN_H
#define DPG_GRAPHICS_VULKAN_H

//-----------------------------------------------------------------------------
// [SECTION] header mess
//-----------------------------------------------------------------------------

#include <stdint.h>  // uint32_t
#include <stdbool.h> // bool

#define DPG_DECLARE_STRUCT(name) typedef struct _ ## name  name

#if defined(_WIN32)
#define VK_USE_PLATFORM_WIN32_KHR
#elif defined(__APPLE__)
#define VK_USE_PLATFORM_MACOS_MVK
#define VK_USE_PLATFORM_METAL_EXT
#else // linux
#define VK_USE_PLATFORM_XLIB_KHR
#endif
#include "vulkan/vulkan.h"

#ifndef DPG_VULKAN
#include <assert.h>
#define DPG_VULKAN(x) assert(x == VK_SUCCESS)
#endif

#ifdef __cplusplus
extern "C" {
#endif

//-----------------------------------------------------------------------------
// [SECTION] forward declarations
//-----------------------------------------------------------------------------

DPG_DECLARE_STRUCT(dpgVulkanSwapchain);    // swapchain resources & info
DPG_DECLARE_STRUCT(dpgVulkanDevice);       // device resources & info
DPG_DECLARE_STRUCT(dpgVulkanGraphics);     // graphics context
DPG_DECLARE_STRUCT(dpgVulkanFrameContext); // per frame resource

//-----------------------------------------------------------------------------
// [SECTION] public api
//-----------------------------------------------------------------------------

// setup
void                   dpg_create_instance       (dpgVulkanGraphics* ptGraphics, uint32_t uVersion, bool bEnableValidation);
void                   dpg_create_instance_ex    (dpgVulkanGraphics* ptGraphics, uint32_t uVersion, uint32_t uLayerCount, const char** ppcEnabledLayers, uint32_t uExtensioncount, const char** ppcEnabledExtensions);
void                   dpg_create_frame_resources(dpgVulkanGraphics* ptGraphics, dpgVulkanDevice* ptDevice);
void                   dpg_create_device         (VkInstance tInstance, VkSurfaceKHR tSurface, dpgVulkanDevice* ptDeviceOut, bool bEnableValidation);

// swapchain ops
void                   dpg_create_swapchain      (dpgVulkanDevice* ptDevice, VkSurfaceKHR tSurface, uint32_t uWidth, uint32_t uHeight, dpgVulkanSwapchain* ptSwapchainOut);
void                   dpg_create_framebuffers   (dpgVulkanDevice* ptDevice, VkRenderPass tRenderPass, dpgVulkanSwapchain* ptSwapchain);

// cleanup
void                   dpg_cleanup_graphics      (dpgVulkanGraphics* ptGraphics, dpgVulkanDevice* ptDevice);

// misc
dpgVulkanFrameContext* dpg_get_frame_resources    (dpgVulkanGraphics* ptGraphics);
uint32_t               dpg_find_memory_type       (VkPhysicalDeviceMemoryProperties tMemProps, uint32_t uTypeFilter, VkMemoryPropertyFlags tProperties);
void                   dpg_transition_image_layout(VkCommandBuffer tCommandBuffer, VkImage tImage, VkImageLayout tOldLayout, VkImageLayout tNewLayout, VkImageSubresourceRange tSubresourceRange, VkPipelineStageFlags tSrcStageMask, VkPipelineStageFlags tDstStageMask);

//-----------------------------------------------------------------------------
// [SECTION] structs
//-----------------------------------------------------------------------------

typedef struct _dpgVulkanFrameContext
{
    VkSemaphore     tImageAvailable;
    VkSemaphore     tRenderFinish;
    VkFence         tInFlight;
    VkCommandBuffer tCmdBuf;

} dpgVulkanFrameContext;

typedef struct _dpgVulkanSwapchain
{
    VkSwapchainKHR tSwapChain;
    VkExtent2D     tExtent;
    VkFramebuffer* ptFrameBuffers;
    VkFormat       tFormat;
    VkImage*       ptImages;
    VkImageView*   ptImageViews;
    uint32_t       uImageCount;
    uint32_t       uImageCapacity;
    uint32_t       uCurrentImageIndex; // current image to use within the swap chain
    bool           bVSync;

    VkSurfaceFormatKHR* ptSurfaceFormats_;
    uint32_t            uSurfaceFormatCapacity_;

} dpgVulkanSwapchain;

typedef struct _dpgVulkanDevice
{
    VkDevice                                  tLogicalDevice;
    VkPhysicalDevice                          tPhysicalDevice;
    int                                       iGraphicsQueueFamily;
    int                                       iPresentQueueFamily;
    VkQueue                                   tGraphicsQueue;
    VkQueue                                   tPresentQueue;
    VkPhysicalDeviceProperties                tDeviceProps;
    VkPhysicalDeviceMemoryProperties          tMemProps;
    VkPhysicalDeviceMemoryProperties2         tMemProps2;
    VkPhysicalDeviceMemoryBudgetPropertiesEXT tMemBudgetInfo;
    VkDeviceSize                              tMaxLocalMemSize;

} dpgVulkanDevice;

typedef struct _dpgVulkanGraphics
{
    VkInstance               tInstance;
    VkSurfaceKHR             tSurface;
    VkDebugUtilsMessengerEXT tDbgMessenger;
    VkCommandPool            tCmdPool;
    VkRenderPass             tRenderPass;
    dpgVulkanFrameContext*   sbFrames;
    uint32_t                 uFramesInFlight;    // number of frames in flight (should be less then PL_MAX_FRAMES_IN_FLIGHT)
    size_t                   szCurrentFrameIndex; // current frame being used
} dpgVulkanGraphics;

#ifdef __cplusplus
}
#endif

#endif //PL_GRAPHICS_VULKAN_H