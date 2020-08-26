/*
 */

#include "ScoreGDX.h"

#include "embed.h"
#include "export.h"

#include "ScoreGDXView.h"

extern "C"
{

Plugin::Descriptor PLUGIN_EXPORT scoregdx_plugin_descriptor{
	STRINGIFY(PLUGIN_NAME),
	"ScoreGDX",
	QT_TRANSLATE_NOOP("ScoreGDX", "Display the score in traditional notation"),
	"gi0e5b06 (on github.com)",
	0x0001,
	Plugin::Tool,
	new PluginPixmapLoader("logo"),
	nullptr,
	nullptr
};

PLUGIN_EXPORT Plugin *lmms_plugin_main(Model *parent, void *data)
{
	return new ScoreGDX(parent);
}

} // extern "C"

ScoreGDX::ScoreGDX(Model *parent) :
	ToolPlugin(&scoregdx_plugin_descriptor, parent)
{ }

PluginView *ScoreGDX::instantiateView(QWidget *parent)
{
	return new ScoreGDXView(this);
}

QString ScoreGDX::nodeName() const
{
	return "scoregdx";
}

void ScoreGDX::saveSettings(QDomDocument &doc, QDomElement &elem)
{ }

void ScoreGDX::loadSettings(const QDomElement &elem)
{ }
