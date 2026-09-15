#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include "mechanical_baseline.h"
#include "robot_geometry.h"
#include "robot_jacobian.h"
#include "robot_ik.h"
#include "robot_control.h"
#include "teach_recorder.h"
#include "teach_player.h"
#include "recipe.h"
#include "robot_hw_config.h"
#include "can_transport.h"
#include "platform_mock.h"
static uint32_t rng = 0x5419C0DEU;
static float Test_Random(void)
{
    rng = 1664525U * rng + 1013904223U;
    return (float)(rng >> 8) / 16777216.0f;
}
static void Test_Close(float a, float b, float tolerance)
{
    if (!(fabsf(a - b) <= tolerance))
    {
        printf("Mismatch %.9g vs %.9g tolerance %.9g\n", (double)a, (double)b, (double)tolerance);
        fflush(stdout);
        assert(false);
    }
}
static void Test_Quaternion(void)
{
    Quatf q = {2, 0, 0, 0};
    assert(Quat_Normalize(&q) == ARM_OK);
    Test_Close(q.w, 1, 1e-6f);
    q = Quat_FromAxisAngle((Vec3f){0, 0, 1}, ROBOT_PI / 2);
    Vec3f v = Quat_Rotate(q, (Vec3f){1, 0, 0});
    Test_Close(v.y, 1, 1e-6f);
    Quatf id = Quat_Multiply(q, Quat_Conjugate(q));
    Test_Close(id.w, 1, 1e-6f);
    Vec3f error;
    assert(Quat_OrientationError((Quatf){-q.w, -q.x, -q.y, -q.z}, q, &error) == ARM_OK);
    Test_Close(Vec3_Norm(error), 0, 1e-6f);
    assert(Quat_OrientationError(Quat_FromAxisAngle((Vec3f){1, 0, 0}, ROBOT_PI), (Quatf){1, 0, 0, 0},
                                 &error) == ARM_OK);
    Test_Close(Vec3_Norm(error), ROBOT_PI, 1e-5f);
    q = (Quatf){0};
    assert(Quat_Normalize(&q) == ARM_INVALID_ARGUMENT);
    q = (Quatf){NAN, 0, 0, 0};
    assert(Quat_Normalize(&q) == ARM_INVALID_ARGUMENT);
    puts("Quaternion PASS");
}
static void Test_FK(void)
{
    float peak = 0;
    for (unsigned n = 0; n < 5; ++n)
    {
        const BaselineVector *b = &baseline_vectors[n];
        JointVec6f q;
        memcpy(q.q, b->q, sizeof(q.q));
        RobotFrames f;
        assert(RobotFK_ComputeFrames(&q, &f) == ARM_OK);
        if (n < 4)
        {
            JointVec6f frozen;
            assert(RobotGeometry_GetFrozenPose((RobotFrozenPose)n, &frozen) == ARM_OK);
            for (unsigned i = 0; i < 6; ++i)
                Test_Close(q.q[i], frozen.q[i], 5e-7f);
        }
        for (unsigned i = 0; i < 6; ++i)
        {
            Vec3f d = Vec3_Subtract(f.joints[i].position_mm,
                                    (Vec3f){b->origins[i][0], b->origins[i][1], b->origins[i][2]});
            peak = fmaxf(peak, Vec3_Norm(d));
            assert(Vec3_Norm(d) < 0.0003f);
            d = Vec3_Subtract(f.world_axes[i], (Vec3f){b->axes[i][0], b->axes[i][1], b->axes[i][2]});
            assert(Vec3_Norm(d) < 1e-6f);
            assert(Quat_OrientationError(f.joints[i].orientation,
                                         (Quatf){b->rotations[i][0], b->rotations[i][1], b->rotations[i][2],
                                                 b->rotations[i][3]},
                                         &d) == ARM_OK);
            assert(Vec3_Norm(d) < 1e-6f);
        }
        Vec3f d = Vec3_Subtract(f.tcp.position_mm, (Vec3f){b->tcp[0], b->tcp[1], b->tcp[2]});
        peak = fmaxf(peak, Vec3_Norm(d));
        assert(Vec3_Norm(d) < 0.0003f);
        assert(Quat_OrientationError(f.tcp.orientation,
                                     (Quatf){b->quat[0], b->quat[1], b->quat[2], b->quat[3]}, &d) == ARM_OK);
        assert(Vec3_Norm(d) < 1e-6f);
        printf("FK vector %u TCP [%.6f, %.6f, %.6f] mm\n", n, (double)f.tcp.position_mm.x,
               (double)f.tcp.position_mm.y, (double)f.tcp.position_mm.z);
    }
    printf("FK tests PASS maximum reference error %.9f mm\n", (double)peak);
}
static void Test_Jacobian(void)
{
    RobotLimits hard, soft;
    assert(RobotLimits_MakeCad(&hard, &soft, 0.1f) == ARM_OK);
    float max_v = 0, max_w = 0;
    const float h = 1e-4f;
    for (unsigned n = 0; n < 250; ++n)
    {
        JointVec6f q;
        for (unsigned i = 0; i < 6; ++i)
            q.q[i] = soft.lower_rad.q[i] + Test_Random() * (soft.upper_rad.q[i] - soft.lower_rad.q[i]);
        Jacobian6f j;
        assert(RobotJacobian_Compute(&q, &j) == ARM_OK);
        for (unsigned axis = 0; axis < 6; ++axis)
        {
            JointVec6f a = q, b = q;
            a.q[axis] += h;
            b.q[axis] -= h;
            RobotPose pa, pb;
            assert(RobotFK_Solve(&a, &pa) == ARM_OK);
            assert(RobotFK_Solve(&b, &pb) == ARM_OK);
            float delta = a.q[axis] - b.q[axis];
            Vec3f dv = Vec3_Subtract(pa.position_mm, pb.position_mm), dw;
            assert(Quat_OrientationError(pa.orientation, pb.orientation, &dw) == ARM_OK);
            Vec3f ev =
                Vec3_Subtract(dv, (Vec3f){j.m[0][axis] * delta, j.m[1][axis] * delta, j.m[2][axis] * delta});
            Vec3f ew =
                Vec3_Subtract(dw, (Vec3f){j.m[3][axis] * delta, j.m[4][axis] * delta, j.m[5][axis] * delta});
            max_v = fmaxf(max_v, Vec3_Norm(ev));
            max_w = fmaxf(max_w, Vec3_Norm(ew));
            assert(Vec3_Norm(ev) < 0.0006f);
            assert(Vec3_Norm(ew) < 1.2e-6f);
        }
    }
    printf("Jacobian finite difference PASS 250 poses / 1500 columns: max displacement error %.9f mm, "
           "orientation %.9g rad\n",
           (double)max_v, (double)max_w);
}
static void Test_IK(void)
{
    RobotIKOptions o = RobotIK_DefaultOptions();
    RobotLimits soft;
    assert(RobotLimits_MakeCad(&o.limits, &soft, 0) == ARM_OK);
    float max_p = 0, max_w = 0;
    for (unsigned n = 0; n < 104; ++n)
    {
        JointVec6f source, seed;
        if (n < 4)
            assert(RobotGeometry_GetFrozenPose((RobotFrozenPose)n, &source) == ARM_OK);
        else
            for (unsigned i = 0; i < 6; ++i)
                source.q[i] = o.limits.lower_rad.q[i] + 0.2f +
                              Test_Random() * (o.limits.upper_rad.q[i] - o.limits.lower_rad.q[i] - 0.4f);
        RobotPose target;
        assert(RobotFK_Solve(&source, &target) == ARM_OK);
        seed = source;
        RobotIKSolution result;
        assert(RobotIK_Solve(&target, &seed, &o, &result) == IK_OK);
        assert(result.iterations == 0);
        for (unsigned i = 0; i < 6; ++i)
            seed.q[i] += 0.06f * (Test_Random() - 0.5f);
        (void)RobotLimits_Clamp(&o.limits, &seed);
        RobotIKResult r = RobotIK_Solve(&target, &seed, &o, &result);
        if (r != IK_OK)
            printf("IK fail sample %u status %d p %.6f w %.6f\n", n, r, (double)result.position_error_mm,
                   (double)result.orientation_error_rad);
        assert(r == IK_OK);
        assert(RobotLimits_Check(&o.limits, &result.q) == ARM_OK);
        RobotPose actual;
        Vec3f w;
        assert(RobotFK_Solve(&result.q, &actual) == ARM_OK);
        assert(Quat_OrientationError(target.orientation, actual.orientation, &w) == ARM_OK);
        float p = Vec3_Norm(Vec3_Subtract(target.position_mm, actual.position_mm));
        max_p = fmaxf(max_p, p);
        max_w = fmaxf(max_w, Vec3_Norm(w));
        assert(p <= o.position_tolerance_mm);
        assert(Vec3_Norm(w) <= o.orientation_tolerance_rad);
    }
    JointVec6f zero = {0};
    RobotPose t;
    assert(RobotFK_Solve(&zero, &t) == ARM_OK);
    t.position_mm.x += 5;
    RobotIKSolution s;
    RobotIKResult r = RobotIK_Solve(&t, &zero, &o, &s);
    assert(r != IK_OK);
    assert(RobotMath_IsFiniteJoint(&s.q));
    assert(s.iterations <= o.max_iterations);
    t.position_mm.x = NAN;
    assert(RobotIK_Solve(&t, &zero, &o, &s) == IK_INVALID_TARGET);
    t.position_mm.x = 500;
    zero.q[0] = 10;
    assert(RobotIK_Solve(&t, &zero, &o, &s) == IK_OUT_OF_LIMIT);
    printf("IK roundtrip PASS 104 poses (same seed and perturbed seed); max %.6f mm / %.9f rad; "
           "singular/invalid bounded PASS\n",
           (double)max_p, (double)max_w);
}
static RobotMotionLimits Test_MotionLimits(void)
{
    RobotMotionLimits m;
    for (unsigned i = 0; i < 6; ++i)
    {
        m.velocity_rad_s.q[i] = 0.5f;
        m.acceleration_rad_s2.q[i] = 1.0f;
    }
    return m;
}
static void Test_Motion(void)
{
    RobotLimits hard, soft;
    assert(RobotLimits_MakeCad(&hard, &soft, RobotMath_DegToRad(2)) == ARM_OK);
    JointVec6f a, b, stow;
    assert(RobotGeometry_GetFrozenPose(ROBOT_HOME, &a) == ARM_OK);
    assert(RobotGeometry_GetFrozenPose(ROBOT_SAFE_UNFOLD, &b) == ARM_OK);
    assert(RobotGeometry_GetFrozenPose(ROBOT_STOW, &stow) == ARM_OK);
    assert(RobotLimits_Check(&hard, &stow) == ARM_OK);
    assert(RobotLimits_Check(&soft, &stow) == ARM_OUT_OF_LIMIT);
    JointVec6f d;
    assert(RobotLimits_DistanceToBoundary(&hard, &stow, &d) == ARM_OK);
    Test_Close(d.q[2], 0, 1e-6f);
    JointVec6f invalid = a;
    invalid.q[0] = NAN;
    assert(RobotLimits_Clamp(&hard, &invalid) == ARM_INVALID_ARGUMENT);
    RobotMotionLimits motion = Test_MotionLimits();
    RobotTrajectory t;
    RobotTrajectorySample s;
    assert(Trajectory_Start(&t, &stow, &b, &soft, &motion, 0) == ARM_OUT_OF_LIMIT);
    assert(Trajectory_Start(&t, &a, &stow, &soft, &motion, 0) == ARM_OUT_OF_LIMIT);
    assert(Trajectory_Start(&t, &a, &b, &soft, &motion, 0) == ARM_OK);
    assert(Trajectory_Step(&t, 0, &s) == ARM_OK);
    for (unsigned i = 0; i < 6; ++i)
    {
        Test_Close(s.q.q[i], a.q[i], 1e-7f);
        Test_Close(s.dq_rad_s.q[i], 0, 1e-7f);
        Test_Close(s.ddq_rad_s2.q[i], 0, 1e-7f);
    }
    for (unsigned k = 0; k < 1000; ++k)
    {
        assert(Trajectory_Step(&t, t.duration_s / 999.0f, &s) == ARM_OK);
        assert(RobotLimits_Check(&soft, &s.q) == ARM_OK);
        for (unsigned i = 0; i < 6; ++i)
        {
            assert(fabsf(s.dq_rad_s.q[i]) <= 0.50001f);
            assert(fabsf(s.ddq_rad_s2.q[i]) <= 1.00001f);
        }
    }
    assert(Trajectory_IsFinished(&t));
    for (unsigned i = 0; i < 6; ++i)
    {
        Test_Close(s.q.q[i], b.q[i], 1e-6f);
        Test_Close(s.dq_rad_s.q[i], 0, 1e-6f);
        Test_Close(s.ddq_rad_s2.q[i], 0, 1e-6f);
    }
    Trajectory_Stop(&t);
    assert(Trajectory_Step(&t, 0.005f, &s) == ARM_NOT_READY);
    assert(Trajectory_Start(&t, &a, &a, &soft, &motion, 0) == ARM_OK);
    assert(Trajectory_Step(&t, 1000, &s) == ARM_OK);
    assert(Trajectory_IsFinished(&t));
    TeachRecorder recorder;
    assert(TeachRecorder_Init(&recorder, &soft) == ARM_OK);
    assert(TeachRecorder_Start(&recorder) == ARM_OK);
    TeachSample samples[2] = {{.time_ms = 0, .q_rad = a}, {.time_ms = 10000, .q_rad = b}};
    assert(TeachRecorder_PushSample(&recorder, &samples[0]) == ARM_OK);
    assert(TeachRecorder_PushSample(&recorder, &samples[0]) == ARM_INVALID_ARGUMENT);
    assert(TeachRecorder_PushSample(&recorder, &samples[1]) == ARM_OK);
    TeachRecorder_Stop(&recorder);
    TeachPlayer player;
    assert(TeachPlayer_Load(&player, recorder.samples, recorder.count, &soft, &motion) == ARM_OK);
    assert(TeachPlayer_Start(&player) == ARM_OK);
    assert(TeachPlayer_Step(&player, 5, &s) == ARM_OK);
    for (unsigned i = 0; i < 6; ++i)
        Test_Close(s.q.q[i], (a.q[i] + b.q[i]) * 0.5f, 1e-6f);
    assert(TeachPlayer_Step(&player, 5, &s) == ARM_OK);
    assert(!player.active);
    samples[1].time_ms = 1;
    assert(TeachPlayer_Load(&player, samples, 2, &soft, &motion) == ARM_OUT_OF_LIMIT);
    assert(TeachRecorder_Start(&recorder) == ARM_OK);
    for (unsigned i = 0; i < TEACH_CAPACITY; ++i)
    {
        samples[0].time_ms = i;
        assert(TeachRecorder_PushSample(&recorder, &samples[0]) == ARM_OK);
    }
    samples[0].time_ms = TEACH_CAPACITY;
    assert(TeachRecorder_PushSample(&recorder, &samples[0]) == ARM_QUEUE_FULL);
    RecipeBook book;
    Recipe_Init(&book);
    assert(Recipe_Select(&book, RECIPE_SMALL) == ARM_NOT_CONFIGURED);
    RecipeEntry e = {.configured = true, .geometry = {100, 50}, .teach_slot = 2, .trajectory_slot = 3};
    assert(Recipe_Configure(&book, RECIPE_CUSTOM, &e) == ARM_OK);
    assert(Recipe_Select(&book, RECIPE_CUSTOM) == ARM_OK);
    CakeGeometry g;
    assert(Recipe_GetGeometry(&book, &g) == ARM_OK);
    Test_Close(g.diameter_mm, 100, 0);
    uint16_t ts, ps;
    assert(Recipe_GetSlots(&book, &ts, &ps) == ARM_OK && ts == 2 && ps == 3);
    e.geometry.height_mm = NAN;
    assert(Recipe_Configure(&book, RECIPE_CUSTOM, &e) == ARM_INVALID_ARGUMENT);
    puts("Limits / trajectory / teach recorder-player / recipe PASS");
}
static void Test_Protocol(void)
{
    CanFrame f;
    CyberGearCommand c = {0};
    assert(CyberGear_EncodeMotion(1, CG_PROFILE_MANUAL_4PI, &c, &f) == ARM_OK);
    assert(f.ext_id == 0x017FFF01U);
    const uint8_t zero[8] = {0x7f, 0xff, 0x7f, 0xff, 0, 0, 0, 0};
    assert(memcmp(f.data, zero, 8) == 0);
    c = (CyberGearCommand){
        .position_rad = 100, .velocity_rad_s = -100, .torque_nm = 100, .kp = 1000, .kd = 10};
    assert(CyberGear_EncodeMotion(2, CG_PROFILE_MANUAL_4PI, &c, &f) == ARM_OK);
    assert(f.ext_id == 0x01FFFF02U);
    assert(f.data[0] == 255 && f.data[1] == 255 && f.data[2] == 0 && f.data[3] == 0 && f.data[4] == 255 &&
           f.data[7] == 255);
    c.kp = NAN;
    assert(CyberGear_EncodeMotion(2, CG_PROFILE_MANUAL_4PI, &c, &f) == ARM_INVALID_ARGUMENT);
    assert(CyberGear_EncodeSimple(1, 0, CG_ENABLE, &f) == ARM_OK && f.ext_id == 0x03000001U);
    assert(CyberGear_EncodeSimple(1, 0, CG_SET_ZERO, &f) == ARM_OK && f.data[0] == 1);
    assert(CyberGear_EncodeSimple(255, 0, CG_ENABLE, &f) == ARM_INVALID_ARGUMENT);
    assert(CyberGear_EncodeParameter(1, 0, CG_PARAM_VELOCITY_LIMIT, 1.0f, &f) == ARM_OK);
    const uint8_t parameter[8] = {0x17, 0x70, 0, 0, 0, 0, 0x80, 0x3f};
    assert(memcmp(f.data, parameter, 8) == 0 && f.ext_id == 0x12000001U);
    assert(CyberGear_EncodeParameter(1, 0, CG_PARAM_RUN_MODE, 1.5f, &f) == ARM_INVALID_ARGUMENT);
    f = (CanFrame){.extended = true,
                   .dlc = 8,
                   .ext_id = 0x02800100U,
                   .tick = 123,
                   .data = {0xff, 0xff, 0, 0, 0xff, 0xff, 0x01, 0x90}};
    CyberGearState state;
    assert(CyberGear_DecodeFeedback(&f, 1, 0, CG_PROFILE_MANUAL_4PI, &state) == ARM_OK);
    Test_Close(state.position_rad, 4 * ROBOT_PI, 1e-6f);
    Test_Close(state.temperature_c, 40, 1e-5f);
    assert(state.enabled && state.online && state.last_rx_tick == 123);
    f.dlc = 7;
    assert(CyberGear_DecodeFeedback(&f, 1, 0, CG_PROFILE_MANUAL_4PI, &state) == ARM_INVALID_ARGUMENT);
    f.dlc = 8;
    f.remote = true;
    assert(CyberGear_DecodeFeedback(&f, 1, 0, CG_PROFILE_MANUAL_4PI, &state) == ARM_INVALID_ARGUMENT);
    f.remote = false;
    f.ext_id |= 0x20000000;
    assert(CyberGear_DecodeFeedback(&f, 1, 0, CG_PROFILE_MANUAL_4PI, &state) == ARM_INVALID_ARGUMENT);
    f.ext_id = 0x02C00100;
    assert(CyberGear_DecodeFeedback(&f, 1, 0, CG_PROFILE_MANUAL_4PI, &state) == ARM_INVALID_ARGUMENT);
    RobotJointMap map = {.joint_sign = -1, .motor_zero_offset_rad = 0.2f, .calibrated = true};
    float q, m;
    assert(RobotJointMap_ToRobot(&map, 1.2f, &q) == ARM_OK);
    Test_Close(q, -1, 1e-6f);
    assert(RobotJointMap_ToMotor(&map, q, &m) == ARM_OK);
    Test_Close(m, 1.2f, 1e-6f);
    puts("CyberGear golden encode/decode / endian / clamp / malformed frames / joint mapping PASS");
}
static void Test_FeedbackAll(uint32_t tick, bool enabled)
{
    for (unsigned i = 0; i < 6; ++i)
    {
        CanFrame f = {.extended = true,
                      .dlc = 8,
                      .ext_id = (2U << 24) | ((enabled ? 2U : 0U) << 22) | ((i + 1) << 8),
                      .tick = tick,
                      .data = {0x7f, 0xff, 0x7f, 0xff, 0x7f, 0xff, 0x01, 0x90}};
        assert(MotorManager_ProcessFeedback(&f) == ARM_OK);
    }
}
static void Test_Safety(void)
{
    Mock_Reset();
    MotorAxisConfig cfg[6] = {0};
    assert(MotorManager_Init(cfg) == ARM_NOT_CONFIGURED);
    RobotControl c;
    assert(RobotControl_Init(&c) == ARM_OK);
    assert(c.machine.state == ROBOT_DISABLED);
    assert(Mock_GetPending() == 0);
    RobotCommand enable = {.type = ROBOT_CMD_ENABLE};
    assert(RobotControl_Execute(&c, &enable, 0) == ARM_NOT_READY);
    assert(Mock_GetPending() == 0);
    for (unsigned i = 0; i < 6; ++i)
        cfg[i] = (MotorAxisConfig){.motor_id = (uint8_t)(i + 1),
                                   .map = {1, 0, true},
                                   .protocol_confirmed = true,
                                   .profile = CG_PROFILE_MANUAL_4PI,
                                   .kp = 1,
                                   .kd = 0.1f,
                                   .velocity_limit_rad_s = 0.5f,
                                   .acceleration_limit_rad_s2 = 1,
                                   .torque_limit_nm = 0.5f,
                                   .current_limit_a = 1};
    cfg[5].motor_id = 1;
    assert(MotorManager_Init(cfg) == ARM_NOT_CONFIGURED);
    cfg[5].motor_id = 6;
    assert(MotorManager_Init(cfg) == ARM_OK);
    Test_FeedbackAll(10, false);
    assert(RobotControl_Execute(&c, &enable, 10) == ARM_OK);
    assert(c.enable_pending && c.machine.state == ROBOT_DISABLED);
    assert(Mock_GetPending() == 36);
    CanTransport_Pump(10);
    Test_FeedbackAll(15, true);
    assert(RobotControl_Step(&c, 15) == ARM_OK);
    assert(c.machine.state == ROBOT_READY);
    assert(Mock_GetPending() == 6);
    CanTransport_Pump(15);
    RobotCommand move = {.type = ROBOT_CMD_JOINT_MOVE};
    move.target_rad.q[0] = 0.1f;
    assert(RobotControl_Execute(&c, &move, 15) == ARM_OK);
    assert(RobotControl_Step(&c, 20) == ARM_OK);
    assert(c.machine.state == ROBOT_RUNNING);
    assert(RobotControl_Step(&c, 115) == ARM_FAULT);
    assert(c.machine.state == ROBOT_FAULT);
    assert(!CanTransport_IsArmed());
    assert(Mock_GetPending() == 6);
    for (size_t i = 0; i < Mock_GetPending(); ++i)
        assert((Mock_GetFrame(i)->ext_id >> 24) == CG_DISABLE);
    assert(RobotControl_Execute(&c, &move, 115) == ARM_NOT_READY);
    assert(RobotControl_Execute(&c, &enable, 115) == ARM_NOT_READY);
    Test_FeedbackAll(UINT32_MAX - 20U, false);
    MotorSnapshot snap;
    MotorManager_GetSnapshot(10, &snap);
    assert(MotorManager_IsHealthy(&snap, false));
    MotorManager_GetSnapshot(100, &snap);
    assert(!MotorManager_IsHealthy(&snap, false));
    RobotStateMachine state;
    RobotState_Init(&state);
    assert(RobotState_StartMotion(&state) == ARM_NOT_READY);
    assert(RobotState_SetReady(&state, true, false) == ARM_NOT_READY);
    RobotState_SetFault(&state, ARM_FAULT);
    RobotState_Disable(&state);
    assert(state.state == ROBOT_FAULT);
    assert(RobotState_ClearFault(&state, false) == ARM_NOT_READY);
    puts("Safety PASS: safe boot, duplicate/unassigned IDs, enable feedback handshake, timeout all-axis "
         "stop, queue invalidation, fault latch, tick wrap");
}
int main(void)
{
    setbuf(stdout, NULL);
    Test_Quaternion();
    Test_FK();
    Test_Jacobian();
    Test_IK();
    Test_Motion();
    Test_Protocol();
    Test_Safety();
    puts("ALL HOST TESTS PASS");
    return 0;
}
