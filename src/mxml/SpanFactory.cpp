//  Created by Alejandro Isaza on 2014-04-30.
//  Copyright (c) 2014 Venture Media Labs. All rights reserved.

#include "SpanFactory.h"

#include <mxml/geometry/AccidentalGeometry.h>
#include <mxml/geometry/BarlineGeometry.h>
#include <mxml/geometry/ClefGeometry.h>
#include <mxml/geometry/KeyGeometry.h>
#include <mxml/geometry/NoteGeometry.h>
#include <mxml/geometry/StemGeometry.h>
#include <mxml/geometry/TimeSignatureGeometry.h>

#include <mxml/dom/Backup.h>
#include <mxml/dom/Forward.h>


namespace mxml {

static const coord_t kMeasureLeftPadding = 4;
static const coord_t kMeasureRightPadding = 4;

static const coord_t kAttributeMargin = 10;
static const coord_t kRestWidth = 12;
static const coord_t kNoteMargin = 20;

SpanFactory::SpanFactory(const dom::Score& score, bool naturalSpacing) : _score(score), _currentTime(0), _spans() {
    _spans.setNaturalSpacing(naturalSpacing);
}
    
const SpanCollection& SpanFactory::build() {
    for (auto& part : _score.parts()) {
        build(part.get());
    }
    removeRedundantSpans();

    _spans.fillStarts();
    _spans.generateNodesMap();
    return _spans;
}

void SpanFactory::build(const dom::Part* part) {
    for (_measureIndex = 0; _measureIndex < part->measures().size(); _measureIndex += 1) {
        _currentTime = 0;
        build(part->measures().at(_measureIndex).get());
    }
}

void SpanFactory::build(const dom::Measure* measure) {
    for (auto& node : measure->nodes()) {
        if (const dom::Barline* barline = dynamic_cast<const dom::Barline*>(node.get())) {
            build(barline);
        } else if (const dom::Attributes* attributes = dynamic_cast<const dom::Attributes*>(node.get())) {
            build(attributes);
        } else if (const dom::Direction* direction = dynamic_cast<const dom::Direction*>(node.get())) {
            build(direction);
        } else if (const dom::TimedNode* timedNode = dynamic_cast<const dom::TimedNode*>(node.get())) {
            build(timedNode);
        }
    }

    auto measureRange = _spans.range(_measureIndex);
    if (measureRange.first == measureRange.second) {
        // Set measure padding for an empty measure
        auto span = _spans.add(_measureIndex, 0);
        span->pushLeftMargin(kMeasureLeftPadding);
        span->pushRightMargin(kMeasureRightPadding);
    } else {
        // Set measure padding unless the edge spans specifically want 0 margin (i.e. barlines)
        auto first = measureRange.first;
        if (first->leftMargin() > 0)
            first->pushLeftMargin(kMeasureLeftPadding);

        auto last = measureRange.second;
        --last;
        if (last->rightMargin() > 0)
            last->pushRightMargin(kMeasureRightPadding);
    }
}

void SpanFactory::build(const dom::Barline* barline) {
    dom::time_t time = _currentTime;
    const dom::Measure* measure = dynamic_cast<const dom::Measure*>(barline->parent());
    if (barline == measure->nodes().back().get())
        time = std::numeric_limits<int>::max();

    SpanCollection::iterator span = _spans.withType<dom::Barline>(_measureIndex, time);
    if (span == _spans.end()) {
        span = _spans.add(_measureIndex, time);
        span->setEvent(false);
    }
    span->pushWidth(BarlineGeometry::Width(*barline));
    
    if (span->time() == 0) {
        span->setRightMargin(kMeasureLeftPadding);
    } else if (span->time() == std::numeric_limits<int>::max()) {
        span->setLeftMargin(kMeasureRightPadding);
        span->setRightMargin(-1);
    }
    
    span->addNode(barline);
}

void SpanFactory::build(const dom::Attributes* attributes) {
    dom::time_t time = _currentTime;
    const dom::Measure* measure = dynamic_cast<const dom::Measure*>(attributes->parent());
    if (attributes == measure->nodes().back().get())
        time = std::numeric_limits<int>::max();

    SpanCollection::iterator clefSpan = _spans.withType<dom::Clef>(_measureIndex, time);
    for (int staff = 1; staff <= attributes->staves(); staff += 1) {
        if (!attributes->clef(staff).isPresent())
            continue;
        const dom::Clef& clef = attributes->clef(staff);
        
        if (clefSpan == _spans.end())
            clefSpan = _spans.add(_measureIndex, time);
        clefSpan->setEvent(false);
        clefSpan->pushLeftMargin(kAttributeMargin);
        clefSpan->pushRightMargin(kAttributeMargin);
        clefSpan->pushWidth(ClefGeometry::kSize.width);
        clefSpan->addNode(&clef);
    }
    
    SpanCollection::iterator timeSpan = _spans.withType<dom::Time>(_measureIndex, time);
    if (attributes->time().isPresent()) {
        const dom::Time& timeNode = attributes->time();
        
        if (timeSpan == _spans.end())
            timeSpan = _spans.add(_measureIndex, time);
        timeSpan->setEvent(false);
        timeSpan->pushLeftMargin(kAttributeMargin);
        timeSpan->pushRightMargin(kAttributeMargin);
        timeSpan->pushWidth(TimeSignatureGeometry(timeNode).size().width);
        timeSpan->addNode(&timeNode);
    }
    
    SpanCollection::iterator keySpan = _spans.withType<dom::Key>(_measureIndex, time);
    for (int staff = 1; staff <= attributes->staves(); staff += 1) {
        if (!attributes->key(staff).isPresent())
            continue;
        const dom::Key& key = attributes->key(staff);
        coord_t width = KeyGeometry::keySize(key).width;
        if (width == 0)
            continue;
        
        if (keySpan == _spans.end())
            keySpan = _spans.add(_measureIndex, time);
        keySpan->setEvent(false);
        keySpan->pushLeftMargin(kAttributeMargin);
        keySpan->pushRightMargin(kAttributeMargin);
        keySpan->pushWidth(KeyGeometry::keySize(key).width);
        keySpan->addNode(&key);
    }
}

void SpanFactory::build(const dom::Direction* direction) {
    _currentTime = direction->start();

    SpanCollection::iterator span = _spans.eventSpan(_measureIndex, _currentTime);
    if (span == _spans.end()) {
        span = _spans.add(_measureIndex, _currentTime);
    }
    
    span->addNode(direction);
}

void SpanFactory::build(const dom::TimedNode* node) {
    _currentTime = node->start();
    
    if (auto chord = dynamic_cast<const dom::Chord*>(node)) {
        build(chord);
    } else if (auto note = dynamic_cast<const dom::Note*>(node)) {
        build(note);
    }
}

void SpanFactory::build(const dom::Chord* chord) {
    if (!chord->firstNote()->printObject())
        return;

    SpanCollection::iterator span;
    if (chord->firstNote()->grace()) {
        span = graceNoteSpan(chord);
    } else {
        span = _spans.eventSpan(_measureIndex, _currentTime);
        if (span == _spans.end()) {
            span = _spans.add(_measureIndex, _currentTime);
            span->setEvent(true);
        }
    }
    
    coord_t accidentalWidth = 0;
    coord_t headWidth = 0;
    coord_t stemWidth = 0;
    coord_t naturalWidth = -1;
    for (auto& note : chord->notes()) {
        headWidth = std::max(headWidth, NoteGeometry::Size(*note).width);
        if (note->accidental().isPresent())
            accidentalWidth = AccidentalGeometry::Size(note->accidental()).width;
        if (note->stem() == dom::STEM_UP && note->beams().empty() && chord->firstNote()->beams().empty())
            stemWidth = StemGeometry::Size(*note, true).width - StemGeometry::kNoFlagWidth;
        
        if (naturalWidth == -1)
            naturalWidth = naturalWidthForNote(*note);
        else
            naturalWidth = std::min(naturalWidth, naturalWidthForNote(*note));
    }
    
    coord_t width = headWidth;

    // The accidental should overlap the left margin
    if (accidentalWidth > 0)
        width += std::max(coord_t(0), accidentalWidth - kNoteMargin);

    // The stem flag should overlap the right margin
    if (stemWidth > 0)
        width += std::max(coord_t(0), stemWidth - kNoteMargin);

    if (chord->firstNote()->grace()) {
        width *= MeasureGeometry::kGraceNoteScale;
        if (width > 0) {
            span->pushLeftMargin(kNoteMargin * MeasureGeometry::kGraceNoteScale);
            span->pushRightMargin(kNoteMargin * MeasureGeometry::kGraceNoteScale);
        }
        span->pushWidth(width * MeasureGeometry::kGraceNoteScale);
        span->pushEventOffset((accidentalWidth + headWidth/2) * MeasureGeometry::kGraceNoteScale);
    } else if (width > 0) {
        span->pushLeftMargin(kNoteMargin);
        span->pushRightMargin(kNoteMargin);
        span->pushWidth(width);
        span->pullNaturalWidth(naturalWidth);
        span->pushEventOffset(std::max(coord_t(0), accidentalWidth - kNoteMargin) + headWidth/2);
    }

    span->addNode(chord);
    for (auto& note : chord->notes())
        span->addNode(note.get());
}

SpanCollection::iterator SpanFactory::graceNoteSpan(const dom::Chord* chord) {
    const auto range = _spans.range(_measureIndex, _currentTime);
    const int staff = chord->firstNote()->staff();
    SpanCollection::iterator span = _spans.end();

    // Count number of grace notes on the same staff
    auto count = std::count_if(range.first, range.second, [staff](const Span& s) {
        const std::set<const dom::Node*>& nodes = s.nodes();
        auto it = std::find_if(nodes.begin(), nodes.end(), [staff](const dom::Node* n) {
            const dom::Chord* chord = dynamic_cast<const dom::Chord*>(n);
            if (!chord)
                return false;

            auto note = chord->firstNote();
            return note->grace() && note->staff() == staff;
        });
        return it != nodes.end();
    });

    // Get grace note span matching the count
    std::size_t n = 0;
    for (auto it = range.first; it != range.second; ++it) {
        for (auto node : it->nodes()) {
            const dom::Chord* chord = dynamic_cast<const dom::Chord*>(node);
            if (!chord)
                continue;

            auto note = chord->firstNote();
            if (note->grace() && note->staff() != staff) {
                if (n == count)
                    span = it;
                n += 1;
            }
        }
    }

    if (span == _spans.end())
        span = _spans.addBeforeEvent(_measureIndex, _currentTime); // New grace note span

    return span;
}

void SpanFactory::build(const dom::Note* note) {
    assert (note->rest().isPresent());
    
    if (!note->printObject())
        return;
        
    auto span = _spans.eventSpan(_measureIndex, _currentTime);
    if (span == _spans.end()) {
        span = _spans.add(_measureIndex, _currentTime);
        span->setEvent(true);
    }
    span->pushWidth(kRestWidth);
    span->pullNaturalWidth(naturalWidthForNote(*note));
    span->pushEventOffset(kRestWidth/2);
    span->pushLeftMargin(kNoteMargin);
    span->pushRightMargin(kNoteMargin);
    span->addNode(note);
}

void SpanFactory::removeRedundantSpans() {
    auto first = _spans.first();
    auto last = _spans.last();
    if (first == _spans.end() || last == _spans.end())
        return;

    std::size_t measureCount = 0;
    for (auto& part : _score.parts())
        measureCount = std::max(measureCount, part->measures().size());

    for (std::size_t index = 0; index < measureCount; index += 1) {
        auto r = _spans.range(index);
        if (r.first == r.second)
            break;

        // Skip measures consisting of attributes only
        if (std::all_of(r.first, r.second, [](const Span& span) { return isAttributeOnlySpan(span); }))
            continue;

        // Remove attribute-only spans at the end, if next measure starts with attributes only
        for (auto it = r.second; it != r.first; ) {
            --it;
            if (it->event())
                break;

            auto rn = _spans.range(index + 1);
            if (isAttributeOnlySpan(*it) && isAttributeOnlySpan(*rn.first))
                _spans.erase(it);
        }
    }
}

bool SpanFactory::isAttributeOnlySpan(const Span& span) {
    if (span.event())
        return false;

    for (auto node : span.nodes()) {
        if (!dynamic_cast<const dom::Clef*>(node) && !dynamic_cast<const dom::Time*>(node) && !dynamic_cast<const dom::Key*>(node)) {
            return false;
        }
    }
    return true;
}

coord_t SpanFactory::naturalWidthForNote(const dom::Note& note) {
    static const coord_t kBaseWidth = 12;
    static const coord_t kWidthIncrease = 20;

    switch (note.type()) {
        case dom::Note::TYPE_1024TH:
        case dom::Note::TYPE_512TH:
        case dom::Note::TYPE_256TH:
        case dom::Note::TYPE_128TH:
        case dom::Note::TYPE_64TH:
        case dom::Note::TYPE_32ND:
        case dom::Note::TYPE_16TH:
            return kBaseWidth;

        case dom::Note::TYPE_EIGHTH:
            return kBaseWidth + kWidthIncrease;

        case dom::Note::TYPE_QUARTER:
            return kBaseWidth + 2*kWidthIncrease;

        case dom::Note::TYPE_HALF:
            return kBaseWidth + 4*kWidthIncrease;

        case dom::Note::TYPE_WHOLE:
            return kBaseWidth + 8*kWidthIncrease;

        case dom::Note::TYPE_BREVE:
            return kBaseWidth + 16*kWidthIncrease;
            
        case dom::Note::TYPE_LONG:
            return kBaseWidth + 32*kWidthIncrease;
            
        case dom::Note::TYPE_MAXIMA:
            return kBaseWidth + 64*kWidthIncrease;
    }
}

} // namespace mxml