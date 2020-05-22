#include "openedit/errorStatus.h"

namespace openedit { namespace OPENEDIT_VERSION {

std::string ErrorStatus::outcome_to_string(Outcome o) {
    switch(o) {
    case OK:
        return std::string();
    case INVALID_EDIT:
        return "invalid edit request";
    case OPENTIME_ERROR:
        return "error with an opentime request";
    case OPENTIMELINE_ERROR:
        return "error with an opentimelineio request";
    case NO_SOURCE_RANGE_ON_CHILD:
        return "source range required on items for editing";
    default:
        return "unknown/illegal ErrorStatus::Outcome code";
    };
}

} }