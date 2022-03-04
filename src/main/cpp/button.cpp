
#ifdef __FRC_ROBORIO__
#include "hal/ChipObject.h"
#include "FRC_NetworkCommunication/LoadOut.h"
#include <memory>

using namespace hal;

static std::unique_ptr<tGlobal> rioGlobal;

extern "C" int RDO_InitializeButton() {
    int32_t statusCode = 0;

    g_currentTargetClass = nLoadOut::getTargetClass();

    rioGlobal.reset(tGlobal::create(&statusCode));
    return statusCode == 0;
}

extern "C" int RDO_GetButton() {
    int32_t status = 0;
    return rioGlobal != nullptr && rioGlobal->readUserButton(&status);
}

#else

extern "C" int RDO_InitializeButton() {
    return 1;   
}

extern "C" int RDO_GetButton() {
    return 1;
}

#endif