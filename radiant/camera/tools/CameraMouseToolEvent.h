#pragma once

#include "icameraview.h"
#include "imousetoolevent.h"

namespace ui
{

// Special type for events generated by the camera view.
class CameraMouseToolEvent :
    public MouseToolEvent
{
private:
    ICameraView& _view;

public:
    CameraMouseToolEvent(ICameraView& view, const Vector2& devicePos) :
        MouseToolEvent(view, devicePos),
        _view(view)
    {}

    CameraMouseToolEvent(ICameraView& view, const Vector2& devicePos, const Vector2& delta) :
        MouseToolEvent(view, devicePos, delta),
        _view(view)
    {}

    ICameraView& getView()
    {
        return _view;
    }
};

}
