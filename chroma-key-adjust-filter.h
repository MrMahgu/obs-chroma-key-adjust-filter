#pragma once

#include <obs-module.h>
#include <graphics/graphics.h>

#include "forms/ColorSelectWidget.h"

#define OBS_PLUGIN "chroma-key-adjust-filter"
#define OBS_PLUGIN_ "chroma_key_adjust_filter"
#define OBS_PLUGIN_VERSION_MAJOR 0
#define OBS_PLUGIN_VERSION_MINOR 0
#define OBS_PLUGIN_VERSION_RELEASE 1
#define OBS_PLUGIN_VERSION_STRING "0.0.1"
#define OBS_PLUGIN_LANG "en-US"

#define OBS_UI_SETTING_FILTER_NAME "mahgu.chromakeyadjust.ui.filter_title"

#define obs_log(level, format, ...) \
	blog(level, "[chroma-key-adjust-filter] " format, ##__VA_ARGS__)

#define error(format, ...) obs_log(LOG_ERROR, format, ##__VA_ARGS__)
#define warn(format, ...) obs_log(LOG_WARNING, format, ##__VA_ARGS__)
#define info(format, ...) obs_log(LOG_INFO, format, ##__VA_ARGS__)
#define debug(format, ...) obs_log(LOG_DEBUG, format, ##__VA_ARGS__)

bool obs_module_load(void);
void obs_module_unload();

namespace ChromaKeyAdjust {

void report_version();

static const char *filter_get_name(void *unused);
static obs_properties_t *filter_properties(void *data);
static void filter_defaults(obs_data_t *settings);

static void *filter_create(obs_data_t *settings, obs_source_t *source);
static void filter_destroy(void *data);

struct filter {
	obs_source_t *context;
};

struct obs_source_info create_filter_info()
{
	struct obs_source_info filter_info = {};

	filter_info.id = OBS_PLUGIN_;
	filter_info.type = OBS_SOURCE_TYPE_FILTER;
	filter_info.output_flags = OBS_SOURCE_VIDEO | OBS_SOURCE_SRGB;

	filter_info.get_name = filter_get_name;
	filter_info.get_properties = filter_properties;
	filter_info.get_defaults = filter_defaults;
	filter_info.create = filter_create;
	filter_info.destroy = filter_destroy;

	return filter_info;
};

} // namespace ChromaKeyAdjust
