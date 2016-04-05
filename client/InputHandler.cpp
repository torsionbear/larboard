#include "InputHandler.h"

auto InputHandler::operator()(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) -> void {
    switch (uMsg) {
    case WM_LBUTTONDOWN:
        break;
    case WM_SIZE:
        break;
    case WM_MBUTTONDOWN:
    {
        if (_isFpsMode) {
        } else {
            auto coordinate = GetClientSpaceCoordinates(hWnd, wParam, lParam, false);
            auto ray = _scene->GetActiveCamera()->GetRayTo(core::Vector2f{ static_cast<float>(coordinate.x) * _widthInverse, static_cast<float>(coordinate.y) * _heightInverse });
            if (_scene->Picking(ray)) {
                pickPoint = ray.GetHead();
            } else {
                pickPoint = core::Point4f{ 0, 0, 0, 1 };
            }
        }
        break;
    }
    case WM_MOUSEWHEEL:	// mouse wheel: zoom
    {
        if (_isFpsMode) {
        } else {
            // unlike WM_MBUTTONDOWN, WM_MOUSEWHEEL carries screen-based coordinates in lParam. Need to convert them to client-based coordinates
            auto coordinate = GetClientSpaceCoordinates(hWnd, wParam, lParam, true);
            auto ray = _scene->GetActiveCamera()->GetRayTo(core::Vector2f{ static_cast<float>(coordinate.x) * _widthInverse, static_cast<float>(coordinate.y) * _heightInverse });
            auto step = 0.01f;
            if (_scene->Picking(ray)) {
                step = ray.length * 0.0008f;
            }

            _status = none;
            _scene->GetActiveCamera()->Translate(ray.direction * GET_WHEEL_DELTA_WPARAM(wParam) * step);
        }
        break;
    }
    case WM_MOUSEMOVE:
    {
        auto * camera = _scene->GetActiveCamera();
        auto coordinate = GetClientSpaceCoordinates(hWnd, wParam, lParam, false);
        if (_isFpsMode) {
            auto deltaX = coordinate.x - _center.x;
            auto deltaY = coordinate.y - _center.y;
            camera->Rotate(camera->GetPosition(), core::Vector4f{ 0, 0, 1, 0 }, -deltaX * _widthInverse * 2 );
            camera->Rotate(camera->GetPosition(), camera->GetRightDirection(), -deltaY * _heightInverse * 2 );
            // lock curser to window's center
            if (deltaX != 0 || deltaY != 0) {
                auto center = _center;
                ClientToScreen(hWnd, &center);
                SetCursorPos(center.x, center.y);
            }
            break;
        } else {
            auto clientSpaceCoordinate = core::Vector2f{ static_cast<float>(coordinate.x) * _widthInverse, static_cast<float>(coordinate.y) * _heightInverse };
            auto delta = clientSpaceCoordinate - _lastCoordinate;
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
        }
        break;
    }
    case WM_KEYDOWN:
    {
        // Skip repeated message. Bit 30 indicates if the key is pressed before. See https://msdn.microsoft.com/en-us/library/windows/desktop/ms646280(v=vs.85).aspx
        if (lParam & 0x40000000) {
            break;
        }
        switch (wParam) {
        case VK_ESCAPE:
            break;
        case VK_F1:
            _renderer->ToggleBackFace();
            break;
        case VK_F2:
            _renderer->ToggleWireframe();
            break;
        case VK_F3:
            _scene->ToggleBvh();
            break;
        case VK_TAB:
            _isFpsMode = !_isFpsMode;
            if (_isFpsMode) {
                _cameraController->Enable();
                auto center = _center;
                ClientToScreen(hWnd, &center);
                SetCursorPos(center.x, center.y);
            } else {
                _cameraController->Disable();
            }
            break;
        case 0x57:	// W key
        {
            if (_isFpsMode) {
                _cameraController->IncreaseForwardSpeed(_moveSpeed);
            }
            break;
        }
        case 0x41:	// A key
        {
            if (_isFpsMode) {
                _cameraController->IncreaseRightSpeed(-_moveSpeed);
            }
            break;
        }
        case 0x53:	// S key
        {
            if (_isFpsMode) {
                _cameraController->IncreaseForwardSpeed(-_moveSpeed);
            }
            break;
        }
        case 0x44:	// D key
        {
            if (_isFpsMode) {
                _cameraController->IncreaseRightSpeed(_moveSpeed);
            }
            break;
        }
        case VK_SPACE:
            break;
        case 0x46:  // F key
        {
            if (_scene->_directionalLights.empty()) {
                break;
            }
            auto sunlight = _scene->_directionalLights.front().get();
            auto sunSpeed = 0.1f;
            _sunlightPhase += sunSpeed;
            sunlight->Rotate(core::Vector4f{ 0.0f, 1.0f, 0.0f, 0.0f }, sunSpeed);
            auto sunlightIntensityMultiplier = cos(_sunlightPhase);
            sunlightIntensityMultiplier = sunlightIntensityMultiplier > 0.2 ? sunlightIntensityMultiplier : 0;
            sunlight->SetColor(core::Vector3f{ 0.8f, 0.8f, 0.8f} * sunlightIntensityMultiplier);
            _scene->_ambientLights.front()->SetColor(core::Vector4f{ 0.4f, 0.4f, 0.4f, 0 } * sunlightIntensityMultiplier);
            break;
        }
        case 0x49:	// I key
            break;
        case 0x4B:	// K key
            break;
        default:
            break;
        }
        break;
    }
    case WM_KEYUP:
        switch (wParam) {
        case 0x57:	// W key
        {
            if (_isFpsMode) {
                _cameraController->IncreaseForwardSpeed(-_moveSpeed);
            }
            break;
        }
        case 0x41:	// A key
        {
            if (_isFpsMode) {
                _cameraController->IncreaseRightSpeed(_moveSpeed);
            }
            break;
        }
        case 0x53:	// S key
        {
            if (_isFpsMode) {
                _cameraController->IncreaseForwardSpeed(_moveSpeed);
            }
            break;
        }
        case 0x44:	// D key
        {
            if (_isFpsMode) {
                _cameraController->IncreaseRightSpeed(-_moveSpeed);
            }
            break;
        }
        }
        break;
    }
}

auto InputHandler::GetClientSpaceCoordinates(HWND hWnd, WPARAM wParam, LPARAM lParam, bool isScreenSpaceCoordinate) -> POINT {
    POINT point;
    point.x = GET_X_LPARAM(lParam);
    point.y = GET_Y_LPARAM(lParam);
    if (isScreenSpaceCoordinate) {
        ScreenToClient(hWnd, &point);
    }
    return point;
}
