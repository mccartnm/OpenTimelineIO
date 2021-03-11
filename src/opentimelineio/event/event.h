#pragma once

#include "opentimelineio/version.h"
#include "opentimelineio/errorStatus.h"
#include "opentimelineio/anyDictionary.h"
#include "opentimelineio/serializableObjectWithMetadata.h"


namespace opentimelineio { namespace OPENTIMELINEIO_VERSION {

/*
    Event Requirements:
    - Atomic
        - No outside-state awareness (other than members required)
        - It should pass or fail completely
        - If failure occurs, rollback is mandatory
        - If all items have this, EventStack can also be an event
          which is the sum of all it's children events.
        - ***Note: Atomic does not mean stateless
            - Statelessness is maintained with the EventType
    - Rerunnable
        - Because of the rules above, an event should be consistently
          runnable both forward and reverse
        - **Not** required to handle the case of:

            event1.forward()
            event1.forward()

            or

            event1.reverse()
            event1.reverse()
*/
class Event : public SerializableObjectWithMetadata {
public:
    struct Schema {
        static auto constexpr name = "Event";
        static int constexpr version = 1;
    };

    using Parent = SerializableObjectWithMetadata;

    Event(std::string const& name = std::string(),
          AnyDictionary const& metadata = AnyDictionary());

    void run(ErrorStatus* error_status);
    void revert(ErrorStatus* error_status);

protected:
    virtual ~Event() {}

	virtual void forward(ErrorStatus* error_status);
	virtual void reverse(ErrorStatus* error_status);

    // Overload to provide validation before forward/reverse are called
    virtual bool _validate(ErrorStatus*) { return true; }

private:
    bool _has_run = false;
};

} }

