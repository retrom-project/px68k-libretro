#include <assert.h>
#include "../bridge.c"
static retro_environment_t host_environment;
static retro_video_refresh_t host_video;
static retro_input_state_t host_input;
static int executions;
void retro_set_environment(retro_environment_t callback) { host_environment = callback; }
void retro_set_video_refresh(retro_video_refresh_t callback) { host_video = callback; }
void retro_set_audio_sample(retro_audio_sample_t callback) { (void)callback; }
void retro_set_audio_sample_batch(retro_audio_sample_batch_t callback) { (void)callback; }
void retro_set_input_poll(retro_input_poll_t callback) { (void)callback; }
void retro_set_input_state(retro_input_state_t callback) { host_input = callback; }
void retro_init(void) {
    struct retro_log_callback log = {0};
    assert(host_environment(RETRO_ENVIRONMENT_GET_LOG_INTERFACE, &log));
    assert(log.log); log.log(RETRO_LOG_INFO, "initializing");
    struct retro_variable option = {"px68k_no_wait_mode", NULL};
    assert(host_environment(RETRO_ENVIRONMENT_GET_VARIABLE, &option));
    assert(strcmp(option.value, "enabled") == 0);
}
bool retro_load_game(const struct retro_game_info *game) { return game->path != NULL; }
void retro_set_controller_port_device(unsigned port, unsigned device) { (void)port; (void)device; }
void retro_run(void) {
    const uint16_t frame[] = {0xf800, 0x07e0, 0x001f};
    executions++; host_video(frame, 3, 1, sizeof(frame));
}
void retro_get_system_av_info(struct retro_system_av_info *av) { av->timing.fps = 60; }
int retrom_menu_active(void) { return 0; }
size_t retro_serialize_size(void) { return sizeof(executions); }
bool retro_serialize(void *data, size_t size) { memcpy(data, &executions, size); return true; }
bool retro_unserialize(const void *data, size_t size) {
    if (size != sizeof(executions)) return false;
    memcpy(&executions, data, size); return true;
}
void retro_unload_game(void) {}
void retro_deinit(void) {}
int main(void) {
    assert(retrom_load("disk0.dim") == 1);
    assert(retrom_width() == 3 && retrom_height() == 1);
    assert(pixels[0] == 0xff0000ff && pixels[1] == 0xff00ff00 && pixels[2] == 0xffff0000);
    assert(retrom_step(1 << 8, 1) == 1);
    assert(host_input(0, RETRO_DEVICE_JOYPAD, 0, 8) == 1);
    assert(host_input(1, RETRO_DEVICE_JOYPAD, 0, 0) == 1);
    retrom_key(13, 1); assert(host_input(0, RETRO_DEVICE_KEYBOARD, 0, 13) == 1);
    int saved = executions;
    void *checkpoint = retrom_state(); assert(checkpoint && retrom_state_size() == sizeof(int));
    retrom_step(0, 0); assert(retrom_restore(checkpoint, sizeof(int)) == 1 && executions == saved);
    assert(retrom_restore(checkpoint, 0) == 0);
    retrom_stop(); assert(retrom_ready() == 0 && retrom_step(0, 0) == 0);
    assert(host_input(0, RETRO_DEVICE_KEYBOARD, 0, 13) == 0);
    return 0;
}
