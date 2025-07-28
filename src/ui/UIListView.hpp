/* Copyright (c) 2024 No Tomorrow Games. All rights reserved. */

#ifndef HYPERION_UI_LIST_VIEW_HPP
#define HYPERION_UI_LIST_VIEW_HPP

#include <ui/UIPanel.hpp>

#include <core/containers/Array.hpp>

#include <core/memory/UniquePtr.hpp>

namespace hyperion {

class UIDataSourceElement;

HYP_ENUM()
enum class UIListViewOrientation : uint8
{
    VERTICAL = 0,
    HORIZONTAL
};

#pragma region UIListViewItem

HYP_CLASS()
class HYP_API UIListViewItem : public UIObject
{
    HYP_OBJECT_BODY(UIListViewItem);

public:
    friend class UIListView;

    UIListViewItem();
    UIListViewItem(const UIListViewItem& other) = delete;
    UIListViewItem& operator=(const UIListViewItem& other) = delete;
    UIListViewItem(UIListViewItem&& other) noexcept = delete;
    UIListViewItem& operator=(UIListViewItem&& other) noexcept = delete;
    virtual ~UIListViewItem() override = default;

    virtual void AddChildUIObject(const Handle<UIObject>& uiObject) override;
    virtual bool RemoveChildUIObject(UIObject* uiObject) override;

    bool HasSubItems() const;

    HYP_FORCE_INLINE bool IsExpanded() const
    {
        return m_isExpanded;
    }

    void SetIsExpanded(bool isExpanded);

protected:
    virtual void Init() override;

    virtual void SetIsSelectedItem(bool isSelectedItem);

    virtual void SetFocusState_Internal(EnumFlags<UIObjectFocusState> focusState) override;

    virtual Material::ParameterTable GetMaterialParameters() const override;

private:
    UIObject* m_expandedElement;
    bool m_isSelectedItem;
    bool m_isExpanded;
    Color m_initialBackgroundColor;
};

#pragma endregion UIListViewItem

#pragma region UIListView

HYP_CLASS()
class HYP_API UIListView : public UIPanel
{
    HYP_OBJECT_BODY(UIListView);

public:
    friend class UIListViewItem;

    UIListView();
    UIListView(const UIListView& other) = delete;
    UIListView& operator=(const UIListView& other) = delete;
    UIListView(UIListView&& other) noexcept = delete;
    UIListView& operator=(UIListView&& other) noexcept = delete;
    virtual ~UIListView() override;

    HYP_FORCE_INLINE const WeakHandle<UIListViewItem>& GetSelectedItem() const
    {
        return m_selectedItem;
    }

    HYP_METHOD()
    void SetSelectedItem(UIListViewItem* listViewItem);

    HYP_METHOD()
    HYP_FORCE_INLINE int GetSelectedItemIndex() const
    {
        if (!m_selectedItem.IsValid())
        {
            return -1;
        }

        auto it = m_listViewItems.FindAs(m_selectedItem.GetUnsafe());

        if (it != m_listViewItems.End())
        {
            return int(it - m_listViewItems.Begin());
        }

        return -1;
    }

    HYP_METHOD()
    void SetSelectedItemIndex(int index);

    HYP_FORCE_INLINE const Array<UIListViewItem*>& GetListViewItems() const
    {
        return m_listViewItems;
    }

    HYP_METHOD(Property = "Orientation")
    HYP_FORCE_INLINE UIListViewOrientation GetOrientation() const
    {
        return m_orientation;
    }

    HYP_METHOD(Property = "Orientation")
    void SetOrientation(UIListViewOrientation orientation);

    virtual void AddChildUIObject(const Handle<UIObject>& uiObject) override;
    virtual bool RemoveChildUIObject(UIObject* uiObject) override;

    UIListViewItem* FindListViewItem(const UUID& dataSourceElementUuid) const;

    Delegate<void, UIListViewItem*> OnSelectedItemChange;

protected:
    virtual void Init() override;

    virtual void UpdateSize_Internal(bool updateChildren) override;

    virtual void SetDataSource_Internal(UIDataSourceBase* dataSource) override;

    void UpdateLayout();

private:
    static UIListViewItem* FindListViewItem(const UIObject* parentObject, const UUID& dataSourceElementUuid);

    void AddDataSourceElement(UIDataSourceBase* dataSource, UIDataSourceElement* element, UIDataSourceElement* parent);

    Array<UIListViewItem*> m_listViewItems;
    WeakHandle<UIListViewItem> m_selectedItem;

    UIListViewOrientation m_orientation;
};

#pragma endregion UIListView

} // namespace hyperion

#endif