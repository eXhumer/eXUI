#if !defined(UI_STATE_HPP)
#define UI_STATE_HPP
#include <nanovg_dk.h>
#include "eXUI/perf.hpp"

namespace eXUI
{
    class FontStash
    {
    public:
        FontStash(NVGcontext *nvgCtx);
        ~FontStash();

    private:
        int m_standard;
        int m_korean;
        int m_sharedSymbols;
    };

    class DkUIState
    {
    private:
        nvg::DkRenderer *m_renderer;
        NVGcontext* m_vg;
        uint32_t m_w, m_h;
        FontStash *m_fontStash;
        PerfGraph *m_fps;
        PadState m_pad;
      	float m_prevTime;

    public:
        DkUIState(nvg::DkRenderer *renderer, uint32_t w = 1280, uint32_t h = 720);
        ~DkUIState();
		void render(u64 ns, float fbW, float fbH);
        bool onFrame(u64 ns);
    };
} // namespace eXUI
#endif /* UI_STATE_HPP */