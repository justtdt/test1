/*
Plugin Name
Copyright (C) <Year> <Developer> <Email Address>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with this program. If not, see <https://www.gnu.org/licenses/>
*/

#include <obs-module.h>
#include <plugin-support.h>

OBS_DECLARE_MODULE()
OBS_MODULE_USE_DEFAULT_LOCALE(PLUGIN_NAME, "en-US")

bool obs_module_load(void)
{
	obs_log(LOG_INFO, "plugin loaded successfully (version %s)", PLUGIN_VERSION);
	return true;
}

void obs_module_unload(void)
{
	obs_log(LOG_INFO, "plugin unloaded");
}
#include <obs-module.h>
#include <string.h>

// 检查并修复源的设置
static bool fix_source_proc(void *unused, obs_source_t *source) {
    if (!source) return true;

    const char *id = obs_source_get_unversioned_id(source);
    if (id && strcmp(id, "ffmpeg_source") == 0) {
        obs_data_t *settings = obs_source_get_settings(source);
        if (settings) {
            // 只要没开硬解或没开启不活跃关闭，就强制修正
            bool hw = obs_data_get_bool(settings, "hw_decode");
            bool cl = obs_data_get_bool(settings, "close_when_inactive");

            if (!hw || !cl) {
                obs_data_set_bool(settings, "hw_decode", true);
                obs_data_set_bool(settings, "close_when_inactive", true);
                obs_data_set_bool(settings, "restart_on_active", true);
                obs_source_update(source, settings);
            }
            obs_data_release(settings);
        }
    }
    return true;
    (void)unused;
}

// 计时器：每 120 帧（2秒）扫射一次全场，解决切换场景集合的问题
static void global_tick(void *param, uint64_t tick) {
    static int frames = 0;
    if (++frames >= 120) {
        frames = 0;
        obs_enum_sources(fix_source_proc, NULL);
    }
    (void)param; (void)tick;
}

// 官方标准：模块加载入口
bool obs_module_load(void) {
    obs_add_tick_callback(global_tick, NULL);
    return true;
}

// 官方标准：模块卸载
void obs_module_unload(void) {
    obs_remove_tick_callback(global_tick, NULL);
}

OBS_DECLARE_MODULE()
const char *obs_module_name() { return "Global Media Fixer"; }
