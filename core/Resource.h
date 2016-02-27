#pragma once

#include <string>

namespace core {

class Resource {
public:
    enum class status {
        Unloaded,
        Loaded,
        SentToCard
    };

protected:
    Resource()
        : _status(status::Unloaded) {
    }

public:
    auto Status() -> status {
        return _status;
    }
    auto Load() -> void {
        if (status::Unloaded != _status) {
            return;
        }
        if (LoadImpl()) {
            _status = status::Loaded;
        }
    }
    auto Unload() -> void {
        switch (_status) {
        case status::Unloaded:
            return;
        case status::Loaded:
            if (UnloadImpl()) {
                _status = status::Unloaded;
            }
            break;
        case status::SentToCard:
            if (!FreeFromCardImpl()) {
                return;
            }
            _status = status::Loaded;
            if (UnloadImpl()) {
                _status = status::Unloaded;
            }
            break;
        }
    }
    auto SendToCard() -> void {
        switch (_status) {
        case status::Unloaded:
            if (!LoadImpl()) {
                return;
            }
            _status = status::Loaded;
            if (SendToCardImpl()) {
                _status = status::SentToCard;
            }
            break;
        case status::Loaded:
            if (SendToCardImpl()) {
                _status = status::SentToCard;
            }
            break;
        case status::SentToCard:
            return;
        }
    }
    auto FreeFromCard() -> void {
        if (status::SentToCard != _status) {
            return;
        }
        if (FreeFromCardImpl()) {
            _status = status::Loaded;
        }

    }

private:
    virtual auto LoadImpl() -> bool = 0;
    virtual auto UnloadImpl() -> bool = 0;
    virtual auto SendToCardImpl() -> bool = 0;
    virtual auto FreeFromCardImpl() -> bool = 0;

protected:
    status _status;
};

}