#include "chroma-key-adjust-filter.h"

#include "forms/ColorSelectWidget.h"

#ifdef DEBUG
#include <string>
#endif

#include <QList>
#include <QMainWindow>
#include <QApplication>
#include <QWindow>

OBS_DECLARE_MODULE()
OBS_MODULE_USE_DEFAULT_LOCALE(OBS_PLUGIN, OBS_PLUGIN_LANG)

ColorSelectWidget *_widget = nullptr;

bool _widget_bound = false;
bool _widget_visible = true;
bool _handled_signal = false;

obs_source_t *_child = nullptr;

static inline QColor color_from_int(long long val)
{
	return QColor(val & 0xff, (val >> 8) & 0xff, (val >> 16) & 0xff,
		      (val >> 24) & 0xff);
}

static inline long long color_to_int(QColor color)
{
	auto shift = [&](unsigned val, int shift) {
		return ((val & 0xff) << shift);
	};

	return shift(color.red(), 0) | shift(color.green(), 8) |
	       shift(color.blue(), 16) | shift(color.alpha(), 24);
}

void UpdateLinkedChromaKeyFilterColorSetting(QColor color)
{
	if (!_child)
		return;

	obs_data_t *_filter_settings = obs_source_get_settings(_child);
	obs_data_set_int(_filter_settings, "key_color", color_to_int(color));
	obs_source_update(_child, _filter_settings);
	obs_data_release(_filter_settings);
}

namespace ChromaKeyAdjust {

static void update_child_source_pointer(obs_source_t *source)
{
	_child = source;
}

static void source_filter_removed(void *data, calldata_t *cd)
{
	obs_source_t *filter = (obs_source_t *)calldata_ptr(cd, "filter");
	if (filter == _child) {
		_child = nullptr;
	}
}

static bool filter_btn_test(obs_properties_t *, obs_property_t *, void *data)
{
	auto filter = (struct filter *)data;
	if (!_widget) {

		uint32_t _known_color = color_to_int(QColor(Qt::green));

		// Reset our child?
		if (_child) {
			_child = nullptr;
		}

		// Bind our chroma key source filter
		obs_source_enum_filters(
			obs_filter_get_parent(filter->context),
			[](obs_source_t *parent, obs_source_t *child,
			   void *param) {
				UNUSED_PARAMETER(parent);
				UNUSED_PARAMETER(param);

				obs_data_t *_tmp_settings =
					obs_source_get_settings(child);

				const char *key_color = "key_color";
				const char *key_color_type = "key_color_type";

				if (strcmp(obs_source_get_id(child),
					   "chroma_key_filter_v2") == 0) {

					bool update_custom = false;
					bool leave_color = false;


					// check if we need to update it
					// key, is set, make sure its set to "custom"
					if (obs_data_has_default_value(
						    _tmp_settings, key_color)) {



						if (strcmp(obs_data_get_string(
								   _tmp_settings,
								   key_color_type),
							   "custom") != 0) {

							obs_data_set_string(
								_tmp_settings,
								key_color_type,
								"custom");

							obs_data_set_int(
								_tmp_settings,
								key_color,
								color_to_int(QColor(
									Qt::green)));

							obs_source_update(
								_child,
								_tmp_settings);

							info("chroma key was forced into custom mode");
						}
					}
					update_child_source_pointer(child);
				}
				obs_data_release(_tmp_settings);
			},
			nullptr);

		if (_child) {
			std::string filter_start("Chroma key filter found: ");
			std::string filter_name(obs_source_get_name(_child));
			std::string filter_final = filter_start + filter_name;
			blog(LOG_INFO, filter_final.c_str());

			// register some bad ass singals
			signal_handler_t *handler =
				obs_source_get_signal_handler(
					obs_filter_get_parent(filter->context));

			// disconnect any old stuff
			if (_handled_signal && handler) {
				signal_handler_disconnect(
					handler, "filter_remove",
					&source_filter_removed, filter);
				_handled_signal = false;
			}

			if (handler) {
				signal_handler_connect(handler, "filter_remove",
						       &source_filter_removed,
						       filter);
				_handled_signal = true;
			}

			// Get known color

			obs_data_t *_tmp_settings =
				obs_source_get_settings(_child);

			_known_color =
				obs_data_get_int(_tmp_settings, "key_color");

			obs_data_release(_tmp_settings);

		} else {
			info("No Chroma key filter could be detected");
			return false;
		}

		QColor color = color_from_int(_known_color);

		_widget = new ColorSelectWidget(nullptr, color);
		_widget->setWindowTitle("Key Color");
		_widget->setWindowFlags(Qt::WindowStaysOnTopHint);

		QObject::connect(_widget, &ColorSelectWidget::closed, []() {
			_widget = nullptr;
			_child = nullptr;
		});

		// connect color change
		QObject::connect(
			_widget, &ColorSelectWidget::colorChanged,
			[=](const QColor &color) {
				UpdateLinkedChromaKeyFilterColorSetting(color);
			});

		// additional check
		if (_widget) {
			// Show and focus widget
			_widget->showNormal();
			_widget->focusWidget();
		}
		return true;
	}
	return true;
}

static const char *filter_get_name(void *unused)
{
	UNUSED_PARAMETER(unused);
	return obs_module_text(OBS_UI_SETTING_FILTER_NAME);
}

static obs_properties_t *filter_properties(void *data)
{
	UNUSED_PARAMETER(data);

	auto props = obs_properties_create();

	obs_properties_add_button(props, "btn_color_widget",
				  "Open Color Widget", filter_btn_test);

	return props;
}

static void filter_defaults(obs_data_t *settings)
{
	UNUSED_PARAMETER(settings);
}

static void filter_video_render(void *data, gs_effect_t *effect)
{
	UNUSED_PARAMETER(effect);

	auto filter = (struct filter *)data;

	if (!filter->context)
		return;

	obs_source_skip_video_filter(filter->context);
}

static void *filter_create(obs_data_t *settings, obs_source_t *source)
{
	UNUSED_PARAMETER(settings);

	auto filter = (struct filter *)bzalloc(sizeof(ChromaKeyAdjust::filter));

	// Setup the obs context
	filter->context = source;

	return filter;
}

static void filter_destroy(void *data)
{
	auto filter = (struct filter *)data;

	if (filter) {

		// Remove our signal (not sure if we need to do this)
		if (_handled_signal) {
			signal_handler_t *handler =
				obs_source_get_signal_handler(
					obs_filter_get_parent(filter->context));

			if (handler) {
				signal_handler_disconnect(
					handler, "filter_remove",
					&source_filter_removed, filter);
			}
		}
		bfree(filter);
	}
}

// Writes a simple log entry to OBS
void report_version()
{
#ifdef DEBUG
	info("you can haz chroma-key-adjust tooz (Version: %s)",
	     OBS_PLUGIN_VERSION_STRING);
#else
	info("obs-chroma-key-adjust-filter [mrmahgu] - version %s",
	     OBS_PLUGIN_VERSION_STRING);
#endif
}

} // namespace ChromaKeyAdjust

bool obs_module_load(void)
{
	auto filter_info = ChromaKeyAdjust::create_filter_info();

	obs_register_source(&filter_info);

	ChromaKeyAdjust::report_version();

	return true;
}

void obs_module_unload() {}
