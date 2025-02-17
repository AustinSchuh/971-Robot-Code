// Copyright (c) FIRST and other WPILib contributors.
// Open Source Software; you can modify and/or share it under the terms of
// the WPILib BSD license file in the root directory of this project.

#include "frc/DutyCycleEncoder.h"

#include <wpi/NullDeleter.h>
#include <wpi/sendable/SendableBuilder.h>

#include "frc/Counter.h"
#include "frc/DigitalInput.h"
#include "frc/DigitalSource.h"
#include "frc/DutyCycle.h"
#include "frc/Errors.h"

using namespace frc;

DutyCycleEncoder::DutyCycleEncoder(int channel)
    : m_dutyCycle{std::make_shared<DutyCycle>(
          std::make_shared<DigitalInput>(channel))} {
  Init();
}

DutyCycleEncoder::DutyCycleEncoder(DutyCycle& dutyCycle)
    : m_dutyCycle{&dutyCycle, wpi::NullDeleter<DutyCycle>{}} {
  Init();
}

DutyCycleEncoder::DutyCycleEncoder(DutyCycle* dutyCycle)
    : m_dutyCycle{dutyCycle, wpi::NullDeleter<DutyCycle>{}} {
  Init();
}

DutyCycleEncoder::DutyCycleEncoder(std::shared_ptr<DutyCycle> dutyCycle)
    : m_dutyCycle{std::move(dutyCycle)} {
  Init();
}

DutyCycleEncoder::DutyCycleEncoder(DigitalSource& digitalSource)
    : m_dutyCycle{std::make_shared<DutyCycle>(digitalSource)} {
  Init();
}

DutyCycleEncoder::DutyCycleEncoder(DigitalSource* digitalSource)
    : m_dutyCycle{std::make_shared<DutyCycle>(digitalSource)} {
  Init();
}

DutyCycleEncoder::DutyCycleEncoder(std::shared_ptr<DigitalSource> digitalSource)
    : m_dutyCycle{std::make_shared<DutyCycle>(digitalSource)} {
  Init();
}

void DutyCycleEncoder::Init() {
  m_simDevice = hal::SimDevice{"DutyCycle:DutyCycleEncoder",
                               m_dutyCycle->GetSourceChannel()};

  if (m_simDevice) {
    m_simPosition =
        m_simDevice.CreateDouble("position", hal::SimDevice::kInput, 0.0);
    m_simDistancePerRotation = m_simDevice.CreateDouble(
        "distance_per_rot", hal::SimDevice::kOutput, 1.0);
    m_simIsConnected =
        m_simDevice.CreateBoolean("connected", hal::SimDevice::kInput, true);
  } else {
    m_analogTrigger = std::make_unique<AnalogTrigger>(m_dutyCycle.get());
    m_analogTrigger->SetLimitsDutyCycle(0.25, 0.75);
    m_counter = std::make_unique<Counter>();
    m_counter->SetUpSource(
        m_analogTrigger->CreateOutput(AnalogTriggerType::kRisingPulse));
    m_counter->SetDownSource(
        m_analogTrigger->CreateOutput(AnalogTriggerType::kFallingPulse));
  }

  wpi::SendableRegistry::AddLW(this, "DutyCycle Encoder",
                               m_dutyCycle->GetSourceChannel());
}

units::turn_t DutyCycleEncoder::Get() const {
  if (m_simPosition) {
    return units::turn_t{m_simPosition.Get()};
  }

  // As the values are not atomic, keep trying until we get 2 reads of the same
  // value If we don't within 10 attempts, error
  for (int i = 0; i < 10; i++) {
    auto counter = m_counter->Get();
    auto pos = m_dutyCycle->GetOutput();
    auto counter2 = m_counter->Get();
    auto pos2 = m_dutyCycle->GetOutput();
    if (counter == counter2 && pos == pos2) {
      units::turn_t turns{counter + pos - m_positionOffset};
      m_lastPosition = turns;
      return turns;
    }
  }

  FRC_ReportError(
      warn::Warning, "{}",
      "Failed to read DutyCycle Encoder. Potential Speed Overrun. Returning "
      "last value");
  return m_lastPosition;
}

void DutyCycleEncoder::SetDistancePerRotation(double distancePerRotation) {
  m_distancePerRotation = distancePerRotation;
  m_simDistancePerRotation.Set(distancePerRotation);
}

double DutyCycleEncoder::GetDistancePerRotation() const {
  return m_distancePerRotation;
}

double DutyCycleEncoder::GetDistance() const {
  return Get().value() * GetDistancePerRotation();
}

int DutyCycleEncoder::GetFrequency() const {
  return m_dutyCycle->GetFrequency();
}

void DutyCycleEncoder::Reset() {
  if (m_counter) {
    m_counter->Reset();
  }
  m_positionOffset = m_dutyCycle->GetOutput();
}

bool DutyCycleEncoder::IsConnected() const {
  if (m_simIsConnected) {
    return m_simIsConnected.Get();
  }
  return GetFrequency() > m_frequencyThreshold;
}

void DutyCycleEncoder::SetConnectedFrequencyThreshold(int frequency) {
  if (frequency < 0) {
    frequency = 0;
  }
  m_frequencyThreshold = frequency;
}

int DutyCycleEncoder::GetFPGAIndex() const {
  return m_dutyCycle->GetFPGAIndex();
}

int DutyCycleEncoder::GetSourceChannel() const {
  return m_dutyCycle->GetSourceChannel();
}

void DutyCycleEncoder::InitSendable(wpi::SendableBuilder& builder) {
  builder.SetSmartDashboardType("AbsoluteEncoder");
  builder.AddDoubleProperty(
      "Distance", [this] { return this->GetDistance(); }, nullptr);
  builder.AddDoubleProperty(
      "Distance Per Rotation",
      [this] { return this->GetDistancePerRotation(); }, nullptr);
  builder.AddDoubleProperty(
      "Is Connected", [this] { return this->IsConnected(); }, nullptr);
}
