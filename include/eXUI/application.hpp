#if !defined(APPLICATION_HPP)
#define APPLICATION_HPP
#include <nanovg/framework/CApplication.h>
#include <nanovg/framework/CMemPool.h>
#include <nanovg_dk.h>
#include <optional>

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
		static constexpr uint32_t FramebufferWidth = 1280;
		static constexpr uint32_t FramebufferHeight = 720;

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
		NVGcontext* m_vg;

		// float m_prevTime;
		int m_standard_font;
		PadState m_pad;

		void createFramebufferResources();
		void recordStaticCommands();
		void destroyFramebufferResources();
		void render(u64 ns);

	protected:
		bool onFrame(u64 ns);
	};
} // namespace eXUI
#endif /* APPLICATION_HPP */