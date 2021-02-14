/*
    eXUI, Nintendo Switch UI Library
    Copyright (C) 2019 natinusala
    Copyright (C) 2019 p-sam

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/
#if !defined(FRAME_CONTEXT_HPP)
#define FRAME_CONTEXT_HPP
#include <nanovg.h>
#include "eXUI/theme.hpp"

namespace eXUI
{
    class FontStash
    {
    public:
        int regular = 0;
        int korean = 0;

        int material = 0;
        int sharedSymbols = 0;
    };

    class FrameContext
    {
    public:
        NVGcontext* vg       = nullptr;
        float pixelRatio     = 0.0;
        FontStash* fontStash = nullptr;
        Theme* theme         = nullptr;
    };
} // namespace eXUI
#endif /* FRAME_CONTEXT_HPP */