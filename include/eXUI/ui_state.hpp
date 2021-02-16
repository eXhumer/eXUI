#if !defined(UI_STATE_HPP)
#define UI_STATE_HPP
#include <nanovg_dk.h>

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
        FontStash *m_fontStash;
        PadState m_pad;

    public:
        DkUIState(nvg::DkRenderer *renderer);
        ~DkUIState();
		void render(u64 ns, float windowW, float windowH, float devicePixelRatio);
        bool onFrame(u64 ns);
    };
} // namespace eXUI
#endif /* UI_STATE_HPP */