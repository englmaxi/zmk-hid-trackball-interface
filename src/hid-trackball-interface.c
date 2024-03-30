/*
 * Copyright (c) 2024 The ZMK Contributors
 *
 * SPDX-License-Identifier: MIT
 */

#define DT_DRV_COMPAT zmk_hid_trackball_interface

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zmk/behavior.h>
#include <zmk/behavior_queue.h>
#include <zmk/events/hid_indicators_changed.h>
#include <zmk/events/layer_state_changed.h>
#include <zmk/keymap.h>

LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

#if DT_HAS_COMPAT_STATUS_OKAY(DT_DRV_COMPAT)

enum interface_input_mode {
    MOVE,
    SCROLL,
    SNIPE
} curr_mode;

static int32_t scroll_layers[] = DT_PROP(DT_INST(0, DT_DRV_COMPAT), scroll_layers);
static int32_t snipe_layers[] = DT_PROP(DT_INST(0, DT_DRV_COMPAT), snipe_layers);

#define NUM_SCROLL_LAYERS DT_PROP_LEN(DT_INST(0, DT_DRV_COMPAT), scroll_layers)
#define NUM_SNIPE_LAYERS DT_PROP_LEN(DT_INST(0, DT_DRV_COMPAT), snipe_layers)

static void toggle_scroll() {
    struct zmk_behavior_binding binding = {
        .behavior_dev = DEVICE_DT_NAME(DT_PHANDLE(DT_INST(0, DT_DRV_COMPAT), tog_scroll_bindings)),
    };
    zmk_behavior_queue_add(-1, binding, true, 0);
    LOG_INF("scroll toggled"); 
}

static void cycle_dpi() {
    struct zmk_behavior_binding binding = {
        .behavior_dev = DEVICE_DT_NAME(DT_PHANDLE(DT_INST(0, DT_DRV_COMPAT), cyc_dpi_bindings)),
    };
    zmk_behavior_queue_add(-1, binding, true, 0);
    LOG_INF("cycle dpi");
}

#define AUTOMOUSE_LAYER (DT_PROP(DT_INST(0, DT_DRV_COMPAT), automouse_layer))
#if AUTOMOUSE_LAYER > 0
#define LED_SLCK 0x04
static bool automouse_triggered = false;

static void activate_automouse_layer() {
    automouse_triggered = true;
    zmk_keymap_layer_activate(AUTOMOUSE_LAYER);
    LOG_INF("mouse layer activated");
}

static void deactivate_automouse_layer() {
    automouse_triggered = false;
    zmk_keymap_layer_deactivate(AUTOMOUSE_LAYER);
    LOG_INF("mouse layer deactivated");
}

static int hid_indicators_listener_cb(const zmk_event_t *eh) {
    struct zmk_hid_indicators_changed *ev = as_zmk_hid_indicators_changed(eh);
    if (!automouse_triggered && (ev->indicators & LED_SLCK) && !zmk_keymap_layer_active(AUTOMOUSE_LAYER)) {
        activate_automouse_layer();
    } else if (automouse_triggered && !(ev->indicators & LED_SLCK)) {
        deactivate_automouse_layer();
    }
    return 0;
}

ZMK_LISTENER(hid_indicators_listener, hid_indicators_listener_cb);
ZMK_SUBSCRIPTION(hid_indicators_listener, zmk_hid_indicators_changed);
#endif

static enum interface_input_mode get_input_mode_for_current_layer() {
    for (int i = 0; i < NUM_SCROLL_LAYERS; i++) {
        if (zmk_keymap_layer_active(scroll_layers[i])) {
            return SCROLL;
        }
    }
    for (int i = 0; i < NUM_SNIPE_LAYERS; i++) {
        if (zmk_keymap_layer_active(snipe_layers[i])) {
            return SNIPE;
        }
    }
    return MOVE;
}

static int layer_state_listener_cb(const zmk_event_t *eh) {
    enum interface_input_mode input_mode = get_input_mode_for_current_layer();
    if (input_mode != curr_mode) {
        LOG_INF("input mode changed to %d", input_mode);

        switch (input_mode) {
            case MOVE:
                if (curr_mode == SCROLL) {
                    toggle_scroll();
                } else if (curr_mode == SNIPE) {
                    cycle_dpi();
                }
                break;
            case SCROLL:
                if (curr_mode == MOVE) {
                    toggle_scroll();
                } else if (curr_mode == SNIPE) {
                    cycle_dpi();
                    toggle_scroll();
                }
                break;
            case SNIPE:
                if (curr_mode == MOVE) {
                    cycle_dpi();
                } else if (curr_mode == SCROLL) {
                    toggle_scroll();
                    cycle_dpi();
                }
                break;
        }
        curr_mode = input_mode;
    }
    return 0;
}

ZMK_LISTENER(layer_state_listener, layer_state_listener_cb);
ZMK_SUBSCRIPTION(layer_state_listener, zmk_layer_state_changed);

#endif
