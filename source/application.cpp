#include "eXUI/application.hpp"
#include "eXUI/logger.hpp"
#include <cstring>
#include <unistd.h>

static const SocketInitConfig socket_config =
{
    .bsdsockets_version = 1,
    .tcp_tx_buf_size = 0x8000,
    .tcp_rx_buf_size = 0x10000,
    .tcp_tx_buf_max_size = 0x40000,
    .tcp_rx_buf_max_size = 0x40000,
    .udp_tx_buf_size = 0x2400,
    .udp_rx_buf_size = 0xA500,
    .sb_efficiency = 4,
    .num_bsd_sessions = 3,
    .bsd_service_type = BsdServiceType_User,
};

#if defined(DEBUG_NXLINK)
static int nxlinkSocket = -1;
#endif /* DEBUG_NXLINK */

float ieee_float(uint32_t f)
{
    static_assert(sizeof(float) == sizeof f, "`float` has a weird size.");
    float ret;
    std::memcpy(&ret, &f, sizeof(float));
    return ret;
}

namespace eXUI
{
    void OutputDkDebug(void* userData, const char* context, DkResult result, const char* message)
    {
        Logger::debug("Context: %s\nResult: %d\nMessage: %s\n", context, result, message);
    }

    DkApplication::DkApplication()
    {
        Logger::setLogLevel(LogLevel::DEBUG);

        Result res;

        if (R_FAILED(res = romfsInit()))
            diagAbortWithResult(res);

        if (R_FAILED(res = socketInitialize(&socket_config)))
            diagAbortWithResult(res);

#if defined(DEBUG_NXLINK)
        nxlinkSocket = nxlinkStdio();
        Logger::debug("nxlink host is %s", inet_ntoa(__nxlink_host));
#endif /* DEBUG_NXLINK */

        AppletOperationMode initOpMode = appletGetOperationMode();
        FramebufferWidth = (initOpMode == AppletOperationMode_Console) ? 1080 : 720;
        FramebufferHeight = (initOpMode == AppletOperationMode_Console) ? 1920 : 1280;

        this->m_device = dk::DeviceMaker{}.setCbDebug(OutputDkDebug).create();
        this->m_queue = dk::QueueMaker{this->m_device}.setFlags(DkQueueFlags_Graphics).create();

        this->m_pool_images.emplace(this->m_device, DkMemBlockFlags_GpuCached | DkMemBlockFlags_Image, 16*1024*1024);
        this->m_pool_code.emplace(this->m_device, DkMemBlockFlags_CpuUncached | DkMemBlockFlags_GpuCached | DkMemBlockFlags_Code, 128*1024);
        this->m_pool_data.emplace(this->m_device, DkMemBlockFlags_CpuUncached | DkMemBlockFlags_GpuCached, 1*1024*1024);

        this->m_cmdbuf = dk::CmdBufMaker{this->m_device}.create();
        CMemPool::Handle cmdmem = this->m_pool_data->allocate(StaticCmdSize);
        this->m_cmdbuf.addMemory(cmdmem.getMemBlock(), cmdmem.getOffset(), cmdmem.getSize());

        this->createFramebufferResources();

        this->m_renderer.emplace(FramebufferWidth, FramebufferHeight, this->m_device, this->m_queue, *this->m_pool_images, *this->m_pool_code, *this->m_pool_data);
        this->m_uiState.emplace(&*this->m_renderer);
    }

    DkApplication::~DkApplication()
    {
        // Destroy the framebuffer resources. This should be done first.
        this->destroyFramebufferResources();

        // Destroy the renderer
        this->m_renderer.reset();

#if defined(DEBUG_NXLINK)
        if (nxlinkSocket != -1)
        {
            close(nxlinkSocket);
            nxlinkSocket = -1;
        }
#endif /* DEBUG_NXLINK */
        socketExit();
        romfsExit();
    }

    void DkApplication::createFramebufferResources()
    {
        // Create layout for the depth buffer
        dk::ImageLayout layout_depthbuffer;
        dk::ImageLayoutMaker{this->m_device}
            .setFlags(DkImageFlags_UsageRender | DkImageFlags_HwCompression)
            .setFormat(DkImageFormat_S8)
            .setDimensions(FramebufferWidth, FramebufferHeight)
            .initialize(layout_depthbuffer);

        // Create the depth buffer
        this->m_depthBuffer_mem = this->m_pool_images->allocate(layout_depthbuffer.getSize(), layout_depthbuffer.getAlignment());
        this->m_depthBuffer.initialize(layout_depthbuffer, this->m_depthBuffer_mem.getMemBlock(), this->m_depthBuffer_mem.getOffset());

        // Create layout for the framebuffers
        dk::ImageLayout layout_framebuffer;
        dk::ImageLayoutMaker{this->m_device}
            .setFlags(DkImageFlags_UsageRender | DkImageFlags_UsagePresent | DkImageFlags_HwCompression)
            .setFormat(DkImageFormat_RGBA8_Unorm)
            .setDimensions(FramebufferWidth, FramebufferHeight)
            .initialize(layout_framebuffer);

        // Create the framebuffers
        std::array<DkImage const*, NumFramebuffers> fb_array;
        uint64_t fb_size  = layout_framebuffer.getSize();
        uint32_t fb_align = layout_framebuffer.getAlignment();
        for (unsigned i = 0; i < NumFramebuffers; i ++)
        {
            // Allocate a framebuffer
            this->m_framebuffers_mem[i] = this->m_pool_images->allocate(fb_size, fb_align);
            this->m_framebuffers[i].initialize(layout_framebuffer, this->m_framebuffers_mem[i].getMemBlock(), this->m_framebuffers_mem[i].getOffset());

            // Generate a command list that binds it
            dk::ImageView colorTarget{ this->m_framebuffers[i] }, depthTarget{ this->m_depthBuffer };
            this->m_cmdbuf.bindRenderTargets(&colorTarget, &depthTarget);
            this->m_framebuffer_cmdlists[i] = this->m_cmdbuf.finishList();

            // Fill in the array for use later by the swapchain creation code
            fb_array[i] = &this->m_framebuffers[i];
        }

        // Create the swapchain using the framebuffers
        this->m_swapchain = dk::SwapchainMaker{this->m_device, nwindowGetDefault(), fb_array}.create();

        // Generate the main rendering cmdlist
        this->recordStaticCommands();
    }

    void DkApplication::destroyFramebufferResources()
    {
        // Return early if we have nothing to destroy
        if (!this->m_swapchain) return;

        // Make sure the queue is idle before destroying anything
        this->m_queue.waitIdle();

        // Clear the static cmdbuf, destroying the static cmdlists in the process
        this->m_cmdbuf.clear();

        // Destroy the swapchain
        this->m_swapchain.destroy();

        // Destroy the framebuffers
        for (unsigned i = 0; i < NumFramebuffers; i ++)
            this->m_framebuffers_mem[i].destroy();

        // Destroy the depth buffer
        this->m_depthBuffer_mem.destroy();
    }

    void DkApplication::recordStaticCommands()
    {
        // Initialize state structs with deko3d defaults
        dk::RasterizerState rasterizerState;
        dk::ColorState colorState;
        dk::ColorWriteState colorWriteState;
        dk::BlendState blendState;

        // Configure the viewport and scissor
        this->m_cmdbuf.setViewports(0, { { 0.0f, 0.0f, ieee_float(FramebufferWidth), ieee_float(FramebufferHeight), 0.0f, 1.0f } });
        this->m_cmdbuf.setScissors(0, { { 0, 0, FramebufferWidth, FramebufferHeight } });

        // Clear the color and depth buffers
        this->m_cmdbuf.clearColor(0, DkColorMask_RGBA, 0.2f, 0.3f, 0.3f, 1.0f);
        this->m_cmdbuf.clearDepthStencil(true, 1.0f, 0xFF, 0);

        // Bind required state
        this->m_cmdbuf.bindRasterizerState(rasterizerState);
        this->m_cmdbuf.bindColorState(colorState);
        this->m_cmdbuf.bindColorWriteState(colorWriteState);

        this->m_render_cmdlist = this->m_cmdbuf.finishList();
    }

    void DkApplication::onFramebufferDimensionChange()
    {
        this->destroyFramebufferResources();
        this->m_uiState.reset();
        this->m_renderer.reset();
        this->createFramebufferResources();
        this->m_renderer.emplace(FramebufferWidth, FramebufferHeight, this->m_device, this->m_queue, *this->m_pool_images, *this->m_pool_code, *this->m_pool_data);
        this->m_uiState.emplace(&*this->m_renderer);
    }

    void DkApplication::render(u64 ns)
    {
        // Acquire a framebuffer from the swapchain (and wait for it to be available)
        int slot = this->m_queue.acquireImage(this->m_swapchain);

        // Run the command list that attaches said framebuffer to the queue
        this->m_queue.submitCommands(this->m_framebuffer_cmdlists[slot]);

        // Run the main rendering command list
        this->m_queue.submitCommands(this->m_render_cmdlist);

        this->m_uiState->render(ns, FramebufferWidth, FramebufferHeight, 1.0f);

        // Now that we are done rendering, present it to the screen
        this->m_queue.presentImage(this->m_swapchain, slot);
    }

    bool DkApplication::onFrame(u64 ns)
    {
        if (!this->m_uiState->onFrame(ns))
            return false;

        this->render(ns);
        return true;
    }

    void DkApplication::onOperationMode(AppletOperationMode opMode)
    {
        switch (opMode)
        {
        case AppletOperationMode_Handheld:
            FramebufferHeight = 1280;
            FramebufferWidth = 720;
            break;
        
        case AppletOperationMode_Console:
            FramebufferHeight = 1920;
            FramebufferWidth = 1080;
            break;

        default:
            break;
        }

        this->onFramebufferDimensionChange();
    }
} // namespace eXUI