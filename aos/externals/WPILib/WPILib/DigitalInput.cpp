/*----------------------------------------------------------------------------*/
/* Copyright (c) FIRST 2008. All Rights Reserved.							  */
/* Open Source Software - may be modified and shared by FRC teams. The code   */
/* must be accompanied by the FIRST BSD license file in $(WIND_BASE)/WPILib.  */
/*----------------------------------------------------------------------------*/

#include "DigitalInput.h"
#include "DigitalModule.h"
#include "NetworkCommunication/UsageReporting.h"
#include "Resource.h"
#include "WPIErrors.h"

/**
 * Create an instance of a DigitalInput.
 * Creates a digital input given a slot and channel. Common creation routine
 * for all constructors.
 */
void DigitalInput::InitDigitalInput(UINT8 moduleNumber, UINT32 channel)
{
	char buf[64];
	if (!CheckDigitalModule(moduleNumber))
	{
		snprintf(buf, 64, "Digital Module %d", moduleNumber);
		wpi_setWPIErrorWithContext(ModuleIndexOutOfRange, buf);
		return;
	}
	if (!CheckDigitalChannel(channel))
	{
		snprintf(buf, 64, "Digital Channel %d", channel);
		wpi_setWPIErrorWithContext(ChannelIndexOutOfRange, buf);
		return;
	}
	m_channel = channel;
	m_module = DigitalModule::GetInstance(moduleNumber);
	m_module->AllocateDIO(channel, true);

	nUsageReporting::report(nUsageReporting::kResourceType_DigitalInput, channel, moduleNumber - 1);
}

/**
 * Create an instance of a Digital Input class.
 * Creates a digital input given a channel and uses the default module.
 *
 * @param channel The digital channel (1..14).
 */
DigitalInput::DigitalInput(UINT32 channel)
{
	InitDigitalInput(GetDefaultDigitalModule(), channel);
}

/**
 * Create an instance of a Digital Input class.
 * Creates a digital input given an channel and module.
 *
 * @param moduleNumber The digital module (1 or 2).
 * @param channel The digital channel (1..14).
 */
DigitalInput::DigitalInput(UINT8 moduleNumber, UINT32 channel)
{
	InitDigitalInput(moduleNumber, channel);
}

/**
 * Free resources associated with the Digital Input class.
 */
DigitalInput::~DigitalInput()
{
	if (StatusIsFatal()) return;
	m_module->FreeDIO(m_channel);
}

/*
 * Get the value from a digital input channel.
 * Retrieve the value of a single digital input channel from the FPGA.
 */
UINT32 DigitalInput::Get()
{
	if (StatusIsFatal()) return 0;
	return m_module->GetDIO(m_channel);
}

/**
 * @return The GPIO channel number that this object represents.
 */
UINT32 DigitalInput::GetChannel()
{
	return m_channel;
}

/**
 * @return The value to be written to the channel field of a routing mux.
 */
UINT32 DigitalInput::GetChannelForRouting()
{
	return DigitalModule::RemapDigitalChannel(GetChannel() - 1);
}

/**
 * @return The value to be written to the module field of a routing mux.
 */
UINT32 DigitalInput::GetModuleForRouting()
{
	if (StatusIsFatal()) return 0;
	return m_module->GetNumber() - 1;
}

/**
 * @return The value to be written to the analog trigger field of a routing mux.
 */
bool DigitalInput::GetAnalogTriggerForRouting()
{
	return false;
}

void DigitalInput::UpdateTable() {
	if (m_table != NULL) {
		m_table->PutBoolean("Value", Get());
	}
}

void DigitalInput::StartLiveWindowMode() {
	
}

void DigitalInput::StopLiveWindowMode() {
	
}

std::string DigitalInput::GetSmartDashboardType() {
	return "DigitalInput";
}

void DigitalInput::InitTable(ITable *subTable) {
	m_table = subTable;
	UpdateTable();
}

ITable * DigitalInput::GetTable() {
	return m_table;
}
