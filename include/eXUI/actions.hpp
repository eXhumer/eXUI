/*
    eXUI, Nintendo Switch UI Library
    Copyright (C) 2021 eXhumer

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

#if !defined(ACTIONS_HPP)
#define ACTIONS_HPP
#include <functional>
#include <switch.h>
#include <string>

namespace eXUI::Actions
{
    typedef std::function<bool(void)> ActionListener;

    enum class NXButton
    {
        A = HidNpadButton_A,
        B = HidNpadButton_B,
        X = HidNpadButton_X,
        Y = HidNpadButton_Y,
        StickL = HidNpadButton_StickL,
        StickR = HidNpadButton_StickR,
        L = HidNpadButton_L,
        R = HidNpadButton_R,
        ZL = HidNpadButton_ZL,
        ZR = HidNpadButton_ZR,
        Plus = HidNpadButton_Plus,
        Minus = HidNpadButton_Minus,
        DpadLeft = HidNpadButton_Left,
        DpadUp = HidNpadButton_Up,
        DpadRight = HidNpadButton_Right,
        DpadDown = HidNpadButton_Down,
        StickLLeft = HidNpadButton_StickLLeft,
        StickLUp = HidNpadButton_StickLUp,
        StickLRight = HidNpadButton_StickLRight,
        StickLDown = HidNpadButton_StickLDown,
        StickRLeft = HidNpadButton_StickRLeft,
        StickRUp = HidNpadButton_StickRUp,
        StickRRight = HidNpadButton_StickRRight,
        StickRDown = HidNpadButton_StickRDown,
        LeftSL = HidNpadButton_LeftSL,
        LeftSR = HidNpadButton_LeftSR,
        RightSL = HidNpadButton_RightSL,
        RightSR = HidNpadButton_RightSR,
        Palma = HidNpadButton_Palma,
        Bit29 = HidNpadButton_29, // undocumented on libnx
        HandheldLeftB = HidNpadButton_HandheldLeftB,
        AnyLeft = HidNpadButton_AnyLeft,
        AnyUp = HidNpadButton_AnyUp,
        AnyRight = HidNpadButton_AnyRight,
        AnyDown = HidNpadButton_AnyDown,
        AnySL = HidNpadButton_AnySL,
        AnySR = HidNpadButton_AnySR,
    };

    struct Action
    {
        NXButton nxBtn;
        std::string hintText;
        bool available;
        bool hidden;
        ActionListener actionListener;

        bool operator==(const NXButton other)
        {
            return this->nxBtn == other;
        }
    };
} // namespace eXUI
#endif /* ACTIONS_HPP */