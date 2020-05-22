#pragma once

#include "openedit/version.h"

#include "opentime/errorStatus.h"
#include "opentimelineio/errorStatus.h"

#include <string>
#include <memory>

namespace openedit { namespace OPENEDIT_VERSION {

using OTIOErrorStatus = opentimelineio::OPENTIMELINEIO_VERSION::ErrorStatus;
using OpenTimeErrorStatus = opentime::OPENTIME_VERSION::ErrorStatus;

struct ErrorStatus {
    operator bool () {
        return outcome != Outcome::OK;
    }

    enum Outcome {
        OK = 0,
        INVALID_EDIT,
        OPENTIME_ERROR,
        OPENTIMELINE_ERROR,
        NO_SOURCE_RANGE_ON_CHILD,
    };

    static std::shared_ptr<OTIOErrorStatus> otio_error_status() {
        return std::make_shared<OTIOErrorStatus>();
    }

    static std::shared_ptr<OpenTimeErrorStatus> opentime_error_status() {
        return std::make_shared<OpenTimeErrorStatus>();
    }

    ErrorStatus() : outcome {OK} {}

    ErrorStatus(Outcome in_outcome) :
        outcome {in_outcome},
        details {outcome_to_string(in_outcome)} {}

    ErrorStatus(Outcome in_outcome, std::string const& in_details)
        : outcome {in_outcome},
          details {in_details} {}

    ErrorStatus(std::shared_ptr<OTIOErrorStatus> otio_error)
        : outcome{ OPENTIMELINE_ERROR }
        , details{ otio_error->details }
    {}

    ErrorStatus(std::shared_ptr<OpenTimeErrorStatus> opentime_error)
        : outcome{ OPENTIME_ERROR }
        , details{ opentime_error->details }
    {}

    Outcome outcome;
    std::string details;

    static std::string outcome_to_string(Outcome);
};

} }