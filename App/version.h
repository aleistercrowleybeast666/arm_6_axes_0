#ifndef __VERSION_H
#define __VERSION_H
typedef struct
{
    const char *project, *version, *firmware_sha, *mechanical_sha;
} ArmVersion;
const ArmVersion *Version_GetInfo(void);
#endif
