// Copyright (c) FIRST and other WPILib contributors.
// Open Source Software; you can modify and/or share it under the terms of
// the WPILib BSD license file in the root directory of this project.

#include "hal/REVPH.h"

#include "HALInitializer.h"
#include "HALInternal.h"
#include "PortsInternal.h"
#include "hal/CANAPI.h"
#include "hal/Errors.h"
#include "hal/handles/IndexedHandleResource.h"
#include "mockdata/REVPHDataInternal.h"

using namespace hal;

namespace {
struct PCM {
  int32_t module;
  wpi::mutex lock;
  std::string previousAllocation;
};
}  // namespace

static IndexedHandleResource<HAL_REVPHHandle, PCM, kNumREVPHModules,
                             HAL_HandleEnum::REVPH>* pcmHandles;

namespace hal::init {
void InitializeREVPH() {
  static IndexedHandleResource<HAL_REVPHHandle, PCM, kNumREVPHModules,
                               HAL_HandleEnum::REVPH>
      pH;
  pcmHandles = &pH;
}
}  // namespace hal::init

HAL_REVPHHandle HAL_InitializeREVPH(int32_t module,
                                    const char* allocationLocation,
                                    int32_t* status) {
  hal::init::CheckInit();

  if (module == 0) {
    hal::SetLastErrorIndexOutOfRange(status, "Invalid Index for REV PH", 1,
                                     kNumREVPHModules, module);
    return HAL_kInvalidHandle;
  }

  HAL_REVPHHandle handle;
  auto pcm = pcmHandles->Allocate(module, &handle, status);

  if (*status != 0) {
    if (pcm) {
      hal::SetLastErrorPreviouslyAllocated(status, "REV PH", module,
                                           pcm->previousAllocation);
    } else {
      hal::SetLastErrorIndexOutOfRange(status, "Invalid Index for REV PH", 1,
                                       kNumREVPHModules, module);
    }
    return HAL_kInvalidHandle;  // failed to allocate. Pass error back.
  }

  pcm->previousAllocation = allocationLocation ? allocationLocation : "";
  pcm->module = module;

  SimREVPHData[module].initialized = true;
  // Enable closed loop
  SimREVPHData[module].closedLoopEnabled = true;

  return handle;
}

void HAL_FreeREVPH(HAL_REVPHHandle handle) {
  auto pcm = pcmHandles->Get(handle);
  if (pcm == nullptr) {
    pcmHandles->Free(handle);
    return;
  }
  SimREVPHData[pcm->module].initialized = false;
  pcmHandles->Free(handle);
}

HAL_Bool HAL_CheckREVPHModuleNumber(int32_t module) {
  return module >= 1 && module < kNumREVPDHModules;
}

HAL_Bool HAL_CheckREVPHSolenoidChannel(int32_t channel) {
  return channel < kNumREVPHChannels && channel >= 0;
}

HAL_Bool HAL_GetREVPHCompressor(HAL_REVPHHandle handle, int32_t* status) {
  auto pcm = pcmHandles->Get(handle);
  if (pcm == nullptr) {
    *status = HAL_HANDLE_ERROR;
    return false;
  }

  return SimREVPHData[pcm->module].compressorOn;
}

void HAL_SetREVPHClosedLoopControl(HAL_REVPHHandle handle, HAL_Bool enabled,
                                   int32_t* status) {
  auto pcm = pcmHandles->Get(handle);
  if (pcm == nullptr) {
    *status = HAL_HANDLE_ERROR;
    return;
  }

  SimREVPHData[pcm->module].closedLoopEnabled = enabled;
}

HAL_Bool HAL_GetREVPHClosedLoopControl(HAL_REVPHHandle handle,
                                       int32_t* status) {
  auto pcm = pcmHandles->Get(handle);
  if (pcm == nullptr) {
    *status = HAL_HANDLE_ERROR;
    return false;
  }

  return SimREVPHData[pcm->module].closedLoopEnabled;
}

HAL_Bool HAL_GetREVPHPressureSwitch(HAL_REVPHHandle handle, int32_t* status) {
  auto pcm = pcmHandles->Get(handle);
  if (pcm == nullptr) {
    *status = HAL_HANDLE_ERROR;
    return false;
  }

  return SimREVPHData[pcm->module].pressureSwitch;
}

double HAL_GetREVPHAnalogPressure(HAL_REVPHHandle handle, int32_t channel,
                                  int32_t* status) {
  return 0;
}

double HAL_GetREVPHCompressorCurrent(HAL_REVPHHandle handle, int32_t* status) {
  auto pcm = pcmHandles->Get(handle);
  if (pcm == nullptr) {
    *status = HAL_HANDLE_ERROR;
    return 0;
  }

  return SimREVPHData[pcm->module].compressorCurrent;
}

int32_t HAL_GetREVPHSolenoids(HAL_REVPHHandle handle, int32_t* status) {
  auto pcm = pcmHandles->Get(handle);
  if (pcm == nullptr) {
    *status = HAL_HANDLE_ERROR;
    return 0;
  }

  std::scoped_lock lock{pcm->lock};
  auto& data = SimREVPHData[pcm->module].solenoidOutput;
  uint8_t ret = 0;
  for (int i = 0; i < kNumREVPHChannels; i++) {
    ret |= (data[i] << i);
  }
  return ret;
}
void HAL_SetREVPHSolenoids(HAL_REVPHHandle handle, int32_t mask, int32_t values,
                           int32_t* status) {
  auto pcm = pcmHandles->Get(handle);
  if (pcm == nullptr) {
    *status = HAL_HANDLE_ERROR;
    return;
  }

  auto& data = SimREVPHData[pcm->module].solenoidOutput;
  std::scoped_lock lock{pcm->lock};
  for (int i = 0; i < kNumREVPHChannels; i++) {
    auto indexMask = (1 << i);
    if ((mask & indexMask) != 0) {
      data[i] = (values & indexMask) != 0;
    }
  }
}

void HAL_FireREVPHOneShot(HAL_REVPHHandle handle, int32_t index, int32_t durMs,
                          int32_t* status) {}
