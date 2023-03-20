/*
 * This file is part of JS80P, a synthesizer plugin.
 * Copyright (C) 2023  Attila M. Magyar
 *
 * JS80P is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * JS80P is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#ifndef JS80P__GUI_STUBS_CPP
#define JS80P__GUI_STUBS_CPP

#include "js80p.hpp"
#include "synth.hpp"

#include "gui/gui.hpp"


namespace JS80P {

class Widget : public WidgetBase
{
    public:
        Widget()
        {
        }

        virtual ~Widget()
        {
            destroy_children();
        }

        virtual GUI::Bitmap load_bitmap(
                GUI::PlatformData platform_data,
                char const* name
        ) override {
            return (GUI::Bitmap)new DummyObject();
        }

        virtual void delete_bitmap(GUI::Bitmap bitmap) override
        {
            delete (DummyObject*)bitmap;
        }

    protected:
        Widget(
                char const* const label,
                int const left,
                int const top,
                int const width,
                int const height,
                Type const type = Type::CLICKABLE
        ) : WidgetBase(left, top, width, height)
        {
        }

        Widget(GUI::PlatformData platform_data, GUI::PlatformWidget platform_widget)
            : WidgetBase(platform_data, platform_widget)
        {
        }

        GUI::Bitmap copy_bitmap_region(
                GUI::Bitmap source,
                int const left,
                int const top,
                int const width,
                int const height
        ) {
            return (GUI::Bitmap)new DummyObject();
        }

    private:
        class DummyObject
        {
        };
};

}


#include "gui/widgets.hpp"


namespace JS80P
{

void ImportPatchButton::click()
{
}


void ExportPatchButton::click()
{
}

}

#endif
