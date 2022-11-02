/*
   dpg_main.c
*/

/*
Index of this file:
// [SECTION] header mess
// [SECTION] includes
// [SECTION] globals
// [SECTION] forward declarations
// [SECTION] entry point
// [SECTION] implementations
*/

//-----------------------------------------------------------------------------
// [SECTION] header mess
//-----------------------------------------------------------------------------

#ifdef _WIN32
    #define VK_USE_PLATFORM_WIN32_KHR
    #define GLFW_EXPOSE_NATIVE_WIN32
#elif defined(__APPLE__)
    #define GLFW_EXPOSE_NATIVE_COCOA
    #define VK_USE_PLATFORM_MACOS_MVK
    #define VK_USE_PLATFORM_METAL_EXT
#else // linux
    #define VK_USE_PLATFORM_XLIB_KHR
    #define GLFW_EXPOSE_NATIVE_X11
    #endif
#define GLFW_INCLUDE_VULKAN

//-----------------------------------------------------------------------------
// [SECTION] includes
//-----------------------------------------------------------------------------

#include <stdlib.h>
#include <stdio.h>
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
#include "dpg_graphics.h"

//-----------------------------------------------------------------------------
// [SECTION] globals
//-----------------------------------------------------------------------------

dpgVulkanDevice    gtDevice;
dpgVulkanGraphics  gtGraphics;
dpgVulkanSwapchain gtSwapchain;

//-----------------------------------------------------------------------------
// [SECTION] forward declarations
//-----------------------------------------------------------------------------

static void dpg_update              (GLFWwindow* ptWindow);
static void dpg_window_size_callback(GLFWwindow* ptWindow, int width, int height);

//-----------------------------------------------------------------------------
// [SECTION] entry point
//-----------------------------------------------------------------------------

int main()
{
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    GLFWwindow* ptWindow = glfwCreateWindow(800, 600, "Dear PyGui 2", NULL, NULL);

    // create vulkan instance
    dpg_create_instance(&gtGraphics, VK_API_VERSION_1_2, true);

    // create surface
    #if defined(_WIN32) || defined(__APPLE__)
        glfwCreateWindowSurface(gtGraphics.tInstance, ptWindow, NULL, &gtGraphics.tSurface);
    #else // linux (not sure why glfwCreateWindowSurface fails for linux)
        VkXlibSurfaceCreateInfoKHR  tSurfaceCreateInfo = {
            .sType  = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR,
            .pNext  = NULL,
            .flags  = 0,
            .dpy    = glfwGetX11Display(),
            .window = glfwGetX11Window(ptWindow)
        };
        DPG_VULKAN(vkCreateXlibSurfaceKHR(gtGraphics.tInstance, &tSurfaceCreateInfo, NULL, &gtGraphics.tSurface));
    #endif
    
    // create devices
    dpg_create_device(gtGraphics.tInstance, gtGraphics.tSurface, &gtDevice, true);
    
    // create gtSwapchain
    dpg_create_swapchain(&gtDevice, gtGraphics.tSurface, (uint32_t)800, (uint32_t)600, &gtSwapchain);

    // create main render pass
    VkAttachmentDescription tColorAttachment = {
        .flags          = VK_ATTACHMENT_DESCRIPTION_MAY_ALIAS_BIT,
        .format         = gtSwapchain.tFormat,
        .samples        = VK_SAMPLE_COUNT_1_BIT,
        .loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp        = VK_ATTACHMENT_STORE_OP_STORE,
        .stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED,
        .finalLayout    = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
    };

    VkAttachmentReference tAttachmentReference = {
        .attachment = 0,
        .layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
    };

    VkSubpassDescription tSubpass = {
        .pipelineBindPoint      = VK_PIPELINE_BIND_POINT_GRAPHICS,
        .colorAttachmentCount   = 1,
        .pColorAttachments      = &tAttachmentReference,
        .pDepthStencilAttachment = VK_NULL_HANDLE
    };

    VkRenderPassCreateInfo tRenderPassInfo = {
        .sType           = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
        .attachmentCount = 1u,
        .pAttachments    = &tColorAttachment,
        .subpassCount    = 1,
        .pSubpasses      = &tSubpass,
        .dependencyCount = 0,
        .pDependencies   = VK_NULL_HANDLE
    };
    DPG_VULKAN(vkCreateRenderPass(gtDevice.tLogicalDevice, &tRenderPassInfo, NULL, &gtGraphics.tRenderPass));

    // create frame buffers
    dpg_create_framebuffers(&gtDevice, gtGraphics.tRenderPass, &gtSwapchain);
    
    // create per frame resources
    dpg_create_frame_resources(&gtGraphics, &gtDevice);

    glfwSetWindowSizeCallback(ptWindow, dpg_window_size_callback);

    while(!glfwWindowShouldClose(ptWindow)) 
    {
        glfwPollEvents();
        dpg_update(ptWindow);
    }

    glfwDestroyWindow(ptWindow);

    glfwTerminate();

}

//-----------------------------------------------------------------------------
// [SECTION] implementations
//-----------------------------------------------------------------------------

static void
dpg_update(GLFWwindow* ptWindow)
{
    int iWidth = 0;
    int iHeight = 0;
    glfwGetFramebufferSize(ptWindow, &iWidth, &iHeight);

    dpgVulkanFrameContext* ptCurrentFrame = dpg_get_frame_resources(&gtGraphics);

    // begin frame
    DPG_VULKAN(vkWaitForFences(gtDevice.tLogicalDevice, 1, &ptCurrentFrame->tInFlight, VK_TRUE, UINT64_MAX));
    VkResult tErr = vkAcquireNextImageKHR(gtDevice.tLogicalDevice, gtSwapchain.tSwapChain, UINT64_MAX, ptCurrentFrame->tImageAvailable, VK_NULL_HANDLE, &gtSwapchain.uCurrentImageIndex);
    if(tErr == VK_SUBOPTIMAL_KHR || tErr == VK_ERROR_OUT_OF_DATE_KHR)
    {
        if(tErr == VK_ERROR_OUT_OF_DATE_KHR)
        {
            dpg_create_swapchain(&gtDevice, gtGraphics.tSurface, (uint32_t)iWidth, (uint32_t)iHeight, &gtSwapchain);
            dpg_create_framebuffers(&gtDevice, gtGraphics.tRenderPass, &gtSwapchain);
            return;
        }
    }
    else
    {
        DPG_VULKAN(tErr);
    }

    if (ptCurrentFrame->tInFlight != VK_NULL_HANDLE)
        DPG_VULKAN(vkWaitForFences(gtDevice.tLogicalDevice, 1, &ptCurrentFrame->tInFlight, VK_TRUE, UINT64_MAX));

    // begin recording
    VkCommandBufferBeginInfo tBeginInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO
    };
    DPG_VULKAN(vkBeginCommandBuffer(ptCurrentFrame->tCmdBuf, &tBeginInfo));

    // begin render pass

    static const VkClearValue atClearValues[2] = 
    {
        {
            .color.float32[0] = 0.1f,
            .color.float32[1] = 0.0f,
            .color.float32[2] = 0.0f,
            .color.float32[3] = 1.0f
        },
        {
            .depthStencil.depth = 1.0f,
            .depthStencil.stencil = 0
        }    
    };

    VkRenderPassBeginInfo tRenderPassBeginInfo = {
        .sType               = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
        .renderPass          = gtGraphics.tRenderPass,
        .framebuffer         = gtSwapchain.ptFrameBuffers[gtSwapchain.uCurrentImageIndex],
        .renderArea.offset.x = 0,
        .renderArea.offset.y = 0,
        .renderArea.extent   = gtSwapchain.tExtent,
        .clearValueCount     = 2,
        .pClearValues        = atClearValues
    };
    vkCmdBeginRenderPass(ptCurrentFrame->tCmdBuf, &tRenderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

    // set viewport
    VkViewport tViewport = {
        .x        = 0.0f,
        .y        = 0.0f,
        .width    = (float)gtSwapchain.tExtent.width,
        .height   = (float)gtSwapchain.tExtent.height,
        .minDepth = 0.0f,
        .maxDepth = 1.0f
    };
    vkCmdSetViewport(ptCurrentFrame->tCmdBuf, 0, 1, &tViewport);

    // set scissor
    VkRect2D tDynamicScissor = {.extent = gtSwapchain.tExtent};
    vkCmdSetScissor(ptCurrentFrame->tCmdBuf, 0, 1, &tDynamicScissor);

    // draw stuff

    // end render pass
    vkCmdEndRenderPass(ptCurrentFrame->tCmdBuf);

    // end recording
    DPG_VULKAN(vkEndCommandBuffer(ptCurrentFrame->tCmdBuf));

    // submit
    VkPipelineStageFlags atWaitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
    VkSubmitInfo tSubmitInfo = {
        .sType                = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .waitSemaphoreCount   = 1,
        .pWaitSemaphores      = &ptCurrentFrame->tImageAvailable,
        .pWaitDstStageMask    = atWaitStages,
        .commandBufferCount   = 1,
        .pCommandBuffers      = &ptCurrentFrame->tCmdBuf,
        .signalSemaphoreCount = 1,
        .pSignalSemaphores    = &ptCurrentFrame->tRenderFinish
    };
    DPG_VULKAN(vkResetFences(gtDevice.tLogicalDevice, 1, &ptCurrentFrame->tInFlight));
    DPG_VULKAN(vkQueueSubmit(gtDevice.tGraphicsQueue, 1, &tSubmitInfo, ptCurrentFrame->tInFlight));          
    
    // present                        
    VkPresentInfoKHR tPresentInfo = {
        .sType              = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores    = &ptCurrentFrame->tRenderFinish,
        .swapchainCount     = 1,
        .pSwapchains        = &gtSwapchain.tSwapChain,
        .pImageIndices      = &gtSwapchain.uCurrentImageIndex,
    };
    tErr = vkQueuePresentKHR(gtDevice.tPresentQueue, &tPresentInfo);
    if(tErr == VK_SUBOPTIMAL_KHR || tErr == VK_ERROR_OUT_OF_DATE_KHR)
    {
        dpg_create_swapchain(&gtDevice, gtGraphics.tSurface, (uint32_t)iWidth, (uint32_t)iHeight, &gtSwapchain);
        dpg_create_framebuffers(&gtDevice, gtGraphics.tRenderPass, &gtSwapchain);
    }
    else
    {
        DPG_VULKAN(tErr);
    }

    gtGraphics.szCurrentFrameIndex = (gtGraphics.szCurrentFrameIndex + 1) % gtGraphics.uFramesInFlight;
}

static void
dpg_window_size_callback(GLFWwindow* ptWindow, int iWidth, int iHeight)
{
    glfwGetFramebufferSize(ptWindow, &iWidth, &iHeight);
    dpg_create_swapchain(&gtDevice, gtGraphics.tSurface, (uint32_t)iWidth, (uint32_t)iHeight, &gtSwapchain);
    dpg_create_framebuffers(&gtDevice, gtGraphics.tRenderPass, &gtSwapchain);
    dpg_update(ptWindow);
}