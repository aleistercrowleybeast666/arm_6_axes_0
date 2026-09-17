#ifndef __BUSINESS_MOCK_H
#define __BUSINESS_MOCK_H
#include "motor_backend.h"
#include "app_snapshot.h"
#include "storage_backend.h"
void BusinessMock_Reset(void);
void BusinessMock_Tick(uint32_t now_ms);
void BusinessMock_SetJoint(const JointVec6f *q);
void BusinessMock_SetTeachAllowed(bool allowed);
void BusinessMock_SetOnline(bool online);
void BusinessMock_SetMounted(bool mounted);
void BusinessMock_FailAfter(int operations);
TrackResult BusinessMock_PutFile(const char *path, const uint8_t *bytes, size_t count);
bool BusinessMock_FileExists(const char *path);
unsigned BusinessMock_GetWrites(void);
void BusinessMock_SetApp(const AppSnapshot *snapshot);
#endif
