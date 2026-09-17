#include "version.h"
#include "version_build.h"
static const ArmVersion version = {"arm_6_axes_0", "0.0.2", ARM_FIRMWARE_SHA, ARM_MECHANICAL_SHA};
const ArmVersion *Version_GetInfo(void)
{
    return &version;
}
