#include "InputHandler.h"

auto InputHandler::operator()(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) -> void {
    switch (uMsg) {
    case WM_LBUTTONDOWN:
        break;
    case WM_SIZE:
        break;
    case WM_MBUTTONDOWN:
    {
        auto clientSpaceCoordinate = GetClientSpaceCoordinates(hWnd, wParam, lParam, false);
        auto ray = _scene->GetActiveCamera()->GetRayTo(clientSpaceCoordinate);
        if (_scene->Picking(ray)) {
            pickPoint = ray.GetHead();
        } else {
            pickPoint = core::Point4f{ 0, 0, 0, 1 };
        }
        break;
    }
    case WM_MOUSEWHEEL:	// mouse wheel: zoom
    {
        // unlike WM_MBUTTONDOWN, WM_MOUSEWHEEL carries screen-based coordinates in lParam. Need to convert them to client-based coordinates
        auto clientSpaceCoordinate = GetClientSpaceCoordinates(hWnd, wParam, lParam, true);
        auto ray = _scene->GetActiveCamera()->GetRayTo(clientSpaceCoordinate);
        auto step = 0.01f;
        if (_scene->Picking(ray)) {
            step = ray.length * 0.0008f;
        }

        _status = none;
        _scene->GetActiveCamera()->Translate(ray.direction * GET_WHEEL_DELTA_WPARAM(wParam) * step);

        break;
    }
    case WM_MOUSEMOVE:
    {
        auto clientSpaceCoordinate = GetClientSpaceCoordinates(hWnd, wParam, lParam, false);
        auto delta = clientSpaceCoordinate - _lastCoordinate;

        auto * camera = _scene->GetActiveCamera();
        if (wParam == MK_MBUTTON) {	// middle mouse button: rotate
            if (_status == rotate) {
                camera->Rotate(pickPoint, core::Vector4f{ 0, 0, 1, 0 }, -delta(0) * 5);
                camera->Rotate(pickPoint, camera->GetRightDirection(), -delta(1) * 5);
            }
            _lastCoordinate = clientSpaceCoordinate;
            _status = rotate;
        } else if ((wParam & MK_MBUTTON) && (wParam & MK_SHIFT)) {	// middle mouse button & shift key: pan
            if (_status == pan) {
                auto pickPointVector = static_cast<core::Vector4f>(pickPoint - camera->GetPosition());
                auto panMultiplier = DotProduct(pickPointVector, camera->GetForwardDirection()) / camera->GetNearPlane();
                camera->Leftward(delta(0) * camera->GetHalfWidth() * 2 * panMultiplier);
                camera->Upward(delta(1) * camera->GetHalfHeight() * 2 * panMultiplier);
            }
            _lastCoordinate = clientSpaceCoordinate;
            _status = pan;
        } else {
            _status = none;
        }
        break;
    }
    case WM_KEYDOWN:
        switch (wParam) {
        case VK_ESCAPE:
            break;
        case VK_F1:
            _scene->ToggleBackFace();
            break;
        case VK_F2:
            _scene->ToggleWireframe();
            break;
        case 0x57:	// W key
            _scene->GetActiveCamera()->Forward(0.2f);
            break;
        case 0x41:	// A key
            _scene->GetActiveCamera()->Leftward(0.2f);
            break;
        case 0x53:	// S key
            _scene->GetActiveCamera()->Backward(0.2f);
            break;
        case 0x44:	// D key
            _scene->GetActiveCamera()->Rightward(0.2f);
            break;
        case VK_SPACE:
            break;
        case 0x49:	// I key
            break;
        case 0x4B:	// K key
            break;
        default:
            break;
        }
        break;
    }
}

auto InputHandler::GetClientSpaceCoordinates(HWND hWnd, WPARAM wParam, LPARAM lParam, bool isScreenSpaceCoordinate) -> core::Vector2f {
    POINT point;
    point.x = GET_X_LPARAM(lParam);
    point.y = GET_Y_LPARAM(lParam);
    if (isScreenSpaceCoordinate) {
        ScreenToClient(hWnd, &point);
    }
    auto x = static_cast<float>(point.x) * _widthInverse;
    auto y = static_cast<float>(point.y) * _heightInverse;
    return  core::Vector2f{ x, y };
}
