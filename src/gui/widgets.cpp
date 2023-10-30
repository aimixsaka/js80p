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

#ifndef JS80P__GUI__WIDGETS_CPP
#define JS80P__GUI__WIDGETS_CPP

#include <algorithm>
#include <cmath>
#include <cstring>

#include "gui/widgets.hpp"


namespace JS80P
{

ExternallyCreatedWindow::ExternallyCreatedWindow(
        GUI::PlatformData platform_data,
        GUI::PlatformWidget window
) : Widget(platform_data, window, Type::EXTERNALLY_CREATED_WINDOW)
{
}


ExternallyCreatedWindow::~ExternallyCreatedWindow()
{
    platform_widget = NULL;
}


TransparentWidget::TransparentWidget(
        char const* const text,
        int const left,
        int const top,
        int const width,
        int const height,
        Type const type
) : Widget(text, left, top, width, height, type)
{
}


bool TransparentWidget::paint()
{
    Widget::paint();

    return true;
}


ImportPatchButton::ImportPatchButton(
        GUI& gui,
        int const left,
        int const top,
        int const width,
        int const height,
        Synth& synth,
        TabBody* synth_gui_body
) : TransparentWidget("Import Patch", left, top, width, height, Type::IMPORT_PATCH_BUTTON),
    synth(synth),
    synth_gui_body(synth_gui_body)
{
    set_gui(gui);
}


void ImportPatchButton::import_patch(char const* buffer, Integer const size) const
{
    std::string const patch(
        buffer,
        std::min(
            (std::string::size_type)size,
            (std::string::size_type)Serializer::MAX_SIZE
        )
    );

    Serializer::import_patch_in_gui_thread(synth, patch);

    synth_gui_body->stop_editing();
    synth_gui_body->refresh_all_params();
}


bool ImportPatchButton::mouse_move(int const x, int const y, bool const modifier)
{
    TransparentWidget::mouse_move(x, y, modifier);
    gui->set_status_line(text);

    return true;
}


bool ImportPatchButton::mouse_leave(int const x, int const y)
{
    TransparentWidget::mouse_leave(x, y);
    gui->set_status_line("");

    return true;
}


ExportPatchButton::ExportPatchButton(
        GUI& gui,
        int const left,
        int const top,
        int const width,
        int const height,
        Synth& synth
) : TransparentWidget("Export Patch", left, top, width, height, Type::EXPORT_PATCH_BUTTON),
    synth(synth)
{
    set_gui(gui);
}


bool ExportPatchButton::mouse_move(int const x, int const y, bool const modifier)
{
    TransparentWidget::mouse_move(x, y, modifier);
    gui->set_status_line(text);

    return true;
}


bool ExportPatchButton::mouse_leave(int const x, int const y)
{
    TransparentWidget::mouse_leave(x, y);
    gui->set_status_line("");

    return true;
}


TabBody::TabBody(char const* const text)
    : TransparentWidget(text, LEFT, TOP, WIDTH, HEIGHT, Type::TAB_BODY)
{
}


KnobParamEditor* TabBody::own(KnobParamEditor* knob_param_editor)
{
    Widget::own(knob_param_editor);

    knob_param_editors.push_back(knob_param_editor);

    return knob_param_editor;
}


ToggleSwitch* TabBody::own(ToggleSwitch* toggle_switch)
{
    Widget::own(toggle_switch);

    toggle_switches.push_back(toggle_switch);

    return toggle_switch;
}


TuningSelector* TabBody::own(TuningSelector* tuning_selector)
{
    Widget::own(tuning_selector);

    tuning_selectors.push_back(tuning_selector);

    return tuning_selector;
}


void TabBody::stop_editing()
{
    for (GUI::KnobParamEditors::iterator it = knob_param_editors.begin(); it != knob_param_editors.end(); ++it) {
        (*it)->stop_editing();
    }
}


void TabBody::refresh_controlled_knob_param_editors()
{
    for (GUI::KnobParamEditors::iterator it = knob_param_editors.begin(); it != knob_param_editors.end(); ++it) {
        KnobParamEditor* editor = *it;

        if (editor->has_controller()) {
            editor->refresh();
        }
    }
}


void TabBody::refresh_all_params()
{
    for (GUI::KnobParamEditors::iterator it = knob_param_editors.begin(); it != knob_param_editors.end(); ++it) {
        (*it)->refresh();
    }

    for (GUI::ToggleSwitches::iterator it = toggle_switches.begin(); it != toggle_switches.end(); ++it) {
        (*it)->refresh();
    }

    for (GUI::TuningSelectors::iterator it = tuning_selectors.begin(); it != tuning_selectors.end(); ++it) {
        (*it)->refresh();
    }
}


Background::Background()
    : Widget("JS80P", 0, 0, GUI::WIDTH, GUI::HEIGHT, Type::BACKGROUND),
    body(NULL),
    next_full_refresh(FULL_REFRESH_TICKS)
{
}


Background::~Background()
{
}


void Background::replace_body(TabBody* new_body)
{
    if (body != NULL) {
        body->hide();
    }

    body = new_body;
    body->show();
}


void Background::hide_body()
{
    if (body != NULL) {
        body->hide();
    }
}


void Background::show_body()
{
    if (body != NULL) {
        body->show();
    }
}


void Background::refresh()
{
    if (body == NULL) {
        return;
    }

    --next_full_refresh;

    if (next_full_refresh == 0) {
        next_full_refresh = FULL_REFRESH_TICKS;
        body->refresh_all_params();
    } else {
        body->refresh_controlled_knob_param_editors();
    }
}


TabSelector::TabSelector(
        Background* background,
        GUI::Image image,
        TabBody* tab_body,
        char const* const text,
        int const left
) : TransparentWidget(text, left, TOP, WIDTH, HEIGHT, Type::TAB_SELECTOR),
    background(background),
    tab_body(tab_body),
    image(image)
{
}


void TabSelector::click()
{
    TransparentWidget::click();

    background->set_image(image);
    background->replace_body(tab_body);
}


ControllerSelector::ControllerSelector(
        Background& background,
        Synth& synth
) : Widget("Select controller", LEFT, TOP, WIDTH, HEIGHT, Type::CONTROLLER_SELECTOR),
    background(background),
    synth(synth),
    knob_param_editor(NULL),
    param_id(Synth::ParamId::INVALID_PARAM_ID),
    selected_controller_id(Synth::ControllerId::INVALID_CONTROLLER_ID)
{
}


void ControllerSelector::set_up(GUI::PlatformData platform_data, WidgetBase* parent)
{
    Widget::set_up(platform_data, parent);

    constexpr int max_top = HEIGHT - Controller::HEIGHT;
    constexpr int group_separation = 3;

    GUI::ControllerCapability previous_required_capability = GUI::ControllerCapability::NONE;
    Synth::ControllerId previous_id = Synth::ControllerId::NONE;
    int top = TITLE_HEIGHT;
    int left = 10;

    for (int i = 0; i != GUI::CONTROLLERS_COUNT; ++i) {
        Synth::ControllerId const id = GUI::CONTROLLERS[i].id;
        char const* const text = GUI::CONTROLLERS[i].long_name;
        GUI::ControllerCapability const required_capability = (
            GUI::CONTROLLERS[i].required_capability
        );

        if (
                required_capability != previous_required_capability
                || previous_id == Synth::ControllerId::MIDI_LEARN
        ) {
            top += group_separation;
            previous_required_capability = required_capability;
        }

        previous_id = id;

        controllers[i] = (Controller*)this->own(
            new Controller(*this, required_capability, text, left, top, id)
        );

        top += Controller::HEIGHT;

        if (top > max_top) {
            top = TITLE_HEIGHT;
            left += Controller::WIDTH;
        }
    }
}


void ControllerSelector::select_controller(
        Synth::ParamId const param_id,
        int const controller_choices,
        KnobParamEditor* knob_param_editor
) {
    Synth::ControllerId const selected_controller_id = (
        synth.get_param_controller_id_atomic(param_id)
    );

    GUI::Controller const* const controller = GUI::get_controller(
        selected_controller_id
    );

    if (controller == NULL) {
        return;
    }

    if (this->selected_controller_id < Synth::ControllerId::CONTROLLER_ID_COUNT) {
        GUI::Controller const* const old_controller = GUI::get_controller(
            this->selected_controller_id
        );
        controllers[old_controller->index]->unselect();
    }

    snprintf(
        title, TITLE_SIZE, "Select controller for \"%s\"", GUI::PARAMS[param_id]
    );

    this->param_id = param_id;
    this->knob_param_editor = knob_param_editor;
    this->selected_controller_id = selected_controller_id;

    controllers[controller->index]->select();

    for (int i = 0; i != GUI::CONTROLLERS_COUNT; ++i) {
        if (
                controllers[i]->required_capability == 0
                || (controllers[i]->required_capability & controller_choices) != 0
        ) {
            controllers[i]->show();
        } else {
            controllers[i]->hide();
        }
    }

    redraw();
    Widget::show();
    background.hide_body();
    bring_to_top();
}


void ControllerSelector::hide()
{
    background.show_body();
    Widget::hide();
}


void ControllerSelector::handle_selection_change(
        Synth::ControllerId const new_controller_id
) {
    hide();

    if (knob_param_editor == NULL || param_id >= Synth::Synth::ParamId::INVALID_PARAM_ID) {
        return;
    }

    knob_param_editor->handle_controller_change(new_controller_id);
}


bool ControllerSelector::paint()
{
    Widget::paint();

    fill_rectangle(0, 0, width, height, GUI::TEXT_BACKGROUND);
    draw_text(
        title,
        12,
        0,
        0,
        WIDTH,
        TITLE_HEIGHT,
        GUI::TEXT_COLOR,
        GUI::TEXT_BACKGROUND,
        FontWeight::BOLD,
        10,
        TextAlignment::LEFT
    );

    return true;
}


ControllerSelector::Controller::Controller(
        ControllerSelector& controller_selector,
        GUI::ControllerCapability const required_capability,
        char const* const text,
        int const left,
        int const top,
        Synth::ControllerId const controller_id
) : Widget(text, left, top, WIDTH, HEIGHT, Type::CONTROLLER),
    required_capability(required_capability),
    controller_id(controller_id),
    controller_selector(controller_selector),
    is_selected(false),
    is_mouse_over(false)
{
}


void ControllerSelector::Controller::select()
{
    is_selected = true;
    redraw();
}


void ControllerSelector::Controller::unselect()
{
    is_selected = false;
    redraw();
}


bool ControllerSelector::Controller::paint()
{
    Widget::paint();

    GUI::Color background;
    GUI::Color color;

    if (is_mouse_over) {
        background = GUI::controller_id_to_bg_color(controller_id);
        color = GUI::TEXT_HIGHLIGHT_COLOR;
    } else if (is_selected) {
        background = GUI::controller_id_to_bg_color(controller_id);
        color = (
            controller_id == Synth::ControllerId::NONE
                ? GUI::TEXT_COLOR
                : GUI::TEXT_BACKGROUND
        );
    } else {
        background = GUI::TEXT_BACKGROUND;
        color = GUI::controller_id_to_text_color(controller_id);
    }

    draw_text(
        text,
        12,
        0,
        0,
        width,
        height,
        color,
        background,
        FontWeight::BOLD,
        6,
        TextAlignment::LEFT
    );

    return true;
}


bool ControllerSelector::Controller::mouse_up(int const x, int const y)
{
    Widget::mouse_up(x, y);

    controller_selector.handle_selection_change(controller_id);

    return true;
}


bool ControllerSelector::Controller::mouse_move(
        int const x,
        int const y,
        bool const modifier
) {
    Widget::mouse_move(x, y, modifier);

    if (!is_mouse_over) {
        is_mouse_over = true;
        redraw();
    }

    return true;
}


bool ControllerSelector::Controller::mouse_leave(int const x, int const y)
{
    Widget::mouse_leave(x, y);

    if (is_mouse_over) {
        is_mouse_over = false;
        redraw();
    }

    return 0;
}


KnobStates::KnobStates(
        WidgetBase* widget,
        GUI::Image free_image,
        GUI::Image controlled_image,
        GUI::Image synced_image,
        GUI::Image none_image,
        size_t const count,
        int const width,
        int const height
) : count(count),
    width(width),
    height(height),
    last_index((Number)(count - 1)),
    widget(widget),
    free_image(free_image),
    controlled_image(controlled_image),
    synced_image(synced_image),
    none_image(none_image)
{
    free_images = split_image(free_image);
    controlled_images = split_image(controlled_image);
    synced_images = split_image(synced_image);
}


GUI::Image* KnobStates::split_image(GUI::Image image) const
{
    if (image == NULL) {
        return NULL;
    }

    GUI::Image* images = new GUI::Image[count];

    for (size_t i = 0; i != count; ++i) {
        int const top = (int)i * height;

        images[i] = widget->copy_image_region(image, 0, top, width, height);
    }

    return images;
}


KnobStates::~KnobStates()
{
    if (free_image != NULL) {
        free_images = free_images_(free_images);

        widget->delete_image(free_image);
        free_image = NULL;
    }

    if (controlled_image != NULL) {
        controlled_images = free_images_(controlled_images);

        widget->delete_image(controlled_image);
        controlled_image = NULL;
    }

    if (synced_image != NULL) {
        synced_images = free_images_(synced_images);

        widget->delete_image(synced_image);
        synced_image = NULL;
    }

    if (none_image != NULL) {
        widget->delete_image(none_image);
        none_image = NULL;
    }
}


GUI::Image* KnobStates::free_images_(GUI::Image* images) const
{
    if (images == NULL) {
        return NULL;
    }

    for (size_t i = 0; i != count; ++i) {
        widget->delete_image(images[i]);
        images[i] = NULL;
    }

    delete[] images;

    return NULL;
}


KnobParamEditor::KnobParamEditor(
        GUI& gui,
        char const* const text,
        int const left,
        int const top,
        int const width,
        int const height,
        int const knob_top,
        ControllerSelector& controller_selector,
        Synth& synth,
        Synth::ParamId const param_id,
        int const controller_choices,
        char const* format,
        double const scale,
        KnobStates* knob_states
) : TransparentWidget(text, left, top, width, height, Type::KNOB_PARAM_EDITOR),
    param_id(param_id),
    knob_top(knob_top),
    has_room_for_texts(height >= knob_top + knob_states->height + TEXTS_HEIGHT),
    format(format),
    scale(scale),
    options(NULL),
    number_of_options(0),
    value_font_size(11),
    controller_choices(controller_choices),
    controller_selector(controller_selector),
    knob_states(knob_states),
    synth(synth),
    ratio(0.0),
    knob(NULL),
    has_controller_(false)
{
    set_gui(gui);
}


KnobParamEditor::KnobParamEditor(
        GUI& gui,
        char const* const text,
        int const left,
        int const top,
        int const width,
        int const height,
        int const knob_top,
        ControllerSelector& controller_selector,
        Synth& synth,
        Synth::ParamId const param_id,
        int const controller_choices,
        char const* const* const options,
        int const number_of_options,
        KnobStates* knob_states
) : TransparentWidget(text, left, top, width, height, Type::KNOB_PARAM_EDITOR),
    param_id(param_id),
    knob_top(knob_top),
    has_room_for_texts(height >= knob_top + knob_states->height + TEXTS_HEIGHT),
    format(NULL),
    scale(1.0),
    options(options),
    number_of_options(number_of_options),
    value_font_size(10),
    controller_choices(controller_choices),
    controller_selector(controller_selector),
    knob_states(knob_states),
    synth(synth),
    ratio(0.0),
    knob(NULL),
    controller_id(Synth::ControllerId::NONE),
    has_controller_(false)
{
    set_gui(gui);
}


void KnobParamEditor::set_up(GUI::PlatformData platform_data, WidgetBase* parent)
{
    TransparentWidget::set_up(platform_data, parent);

    knob = new Knob(
        *this,
        *gui,
        text,
        (width - knob_states->width) / 2,
        knob_top,
        number_of_options > 1 ? (Number)(number_of_options - 1) : 0.0,
        knob_states
    );

    own(knob);
    update_editor(
        synth.get_param_ratio_atomic(param_id),
        synth.get_param_controller_id_atomic(param_id)
    );
}


void KnobParamEditor::set_sync_param_id(Synth::ParamId const param_id)
{
    knob->set_sync_param_id(param_id);
}


bool KnobParamEditor::has_controller() const
{
    return has_controller_;
}


void KnobParamEditor::refresh()
{
    if (knob->is_editing()) {
        return;
    }

    Synth::ControllerId const new_controller_id = (
        synth.get_param_controller_id_atomic(param_id)
    );
    Number const new_ratio = synth.get_param_ratio_atomic(param_id);

    has_controller_ = new_controller_id > Synth::Synth::ControllerId::NONE;

    if (knob->update_sync_status() || new_ratio != ratio || new_controller_id != controller_id) {
        update_editor(new_ratio, new_controller_id);
    } else {
        synth.push_message(
            Synth::MessageType::REFRESH_PARAM, param_id, 0.0, 0
        );
    }
}


void KnobParamEditor::update_editor(
        Number const new_ratio,
        Synth::ControllerId const new_controller_id
) {
    controller_id = new_controller_id;
    update_editor(new_ratio);
}


void KnobParamEditor::update_editor(Number const new_ratio)
{
    ratio = GUI::clamp_ratio(new_ratio);
    update_editor();
}


void KnobParamEditor::update_editor(Synth::ControllerId const new_controller_id)
{
    controller_id = new_controller_id;
    update_editor();
}


void KnobParamEditor::update_editor()
{
    has_controller_ = controller_id > Synth::ControllerId::NONE;

    update_value_str();
    update_controller_str();
    redraw();

    if (has_controller_) {
        knob->make_controlled(controller_id);
    } else {
        knob->make_free();
    }

    knob->update(ratio);
}


void KnobParamEditor::adjust_ratio(Number const delta)
{
    if (param_id < Synth::FLOAT_PARAMS) {
        handle_ratio_change(ratio + delta);
    } else {
        Number const discrete_delta = (
            (delta > 0 ? 1.001 : -1.001) / synth.get_param_max_value(param_id)
        );
        handle_ratio_change(ratio + discrete_delta);
    }
}


void KnobParamEditor::handle_ratio_change(Number const new_ratio)
{
    Number const ratio = GUI::clamp_ratio(new_ratio);

    synth.push_message(Synth::MessageType::SET_PARAM, param_id, ratio, 0);
    update_editor(ratio);
}


void KnobParamEditor::handle_controller_change(Synth::ControllerId const new_controller_id)
{
    synth.push_message(
        Synth::MessageType::ASSIGN_CONTROLLER,
        param_id,
        0.0,
        (Byte)new_controller_id
    );
    has_controller_ = true;
    update_editor(new_controller_id);
}


void KnobParamEditor::update_value_str()
{
    GUI::param_ratio_to_str(
        synth,
        param_id,
        ratio,
        scale,
        format,
        options,
        number_of_options,
        value_str,
        TEXT_MAX_LENGTH
    );

    if (!has_room_for_texts) {
        snprintf(title, TITLE_MAX_LENGTH, "%s: %s", text, value_str);
        title[TITLE_MAX_LENGTH - 1] = '\x00';

        knob->set_text(title);

        if (knob->is_editing()) {
            gui->set_status_line(title);
            gui->redraw_status_line();
        }
    }
}


void KnobParamEditor::update_controller_str()
{
    constexpr size_t last_index = TEXT_MAX_LENGTH - 1;

    strncpy(
        controller_str, GUI::get_controller(controller_id)->short_name, last_index
    );
    controller_str[last_index] = '\x00';
}


void KnobParamEditor::reset_default()
{
    handle_ratio_change(synth.get_param_default_ratio(param_id));
}


void KnobParamEditor::stop_editing()
{
    knob->stop_editing();
}


bool KnobParamEditor::paint()
{
    TransparentWidget::paint();

    if (has_room_for_texts) {
        draw_text(
            Synth::is_controller_polyphonic(controller_id) ? "" : value_str,
            value_font_size,
            1,
            height - VALUE_TEXT_HEIGHT,
            width - 2,
            VALUE_TEXT_HEIGHT,
            GUI::controller_id_to_text_color(controller_id),
            GUI::TEXT_BACKGROUND
        );

        if (controller_choices > 0) {
            draw_text(
                controller_str,
                10,
                1,
                height - TEXTS_HEIGHT,
                width - 2,
                CONTROLLER_TEXT_HEIGHT,
                has_controller_ ? GUI::TEXT_BACKGROUND : GUI::TEXT_COLOR,
                has_controller_
                    ? GUI::controller_id_to_bg_color(controller_id)
                    : GUI::TEXT_BACKGROUND,
                has_controller_ ? FontWeight::BOLD : FontWeight::NORMAL
            );
        }
    }

    return true;
}


bool KnobParamEditor::mouse_up(int const x, int const y)
{
    TransparentWidget::mouse_up(x, y);

    if (is_clicking && controller_choices > 0) {
        controller_selector.select_controller(param_id, controller_choices, this);
    }

    return false;
}


bool KnobParamEditor::mouse_move(int const x, int const y, bool const modifier)
{
    TransparentWidget::mouse_move(x, y, modifier);
    gui->set_status_line(text);

    return true;
}


bool KnobParamEditor::mouse_leave(int const x, int const y)
{
    TransparentWidget::mouse_leave(x, y);
    gui->set_status_line("");

    return true;
}


KnobParamEditor::Knob::Knob(
        KnobParamEditor& editor,
        GUI& gui,
        char const* const text,
        int const left,
        int const top,
        Number const steps,
        KnobStates* knob_states
) : Widget(text, left, top, knob_states->width, knob_states->height, Type::KNOB),
    steps(steps),
    editor(editor),
    knob_states(knob_states),
    knob_state(NULL),
    ratio(0.0),
    mouse_move_delta(0.0),
    sync_param_id(Synth::ParamId::INVALID_PARAM_ID),
    is_controlled(false),
    is_controller_polyphonic(false),
    is_editing_(false),
    is_synced(false)
{
    set_gui(gui);
}


KnobParamEditor::Knob::~Knob()
{
}


void KnobParamEditor::Knob::set_up(
        GUI::PlatformData platform_data,
        WidgetBase* parent
) {
    Widget::set_up(platform_data, parent);

    update(0.0);
}


void KnobParamEditor::Knob::set_sync_param_id(Synth::ParamId const param_id)
{
    sync_param_id = param_id;
}


void KnobParamEditor::Knob::update(Number const ratio)
{
    this->ratio = (
        steps > 0.0 ? std::round(ratio * steps) / steps : ratio
    );

    update_sync_status();
    update();
}


void KnobParamEditor::Knob::update()
{
    if (is_controller_polyphonic) {
        set_image(knob_states->none_image);

        return;
    }

    size_t const index = (size_t)(knob_states->last_index * this->ratio);

    if (is_controlled) {
        set_image(knob_states->controlled_images[index]);
    } else if (is_synced) {
        set_image(knob_states->synced_images[index]);
    } else {
        set_image(knob_states->free_images[index]);
    }
}


bool KnobParamEditor::Knob::update_sync_status()
{
    bool const was_synced = is_synced;

    is_synced = (
        sync_param_id != Synth::ParamId::INVALID_PARAM_ID
        && ratio > 0.0
        && std::fabs(ratio - editor.synth.get_param_ratio_atomic(sync_param_id)) < 0.000001
    );

    return was_synced != is_synced;
}


void KnobParamEditor::Knob::make_free()
{
    is_controlled = false;
    is_controller_polyphonic = false;
    update();
}


void KnobParamEditor::Knob::make_controlled(Synth::ControllerId const controller_id)
{
    is_controlled = true;
    is_controller_polyphonic = Synth::is_controller_polyphonic(controller_id);
    update();
}


bool KnobParamEditor::Knob::is_editing() const
{
    return is_editing_ && !is_controlled;
}


void KnobParamEditor::Knob::start_editing()
{
    is_editing_ = true;
}


void KnobParamEditor::Knob::stop_editing()
{
    is_editing_ = false;
}


bool KnobParamEditor::Knob::double_click()
{
    Widget::double_click();

    if (is_controlled) {
        return true;
    }

    if (!is_clicking) {
        editor.reset_default();
    }

    return true;
}


bool KnobParamEditor::Knob::mouse_down(int const x, int const y)
{
    Widget::mouse_down(x, y);

    if (is_controlled) {
        return true;
    }

    prev_x = (Number)x;
    prev_y = (Number)y;
    mouse_move_delta = 0.0;

    return true;
}


bool KnobParamEditor::Knob::mouse_up(int const x, int const y)
{
    Widget::mouse_up(x, y);

    if (is_controlled) {
        return true;
    }

    focus();

    return true;
}


bool KnobParamEditor::Knob::mouse_move(
        int const x,
        int const y,
        bool const modifier
) {
    Widget::mouse_move(x, y, modifier);

    gui->set_status_line(text);

    start_editing();

    if (is_controlled) {
        return false;
    }

    if (is_clicking) {
        Number const scale = (
            modifier ? MOUSE_MOVE_FINE_SCALE : MOUSE_MOVE_COARSE_SCALE
        );
        Number const float_x = (Number)x;
        Number const float_y = (Number)y;
        Number const dx = float_x - prev_x;
        Number const dy = float_y - prev_y;
        Number const delta = (
            scale * ((std::fabs(dx) > std::fabs(dy)) ? dx : -dy)
        );

        prev_x = float_x;
        prev_y = float_y;

        mouse_move_delta += delta;

        if (
                editor.param_id < Synth::FLOAT_PARAMS
                || std::fabs(mouse_move_delta) > 0.03
        ) {
            editor.adjust_ratio(delta);
            mouse_move_delta = 0.0;
        }
    }

    focus();

    return is_clicking;
}


bool KnobParamEditor::Knob::mouse_leave(int const x, int const y)
{
    Widget::mouse_leave(x, y);
    stop_editing();
    gui->set_status_line("");

    return true;
}


bool KnobParamEditor::Knob::mouse_wheel(Number const delta, bool const modifier)
{
    Widget::mouse_wheel(delta, modifier);

    if (is_controlled) {
        return true;
    }

    Number const scale = (
        modifier ? MOUSE_WHEEL_FINE_SCALE : MOUSE_WHEEL_COARSE_SCALE
    );

    if (steps > 0.0) {
        editor.adjust_ratio(delta * scale * 10.0);
    } else {
        editor.adjust_ratio(delta * scale);
    }

    return true;
}


AboutText::AboutText(char const* sdk_version, GUI::Image logo)
    : Widget(TEXT, LEFT, TOP, WIDTH, HEIGHT, Type::ABOUT_TEXT),
    logo(logo)
{
    std::string line("(Version: ");

    line += VERSION;

    if (sdk_version != NULL) {
        line += ", SDK: ";
        line += sdk_version;
    }

    line += ")";

    lines.push_back(NAME);
    lines.push_back(line);

    line = "";

    for (char const* c = TEXT; *c != '\x00'; ++c) {
        if (*c == '\n') {
            lines.push_back(line);
            line = "";
        } else {
            line += *c;
        }
    }

    lines.push_back(line);
}


bool AboutText::paint()
{
    Widget::paint();

    fill_rectangle(0, 0, width, height, GUI::TEXT_BACKGROUND);

    int const left = logo != NULL ? LOGO_WIDTH + 10 : 0;
    int const text_width = width - left;
    int top = TEXT_TOP;

    for (std::vector<std::string>::const_iterator it = lines.begin(); it != lines.end(); ++it) {
        draw_text(
            it->c_str(),
            FONT_SIZE,
            left,
            top,
            text_width,
            LINE_HEIGHT,
            GUI::TEXT_COLOR,
            GUI::TEXT_BACKGROUND,
            FontWeight::NORMAL,
            PADDING,
            TextAlignment::CENTER
        );

        top += it->length() == 0 ? EMPTY_LINE_HEIGHT : LINE_HEIGHT;
    }

    if (logo != NULL) {
        draw_image(logo, 5, (HEIGHT - LOGO_HEIGHT) / 2, LOGO_WIDTH, LOGO_HEIGHT);
    }

    return true;
}


StatusLine::StatusLine()
    : TransparentWidget("", LEFT, TOP, WIDTH, HEIGHT, Type::STATUS_LINE)
{
}


void StatusLine::set_text(char const* text)
{
    TransparentWidget::set_text(text);

    if (parent == NULL) {
        return;
    }

    if (text[0] == '\x00') {
        hide();
    } else {
        show();
    }
}


bool StatusLine::paint()
{
    TransparentWidget::paint();

    if (text[0] != '\x00') {
        fill_rectangle(0, 0, WIDTH, HEIGHT, GUI::STATUS_LINE_BACKGROUND);
        draw_text(
            text,
            9,
            0,
            3,
            WIDTH,
            20,
            GUI::TEXT_COLOR,
            GUI::STATUS_LINE_BACKGROUND,
            FontWeight::NORMAL,
            5,
            TextAlignment::RIGHT
        );
    }

    return true;
}


ToggleSwitch::ToggleSwitch(
        GUI& gui,
        char const* const text,
        int const left,
        int const top,
        int const width,
        int const height,
        int const box_left,
        Synth& synth,
        Synth::ParamId const param_id
) : TransparentWidget(text, left, top, width, height, Type::TOGGLE_SWITCH),
    param_id(param_id),
    box_left(box_left),
    synth(synth),
    is_editing_(false)
{
    set_gui(gui);
}


void ToggleSwitch::set_up(GUI::PlatformData platform_data, WidgetBase* parent)
{
    TransparentWidget::set_up(platform_data, parent);

    default_ratio = synth.get_param_default_ratio(param_id);
    ratio = default_ratio;
    refresh();
    redraw();
}


void ToggleSwitch::refresh()
{
    if (is_editing()) {
        return;
    }

    Number const new_ratio = synth.get_param_ratio_atomic(param_id);

    if (new_ratio != ratio) {
        ratio = GUI::clamp_ratio(new_ratio);
        redraw();
    } else {
        synth.push_message(
            Synth::MessageType::REFRESH_PARAM, param_id, 0.0, 0
        );
    }
}


bool ToggleSwitch::paint()
{
    TransparentWidget::paint();

    Toggle const toggle = synth.int_param_ratio_to_display_value(param_id, ratio);
    GUI::Color const color = (
        toggle == ToggleParam::ON ? GUI::TOGGLE_ON_COLOR : GUI::TOGGLE_OFF_COLOR
    );

    fill_rectangle(box_left + 5, 8, 11, 8, color);

    return true;
}


bool ToggleSwitch::mouse_up(int const x, int const y)
{
    ratio = ratio < 0.5 ? 1.0 : 0.0;
    synth.push_message(Synth::MessageType::SET_PARAM, param_id, ratio, 0);
    redraw();

    return true;
}


bool ToggleSwitch::mouse_move(int const x, int const y, bool const modifier)
{
    TransparentWidget::mouse_move(x, y, modifier);

    gui->set_status_line(text);
    start_editing();

    return true;
}


bool ToggleSwitch::mouse_leave(int const x, int const y)
{
    TransparentWidget::mouse_leave(x, y);

    gui->set_status_line("");
    stop_editing();

    return true;
}


bool ToggleSwitch::is_editing() const
{
    return is_editing_;
}


void ToggleSwitch::start_editing()
{
    is_editing_ = true;
}


void ToggleSwitch::stop_editing()
{
    is_editing_ = false;
}


TuningSelector::TuningSelector(
        GUI& gui,
        char const* const text,
        int const left,
        int const top,
        Synth& synth,
        Synth::ParamId const param_id
) : TransparentWidget(text, left, top, WIDTH, HEIGHT, Type::TUNING_SELECTOR),
    param_id(param_id),
    synth(synth),
    ratio(0.0),
    is_editing_(false),
    is_mts_esp_connected(false)
{
    set_gui(gui);
    update_value_str();
}


void TuningSelector::refresh()
{
    if (is_editing()) {
        return;
    }

    Number const new_ratio = synth.get_param_ratio_atomic(param_id);
    bool const new_is_mts_esp_connected = gui->is_mts_esp_connected();

    bool const is_changed = (
        std::fabs(new_ratio - ratio) > 0.000001
        || new_is_mts_esp_connected != is_mts_esp_connected
    );

    if (is_changed) {
        ratio = GUI::clamp_ratio(new_ratio);
        is_mts_esp_connected = new_is_mts_esp_connected;
        update_value_str();
        redraw();
    } else {
        synth.push_message(
            Synth::MessageType::REFRESH_PARAM, param_id, 0.0, 0
        );
    }
}


void TuningSelector::update_value_str()
{
    Byte const tuning = synth.int_param_ratio_to_display_value(param_id, ratio);

    if (tuning < Modulator::TUNING_MTS_ESP_NOTE_ON) {
        GUI::param_ratio_to_str(
            synth,
            param_id,
            ratio,
            1.0,
            NULL,
            GUI::TUNINGS,
            GUI::TUNINGS_COUNT,
            value_str,
            TEXT_MAX_LENGTH
        );

        return;
    }

    snprintf(
        value_str,
        TEXT_MAX_LENGTH,
        "%s %s",
        tuning < GUI::TUNINGS_COUNT ? GUI::TUNINGS[tuning] : "???",
        is_mts_esp_connected ? "on" : "off"
    );
}


bool TuningSelector::is_editing() const
{
    return is_editing_;
}


void TuningSelector::start_editing()
{
    is_editing_ = true;
}


void TuningSelector::stop_editing()
{
    is_editing_ = false;
}


void TuningSelector::set_up(GUI::PlatformData platform_data, WidgetBase* parent)
{
    TransparentWidget::set_up(platform_data, parent);
}


bool TuningSelector::paint()
{
    TransparentWidget::paint();

    draw_text(
        value_str, 9, 0, 0, WIDTH, HEIGHT, GUI::TEXT_COLOR, GUI::TEXT_BACKGROUND
    );

    return true;
}


bool TuningSelector::mouse_up(int const x, int const y)
{
    TransparentWidget::mouse_up(x, y);

    if (ratio > 0.99999) {
        ratio = 0.0;
    } else {
        Number const delta = 1.001 / synth.get_param_max_value(param_id);

        ratio = GUI::clamp_ratio(ratio + delta);
    }

    synth.push_message(Synth::MessageType::SET_PARAM, param_id, ratio, 0);
    update_value_str();
    redraw();

    return false;
}



bool TuningSelector::mouse_move(int const x, int const y, bool const modifier)
{
    TransparentWidget::mouse_move(x, y, modifier);

    gui->set_status_line(text);
    start_editing();

    return true;
}


bool TuningSelector::mouse_leave(int const x, int const y)
{
    TransparentWidget::mouse_leave(x, y);

    gui->set_status_line("");
    stop_editing();

    return true;
}

}

#endif
