/*
 * InspectorView.h - Dynamic Qt object inspector/editor for LMMS
 *
 * Copyright (c) 2020 Dominic Clark <mrdomclark/at/gmail.com>
 *
 * This file is part of LMMS - https://lmms.io
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program (see COPYING); if not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301 USA.
 *
 */

#ifndef INSPECTOR_VIEW_H
#define INSPECTOR_VIEW_H

#include <vector>

#include <QAbstractTableModel>
#include <QBrush>
#include <QColor>
#include <QStyledItemDelegate>
#include <QTimer>
#include <QTreeWidget>
#include <QWidget>

#include "ToolPluginView.h"

class PropertyItemDelegate : public QStyledItemDelegate
{
	Q_OBJECT
public:
	using QStyledItemDelegate::QStyledItemDelegate;

	QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
	void setEditorData(QWidget *editor, const QModelIndex &index) const override;
	void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override;
};

class PropertyTableModel : public QAbstractTableModel
{
	Q_OBJECT
public:
	using QAbstractTableModel::QAbstractTableModel;

	QObject *object() const { return m_object; }
	void setObject(QObject *object);

	int columnCount(const QModelIndex &parent = QModelIndex{}) const override;
	QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
	Qt::ItemFlags flags(const QModelIndex &index) const override;
	QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
	int rowCount(const QModelIndex &parent = QModelIndex{}) const override;
	bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;

private:
	QObject *m_object = nullptr;
};

class SelectorWidget : public QWidget
{
	Q_OBJECT
	Q_PROPERTY(QBrush fillBrush READ fillBrush WRITE setFillBrush)
	Q_PROPERTY(QColor borderColor READ borderColor WRITE setBorderColor)
public:
	explicit SelectorWidget(QWidget *parent);
	~SelectorWidget();

	QObject *target() const { return m_currentTarget; }

	const QBrush &fillBrush() const { return m_fillBrush; }
	void setFillBrush(const QBrush &brush) { m_fillBrush = brush; }
	const QColor &borderColor() const { return m_borderColor; }
	void setBorderColor(const QColor &color) { m_borderColor = color; }

	bool eventFilter(QObject *watched, QEvent *event) override;

signals:
	void objectSelected(QObject *object);

public slots:
	void beginSelection();
	void highlightObject(QObject *object);

protected:
	void mouseMoveEvent(QMouseEvent *event) override;
	void mouseReleaseEvent(QMouseEvent *event) override;
	void paintEvent(QPaintEvent *event) override;

private:
	QBrush m_fillBrush = QColor{63, 127, 255, 63};
	QColor m_borderColor = {63, 127, 255};
	QWidget *m_currentTarget = nullptr;
	QTimer m_highlightTimer;
};

class InspectorView : public ToolPluginView
{
	Q_OBJECT
public:
	explicit InspectorView(ToolPlugin *plugin);
	~InspectorView() override;

	bool eventFilter(QObject *watched, QEvent *event) override;

private slots:
	void updateTree();
	void selectionChanged();
	void objectSelected(QObject *object);

private:
	void addObject(QObject *object);
	void removeObject(QObject *object);
	QTreeWidgetItem *createTreeWidgetItemForObject(QObject *object);

	std::vector<QObject *> m_incomingObjects;
	QTimer m_updateTrigger;
	QTreeWidget *m_tree;
	PropertyTableModel *m_propertyModel;
	SelectorWidget *m_selector;
};

#endif // INSPECTOR_VIEW_H
