//  Created by Alejandro Isaza on 2014-06-03.
//  Copyright (c) 2014 Venture Media Labs. All rights reserved.

#pragma once
#include "BaseRecursiveHandler.h"
#include "EmptyPlacementHandler.h"
#include "MordentHandler.h"
#include "TurnHandler.h"

#include "dom/Ornaments.h"

namespace mxml {

class OrnamentsHandler : public lxml::BaseRecursiveHandler<dom::Ornaments> {
public:
    void startElement(const lxml::QName& qname, const AttributeMap& attributes);
    RecursiveHandler* startSubElement(const lxml::QName& qname);
    void endSubElement(const lxml::QName& qname, RecursiveHandler* parser);
    
private:
    EmptyPlacementHandler _emptyPlacementHandler;
    MordentHandler _mordentHandler;
    TurnHandler _turnHandler;
};

} // namespace
