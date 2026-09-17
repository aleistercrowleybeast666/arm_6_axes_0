#include "business_mock.h"
#include "robot_geometry.h"
#include "force_backend.h"
#include "input_local.h"
#include "debug_cli.h"
#include "text_format.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
static RobotLimits hard, soft;
static RobotMotionLimits motion;
static uint32_t now_ms;
static uint8_t file_bytes[TRACK_HEADER_BYTES + TRACK_MAX_SAMPLES * TRACK_SAMPLE_BYTES + 1];
static TrackSample samples[TRACK_MAX_SAMPLES];
static void Test_FreeArena(void)
{
    TrackOwner o = TrackBuffer_GetOwner();
    assert(TrackBuffer_Transfer(o, TRACK_OWNER_FREE) == TRACK_OK);
}
static void Test_Defaults(void)
{
    MotorManager_DisableAll();
    BusinessMock_Reset();
    Test_FreeArena();
    StorageService_Init();
    ForceSensor_Init();
    CommandManager_Init();
    now_ms = 100;
    assert(RobotLimits_MakeCad(&hard, &soft, 2 * ROBOT_DEG_TO_RAD) == ARM_OK);
    MotorAxisConfig c[AXIS_COUNT] = {0};
    for (unsigned i = 0; i < AXIS_COUNT; ++i)
    {
        motion.velocity_rad_s.q[i] = 1;
        motion.acceleration_rad_s2.q[i] = 2;
        c[i] = (MotorAxisConfig){.motor_id = (uint8_t)(i + 1),
                                 .map = {1, 0, true},
                                 .kp = 1,
                                 .kd = 0.1f,
                                 .velocity_limit_rad_s = 1,
                                 .acceleration_limit_rad_s2 = 2,
                                 .torque_limit_nm = 1,
                                 .current_limit_a = 1,
                                 .protocol_confirmed = true};
    }
    assert(MotorManager_Init(c) == ARM_OK);
    JointVec6f stow;
    RobotGeometry_GetFrozenPose(ROBOT_STOW, &stow);
    BusinessMock_SetJoint(&stow);
    BusinessMock_Tick(now_ms);
}
static StorageResponse Test_Request(StorageRequest *r)
{
    uint32_t ticket;
    assert(StorageService_Request(r, &ticket) == TRACK_OK);
    assert(StorageService_ProcessOne());
    StorageResponse out;
    assert(StorageService_TakeResult(ticket, &out) == TRACK_OK);
    return out;
}
static void Test_Mount(void)
{
    StorageRequest r = {.type = STORAGE_MOUNT};
    assert(Test_Request(&r).result == TRACK_OK);
}
static size_t Test_BuildTrack(unsigned count)
{
    JointVec6f home;
    RobotGeometry_GetFrozenPose(ROBOT_HOME, &home);
    TrackSample q;
    assert(TrackFormat_Quantize(&home, &q) == TRACK_OK);
    for (unsigned i = 0; i < count; ++i)
    {
        samples[i] = q;
        samples[i].q_cdeg[0] = (int16_t)i;
        TrackFormat_EncodeSample(&samples[i], file_bytes + TRACK_HEADER_BYTES + i * TRACK_SAMPLE_BYTES);
    }
    TrackHeader h = {20000, count, TrackFormat_GetModelId(),
                     TrackCrc_Compute(file_bytes + TRACK_HEADER_BYTES, count * TRACK_SAMPLE_BYTES)};
    TrackFormat_EncodeHeader(&h, file_bytes);
    return TRACK_HEADER_BYTES + count * TRACK_SAMPLE_BYTES;
}
static void Test_CrcHeader(void)
{
    FILE *vector = fopen(TEST_VECTOR_DIR "/home_track_v1.bin", "rb");
    assert(vector);
    size_t vector_bytes = fread(file_bytes, 1, sizeof(file_bytes), vector);
    assert(!ferror(vector) && fclose(vector) == 0 && vector_bytes == 136);
    TrackHeader golden;
    assert(TrackFormat_DecodeHeader(file_bytes, &golden) == TRACK_OK);
    assert(golden.count == 8 && golden.model_id == 0x6683CDEB);
    assert(TrackCrc_Compute(file_bytes + TRACK_HEADER_BYTES, vector_bytes - TRACK_HEADER_BYTES) ==
           golden.payload_crc);
    assert(TrackCrc_Compute((const uint8_t *)"123456789", 9) == 0xCBF43926);
    size_t n = Test_BuildTrack(30);
    (void)n;
    TrackHeader h;
    assert(TrackFormat_DecodeHeader(file_bytes, &h) == TRACK_OK);
    assert(h.count == 30 && h.period_us == 20000);
    assert(TrackPlayer_Validate(samples, &h, &soft, &motion) == TRACK_OK);
    file_bytes[0] ^= 1;
    assert(TrackFormat_DecodeHeader(file_bytes, &h) == TRACK_BAD_FORMAT);
    file_bytes[0] ^= 1;
    file_bytes[4] = 2;
    assert(TrackFormat_DecodeHeader(file_bytes, &h) == TRACK_UNSUPPORTED_VERSION);
    file_bytes[4] = 1;
    file_bytes[12] ^= 1;
    assert(TrackFormat_DecodeHeader(file_bytes, &h) == TRACK_BAD_CRC);
    file_bytes[12] ^= 1;
    assert(TrackFormat_DecodeHeader(file_bytes, &h) == TRACK_OK);
    ++h.model_id;
    TrackFormat_EncodeHeader(&h, file_bytes);
    assert(TrackFormat_DecodeHeader(file_bytes, &h) == TRACK_WRONG_MODEL);
    --h.model_id;
    h.count = 4097;
    TrackFormat_EncodeHeader(&h, file_bytes);
    assert(TrackFormat_DecodeHeader(file_bytes, &h) == TRACK_BUFFER_FULL);
    h.count = 30;
    h.period_us = 1;
    TrackFormat_EncodeHeader(&h, file_bytes);
    assert(TrackFormat_DecodeHeader(file_bytes, &h) == TRACK_BAD_FORMAT);
    JointVec6f q = {{NAN}};
    TrackSample quantized;
    assert(TrackFormat_Quantize(&q, &quantized) == TRACK_BAD_FORMAT);
    q = (JointVec6f){{10}};
    assert(TrackFormat_Quantize(&q, &quantized) == TRACK_LIMIT_ERROR);
    TrackSample sign = {{-32768, -1, 0, 32767, 100, -100}}, decoded;
    uint8_t b[12];
    TrackFormat_EncodeSample(&sign, b);
    TrackFormat_DecodeSample(b, &decoded);
    assert(!memcmp(&sign, &decoded, sizeof(sign)));
    puts("BIN header/version/CRC/model/count/time/quantization PASS");
}
static void Test_Interpolation(void)
{
    Test_BuildTrack(50);
    TrackHeader h;
    assert(TrackFormat_DecodeHeader(file_bytes, &h) == TRACK_OK);
    assert(TrackPlayer_Validate(samples, &h, &soft, &motion) == TRACK_OK);
    RobotTrajectorySample s, prior = {0};
    assert(TrackPlayer_Sample(samples, &h, 0, &s) == TRACK_OK);
    assert(s.q.q[0] == 0 && s.dq_rad_s.q[0] == 0);
    for (unsigned i = 0; i <= 196; ++i)
    {
        assert(TrackPlayer_Sample(samples, &h, i * 0.005f, &s) == TRACK_OK);
        assert(RobotLimits_Check(&soft, &s.q) == ARM_OK);
        if (i > 0)
            assert(s.q.q[0] >= prior.q.q[0] - 1e-6f);
        prior = s;
    }
    assert(fabsf(s.q.q[0] - RobotMath_DegToRad(0.49f)) < 1e-6f && s.dq_rad_s.q[0] == 0);
    RobotTrajectorySample a, b;
    TrackPlayer_Sample(samples, &h, 0.4f - 0.000001f, &a);
    TrackPlayer_Sample(samples, &h, 0.4f + 0.000001f, &b);
    assert(fabsf(a.dq_rad_s.q[0] - b.dq_rad_s.q[0]) < 1e-5f);
    assert(a.dq_rad_s.q[0] > 0);
    samples[15].q_cdeg[1] = 15000;
    assert(TrackPlayer_Validate(samples, &h, &hard, &motion) == TRACK_LIMIT_ERROR);
    Test_BuildTrack(50);
    samples[15].q_cdeg[0] = 10000;
    assert(TrackPlayer_Validate(samples, &h, &soft, &motion) == TRACK_LIMIT_ERROR);
    Test_BuildTrack(50);
    RobotMotionLimits slow = motion;
    slow.acceleration_rad_s2.q[0] = 0.01f;
    assert(TrackPlayer_Validate(samples, &h, &soft, &slow) == TRACK_LIMIT_ERROR);
    puts("50-to-200 Hz Hermite endpoints/continuous velocity/analytic limits PASS");
}
static void Test_Storage(void)
{
    Test_Defaults();
    Test_Mount();
    size_t n = Test_BuildTrack(30);
    assert(BusinessMock_PutFile("/tracks/custom/Example.bin", file_bytes, n) == TRACK_OK);
    StorageRequest r = {.type = STORAGE_SCAN_TRACKS};
    assert(Test_Request(&r).result == TRACK_OK);
    StorageSnapshot list;
    StorageService_GetSnapshot(&list);
    assert(list.count == 1 && !list.factory_available[0]);
    assert(!Storage_IsSafeName("../factory/small.bin"));
    assert(!Storage_IsSafeName("a/b.bin"));
    assert(!Storage_IsSafeName("x\\y.bin"));
    assert(!Storage_IsSafeName("x..bin"));
    assert(!Storage_IsSafeName(".bin"));
    assert(Storage_IsSafeName("Custom_A-1.bin"));
    r = (StorageRequest){.type = STORAGE_LOAD_TRACK, .limits = soft, .motion = motion};
    strcpy(r.name, "Example.bin");
    assert(Test_Request(&r).result == TRACK_OK);
    assert(TrackBuffer_GetOwner() == TRACK_OWNER_READY);
    uint32_t ticket;
    assert(StorageService_Request(&r, &ticket) == TRACK_BUSY);
    Test_FreeArena();
    file_bytes[n - 1] ^= 1;
    BusinessMock_PutFile("/tracks/custom/Example.bin", file_bytes, n);
    assert(Test_Request(&r).result == TRACK_BAD_CRC);
    assert(TrackBuffer_GetOwner() == TRACK_OWNER_FREE);
    file_bytes[n - 1] ^= 1;
    BusinessMock_PutFile("/tracks/custom/Example.bin", file_bytes, n - 1);
    assert(Test_Request(&r).result == TRACK_BAD_FORMAT);
    BusinessMock_PutFile("/tracks/custom/Example.bin", file_bytes, n + 1);
    assert(Test_Request(&r).result == TRACK_BAD_FORMAT);
    BusinessMock_PutFile("/tracks/custom/Example.bin", file_bytes, n);
    r.type = STORAGE_DELETE_TRACK;
    r.factory = true;
    assert(StorageService_Request(&r, &ticket) == TRACK_INVALID_NAME);
    r.factory = false;
    strcpy(r.name, "../factory/small.bin");
    assert(StorageService_Request(&r, &ticket) == TRACK_INVALID_NAME);
    strcpy(r.name, "Example.bin");
    assert(Test_Request(&r).result == TRACK_OK);
    assert(!BusinessMock_FileExists("/tracks/custom/Example.bin"));
    /* A renamed file is discovered by filename, not a header display name. */
    BusinessMock_PutFile("/tracks/custom/Renamed.bin", file_bytes, n);
    BusinessMock_PutFile("/tracks/custom/Orphan.tmp", file_bytes, n);
    BusinessMock_PutFile("/tracks/factory/small.bin", file_bytes, n);
    r.type = STORAGE_SCAN_TRACKS;
    assert(Test_Request(&r).result == TRACK_OK);
    StorageService_GetSnapshot(&list);
    assert(list.count == 1 && !strcmp(list.names[0], "Renamed.bin") && list.factory_available[0]);
    for (unsigned i = 0; i < 35; ++i)
    {
        char path[64];
        snprintf(path, sizeof(path), "/tracks/custom/Test_%02u.bin", i);
        BusinessMock_PutFile(path, file_bytes, n);
    }
    assert(Test_Request(&r).result == TRACK_LIST_FULL);
    StorageService_GetSnapshot(&list);
    assert(list.count == 32 && list.list_full);
    puts("Storage scan/load/CRC/truncation/exact length/path confinement/delete/rename/list overflow PASS");
}
static void Test_RecordSave(void)
{
    Test_Defaults();
    Test_Mount();
    JointVec6f home;
    RobotGeometry_GetFrozenPose(ROBOT_HOME, &home);
    TrackRecorder recorder;
    assert(TrackRecorder_Start(&recorder, &home, 100) == TRACK_OK);
    for (unsigned i = 0; i < 30; ++i)
    {
        JointVec6f actual = home;
        actual.q[0] = RobotMath_DegToRad(i * 0.01f);
        assert(TrackRecorder_Push(&recorder, &actual, 100 + i * 20, &soft) == TRACK_OK);
    }
    assert(BusinessMock_GetWrites() == 0);
    assert(TrackBuffer_GetSamples(TRACK_OWNER_RECORDER)[29].q_cdeg[0] == 29);
    assert(TrackRecorder_Finish(&recorder) == TRACK_OK);
    StorageRequest req = {.type = STORAGE_SAVE_TRACK, .limits = soft, .motion = motion};
    StorageResponse response = Test_Request(&req);
    assert(response.result == TRACK_OK);
    assert(!strcmp(response.name, "Custom_001.bin"));
    assert(BusinessMock_FileExists("/tracks/custom/Custom_001.bin"));
    assert(!BusinessMock_FileExists("/tracks/custom/Custom_001.tmp"));
    assert(TrackBuffer_GetOwner() == TRACK_OWNER_FREE);
    /* Inject failure at every transactional I/O stage: no broken final BIN is advertised. */
    for (int step = 0; step < 12; ++step)
    {
        Test_Defaults();
        Test_Mount();
        assert(TrackRecorder_Start(&recorder, &home, 100) == TRACK_OK);
        for (unsigned i = 0; i < 4; ++i)
            assert(TrackRecorder_Push(&recorder, &home, 100 + i * 20, &soft) == TRACK_OK);
        assert(TrackRecorder_Finish(&recorder) == TRACK_OK);
        BusinessMock_FailAfter(step);
        response = Test_Request(&req);
        if (response.result != TRACK_OK)
            assert(!BusinessMock_FileExists("/tracks/custom/Custom_001.bin"));
        assert(TrackBuffer_GetOwner() == TRACK_OWNER_FREE);
    }
    Test_Defaults();
    assert(TrackRecorder_Start(&recorder, &home, 100) == TRACK_OK);
    for (unsigned i = 0; i < TRACK_MAX_SAMPLES; ++i)
        assert(TrackRecorder_Push(&recorder, &home, 100 + i * 20, &soft) == TRACK_OK);
    assert(TrackRecorder_Push(&recorder, &home, 100 + TRACK_MAX_SAMPLES * 20, &soft) == TRACK_BUFFER_FULL);
    assert(recorder.count == 4096);
    TrackRecorder_Abort(&recorder);
    assert(TrackBuffer_GetOwner() == TRACK_OWNER_FREE);
    assert(TrackRecorder_Finish(&recorder) == TRACK_NOT_READY);
    assert(TrackRecorder_Start(&recorder, &home, UINT32_MAX - 5) == TRACK_OK);
    assert(TrackRecorder_Push(&recorder, &home, UINT32_MAX - 5, &soft) == TRACK_OK);
    assert(TrackRecorder_Push(&recorder, &home, 14, &soft) == TRACK_OK);
    TrackRecorder_Abort(&recorder);
    assert(TrackRecorder_Start(&recorder, &home, 100) == TRACK_OK);
    assert(TrackRecorder_Push(&recorder, &home, 200, &soft) == TRACK_LIMIT_ERROR);
    puts("Record actual q / 4096 bound / tick wrap / missed sample / temp-sync-reopen-rename failures PASS");
}
static void Test_Force(void)
{
    ForceSensor_Init();
    assert(Hx711_SignExtend(0xFFFFFF) == -1);
    assert(Hx711_SignExtend(0x800000) == -8388608);
    assert(Hx711_SignExtend(0x7FFFFF) == 8388607);
    ForceSnapshot f;
    ForceSensor_PushRaw(100, 10);
    ForceSensor_GetSnapshot(10, &f);
    assert(!f.calibrated && !f.valid && !f.stale);
    ForceCalibration c = {.zero_raw = 100,
                          .newton_per_count = 0.01f,
                          .full_scale_n = 100,
                          .overload_n = 80,
                          .sample_rate_hz = 80,
                          .valid = true};
    assert(ForceSensor_SetCalibration(&c) == ARM_OK);
    ForceSensor_PushRaw(1100, 20);
    ForceSensor_GetSnapshot(20, &f);
    assert(f.valid && fabsf(f.force_n - 10) < 1e-5f);
    ContactDetector d;
    ContactConfig contact = {.on_n = 5, .off_n = 3, .stable_samples = 3, .configured = true};
    assert(ContactDetector_Configure(&d, &contact) == ARM_OK);
    assert(!ContactDetector_Update(&d, 6));
    assert(!ContactDetector_Update(&d, 4));
    assert(!ContactDetector_Update(&d, 6));
    assert(!ContactDetector_Update(&d, 6));
    assert(ContactDetector_Update(&d, 6));
    assert(ContactDetector_Update(&d, 4));
    assert(ContactDetector_Update(&d, 2));
    assert(ContactDetector_Update(&d, 2));
    assert(!ContactDetector_Update(&d, 2));
    ForceSensor_GetSnapshot(171, &f);
    assert(f.stale && !f.valid);
    ForceSensor_PushRaw(8388607, 180);
    ForceSensor_GetSnapshot(180, &f);
    assert(f.overload && !f.valid);
    uint8_t b[FORCE_CAL_BYTES];
    ForceCalibration_Encode(&c, b);
    ForceCalibration decoded;
    assert(ForceCalibration_Decode(b, &decoded) == ARM_OK);
    b[12] ^= 1;
    assert(ForceCalibration_Decode(b, &decoded) == ARM_INVALID_ARGUMENT);
    assert(ForceSensor_SetCalibration(&c) == ARM_OK);
    ForceSensor_PushRaw(100, 200);
    assert(ForceSensor_Tare(200) == ARM_OK);
    ForceSensor_PushRaw(1100, 220);
    assert(ForceSensor_Calibrate(10, 220) == ARM_OK);
    ForceSensor_GetCalibration(&decoded);
    assert(fabsf(decoded.newton_per_count - 0.01f) < 1e-6f);
    ForceCondition condition = {.type = FORCE_CONDITION_CONTACT,
                                .threshold_n = 5,
                                .hysteresis_n = 1,
                                .stable_ms = 20,
                                .timeout_ms = 100,
                                .configured = true};
    ForceConditionState state = {.started_ms = 100};
    f = (ForceSnapshot){.valid = true, .calibrated = true, .force_n = 6};
    assert(ForceCondition_Evaluate(&condition, &state, &f, 100) == ARM_NOT_READY);
    assert(ForceCondition_Evaluate(&condition, &state, &f, 120) == ARM_FINISHED);
    f.stale = true;
    assert(ForceCondition_Evaluate(&condition, &state, &f, 130) == ARM_NOT_READY);
    condition.type = FORCE_CONDITION_RIPPLE_BELOW;
    assert(ForceCondition_Evaluate(&condition, &state, &f, 140) == ARM_NOT_CONFIGURED);
    FactoryRecipe reserved = *FactoryRecipe_Get(0);
    reserved.force_policy = FORCE_POLICY_RESERVED;
    assert(FactoryRecipe_ForceHook(&reserved, &state, &f, 100) == ARM_NOT_CONFIGURED);
    puts("HX711 signed 24-bit / calibration / CRC / hysteresis+debounce / stale / overload / reserved force "
         "PASS");
}
static void Test_Tick(OperationManager *o, RobotControl *c)
{
    now_ms += 5;
    BusinessMock_Tick(now_ms);
    OperationManager_Step(o, c, now_ms);
    ArmResult r = RobotControl_Step(c, now_ms);
    assert(r == ARM_OK);
}
static void Test_Startup(OperationManager *o, RobotControl *c)
{
    assert(RobotControl_Init(c) == ARM_OK);
    OperationManager_Init(o, now_ms);
    float last = -10;
    for (unsigned i = 0; i < 1000 && o->startup.state != STARTUP_READY; ++i)
    {
        Test_Tick(o, c);
        if (c->machine.state == ROBOT_STARTUP)
        {
            assert(RobotLimits_Check(&hard, &c->hold.q) == ARM_OK);
            assert(c->hold.q.q[2] >= last - 1e-6f);
            last = c->hold.q.q[2];
        }
    }
    assert(o->startup.state == STARTUP_READY && c->machine.state == ROBOT_READY && !c->startup_escape);
}
static void Test_Operations(void)
{
    Test_Defaults();
    Test_Mount();
    OperationManager o;
    RobotControl c;
    Test_Startup(&o, &c);
    JointVec6f stow, home;
    RobotGeometry_GetFrozenPose(ROBOT_STOW, &stow);
    RobotGeometry_GetFrozenPose(ROBOT_HOME, &home);
    RobotCommand bad = {.type = ROBOT_CMD_JOINT_MOVE, .target_rad = stow};
    assert(RobotControl_Execute(&c, &bad, now_ms) == ARM_OUT_OF_LIMIT);
    MotorSnapshot snap;
    MotorManager_GetSnapshot(now_ms, &snap);
    assert(snap.coordinates_ready);
    for (unsigned i = 0; i < 6; ++i)
        assert(fabsf(snap.axes[i].position_rad - home.q[i]) < 0.001f);
    size_t n = Test_BuildTrack(30);
    BusinessMock_PutFile("/tracks/custom/Example.bin", file_bytes, n);
    BusinessMock_PutFile("/tracks/factory/small.bin", file_bytes, n);
    AppCommand cmd = {.source = CONTROL_SOURCE_LOCAL, .type = CMD_EXEC_CUSTOM};
    strcpy(cmd.name, "Example.bin");
    assert(OperationManager_Command(&o, &c, &cmd, now_ms) == ARM_OK);
    assert(StorageService_ProcessOne());
    for (unsigned i = 0; i < 200 && o.snapshot.state != OP_DONE; ++i)
        Test_Tick(&o, &c);
    assert(o.snapshot.state == OP_DONE && o.snapshot.force_hook_calls == 0);
    assert(fabsf(c.hold.q.q[0] - RobotMath_DegToRad(0.29f)) < 1e-6f);
    cmd.type = CMD_EXEC_FACTORY_SMALL;
    assert(OperationManager_Command(&o, &c, &cmd, now_ms) == ARM_OK);
    assert(StorageService_ProcessOne());
    for (unsigned i = 0; i < 220 && o.snapshot.state != OP_DONE; ++i)
        Test_Tick(&o, &c);
    assert(o.snapshot.state == OP_DONE && o.snapshot.force_hook_calls == 1);
    /* Sensor absent does not gate Custom or a factory NONE policy. */
    cmd.type = CMD_EXEC_CUSTOM;
    assert(OperationManager_Command(&o, &c, &cmd, now_ms) == ARM_OK);
    StorageService_ProcessOne();
    for (unsigned i = 0; i < 20; ++i)
        Test_Tick(&o, &c);
    assert(o.player.active);
    OperationManager_SoftStop(&o, &c, now_ms);
    assert(!o.player.active && MotorManager_IsArmed());
    for (unsigned i = 0; i < 100 && c.machine.state != ROBOT_SOFT_STOPPED; ++i)
        Test_Tick(&o, &c);
    assert(c.machine.state == ROBOT_SOFT_STOPPED && MotorManager_IsArmed());
    assert(OperationManager_Command(&o, &c, &cmd, now_ms) == ARM_NOT_READY);
    cmd.type = CMD_ACKNOWLEDGE;
    assert(OperationManager_Command(&o, &c, &cmd, now_ms) == ARM_OK);
    cmd.type = CMD_RECORD_START;
    assert(OperationManager_Command(&o, &c, &cmd, now_ms) == ARM_OK);
    for (unsigned i = 0; i < 1000 && o.snapshot.state != OP_RECORDING; ++i)
        Test_Tick(&o, &c);
    assert(o.snapshot.state == OP_RECORDING);
    unsigned writes = BusinessMock_GetWrites();
    for (unsigned i = 0; i < 20; ++i)
        Test_Tick(&o, &c);
    OperationManager_SoftStop(&o, &c, now_ms);
    for (unsigned i = 0; i < 100 && c.machine.state != ROBOT_SOFT_STOPPED; ++i)
        Test_Tick(&o, &c);
    assert(o.recorder.aborted && BusinessMock_GetWrites() == writes && !o.storage_ticket &&
           TrackBuffer_GetOwner() == TRACK_OWNER_FREE);
    cmd.type = CMD_ACKNOWLEDGE;
    assert(OperationManager_Command(&o, &c, &cmd, now_ms) == ARM_OK);
    cmd.type = CMD_RECORD_START;
    assert(OperationManager_Command(&o, &c, &cmd, now_ms) == ARM_OK);
    for (unsigned i = 0; i < 40; ++i)
        Test_Tick(&o, &c);
    assert(o.snapshot.state == OP_RECORDING);
    cmd.type = CMD_RECORD_FINISH;
    assert(OperationManager_Command(&o, &c, &cmd, now_ms) == ARM_OK);
    for (unsigned i = 0; i < 100 && !o.storage_ticket; ++i)
        Test_Tick(&o, &c);
    assert(o.storage_ticket);
    StorageService_ProcessOne();
    Test_Tick(&o, &c);
    assert(o.snapshot.state == OP_DONE && BusinessMock_FileExists("/tracks/custom/Custom_001.bin"));
    cmd.type = CMD_RECORD_START;
    BusinessMock_SetTeachAllowed(false);
    assert(OperationManager_Command(&o, &c, &cmd, now_ms) == ARM_OK);
    Test_Tick(&o, &c);
    assert(o.snapshot.state == OP_ERROR && o.snapshot.result == ARM_NOT_CONFIGURED);
    cmd.source = CONTROL_SOURCE_DEBUG;
    assert(CommandManager_Submit(&cmd) == ARM_OUTPUT_DISABLED);
    puts("Startup escape / ordinary soft-limit rejection / Custom hold / Factory hook / SoftStop / Record "
         "save+abort / teach gate PASS");
}
static void Test_Offsets(void)
{
    Test_Defaults();
    MotorAxisConfig cfg[6] = {0};
    for (unsigned i = 0; i < 6; ++i)
        cfg[i] = (MotorAxisConfig){.motor_id = (uint8_t)(i + 1),
                                   .map = {i % 2 ? -1 : 1, 0, false},
                                   .velocity_limit_rad_s = 1,
                                   .acceleration_limit_rad_s2 = 2};
    assert(MotorManager_Init(cfg) == ARM_OK);
    JointVec6f raw = {{0.1f, -0.2f, 0.3f, -0.4f, 0.5f, -0.6f}}, stow;
    RobotGeometry_GetFrozenPose(ROBOT_STOW, &stow);
    BusinessMock_SetJoint(&raw);
    BusinessMock_Tick(now_ms);
    assert(MotorManager_CaptureStartupPose(&stow, now_ms) == ARM_OK);
    MotorSnapshot s;
    MotorManager_GetSnapshot(now_ms, &s);
    for (unsigned i = 0; i < 6; ++i)
    {
        float expected = raw.q[i] - (float)cfg[i].map.joint_sign * stow.q[i];
        assert(fabsf(s.axes[i].motor_zero_offset - expected) < 1e-6f);
        assert(fabsf(s.axes[i].position_rad - stow.q[i]) < 1e-6f);
    }
    OperationManager operation;
    RobotControl startup_control;
    Test_Startup(&operation, &startup_control);
    assert(operation.startup.state == STARTUP_READY);
    MotorManager_DisableAll();
    RobotControl c;
    RobotControl_Init(&c);
    StartupManager startup;
    StartupManager_Init(&startup, STARTUP_POSITION_ABSOLUTE_ENCODER, 0);
    assert(StartupManager_Step(&startup, &c, 10) == ARM_NOT_CONFIGURED);
    StartupManager_Init(&startup, STARTUP_POSITION_ASSUME_STOW, 0);
    BusinessMock_SetOnline(false);
    assert(StartupManager_Step(&startup, &c, 10) == ARM_NOT_READY);
    assert(StartupManager_Step(&startup, &c, 20000) == ARM_TIMEOUT);
    puts("Runtime zero mapping with both signs / future position policies rejected / startup offline timeout "
         "PASS");
}
static void Test_SafetyEdges(void)
{
    Test_Defaults();
    Test_Mount();
    StorageRequest missing_cal = {.type = STORAGE_LOAD_FORCE_CAL};
    assert(Test_Request(&missing_cal).result == TRACK_NOT_READY);
    StorageSnapshot storage;
    StorageService_GetSnapshot(&storage);
    assert(storage.mounted);
    assert(TrackBuffer_Transfer(TRACK_OWNER_FREE, TRACK_OWNER_PLAYER) == TRACK_BUSY);
    RobotTrajectorySample start = {0}, sample;
    RobotGeometry_GetFrozenPose(ROBOT_HOME, &start.q);
    start.dq_rad_s.q[0] = 0.5f;
    SoftStop stop;
    assert(SoftStop_Start(&stop, &start, &soft, &motion) == ARM_OK);
    float last_v = start.dq_rad_s.q[0];
    while (stop.active)
    {
        assert(SoftStop_Step(&stop, 0.001f, &sample) == ARM_OK);
        assert(sample.dq_rad_s.q[0] <= last_v + 1e-6f);
        assert(fabsf(sample.ddq_rad_s2.q[0]) <= motion.acceleration_rad_s2.q[0] + 1e-5f);
        last_v = sample.dq_rad_s.q[0];
    }
    assert(fabsf(last_v) < 1e-6f);
    assert(fabsf(sample.q.q[0] - (start.q.q[0] + 0.5f * start.dq_rad_s.q[0] * stop.duration_s)) < 1e-6f);
    start.q.q[0] = RobotMath_DegToRad(167.99f);
    assert(SoftStop_Start(&stop, &start, &soft, &motion) == ARM_OUT_OF_LIMIT);
    StorageRequest calibration = {.type = STORAGE_SAVE_FORCE_CAL,
                                  .calibration = {.zero_raw = 100,
                                                  .newton_per_count = 0.001f,
                                                  .full_scale_n = 100,
                                                  .overload_n = 80,
                                                  .sample_rate_hz = 80,
                                                  .valid = true}};
    assert(Test_Request(&calibration).result == TRACK_OK);
    calibration.type = STORAGE_LOAD_FORCE_CAL;
    StorageResponse response = Test_Request(&calibration);
    assert(response.result == TRACK_OK && response.calibration.zero_raw == 100);
    uint8_t corrupt[FORCE_CAL_BYTES] = {0};
    BusinessMock_PutFile("/config/force_cal.bin", corrupt, sizeof(corrupt));
    assert(Test_Request(&calibration).result == TRACK_BAD_FORMAT);
    OperationManager operation;
    RobotControl control;
    Test_Startup(&operation, &control);
    size_t bytes = Test_BuildTrack(30);
    BusinessMock_PutFile("/tracks/custom/Cancel.bin", file_bytes, bytes);
    AppCommand command = {.source = CONTROL_SOURCE_LOCAL, .type = CMD_EXEC_CUSTOM, .name = "Cancel.bin"};
    assert(OperationManager_Command(&operation, &control, &command, now_ms) == ARM_OK);
    OperationManager_SoftStop(&operation, &control, now_ms);
    assert(StorageService_ProcessOne());
    Test_Tick(&operation, &control);
    assert(!operation.player.active && TrackBuffer_GetOwner() == TRACK_OWNER_FREE);
    puts("Ownership / stop bounds / calibration persistence / cancel pending load PASS");
}
static void Test_Ui(void)
{
    LocalUi u;
    LocalUi_Init(&u);
    StorageSnapshot s = {.mounted = true, .count = 1, .factory_available = {true, false, false}};
    strcpy(s.names[0], "Example.bin");
    OperationSnapshot o = {.state = OP_IDLE};
    AppCommand cmd;
    UiModel model;
    LocalUi_Render(&u, STARTUP_READY, ROBOT_READY, &o, &s, &model);
    assert(!strcmp(model.rows[0], UiStrings_Get(UI_TEXT_EXECUTE)));
    assert(!LocalUi_Handle(&u, FORWARD, STARTUP_WAIT_MOTORS, ROBOT_DISABLED, &o, &s, &cmd));
    assert(!LocalUi_Handle(&u, FORWARD, STARTUP_READY, ROBOT_READY, &o, &s, &cmd));
    assert(u.page == UI_PAGE_ACTIONS);
    LocalUi_Handle(&u, ENCODER_NEXT, STARTUP_READY, ROBOT_READY, &o, &s, &cmd);
    LocalUi_Handle(&u, FORWARD, STARTUP_READY, ROBOT_READY, &o, &s, &cmd);
    assert(u.page == UI_PAGE_FACTORY);
    LocalUi_Render(&u, STARTUP_READY, ROBOT_READY, &o, &s, &model);
    assert(!strcmp(model.rows[1], UiStrings_Get(UI_TEXT_RETURN)) && model.rows[2][0] == 0);
    LocalUi_Handle(&u, BACK, STARTUP_READY, ROBOT_READY, &o, &s, &cmd);
    for (unsigned i = 0; i < 4; ++i)
        LocalUi_Handle(&u, ENCODER_NEXT, STARTUP_READY, ROBOT_READY, &o, &s, &cmd);
    LocalUi_Handle(&u, FORWARD, STARTUP_READY, ROBOT_READY, &o, &s, &cmd);
    assert(u.page == UI_PAGE_CUSTOM);
    LocalUi_Handle(&u, ENCODER_NEXT, STARTUP_READY, ROBOT_READY, &o, &s, &cmd);
    assert(!LocalUi_Handle(&u, FORWARD, STARTUP_READY, ROBOT_READY, &o, &s, &cmd));
    assert(u.page == UI_PAGE_DELETE_CONFIRM && u.selected == 1);
    LocalUi_Handle(&u, ENCODER_PREV, STARTUP_READY, ROBOT_READY, &o, &s, &cmd);
    assert(LocalUi_Handle(&u, FORWARD, STARTUP_READY, ROBOT_READY, &o, &s, &cmd));
    assert(cmd.type == CMD_DELETE_CUSTOM && !strcmp(cmd.name, "Example.bin"));
    OperationState states[] = {OP_LOADING, OP_EXECUTING, OP_SAVING, OP_ERROR, OP_RECORDING};
    for (unsigned i = 0; i < 5; ++i)
    {
        o.state = states[i];
        LocalUi_Render(&u, STARTUP_READY, ROBOT_READY, &o, &s, &model);
        assert(model.rows[0][0]);
    }
    LocalInput input = {0};
    assert(InputLocal_Update(&input, 65534, false, false, 0) == UI_EVENT_NONE);
    assert(InputLocal_Update(&input, 2, false, false, 10) == ENCODER_NEXT);
    assert(InputLocal_Update(&input, 2, true, false, 20) == UI_EVENT_NONE);
    assert(InputLocal_Update(&input, 2, true, false, 44) == UI_EVENT_NONE);
    assert(InputLocal_Update(&input, 2, true, false, 45) == FORWARD);
    assert(InputLocal_Update(&input, 2, true, false, 90) == UI_EVENT_NONE);
    char text[16];
    assert(Text_Format(text, sizeof(text), "%03u %ld %x", 4U, -12L, 15U) == 9);
    assert(!strcmp(text, "004 -12 f"));
    Text_Format(text, 4, "abcdef");
    assert(!strcmp(text, "abc"));
    AppSnapshot app = {.startup = STARTUP_READY, .robot = ROBOT_READY};
    BusinessMock_SetApp(&app);
    char response[256];
    DebugCli_Execute("move 1", 0, response, sizeof(response));
    assert(strstr(response, "DISABLED"));
    DebugCli_Execute("force cal nan", 0, response, sizeof(response));
    assert(strstr(response, "valid=0"));
    puts("UI menu/factory protection/custom delete confirmation/status pages/debounce/encoder wrap/debug "
         "authority PASS");
}
int main(void)
{
    Test_Defaults();
    Test_CrcHeader();
    Test_Interpolation();
    Test_Storage();
    Test_RecordSave();
    Test_Force();
    Test_Offsets();
    Test_Operations();
    Test_SafetyEdges();
    Test_Ui();
    puts("ALL BUSINESS TESTS PASS");
    return 0;
}
