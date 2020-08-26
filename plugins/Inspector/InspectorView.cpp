/*
 * InspectorView.cpp - Dynamic Qt object inspector/editor for LMMS
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

#include "InspectorView.h"

#include "GuiApplication.h"
#include "MainWindow.h"
#include "embed.h"

#include <QChildEvent>
#include <QComboBox>
#include <QCoreApplication>
#include <QDebug>
#include <QEvent>
#include <QHeaderView>
#include <QKeyEvent>
#include <QMetaProperty>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QPainter>
#include <QSplitter>
#include <QTableView>
#include <QToolBar>
#include <QVBoxLayout>

#include <algorithm>
#include <iterator>

Q_DECLARE_METATYPE(QMetaProperty)

constexpr const char* INSPECTOR_PROPERTY  = "_lmms_inspector_treeitem";
constexpr int         OBJECT_POINTER_ROLE = Qt::UserRole;
constexpr int         PROPERTY_ROLE       = Qt::UserRole;

QWidget*
        PropertyItemDelegate::createEditor(QWidget*                    parent,
                                           const QStyleOptionViewItem& option,
                                           const QModelIndex& index) const
{
    const auto property = index.data(PROPERTY_ROLE).value<QMetaProperty>();
    if(property.isEnumType() && !property.isFlagType())
    {
        const auto enumerator = property.enumerator();
        const auto comboBox   = new QComboBox{parent};
        comboBox->setFrame(false);
        comboBox->setSizePolicy(QSizePolicy::Ignored,
                                comboBox->sizePolicy().verticalPolicy());
        for(auto i = 0; i < enumerator.keyCount(); ++i)
        {
            comboBox->addItem(enumerator.key(i));
        }
        return comboBox;
    }
    return QStyledItemDelegate::createEditor(parent, option, index);
}

void PropertyItemDelegate::setEditorData(QWidget*           editor,
                                         const QModelIndex& index) const
{
    const auto property = index.data(PROPERTY_ROLE).value<QMetaProperty>();
    if(property.isEnumType() && !property.isFlagType())
    {
        const auto comboBox = static_cast<QComboBox*>(editor);
        const auto value    = property.enumerator().valueToKey(
                index.data(Qt::EditRole).toInt());
        comboBox->setCurrentText(value);
        return;
    }
    QStyledItemDelegate::setEditorData(editor, index);
}

void PropertyItemDelegate::setModelData(QWidget*            editor,
                                        QAbstractItemModel* model,
                                        const QModelIndex&  index) const
{
    const auto property = index.data(PROPERTY_ROLE).value<QMetaProperty>();
    if(property.isEnumType() && !property.isFlagType())
    {
        const auto comboBox = static_cast<QComboBox*>(editor);
        const auto value    = property.enumerator().keyToValue(
                comboBox->currentText().toUtf8().data());
        model->setData(index, value, Qt::EditRole);
        return;
    }
    QStyledItemDelegate::setModelData(editor, model, index);
}

void PropertyTableModel::setObject(QObject* object)
{
    if(m_object != object)
    {
        beginResetModel();
        m_object = object;
        endResetModel();
    }
}

int PropertyTableModel::columnCount(const QModelIndex& parent) const
{
    if(parent.isValid())
    {
        return 0;
    }
    return 2;
}

QVariant PropertyTableModel::data(const QModelIndex& index, int role) const
{
    const auto property = m_object->metaObject()->property(index.row());
    if(role == PROPERTY_ROLE)
    {
        return QVariant::fromValue(property);
    }
    switch(index.column())
    {
        case 0:
            if(role == Qt::DisplayRole)
            {
                return property.name();
            }
            break;
        case 1:
            switch(role)
            {
                case Qt::DisplayRole:
                    if(property.isEnumType() && !property.isFlagType())
                    {
                        return property.enumerator().valueToKey(
                                property.read(m_object).toInt());
                    }
                    return property.read(m_object);
                case Qt::EditRole:
                    return property.read(m_object);
                case Qt::BackgroundRole:
                    if(property.userType() == QMetaType::QColor)
                    {
                        auto color = property.read(m_object).value<QColor>();
                        color.setAlpha(255);
                        return color;
                    }
                    break;
                case Qt::ForegroundRole:
                    if(property.userType() == QMetaType::QColor)
                    {
                        const auto color
                                = property.read(m_object).value<QColor>();
                        const auto isLight = 0.299 * color.redF()
                                                     + 0.587 * color.greenF()
                                                     + 0.114 * color.blueF()
                                             > 0.5;
                        return isLight
                                       // QColorConstants::Black
                                       ? QColor(0, 0, 0)
                                       // QColorConstants::White;
                                       : QColor(255, 255, 255);
                    }
                    break;
            }
            break;
    }
    return {};
}

Qt::ItemFlags PropertyTableModel::flags(const QModelIndex& index) const
{
    auto flags = QAbstractTableModel::flags(index);
    if(index.column() == 1
       && m_object->metaObject()->property(index.row()).isWritable())
    {
        flags |= Qt::ItemIsEditable;
    }
    return flags;
}

QVariant PropertyTableModel::headerData(int             section,
                                        Qt::Orientation orientation,
                                        int             role) const
{
    if(orientation == Qt::Horizontal && role == Qt::DisplayRole)
    {
        switch(section)
        {
            case 0:
                return tr("Property");
            case 1:
                return tr("Value");
        }
    }
    return {};
}

int PropertyTableModel::rowCount(const QModelIndex& parent) const
{
    if(parent.isValid())
    {
        return 0;
    }
    return m_object ? m_object->metaObject()->propertyCount() : 0;
}

bool PropertyTableModel::setData(const QModelIndex& index,
                                 const QVariant&    value,
                                 int                role)
{
    if(role == Qt::EditRole && index.column() == 1)
    {
        if(m_object->metaObject()
                   ->property(index.row())
                   .write(m_object, value))
        {
            static_cast<QWidget*>(m_object)->update();
            return true;
        }
    }
    return false;
}

SelectorWidget::SelectorWidget(QWidget* parent) : QWidget{parent}
{
    m_highlightTimer.setInterval(500);
    m_highlightTimer.setSingleShot(true);
    connect(&m_highlightTimer, &QTimer::timeout, this, &QWidget::hide);
    move(0, 0);
    setFixedSize(parentWidget()->size());
    setMouseTracking(true);
    hide();
    parentWidget()->installEventFilter(this);
}

SelectorWidget::~SelectorWidget()
{
    parentWidget()->removeEventFilter(this);
}

bool SelectorWidget::eventFilter(QObject* watched, QEvent* event)
{
    switch(event->type())
    {
        case QEvent::Resize:
            setFixedSize(parentWidget()->size());
            break;
    }
    return false;
}

void SelectorWidget::beginSelection()
{
    m_currentTarget = nullptr;
    setAttribute(Qt::WA_TransparentForMouseEvents, false);
    m_highlightTimer.stop();
    show();
    raise();
    update();
}

void SelectorWidget::highlightObject(QObject* object)
{
    m_currentTarget = qobject_cast<QWidget*>(object);
    if(m_currentTarget && m_currentTarget->isVisible())
    {
        setAttribute(Qt::WA_TransparentForMouseEvents, true);
        m_highlightTimer.start();
        show();
        raise();
        update();
    }
    else
    {
        hide();
    }
}

void SelectorWidget::mouseMoveEvent(QMouseEvent* event)
{
    auto point  = event->pos();
    auto target = static_cast<QWidget*>(nullptr);
    // Find the deepest child widget containing the cursor location.
    // Code loosely based on QWidgetPrivate::childAtRecursiveHelper.
    auto nextWidget = parentWidget();
    while(nextWidget != nullptr)
    {
        target = nextWidget;
        // Find the topmost child widget containing the cursor location
        nextWidget           = nullptr;
        const auto& children = target->children();
        for(auto i = children.size() - 1; i >= 0; --i)
        {
            const auto child = qobject_cast<QWidget*>(children.at(i));
            if(child && child != this && !child->isHidden()
               && child->geometry().contains(point))
            {
                point -= child->pos();
                nextWidget = child;
                break;
            }
        }
    }
    if(m_currentTarget != target)
    {
        m_currentTarget = target;
        update();
    }
}

void SelectorWidget::mouseReleaseEvent(QMouseEvent* event)
{
    if(event->button() == Qt::LeftButton)
    {
        hide();
        if(m_currentTarget)
        {
            emit objectSelected(m_currentTarget);
        }
    }
}

void SelectorWidget::paintEvent(QPaintEvent* event)
{
    QPainter painter{this};
    if(m_currentTarget)
    {
        const auto origin
                = mapFromGlobal(m_currentTarget->mapToGlobal({0, 0}));
        const auto bounds = m_currentTarget->size();
        painter.setBrush(m_fillBrush);
        painter.setPen(m_borderColor);
        painter.drawRect(QRect{origin, bounds});
    }
}

InspectorView::InspectorView(ToolPlugin* plugin) : ToolPluginView{plugin}
{
    //setBaseSize(750, 500);
    //setPreferredSize(750, 500);

    m_updateTrigger.setSingleShot(true);
    connect(&m_updateTrigger, &QTimer::timeout, this,
            &InspectorView::updateTree);
    m_selector = new SelectorWidget{gui->mainWindow()};
    connect(m_selector, &SelectorWidget::objectSelected, this,
            &InspectorView::objectSelected);
    const auto layout = new QVBoxLayout{this};
    layout->setSpacing(0);
    layout->setMargin(0);
    const auto toolbar = new QToolBar{};
    toolbar->addAction(embed::getIcon("zoom"), tr("Inspect"), m_selector,
                       &SelectorWidget::beginSelection);
    layout->addWidget(toolbar);
    const auto splitter = new QSplitter{};
    m_tree              = new QTreeWidget{};
    m_tree->setHeaderHidden(true);
    connect(m_tree, &QTreeWidget::itemSelectionChanged, this,
            &InspectorView::selectionChanged);
    connect(m_tree, &QAbstractItemView::clicked, this,
            [this](const QModelIndex& index) {
                m_selector->highlightObject(static_cast<QObject*>(
                        index.data(OBJECT_POINTER_ROLE).value<void*>()));
            });
    splitter->addWidget(m_tree);
    const auto table = new QTableView{};
    table->horizontalHeader()->setStretchLastSection(true);
    m_propertyModel = new PropertyTableModel{table};
    table->setModel(m_propertyModel);
    table->setItemDelegateForColumn(1, new PropertyItemDelegate{table});
    splitter->addWidget(table);
    layout->addWidget(splitter, 1);
    const auto root = gui->mainWindow();
    m_tree->addTopLevelItem(createTreeWidgetItemForObject(root));
    for(const auto child: root->children())
    {
        if(child->isWidgetType())
        {
            addObject(child);
        }
    }
    QCoreApplication::instance()->installEventFilter(this);
    /*
    SubWindow* w
            = SubWindow::putWidgetOnWorkspace(this, true, false, true, false);
    w->adjustSize();
    w->update();
    */
    hide();
    if(parentWidget())
    {
        parentWidget()->hide();
        parentWidget()->adjustSize();
        parentWidget()->update();
    }
}

InspectorView::~InspectorView()
{
    QCoreApplication::instance()->removeEventFilter(this);
}

bool InspectorView::eventFilter(QObject* watched, QEvent* event)
{
    switch(event->type())
    {
        case QEvent::KeyPress:
        {
            const auto keyEvent = static_cast<QKeyEvent*>(event);
            if(m_selector->isVisible() && keyEvent->key() == Qt::Key_Escape)
            {
                m_selector->hide();
                return true;
            }
            break;
        }
        case QEvent::ChildAdded:
        {
            const auto childEvent = static_cast<QChildEvent*>(event);
            const auto object     = childEvent->child();
            if(object->isWidgetType())
            {
                // The object's constructor isn't yet complete, so process it
                // later. This isn't even its final form!
                m_incomingObjects.push_back(object);
                m_updateTrigger.start();
                // If the object's parent is deleted before the object is
                // processed, we won't be informed through other means, so
                // listen for the destroyed signal temporarily.
                connect(object, &QObject::destroyed, this,
                        [this](QObject* object) {
                            m_incomingObjects.erase(
                                    std::remove(m_incomingObjects.begin(),
                                                m_incomingObjects.end(),
                                                object),
                                    m_incomingObjects.end());
                        });
            }
            break;
        }
        case QEvent::ChildRemoved:
        {
            const auto childEvent = static_cast<QChildEvent*>(event);
            const auto object     = childEvent->child();
            if(object->isWidgetType())
            {
                m_incomingObjects.erase(std::remove(m_incomingObjects.begin(),
                                                    m_incomingObjects.end(),
                                                    object),
                                        m_incomingObjects.end());
                removeObject(object);
            }
            break;
        }
    }
    return false;
}

void InspectorView::updateTree()
{
    for(const auto object: m_incomingObjects)
    {
        disconnect(object, &QObject::destroyed, this, nullptr);
        // The object may already have been added to the tree by a parent
        if(object->property(INSPECTOR_PROPERTY).isValid())
        {
            continue;
        }
        // If the parent isn't in the tree yet, let it add the object itself
        if(!object->parent()->property(INSPECTOR_PROPERTY).isValid())
        {
            continue;
        }
        addObject(object);
    }
    m_incomingObjects.clear();

    adjustSize();
    update();
}

void InspectorView::selectionChanged()
{
    const auto items = m_tree->selectedItems();
    if(!items.isEmpty())
    {
        const auto object = static_cast<QObject*>(
                items[0]->data(0, OBJECT_POINTER_ROLE).value<void*>());
        m_propertyModel->setObject(object);
    }
}

void InspectorView::objectSelected(QObject* object)
{
    const auto item = static_cast<QTreeWidgetItem*>(
            object->property(INSPECTOR_PROPERTY).value<void*>());
    if(item)
    {
        m_tree->scrollToItem(item);
        m_tree->setCurrentItem(item);
    }
}

void InspectorView::addObject(QObject* object)
{
    // Add the object and all of its widget children to the tree
    auto stack = std::vector<QObject*>{object};
    while(!stack.empty())
    {
        const auto object = stack.back();
        stack.pop_back();
        const auto& children = object->children();
        std::copy_if(children.begin(), children.end(),
                     std::back_inserter(stack),
                     [](QObject* object) { return object->isWidgetType(); });
        const auto parentItem = static_cast<QTreeWidgetItem*>(
                object->parent()
                        ->property(INSPECTOR_PROPERTY)
                        .value<void*>());
        parentItem->addChild(createTreeWidgetItemForObject(object));
    }
}

void InspectorView::removeObject(QObject* object)
{
    const auto item = static_cast<QTreeWidgetItem*>(
            object->property(INSPECTOR_PROPERTY).value<void*>());
    if(!item)
    {
        return;
    }

    // Clear our tree item property from the object and all of its children
    auto objectStack = std::vector<QObject*>{object};
    while(!objectStack.empty())
    {
        const auto object = objectStack.back();
        objectStack.pop_back();
        const auto& children = object->children();
        std::copy(children.begin(), children.end(),
                  std::back_inserter(objectStack));
        object->setProperty(INSPECTOR_PROPERTY, QVariant{});
    }

    // Make sure we're not inspecting the object or any of its children.
    // This has to be done using the QTreeWidgetItem tree since the object's
    // children may already have been deleted.
    auto treeStack = std::vector<QTreeWidgetItem*>{item};
    while(!treeStack.empty())
    {
        const auto item = treeStack.back();
        treeStack.pop_back();
        for(auto i = 0; i < item->childCount(); ++i)
        {
            treeStack.push_back(item->child(i));
        }
        const auto object = static_cast<QObject*>(
                item->data(0, OBJECT_POINTER_ROLE).value<void*>());
        if(m_propertyModel->object() == object)
        {
            m_propertyModel->setObject(nullptr);
        }
        if(m_selector->target() == object)
        {
            m_selector->highlightObject(nullptr);
        }
    }

    delete item;
}

QTreeWidgetItem* InspectorView::createTreeWidgetItemForObject(QObject* object)
{
    const auto item = new QTreeWidgetItem{};
    item->setText(0, object->metaObject()->className());
    item->setData(0, OBJECT_POINTER_ROLE,
                  QVariant::fromValue(static_cast<void*>(object)));
    object->setProperty(INSPECTOR_PROPERTY,
                        QVariant::fromValue(static_cast<void*>(item)));
    return item;
}
