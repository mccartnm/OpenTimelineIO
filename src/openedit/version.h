#pragma once

#include "opentimelineio/version.h"

#define OPENEDIT_VERSION v1_0

namespace openedit {
    namespace OPENEDIT_VERSION {
        // Expose otio objects to the edit system
        using namespace opentimelineio::OPENTIMELINEIO_VERSION;
    }
    
    using namespace OPENEDIT_VERSION;
}
