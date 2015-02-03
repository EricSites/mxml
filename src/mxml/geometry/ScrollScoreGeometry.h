//  Created by Alejandro Isaza on 2014-06-26.
//  Copyright (c) 2014 Venture Media Labs. All rights reserved.

#pragma once
#include "Geometry.h"
#include "PartGeometry.h"

#include <mxml/ScoreProperties.h>
#include <mxml/ScrollMetrics.h>
#include <mxml/SpanCollection.h>
#include <mxml/dom/Score.h>

#include <memory>
#include <vector>


namespace mxml {

class ScrollScoreGeometry : public Geometry {
public:
    ScrollScoreGeometry(const dom::Score& score, bool naturalSpacing = true);

    const dom::Score& score() const {
        return _score;
    }
    const ScoreProperties& scoreProperties() const {
        return _scoreProperties;
    }
    const SpanCollection& spans() const {
        return *_spans;
    }
    const std::vector<PartGeometry*>& partGeometries() const {
        return _partGeometries;
    }
    const ScrollMetrics& metrics(std::size_t partIndex) const {
        return *_metrics[partIndex];
    }

private:
    const dom::Score& _score;

    ScoreProperties _scoreProperties;
    std::unique_ptr<SpanCollection> _spans;
    std::vector<PartGeometry*> _partGeometries;
    std::vector<std::unique_ptr<ScrollMetrics>> _metrics;
};

} // namespace mxml
