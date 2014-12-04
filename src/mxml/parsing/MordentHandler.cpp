//  Created by Alejandro Isaza on 2014-06-23.
//  Copyright (c) 2014 Venture Media Labs. All rights reserved.

#include "MordentHandler.h"
#include "EmptyPlacementHandler.h"

namespace mxml {

static const char* kPlacementAttribute = "placement";
static const char* kLongAttribute = "long";

void MordentHandler::startElement(const lxml::QName& qname, const AttributeMap& attributes) {
    _result = dom::Mordent();

    auto placement = attributes.find(kPlacementAttribute);
    if (placement != attributes.end())
        _result.setPlacement(presentOptional(EmptyPlacementHandler::placementFromString(placement->second)));

    auto longv = attributes.find(kLongAttribute);
    if (longv != attributes.end())
        _result.setLong(longv->second == "yes");
}


} // namespace