#include <obs-module.h>
#include <string.h>

// 插件元数据
OBS_DECLARE_MODULE()
const char *obs_module_name() { return "Global Media Fixer"; }

// 核心逻辑：修复函数
static bool fix_source_proc(void *unused, obs_source_t *source) {
    if (!source) return true;

    const char *id = obs_source_get_unversioned_id(source);
    if (id && strcmp(id, "ffmpeg_source") == 0) {
        obs_data_t *settings = obs_source_get_settings(source);
        if (settings) {
            // 强制检查并补勾
            obs_data_set_bool(settings, "hw_decode", true);
            obs_data_set_bool(settings, "close_when_inactive", true);
            obs_data_set_bool(settings, "restart_on_active", true);
            
            obs_source_update(source, settings);
            obs_data_release(settings);
        }
    }
    return true;
    (void)unused;
}

// 计时器：每120帧（约2秒）扫一遍，解决场景集合切换导致的失效
static void global_tick(void *param, uint64_t tick) {
    static int frames = 0;
    if (++frames >= 120) {
        frames = 0;
        obs_enum_sources(fix_source_proc, NULL);
    }
    (void)param; (void)tick;
}

bool obs_module_load(void) {
    obs_add_tick_callback(global_tick, NULL);
    return true;
}

void obs_module_unload(void) {
    obs_remove_tick_callback(global_tick, NULL);
}
