//  Created by Alejandro Isaza on 2014-04-30.
//  Copyright (c) 2014 Venture Media Labs. All rights reserved.

#pragma once
#include "SpanCollection.h"

#include <mxml/ScoreProperties.h>
#include <mxml/dom/Barline.h>
#include <mxml/dom/Chord.h>
#include <mxml/dom/Direction.h>
#include <mxml/dom/Measure.h>
#include <mxml/dom/Score.h>

#include <memory>


namespace mxml {

/**
 SpanFactory builds a SpanCollection from a Score. This is the first layout pass.
 */
class SpanFactory {
public:
    SpanFactory(const dom::Score& score, const ScoreProperties& scoreProperties);

    /**
     With natural spacing enabled notes with longer durations have proportinaly more space to their right.
     */
    bool naturalSpacing() const {
        return _naturalSpacing;
    }
    void setNaturalSpacing(bool value) {
        _naturalSpacing = value;
    }

    /**
     If this is enabled there will be space for the clef and the key signature on the first measure of every system.
     */
    bool addCleffAndKeyToEverySystem() const {
        return _addClefAndKeyToEverySystem;
    }
    void setAddClefAndKeyToEverySystem(bool value) {
        _addClefAndKeyToEverySystem = value;
    }

    /**
     Build the span collection for the whole score, used in a scroll layout.
     */
    std::unique_ptr<SpanCollection> build();
    
private:
    void build(const dom::Part* part, std::size_t beginMeasureIndex, std::size_t endMeasureIndex);
    void build(const dom::Measure* measure, bool buildMeasureAttributes);
    void build(const dom::Measure* measure);
    void build(const dom::Attributes* attributes);
    void build(const dom::Barline* barline);
    void build(const dom::Direction* direction);
    void build(const dom::TimedNode* node);
    void build(const dom::Chord* chord);
    void build(const dom::Note* note);
    void build(const dom::Clef* clefNode, int staff, int time);
    void build(const dom::Time* timeNode, int staff, int time);
    void build(const dom::Key* keyNode, int staff, int time);

    coord_t naturalWidthForNote(const dom::Note& note);
    SpanCollection::iterator graceNoteSpan(const dom::Chord* chord);

    void removeRedundantSpans();
    static bool isAttributeOnlySpan(const Span& span);

private:
    const dom::Score& _score;
    const ScoreProperties& _scoreProperties;
    bool _naturalSpacing;
    bool _addClefAndKeyToEverySystem;

    std::size_t _partIndex;
    std::size_t _measureIndex;
    int _currentTime;

    std::unique_ptr<SpanCollection> _spans;
};

} // namespace mxml
