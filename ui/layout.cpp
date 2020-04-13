
#include "layout.h"
#include "container.h"

namespace ui {
    namespace {

        class NoLayout : public Layout {
        protected:
            void relayout(Container * widget) override {
                recalculateOverlay(widget);
            }
        };

    }; // anonymous namespace

    Layout * Layout::None = new NoLayout();

    void Layout::recalculateOverlay(Container * widget) {
        std::vector<Widget*> const & children = containerChildren(widget);
        if (widget->isOverlaid()) {
            for (Widget * child : children) 
                child->setOverlay(true);
        } else {
            Rect rect{Rect::Empty()};
            for (auto i = children.rbegin(), e = children.rend(); i != e; ++i) {
                setChildOverlay(*i, ! (rect & (*i)->rect()).empty());
                rect = rect | (*i)->rect();
            }
        }
    }

    std::vector<Widget*> const & Layout::containerChildren(Container * container) {
        Container * c = dynamic_cast<Container*>(container); 
        ASSERT(c != nullptr);
        return c->children_;
    }

}