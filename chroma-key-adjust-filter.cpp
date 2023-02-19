#include "chroma-key-adjust-filter.h"
#include "forms/ColorSelectWidget.h"

#include <QList>
#include <QMainWindow>
#include <QApplication>
#include <QWindow>

OBS_DECLARE_MODULE()
OBS_MODULE_USE_DEFAULT_LOCALE(OBS_PLUGIN, OBS_PLUGIN_LANG)

/* clang-format off */

#define OBS_CHROMA_KEY_FILTER_ID       "chroma_key_filter_v2"

#define SETTING_COLOR_TYPE             "key_color_type"
#define SETTING_KEY_COLOR              "key_color"

#define SOURCE_TYPE_FILTER             "filter"

#define SIGNAL_FILTER_REMOVE           "filter_remove"

/* clang-format on */

namespace Widget {

std::unique_ptr<ColorSelectWidget> _widget;

bool _widget_bound = false;
bool _widget_visible = true;
bool _handled_signal = false;

obs_source_t *_child = nullptr;

static void update_child_pointer(obs_source_t *source)
{
	Widget::_child = source;
}

static void reset_child_pointer()
{
	Widget::_child = nullptr;
}

} // namespace Widget

namespace ColorUtil {

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

} // namespace ColorUtil

void UpdateLinkedChromaKeyFilterColorSetting(QColor color)
{
	if (!Widget::_child)
		return;

	obs_data_t *_filter_settings = obs_source_get_settings(Widget::_child);
	obs_data_set_int(_filter_settings, SETTING_KEY_COLOR,
			 ColorUtil::color_to_int(color));
	obs_source_update(Widget::_child, _filter_settings);
	obs_data_release(_filter_settings);
}

namespace ChromaKeyAdjust {

static void source_filter_removed(void *data, calldata_t *cd)
{
	UNUSED_PARAMETER(data);

	obs_source_t *filter =
		(obs_source_t *)calldata_ptr(cd, SOURCE_TYPE_FILTER);

	if (filter == Widget::_child)
		Widget::reset_child_pointer();
}

static void filter_source_enum_callback(obs_source_t *parent,
					obs_source_t *child, void *param)
{
	UNUSED_PARAMETER(parent);
	UNUSED_PARAMETER(param);

	obs_data_t *_tmp_settings = obs_source_get_settings(child);

	if (strcmp(obs_source_get_id(child), OBS_CHROMA_KEY_FILTER_ID) == 0) {

		// Check if the setting is not custom, if so, force change it
		if (obs_data_has_default_value(_tmp_settings,
					       SETTING_KEY_COLOR)) {

			if (strcmp(obs_data_get_string(_tmp_settings,
						       SETTING_COLOR_TYPE),
				   "custom") != 0) {

				obs_data_set_string(_tmp_settings,
						    SETTING_COLOR_TYPE,
						    "custom");

				obs_data_set_int(_tmp_settings,
						 SETTING_KEY_COLOR,
						 ColorUtil::color_to_int(
							 QColor(Qt::green)));

				obs_source_update(Widget::_child,
						  _tmp_settings);
			}
		}
		Widget::update_child_pointer(child);
	}
	obs_data_release(_tmp_settings);
}

static bool filter_button_open_widget(obs_properties_t *, obs_property_t *,
				      void *data)
{
	auto filter = (struct filter *)data;

	if (!Widget::_widget) {

		uint32_t _known_color =
			ColorUtil::color_to_int(QColor(Qt::green));

		// Reset the child pointer if it exists
		if (Widget::_child)
			Widget::reset_child_pointer();

		// Bind our chroma key source filter
		obs_source_enum_filters(obs_filter_get_parent(filter->context),
					filter_source_enum_callback, nullptr);

		if (Widget::_child) {

			// register some bad ass singals
			signal_handler_t *handler =
				obs_source_get_signal_handler(
					obs_filter_get_parent(filter->context));

			// disconnect any old stuff
			if (Widget::_handled_signal && handler) {
				signal_handler_disconnect(
					handler, SIGNAL_FILTER_REMOVE,
					&source_filter_removed, filter);
				Widget::_handled_signal = false;
			}

			// Connect any new stuff
			if (handler) {
				signal_handler_connect(handler,
						       SIGNAL_FILTER_REMOVE,
						       &source_filter_removed,
						       filter);
				Widget::_handled_signal = true;
			}

			// Get known color
			obs_data_t *_tmp_settings =
				obs_source_get_settings(Widget::_child);

			_known_color = obs_data_get_int(_tmp_settings,
							SETTING_KEY_COLOR);

			obs_data_release(_tmp_settings);

		} else {
			info("No Chroma key filter could be detected");
			return false;
		}

		QColor color = ColorUtil::color_from_int(_known_color);

		Widget::_widget.reset(new ColorSelectWidget(nullptr, color));
		Widget::_widget->setWindowTitle("Key Color");
		Widget::_widget->setWindowFlags(Qt::WindowStaysOnTopHint);

		// Connect to closed event to reset our widget
		QObject::connect(Widget::_widget.get(),
				 &ColorSelectWidget::closed, []() {
					 Widget::_widget.reset(nullptr);
					 Widget::_child = nullptr;
				 });

		// connect color change
		QObject::connect(
			Widget::_widget.get(), &ColorSelectWidget::colorChanged,
			[=](const QColor &color) {
				UpdateLinkedChromaKeyFilterColorSetting(color);
			});

		// additional check
		if (Widget::_widget) {
			// Show and focus widget
			Widget::_widget->showNormal();
			Widget::_widget->focusWidget();
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

	// TODO Convert to OBS_TEXT
	obs_properties_add_button(props, "btn_color_widget",
				  "Open Color Widget",
				  filter_button_open_widget);

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
		if (Widget::_widget) {
			Widget::_widget->close();
		}

		if (Widget::_widget) {
			Widget::_widget.reset(nullptr);
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
