#include "chroma-key-adjust-filter.h"
#include "forms/widget.h"

#ifdef DEBUG
#include <string>
#endif

#include <QList>
#include <QMainWindow>
#include <QApplication>
#include <QWindow>

OBS_DECLARE_MODULE()
OBS_MODULE_USE_DEFAULT_LOCALE(OBS_PLUGIN, OBS_PLUGIN_LANG)

Widget *_widget = nullptr;
bool _widget_bound = false;
bool _widget_visible = true;

obs_source_t *_child = nullptr;

void UpdateLinkedChromaKeyFilterColorSetting(QColor color)
{
	if (!_child)
		return;

	obs_data_t *_filter_settings = obs_source_get_settings(_child);
	obs_data_set_int(_filter_settings, "key_color", color.rgba());
	obs_source_update(_child, _filter_settings);
	obs_data_release(_filter_settings);
}

namespace ChromaKeyAdjust {

static bool filter_btn_test(obs_properties_t *, obs_property_t *, void *data)
{
	auto filter = (struct filter *)data;
	if (!_widget) {
		_widget = new Widget();
		_widget->setWindowTitle("Key Color");
		_widget->setWindowFlags(Qt::WindowStaysOnTopHint);
		QObject::connect(_widget, &Widget::closed, []() {
			_widget = nullptr;
			_child = nullptr;
		});

		// connect color change
		QObject::connect(
			_widget, &Widget::colorChanged,
			[=](const QColor &color) {
				UpdateLinkedChromaKeyFilterColorSetting(color);
			});

		// Bind our chroma key source filter
		obs_source_enum_filters(
			obs_filter_get_parent(filter->context),
			[](obs_source_t *parent, obs_source_t *child,
			   void *param) {
				UNUSED_PARAMETER(parent);
				UNUSED_PARAMETER(param);
				//blog(LOG_INFO, obs_source_get_name(child));
				if (strcmp("Chroma Key",
					   obs_source_get_name(child)) == 0) {
					_child = child;
				}
			},
			nullptr);

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
