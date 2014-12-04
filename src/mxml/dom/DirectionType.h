//  Created by Alejandro Isaza on 2014-03-18.
//  Copyright (c) 2014 Venture Media Labs. All rights reserved.

#pragma once
#include "Node.h"
#include "Optional.h"

#include <string>

namespace mxml {
namespace dom {

class DirectionType : public Node {
public:
    DirectionType() : _defaultX(), _defaultY() {}
    virtual ~DirectionType() = default;
    
    const Optional<float>& defaultX() const {
        return _defaultX;
    }
    void setDefaultX(const Optional<float>& defaultX) {
        _defaultX = defaultX;
    }
    
    const Optional<float>& defaultY() const {
        return _defaultY;
    }
    void setDefaultY(const Optional<float>& defaultY) {
        _defaultY = defaultY;
    }
    
    /** Return true if this direction type has start and stop elements. */
    virtual bool span() const = 0;
    
private:
    Optional<float> _defaultX;
    Optional<float> _defaultY;
    bool _span;
};

class Dynamics : public DirectionType {
public:
    const std::string& string() const {
        return _string;
    }
    void setString(const std::string& string) {
        _string = string;
    }
    
    bool span() const {
        return false;
    }
    
private:
    std::string _string;
};

class Words : public DirectionType {
public:
    const std::string& contents() const {
        return _contents;
    }
    void setContents(const std::string& contents) {
        _contents = contents;
    }
    
    bool span() const {
        return false;
    }
    
private:
    std::string _contents;
};

} // namespace dom
} // namespace mxml