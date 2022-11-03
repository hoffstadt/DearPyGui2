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
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"

//-----------------------------------------------------------------------------
// [SECTION] globals
//-----------------------------------------------------------------------------

dpgVulkanDevice    gtDevice;
dpgVulkanGraphics  gtGraphics;
dpgVulkanSwapchain gtSwapchain;
VkDescriptorPool   gtDescriptorPool;

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
    VkAttachmentDescription tColorAttachment = {};
    tColorAttachment.flags          = VK_ATTACHMENT_DESCRIPTION_MAY_ALIAS_BIT;
    tColorAttachment.format         = gtSwapchain.tFormat;
    tColorAttachment.samples        = VK_SAMPLE_COUNT_1_BIT;
    tColorAttachment.loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
    tColorAttachment.storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
    tColorAttachment.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    tColorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    tColorAttachment.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
    tColorAttachment.finalLayout    = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference tAttachmentReference = {};
    tAttachmentReference.attachment = 0;
    tAttachmentReference.layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription tSubpass = {};
    tSubpass.pipelineBindPoint      = VK_PIPELINE_BIND_POINT_GRAPHICS;
    tSubpass.colorAttachmentCount   = 1;
    tSubpass.pColorAttachments      = &tAttachmentReference;
    tSubpass.pDepthStencilAttachment = VK_NULL_HANDLE;

    VkRenderPassCreateInfo tRenderPassInfo = {};
    tRenderPassInfo.sType           = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    tRenderPassInfo.attachmentCount = 1u;
    tRenderPassInfo.pAttachments    = &tColorAttachment;
    tRenderPassInfo.subpassCount    = 1;
    tRenderPassInfo.pSubpasses      = &tSubpass;
    tRenderPassInfo.dependencyCount = 0;
    tRenderPassInfo.pDependencies   = VK_NULL_HANDLE;
    DPG_VULKAN(vkCreateRenderPass(gtDevice.tLogicalDevice, &tRenderPassInfo, NULL, &gtGraphics.tRenderPass));

    VkDescriptorPoolSize atDescriptorPoolSizes[] =
    {
        { VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
        { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
        { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
        { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
    };
    VkDescriptorPoolCreateInfo tPoolInfo = {};
    tPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    tPoolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    tPoolInfo.maxSets = 1000 * IM_ARRAYSIZE(atDescriptorPoolSizes);
    tPoolInfo.poolSizeCount = (uint32_t)IM_ARRAYSIZE(atDescriptorPoolSizes);
    tPoolInfo.pPoolSizes = atDescriptorPoolSizes;
    DPG_VULKAN(vkCreateDescriptorPool(gtDevice.tLogicalDevice, &tPoolInfo, NULL, &gtDescriptorPool));

    // create frame buffers
    dpg_create_framebuffers(&gtDevice, gtGraphics.tRenderPass, &gtSwapchain);
    
    // create per frame resources
    dpg_create_frame_resources(&gtGraphics, &gtDevice);

    // setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;

    // setup Dear ImGui style
    ImGui::StyleColorsDark();

    // setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForVulkan(ptWindow, true);
    ImGui_ImplVulkan_InitInfo init_info = {};
    init_info.Instance = gtGraphics.tInstance;
    init_info.PhysicalDevice = gtDevice.tPhysicalDevice;
    init_info.Device = gtDevice.tLogicalDevice;
    init_info.QueueFamily = gtDevice.iGraphicsQueueFamily;
    init_info.Queue = gtDevice.tGraphicsQueue;
    init_info.PipelineCache = VK_NULL_HANDLE;
    init_info.DescriptorPool = gtDescriptorPool;
    init_info.Subpass = 0;
    init_info.MinImageCount = gtSwapchain.uImageCount;
    init_info.ImageCount = gtSwapchain.uImageCount;
    init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
    init_info.Allocator = VK_NULL_HANDLE;
    init_info.CheckVkResultFn = NULL;
    ImGui_ImplVulkan_Init(&init_info, gtGraphics.tRenderPass);

    // Upload Fonts
    dpgVulkanFrameContext* ptCurrentFrame = dpg_get_frame_resources(&gtGraphics);

    VkCommandBufferBeginInfo tBeginCommandBuffer = {};
    tBeginCommandBuffer.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    tBeginCommandBuffer.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    DPG_VULKAN(vkBeginCommandBuffer(ptCurrentFrame->tCmdBuf, &tBeginCommandBuffer));
    ImGui_ImplVulkan_CreateFontsTexture(ptCurrentFrame->tCmdBuf);

    VkSubmitInfo tSubmitCommandBuffer = {};
    tSubmitCommandBuffer.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    tSubmitCommandBuffer.commandBufferCount = 1;
    tSubmitCommandBuffer.pCommandBuffers = &ptCurrentFrame->tCmdBuf;
    DPG_VULKAN(vkEndCommandBuffer(ptCurrentFrame->tCmdBuf));
    DPG_VULKAN(vkQueueSubmit(gtDevice.tGraphicsQueue, 1, &tSubmitCommandBuffer, VK_NULL_HANDLE));
    DPG_VULKAN(vkDeviceWaitIdle(gtDevice.tLogicalDevice));
    ImGui_ImplVulkan_DestroyFontUploadObjects();

    glfwSetWindowSizeCallback(ptWindow, dpg_window_size_callback);

    while(!glfwWindowShouldClose(ptWindow)) 
    {
        glfwPollEvents();
        dpg_update(ptWindow);
    }

    vkDeviceWaitIdle(gtDevice.tLogicalDevice);
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    vkDestroyDescriptorPool(gtDevice.tLogicalDevice, gtDescriptorPool, NULL);

    // destroy swapchain
    for (uint32_t i = 0u; i < gtSwapchain.uImageCount; i++)
    {
        vkDestroyImageView(gtDevice.tLogicalDevice, gtSwapchain.ptImageViews[i], NULL);
        vkDestroyFramebuffer(gtDevice.tLogicalDevice, gtSwapchain.ptFrameBuffers[i], NULL);
    }

    // destroy default render pass
    vkDestroyRenderPass(gtDevice.tLogicalDevice, gtGraphics.tRenderPass, NULL);
    vkDestroySwapchainKHR(gtDevice.tLogicalDevice, gtSwapchain.tSwapChain, NULL);

    dpg_cleanup_graphics(&gtGraphics, &gtDevice);

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
    VkCommandBufferBeginInfo tBeginInfo = {};
    tBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    DPG_VULKAN(vkBeginCommandBuffer(ptCurrentFrame->tCmdBuf, &tBeginInfo));

    // begin render pass
    static VkClearValue atClearValues[2] = {};
    atClearValues[0].color.float32[0] = 0.1f;
    atClearValues[0].color.float32[1] = 0.0f;
    atClearValues[0].color.float32[2] = 0.0f;
    atClearValues[0].color.float32[3] = 1.0f;
    atClearValues[1].depthStencil.depth = 1.0f;
    atClearValues[1].depthStencil.stencil = 0;

    VkRenderPassBeginInfo tRenderPassBeginInfo = {};
    tRenderPassBeginInfo.sType               = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    tRenderPassBeginInfo.renderPass          = gtGraphics.tRenderPass;
    tRenderPassBeginInfo.framebuffer         = gtSwapchain.ptFrameBuffers[gtSwapchain.uCurrentImageIndex];
    tRenderPassBeginInfo.renderArea.offset.x = 0;
    tRenderPassBeginInfo.renderArea.offset.y = 0;
    tRenderPassBeginInfo.renderArea.extent   = gtSwapchain.tExtent;
    tRenderPassBeginInfo.clearValueCount     = 2;
    tRenderPassBeginInfo.pClearValues        = atClearValues;
    vkCmdBeginRenderPass(ptCurrentFrame->tCmdBuf, &tRenderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

    // set viewport
    VkViewport tViewport = {};
    tViewport.x        = 0.0f;
    tViewport.y        = 0.0f;
    tViewport.width    = (float)gtSwapchain.tExtent.width;
    tViewport.height   = (float)gtSwapchain.tExtent.height;
    tViewport.minDepth = 0.0f;
    tViewport.maxDepth = 1.0f;
    vkCmdSetViewport(ptCurrentFrame->tCmdBuf, 0, 1, &tViewport);

    // set scissor
    VkRect2D tDynamicScissor = {};
    tDynamicScissor.extent = gtSwapchain.tExtent;
    vkCmdSetScissor(ptCurrentFrame->tCmdBuf, 0, 1, &tDynamicScissor);

    // dear imgui
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    ImGui::ShowDemoWindow();
    ImGui::Render();
    ImDrawData* draw_data = ImGui::GetDrawData();
    ImGui_ImplVulkan_RenderDrawData(draw_data, ptCurrentFrame->tCmdBuf);

    // end render pass
    vkCmdEndRenderPass(ptCurrentFrame->tCmdBuf);

    // end recording
    DPG_VULKAN(vkEndCommandBuffer(ptCurrentFrame->tCmdBuf));

    // submit
    VkPipelineStageFlags atWaitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
    VkSubmitInfo tSubmitInfo = {};
    tSubmitInfo.sType                = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    tSubmitInfo.waitSemaphoreCount   = 1;
    tSubmitInfo.pWaitSemaphores      = &ptCurrentFrame->tImageAvailable;
    tSubmitInfo.pWaitDstStageMask    = atWaitStages;
    tSubmitInfo.commandBufferCount   = 1;
    tSubmitInfo.pCommandBuffers      = &ptCurrentFrame->tCmdBuf;
    tSubmitInfo.signalSemaphoreCount = 1;
    tSubmitInfo.pSignalSemaphores    = &ptCurrentFrame->tRenderFinish;

    DPG_VULKAN(vkResetFences(gtDevice.tLogicalDevice, 1, &ptCurrentFrame->tInFlight));
    DPG_VULKAN(vkQueueSubmit(gtDevice.tGraphicsQueue, 1, &tSubmitInfo, ptCurrentFrame->tInFlight));          
    
    // present                        
    VkPresentInfoKHR tPresentInfo = {};
    tPresentInfo.sType              = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    tPresentInfo.waitSemaphoreCount = 1;
    tPresentInfo.pWaitSemaphores    = &ptCurrentFrame->tRenderFinish;
    tPresentInfo.swapchainCount     = 1;
    tPresentInfo.pSwapchains        = &gtSwapchain.tSwapChain;
    tPresentInfo.pImageIndices      = &gtSwapchain.uCurrentImageIndex;
    
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
    ImGui_ImplVulkan_SetMinImageCount(gtSwapchain.uImageCount);
    dpg_update(ptWindow);
}