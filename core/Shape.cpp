#include "Shape.h"

#include <memory>

using std::make_unique;

namespace core {

auto Shape::AddTexture(Texture * texture) -> void {
	_textures.push_back(texture);
}

auto Shape::GetAabb() -> Aabb const& {
    // todo: need to update Aabb when shape's model is transformed
    if (_aabb == nullptr) {
        _aabb = make_unique<Aabb>();
        for (auto const& vertex : _mesh->GetVertex()) {
            auto transformedVertex = _model->GetTransform() * Point4f { vertex.coord(0), vertex.coord(1), vertex.coord(2), 1.0f };
            _aabb->Expand(transformedVertex);
        }
    }
    return *_aabb;
}

}