#pragma once

#include "widget.h"
#include "layout.h"

namespace ui {

    /** Container manages its child widgets dynamically. 
     
        A container is a basic widget that manages its children dynamically via a list. Child widgets can be added to, or removed from the container at runtime. The container furthermore provides support for automatic layouting of the children and makes sure that the UI events are propagated to them correctly. 

     */
    class Container : public Widget {
    public:

        /** Container destructor also destroys its children. 
         */
        ~Container() override {
            while (!children_.empty()) {
                Widget * w = children_.front();
                remove(w);
                delete w;
            }
            if (layout_ != Layout::None)
                delete layout_;
        }

    protected:

        template<template<typename> typename X, typename T>
        friend class TraitBase;

        Container(Layout * layout = Layout::None):
            layout_{layout},
            layoutScheduled_{false} {
        }


        /** Adds the given widget as child. 
         
            The widget will appear as the topmost widget in the container. If the widget already exists in the container, it will be moved to the topmost position. 
         */
        virtual void add(Widget * widget) {
            for (auto i = children_.begin(), e = children_.end(); i != e; ++i)
                if (*i == widget) {
                    children_.erase(i);
                    break;
                }
            // attach the child widget
            children_.push_back(widget);
            if (widget->parent() != this)
                widget->attachTo(this);
            // relayout the children widgets
            relayout();
        }

        /** Remove the widget from the container. 
         */
        virtual void remove(Widget * widget) {
            for (auto i = children_.begin(), e = children_.end(); i != e; ++i)
                if (*i == widget) {
                    (*i)->detachFrom(this);
                    children_.erase(i);
                    // relayout the children widgets after the child has been removed
                    relayout();
                    return;
                }
            UNREACHABLE;
        }

        /** \name Geometry and Layout
         
         */
        //@{

        /** Returns the layout used by the container. 
         */
        Layout * layout() const {
            return layout_;
        }

        /** Sets the layout for the container. 
         */
        virtual void setLayout(Layout * value) {
            ASSERT(value != nullptr);
            ASSERT(value->container_ == nullptr);
            if (layout_ != value) {
                if (layout_ != Layout::None)
                    delete layout_;
                layout_ = value;
                if (layout_ != Layout::None)
                    layout_->container_ = this;
                relayout();
            }
        }

        /** Relayout the children widgets when the container has been resized. 
         */
        void resized() override {
            relayout();
            Widget::resized();
        }

        /** Change in child's rectangle triggers relayout of the container. 
         */
        void childChanged(Widget * child) override {
            MARK_AS_UNUSED(child);
            relayout();
        }

        void relayout() {
            UI_THREAD_CHECK;
            if (layoutScheduled_ != true) {
                layoutScheduled_ = true;
                repaint();
            }
        }
        //@}

        /** \name Mouse Actions
         */
        //@{

        /** Returns the mouse target. 
         
            Scans the children widgets in their visibility order and if the mouse coordinates are found in the widget, propagates the mouse target request to it. If the mouse is not over any child, returns itself.
         */
        Widget * getMouseTarget(Point coords) override {
            for (auto i = children_.rbegin(), e = children_.rend(); i != e; ++i)
                if ((*i)->visible() && (*i)->rect().contains(coords))
                    return (*i)->getMouseTarget(coords - (*i)->rect().topLeft());
            return this;
        }

        //@}

        /** \name Keyboard Input
         */

        //@{
        Widget * getNextFocusableWidget(Widget * current) override {
            if (enabled()) {
                if (current == nullptr && focusable())
                    return this;
                ASSERT(current == this || current == nullptr || current->parent() == this);
                if (current == this)
                    current = nullptr;
                // see if there is a next child that can be focused
                Widget * next = getNextFocusableChild(current);
                if (next != nullptr)
                    return next;
            }
            // if not the container, nor any of its children can be the next widget, try the parent
            return (parent_ == nullptr) ? nullptr : parent_->getNextFocusableWidget(this);
        }

        /** Returns next focusable child. 
         */
        Widget * getNextFocusableChild(Widget * current) {
            // determine the next element to focus, starting from the current element which we do by setting current to nullptr if we see it and returning the child if current is nullptr
            for (Widget * child : children_) {
                if (current == nullptr) {
                    Widget * result = child->getNextFocusableWidget(nullptr);
                    if (result != nullptr)
                        return result;
                } else if (current == child) {
                    current = nullptr;
                }
            }
            return nullptr;
        }
        //@}

        /** Attaches the container to specified renderer. 

            First the container is attached and then all its children are attached as well. 
         */
        void attachRenderer(Renderer * renderer) override {
            UI_THREAD_CHECK;
            Widget::attachRenderer(renderer);
            for (Widget * child : children_)
                child->attachRenderer(renderer);
        }

        /** Detaches the container from its renderer. 
         
            First detaches all children and then detaches itself. 
         */
        void detachRenderer() override {
            UI_THREAD_CHECK;
            for (Widget * child : children_)
                child->detachRenderer();
            Widget::detachRenderer();
        }

        /** Paints the container. 
         
            The default implementation simply obtains the contents canvas and then paints all visible children. 
         */
        void paint(Canvas & canvas) override {
            UI_THREAD_CHECK;
            Canvas contentsCanvas{getContentsCanvas(canvas)};
            // relayout the widgets if layout was requested
            if (layoutScheduled_) {
                if (! children_.empty()) {
                    layout_->relayout(this, contentsCanvas);
                    // do a check of own size and change according to the calculated layout, this might trigger repaint in parent which is ok
                    autoSize();
                }
                layoutScheduled_ = false;    
            }
            Canvas childrenCanvas{contentsCanvas};
            for (Widget * child : children_)
                paintChild(child, childrenCanvas);
        }

        /** Calculates the autosize of the container so that all its children fit in it without scrolling. 
         */
        std::pair<int, int> calculateAutoSize() override {
            int w = width();
            int h = height();
            for (Widget * child : children_) {
                if (w < child->rect_.right())
                    w = child->rect_.right();
                if (h < child->rect_.bottom())
                    h = child->rect_.bottom();
            }
            return std::make_pair(w, h);
        }

        std::vector<Widget *> children_;

    private:

        friend class Layout;

        Layout * layout_;
        bool layoutScheduled_;

    }; // ui::Container


    class PublicContainer : public Container {
    public:
        PublicContainer(Layout * layout = Layout::None):
            Container{layout} {
        }

        using Container::layout;
        using Container::setLayout;
        using Container::add;
        using Container::remove;
        using Container::setWidthHint;
        using Container::setHeightHint;

        std::vector<Widget *> const & children() const {
            return children_;
        }
    }; // ui::PublicContainer


} // namespace ui