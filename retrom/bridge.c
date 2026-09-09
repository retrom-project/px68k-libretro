/* Retrom's single-instance libretro host. Each Wasm module owns its own machine. */
#include <libretro.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include "libretro/prop.h"
#include "libretro/winui.h"
#include "x68k/fdd.h"

extern int retrom_menu_active(void);

static uint32_t pixels[800 * 600];
static int16_t samples[8192];
static unsigned width = 800, height = 600, sample_count;
static uint16_t buttons[2];
static uint8_t keys[512];
static double fps = 55.45;
static unsigned pixel_format = RETRO_PIXEL_FORMAT_RGB565;
static void *state;
static size_t state_size;
static int loaded;
static struct retro_disk_control_callback disks;

static void log_message(enum retro_log_level level, const char *format, ...) {
    if (level < RETRO_LOG_WARN) return;
    va_list args; va_start(args, format); vfprintf(stderr, format, args); va_end(args);
}
static bool environment(unsigned command, void *data) {
    switch (command) {
    case RETRO_ENVIRONMENT_GET_LOG_INTERFACE:
        ((struct retro_log_callback *)data)->log = log_message; return true;
    case RETRO_ENVIRONMENT_GET_SYSTEM_DIRECTORY:
    case RETRO_ENVIRONMENT_GET_SAVE_DIRECTORY:
    case RETRO_ENVIRONMENT_GET_CONTENT_DIRECTORY:
        *(const char **)data = "/game"; return true;
    case RETRO_ENVIRONMENT_GET_VARIABLE: {
        struct retro_variable *v = data;
        if (!strcmp(v->key, "px68k_ramsize")) v->value = "4MB";
        else if (!strcmp(v->key, "px68k_cpuspeed")) v->value = "10Mhz";
        /* The host schedules one emulated frame per step. */
        else if (!strcmp(v->key, "px68k_no_wait_mode")) v->value = "enabled";
        else if (!strcmp(v->key, "px68k_frameskip")) v->value = "Full Frame";
        else { v->value = NULL; return false; }
        return true;
    }
    case RETRO_ENVIRONMENT_GET_VARIABLE_UPDATE: *(bool *)data = false; return true;
    case RETRO_ENVIRONMENT_GET_CORE_OPTIONS_VERSION: *(unsigned *)data = 0; return true;
    case RETRO_ENVIRONMENT_SET_VARIABLES:
    case RETRO_ENVIRONMENT_SET_INPUT_DESCRIPTORS:
    case RETRO_ENVIRONMENT_SET_CONTROLLER_INFO:
    case RETRO_ENVIRONMENT_SET_SUPPORT_NO_GAME: return true;
    case RETRO_ENVIRONMENT_SET_PIXEL_FORMAT:
        pixel_format = *(unsigned *)data; return pixel_format <= RETRO_PIXEL_FORMAT_RGB565;
    case RETRO_ENVIRONMENT_SET_SYSTEM_AV_INFO:
        fps = ((struct retro_system_av_info *)data)->timing.fps; return true;
    case RETRO_ENVIRONMENT_SET_GEOMETRY: return true;
    case RETRO_ENVIRONMENT_SET_DISK_CONTROL_INTERFACE:
        disks = *(struct retro_disk_control_callback *)data; return true;
    default: return false;
    }
}
static void video(const void *data, unsigned w, unsigned h, size_t pitch) {
    if (!data || w > 800 || h > 600) return;
    width = w; height = h;
    for (unsigned y = 0; y < h; ++y) {
        const uint8_t *row = (const uint8_t *)data + y * pitch;
        for (unsigned x = 0; x < w; ++x) {
            uint32_t r, g, b;
            if (pixel_format == RETRO_PIXEL_FORMAT_XRGB8888) {
                uint32_t p = ((const uint32_t *)row)[x];
                r = (p >> 16) & 255; g = (p >> 8) & 255; b = p & 255;
            } else {
                uint16_t p = ((const uint16_t *)row)[x];
                r = ((p >> (pixel_format == RETRO_PIXEL_FORMAT_RGB565 ? 11 : 10)) & 31) * 255 / 31;
                g = pixel_format == RETRO_PIXEL_FORMAT_RGB565 ? ((p >> 5) & 63) * 255 / 63 : ((p >> 5) & 31) * 255 / 31;
                b = (p & 31) * 255 / 31;
            }
            pixels[y * w + x] = r | (g << 8) | (b << 16) | 0xff000000;
        }
    }
}
static size_t audio_batch(const int16_t *data, size_t count) {
    size_t available = (8192 - sample_count) / 2;
    size_t copy = count < available ? count : available;
    memcpy(samples + sample_count, data, copy * 4); sample_count += copy * 2;
    return count;
}
static void audio_sample(int16_t left, int16_t right) {
    int16_t pair[2] = {left, right}; audio_batch(pair, 1);
}
static void poll(void) {}
static int16_t input(unsigned port, unsigned device, unsigned index, unsigned id) {
    (void)index;
    if (device == RETRO_DEVICE_KEYBOARD && id < 512) return keys[id];
    if (device == RETRO_DEVICE_JOYPAD && port < 2 && id < 16) return (buttons[port] >> id) & 1;
    return 0;
}
int retrom_abi(void) { return 1; }
int retrom_load(const char *path) {
    struct retro_game_info game = {path, NULL, 0, NULL};
    retro_set_environment(environment); retro_set_video_refresh(video);
    retro_set_audio_sample(audio_sample); retro_set_audio_sample_batch(audio_batch);
    retro_set_input_poll(poll); retro_set_input_state(input);
    retro_init();
    loaded = retro_load_game(&game);
    if (!loaded) { retro_deinit(); return 0; }
    retro_set_controller_port_device(0, RETRO_DEVICE_JOYPAD);
    retro_set_controller_port_device(1, RETRO_DEVICE_JOYPAD);
    retro_run();
    struct retro_system_av_info av; retro_get_system_av_info(&av); fps = av.timing.fps;
    return 1;
}
int retrom_step(unsigned pad0, unsigned pad1) {
    if (!loaded) return 0;
    buttons[0] = pad0; buttons[1] = pad1; sample_count = 0; retro_run(); return 1;
}
void retrom_key(unsigned key, unsigned pressed) { if (key < 512) keys[key] = !!pressed; }
int retrom_ready(void) { return loaded && !retrom_menu_active(); }
unsigned retrom_width(void) { return width; }
unsigned retrom_height(void) { return height; }
double retrom_fps(void) { return fps; }
void *retrom_pixels(void) { return pixels; }
void *retrom_audio(void) { return samples; }
unsigned retrom_audio_count(void) { return sample_count; }
void *retrom_state(void) {
    if (!retrom_ready()) return NULL;
    size_t size = retro_serialize_size();
    if (!size || size > 32 * 1024 * 1024) return NULL;
    void *next = realloc(state, size);
    if (!next) return NULL;
    state = next; state_size = size;
    if (!retro_serialize(state, size)) return NULL;
    return state;
}
size_t retrom_state_size(void) { return state_size; }
int retrom_restore(const void *data, size_t size) {
    return loaded && size > 0 && size <= 32 * 1024 * 1024 && retro_unserialize(data, size);
}
void retrom_stop(void) {
    if (loaded) { retro_unload_game(); retro_deinit(); loaded = 0; }
    free(state); state = NULL; state_size = 0; memset(keys, 0, sizeof(keys));
}
