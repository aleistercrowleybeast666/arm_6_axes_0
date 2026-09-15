#ifndef __ROBOT_MATH_TYPES_H
#define __ROBOT_MATH_TYPES_H
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#define ROBOT_PI 3.14159265358979323846f
#define ROBOT_DEG_TO_RAD (ROBOT_PI / 180.0f)
typedef enum
{
    AXIS_J1,
    AXIS_J2,
    AXIS_J3,
    AXIS_J4,
    AXIS_J5,
    AXIS_J6,
    AXIS_COUNT
} RobotAxis;
typedef enum
{
    ARM_OK,
    ARM_INVALID_ARGUMENT,
    ARM_OUT_OF_LIMIT,
    ARM_NOT_READY,
    ARM_OUTPUT_DISABLED,
    ARM_QUEUE_FULL,
    ARM_BUS_ERROR,
    ARM_TIMEOUT,
    ARM_FAULT,
    ARM_NOT_CONFIGURED,
    ARM_FINISHED
} ArmResult;
typedef struct
{
    float x, y, z;
} Vec3f;
typedef struct
{
    float w, x, y, z;
} Quatf;
typedef struct
{
    float m[3][3];
} Mat3f;
typedef struct
{
    float m[4][4];
} Mat4f;
typedef struct
{
    float q[AXIS_COUNT];
} JointVec6f;
typedef struct
{
    float m[6][6];
} Jacobian6f;
typedef struct
{
    Vec3f position_mm;
    Quatf orientation;
} RobotPose;
float RobotMath_DegToRad(float deg);
float RobotMath_RadToDeg(float rad);
float RobotMath_Clamp(float value, float lower, float upper);
bool RobotMath_IsFiniteJoint(const JointVec6f *q);
Vec3f Vec3_Add(Vec3f a, Vec3f b);
Vec3f Vec3_Subtract(Vec3f a, Vec3f b);
Vec3f Vec3_Scale(Vec3f a, float scale);
Vec3f Vec3_Cross(Vec3f a, Vec3f b);
float Vec3_Norm(Vec3f a);
ArmResult Quat_Normalize(Quatf *q);
Quatf Quat_Multiply(Quatf a, Quatf b);
Quatf Quat_Conjugate(Quatf q); /* inverse for unit quaternions */
Quatf Quat_FromAxisAngle(Vec3f unit_axis, float angle_rad);
Vec3f Quat_Rotate(Quatf unit_q, Vec3f v);
ArmResult Quat_OrientationError(Quatf target, Quatf actual, Vec3f *world_rotvec);
#endif
