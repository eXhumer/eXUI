#include "eXUI/ui_state.hpp"

namespace eXUI
{
    FontStash::FontStash(NVGcontext *nvgCtx)
    : m_standard(0),
    m_korean(0),
    m_sharedSymbols(0)
    {
        Result rc;
        PlFontData font;

        if(R_FAILED(rc = plInitialize(PlServiceType_User)))
            diagAbortWithResult(rc);

        if(R_FAILED(rc = plGetSharedFontByType(&font, PlSharedFontType_Standard)))
            diagAbortWithResult(rc);

        this->m_standard = nvgCreateFontMem(nvgCtx, "switch-standard", static_cast<u8*>(font.address), font.size, 0);

        if(R_FAILED(rc = plGetSharedFontByType(&font, PlSharedFontType_KO)))
            diagAbortWithResult(rc);

        this->m_korean = nvgCreateFontMem(nvgCtx, "korean", static_cast<u8*>(font.address), font.size, 0);
        nvgAddFallbackFontId(nvgCtx, this->m_standard, this->m_korean);

        if(R_FAILED(rc = plGetSharedFontByType(&font, PlSharedFontType_NintendoExt)))
            diagAbortWithResult(rc);

        this->m_sharedSymbols = nvgCreateFontMem(nvgCtx, "symbols", static_cast<u8*>(font.address), font.size, 0);
    }

    FontStash::~FontStash()
    {
        plExit();
    }

    DkUIState::DkUIState(nvg::DkRenderer *renderer, uint32_t w, uint32_t h)
    {
        this->m_w = w;
        this->m_h = h;
        this->m_renderer = renderer;
        this->m_vg = nvgCreateDk(this->m_renderer, NVG_ANTIALIAS | NVG_STENCIL_STROKES);
        this->m_fontStash = new FontStash(this->m_vg);
        this->m_fps = new PerfGraph(RenderStyle::FPS, "Frame Timing");
        padConfigureInput(1, HidNpadStyleSet_NpadStandard);
        padInitializeDefault(&this->m_pad);
    }

    DkUIState::~DkUIState()
    {
        delete this->m_fps;
        this->m_fps = nullptr;
        delete this->m_fontStash;
        this->m_fontStash = nullptr;
        nvgDeleteDk(this->m_vg);
        this->m_vg = nullptr;
        this->m_renderer = nullptr;
    }

    void DkUIState::render(u64 ns, float fbW, float fbH)
    {
        float time = ns / 1000000000.0;
        float dt = time - this->m_prevTime;
        this->m_prevTime = time;

        this->m_fps->update(dt);

        nvgBeginFrame(this->m_vg, fbW, fbH, 1.0f);
        nvgScale(this->m_vg, fbW / this->m_w, fbH / this->m_h);
        {
            this->m_fps->render(this->m_vg, 5, 5);
        }
        nvgEndFrame(this->m_vg);
    }

    bool DkUIState::onFrame(u64 ns)
    {
        padUpdate(&this->m_pad);
        u64 kDown = padGetButtonsDown(&this->m_pad);

        if (kDown & KEY_A)
            this->m_fps->nextStyle();

        if (kDown & KEY_PLUS)
            return false;

        return true;
    }
} // namespace eXUI