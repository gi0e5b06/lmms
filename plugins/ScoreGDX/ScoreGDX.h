/*
 */

#ifndef SCOREGDX_H
#define SCOREGDX_H

#include "ToolPlugin.h"

class ScoreGDX : public ToolPlugin
{
	Q_OBJECT
public:
	explicit ScoreGDX(Model *parent);

	PluginView *instantiateView(QWidget *parent) override;
	QString nodeName() const override;
	void saveSettings(QDomDocument &doc, QDomElement &elem) override;
	void loadSettings(const QDomElement &elem) override;
};

#endif // SCOREGDX_H
