//  Created by Alejandro Isaza on 2014-05-06.
//  Copyright (c) 2014 Venture Media Labs. All rights reserved.

#include "Geometry.h"
#include <cassert>
#include <deque>

namespace mxml {

Geometry::Geometry()
: _contentOffset(0, 0),
  _size(),
  _location(),
  _horizontalAnchorPointMultiplier(0.5),
  _horizontalAnchorPointConstant(0),
  _verticalAnchorPointMultiplier(0.5),
  _verticalAnchorPointConstant(0),
  _parentGeometry(),
  _geometries()
{}

void Geometry::addGeometry(std::unique_ptr<Geometry>&& geom) {
    assert(geom);
    geom->_parentGeometry = this;
    _geometries.push_back(std::move(geom));
}

Rect Geometry::subGeometriesFrame() const {
    bool first = true;
    Rect frame;
    for (auto& geom : _geometries) {
        if (first)
            frame = geom->frame();
        else
            frame = join(frame, geom->frame());
        first = false;
    }
    return frame;
}

Geometry* Geometry::collidingGeometry(const Rect& frame) const {
    for (auto& geom : _geometries) {
        if (intersect(geom->frame(), frame))
            return geom.get();
    }
    return nullptr;
}

std::vector<Geometry*> Geometry::collidingGeometries(const Rect& frame) const {
    std::vector<Geometry*> collisions;
    for (auto& geom : _geometries) {
        if (intersect(geom->frame(), frame))
            collisions.push_back(geom.get());
    }
    return collisions;
}

const Geometry* Geometry::rootGeometry() const {
    const Geometry* root = this;
    while (root->parentGeometry())
        root = root->parentGeometry();
    return root;
}

Point Geometry::convertToParent(const Point& point) const {
    Point o = origin();
    return {o.x + point.x - _contentOffset.x, o.y + point.y - _contentOffset.y};
}

Point Geometry::convertFromParent(const Point& point) const {
    Point o = origin();
    return {point.x + _contentOffset.x - o.x, point.y + _contentOffset.y - o.y};
}

Point Geometry::convertToRoot(Point point) const {
    const Geometry* geom = this;
    while (geom->parentGeometry()) {
        point = geom->convertToParent(point);
        geom = geom->parentGeometry();
    }
    return point;
}

Point Geometry::convertFromRoot(Point point) const {
    // Build path to root
    std::deque<const Geometry*> stack;
    const Geometry* geom = this;
    while (geom->parentGeometry()) {
        stack.push_front(geom);
        geom = geom->parentGeometry();
    }

    // Transform from root geometry
    for (const Geometry* geom : stack) {
        point = geom->convertFromParent(point);
    }

    return point;
}

Point Geometry::convertToGeometry(Point point, const Geometry* target) const {
    if (!target)
        return convertToRoot(point);

    assert(rootGeometry() == target->rootGeometry());
    return target->convertFromRoot(convertToRoot(point));
}

Point Geometry::convertFromGeometry(Point point, const Geometry* target) const {
    if (!target)
        return convertFromRoot(point);

    assert(rootGeometry() == target->rootGeometry());
    return convertFromRoot(target->convertToRoot(point));
}

Rect Geometry::convertToParent(const Rect& rect) const {
    return {convertToParent(rect.origin), rect.size};
}

Rect Geometry::convertFromParent(const Rect& rect) const {
    return {convertFromParent(rect.origin), rect.size};
}

Rect Geometry::convertToRoot(Rect rect) const {
    const Geometry* geom = this;
    while (geom->parentGeometry()) {
        rect = geom->convertToParent(rect);
        geom = geom->parentGeometry();
    }
    return rect;
}

Rect Geometry::convertFromRoot(Rect rect) const {
    // Build path to root
    std::deque<const Geometry*> stack;
    const Geometry* geom = this;
    while (geom->parentGeometry()) {
        stack.push_front(geom);
        geom = geom->parentGeometry();
    }

    // Transform from root geometry
    for (const Geometry* geom : stack) {
        rect = geom->convertFromParent(rect);
    }

    return rect;
}

Rect Geometry::convertToGeometry(Rect rect, const Geometry* target) const {
    if (!target)
        return convertToRoot(rect);

    assert(rootGeometry() == target->rootGeometry());
    return target->convertFromRoot(convertToRoot(rect));
}

Rect Geometry::convertFromGeometry(Rect rect, const Geometry* target) const {
    if (!target)
        return convertFromRoot(rect);

    assert(rootGeometry() == target->rootGeometry());
    return convertFromRoot(target->convertToRoot(rect));
}

} // namespace mxml
