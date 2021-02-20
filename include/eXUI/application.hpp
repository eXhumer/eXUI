#if !defined(APPLICATION_HPP)
#define APPLICATION_HPP
#include <nanovg/framework/CApplication.h>
#include <nanovg/framework/CMemPool.h>
#include "eXUI/ui_state.hpp"

namespace eXUI
{
	static constexpr unsigned NumFramebuffers = 2;
	static constexpr unsigned StaticCmdSize = 0x1000;

	class DkApplication : public CApplication
	{
	public:
		DkApplication();
		~DkApplication();

	private:
		const uint32_t FramebufferWidth = 1280;
		const uint32_t FramebufferHeight = 720;
		uint32_t OutputWidth;
		uint32_t OutputHeight;

		dk::UniqueDevice m_device;
		dk::UniqueQueue m_queue;
		dk::UniqueSwapchain m_swapchain;

		std::optional<CMemPool> m_pool_images;
		std::optional<CMemPool> m_pool_code;
		std::optional<CMemPool> m_pool_data;

		dk::UniqueCmdBuf m_cmdbuf;
		DkCmdList m_render_cmdlist;

		dk::Image m_depthBuffer;
		CMemPool::Handle m_depthBuffer_mem;
		dk::Image m_framebuffers[NumFramebuffers];
		CMemPool::Handle m_framebuffers_mem[NumFramebuffers];
		DkCmdList m_framebuffer_cmdlists[NumFramebuffers];

		std::optional<nvg::DkRenderer> m_renderer;
		std::optional<DkUIState> m_uiState;

		void createFramebufferResources();
		void recordStaticCommands();
		void destroyFramebufferResources();
		void onFramebufferDimensionChange();
		void render(u64 ns);

	protected:
		bool onFrame(u64 ns) override;
		void onOperationMode(AppletOperationMode opMode) override;
	};
} // namespace eXUI
#endif /* APPLICATION_HPP */