/*
 * Copyright (c) 2018 Atmosphère-NX
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
 
#pragma once
#include <array>
#include <switch.h>
#include <stratosphere.hpp>

enum NfpUserInterfaceCmd : u32 {
    NfpUserInterfaceCmd_Initialize = 0,
    NfpUserInterfaceCmd_Finalize = 1,
    NfpUserInterfaceCmd_ListDevices = 2,
    NfpUserInterfaceCmd_StartDetection = 3,
    NfpUserInterfaceCmd_StopDetection = 4,
    NfpUserInterfaceCmd_Mount = 5,
    NfpUserInterfaceCmd_Unmount = 6,
    NfpUserInterfaceCmd_OpenApplicationArea = 7,
    NfpUserInterfaceCmd_GetApplicationArea = 8,




    NfpUserInterfaceCmd_GetTagInfo = 13,
    NfpUserInterfaceCmd_GetRegisterInfo = 14,
    NfpUserInterfaceCmd_GetCommonInfo = 15,
    NfpUserInterfaceCmd_GetModelInfo = 16,
    NfpUserInterfaceCmd_AttachActivateEvent = 17,
    NfpUserInterfaceCmd_AttachDeactivateEvent = 18,
    NfpUserInterfaceCmd_GetState = 19,
    NfpUserInterfaceCmd_GetDeviceState = 20,
    NfpUserInterfaceCmd_GetNpadId = 21,
    NfpUserInterfaceCmd_GetApplicationAreaSize = 22,
    NfpUserInterfaceCmd_AttachAvailabilityChangeEvent = 23,
};

struct TagInfo {
    std::array<u8, 10> uuid;
    u8 uuid_length; // TODO(ogniK): Figure out if this is actual the uuid length or does it
                    // mean something else
    u8 pad1[0x15];
    u32 protocol;
    u32 tag_type;
    u8 pad2[0x2c];
};
static_assert(sizeof(TagInfo) == 0x54, "TagInfo is an invalid size");

enum class State : u32 {
    NonInitialized = 0,
    Initialized = 1,
};

enum class DeviceState : u32 {
    Initialized = 0,
    SearchingForTag = 1,
    TagFound = 2,
    TagRemoved = 3,
    TagNearby = 4,
    Unknown5 = 5,
    Finalized = 6
};

struct CommonInfo {
    u16 last_write_year; // be
    u8 last_write_month;
    u8 last_write_day;
    u16 write_counter; // be
    u16 version; // be
    u32 application_area_size; // be
    u8 padding[0x34];
};
static_assert(sizeof(CommonInfo) == 0x40, "CommonInfo is an invalid size");

class NfpUserInterface : public IServiceObject {
    public:
        NfpUserInterface();
        ~NfpUserInterface();
        
    private:
        /* Actual command API. */
        virtual Result Initialize(u64 unk, u64 unk2, PidDescriptor pid_desc, InBuffer<u8> buf) final;
        virtual Result GetState(Out<u32> state) final;
        virtual Result ListDevices(OutBuffer<u8> buffer, Out<u32> size) final;
        virtual Result GetNpadId(u64 handle, Out<u32> npad_id) final;
        virtual Result AttachActivateEvent(u64 handle, Out<CopiedHandle> event) final;
        virtual Result AttachDeactivateEvent(u64 handle, Out<CopiedHandle> event) final;
        virtual Result StopDetection() final;
        virtual Result GetDeviceState(Out<u32> state) final;
        virtual Result StartDetection() final;
        virtual Result GetTagInfo(OutBuffer<u8> buffer) final;
        virtual Result Mount() final;
        virtual Result GetModelInfo(OutBuffer<u8> buffer) final;
        virtual Result Unmount() final;
        virtual Result Finalize() final;
        virtual Result AttachAvailabilityChangeEvent(Out<CopiedHandle> event) final;
        virtual Result GetRegisterInfo() final;
        virtual Result GetCommonInfo(OutBuffer<u8> buffer) final;
        virtual Result OpenApplicationArea() final;
        virtual Result GetApplicationAreaSize(Out<u32> size) final;
        virtual Result GetApplicationArea(Out<u32> unk) final;

        bool has_attached_handle{};
        const u64 device_handle{0x555A5559}; // 'YUZU'
        const u32 npad_id{0}; // Player 1 controller
        State state{State::NonInitialized};
        DeviceState device_state{DeviceState::Initialized};
        IEvent* deactivate_event;
        IEvent* availability_change_event;

    public:
        DEFINE_SERVICE_DISPATCH_TABLE {
            MakeServiceCommandMeta<NfpUserInterfaceCmd_Initialize, &NfpUserInterface::Initialize>(),
            MakeServiceCommandMeta<NfpUserInterfaceCmd_Finalize, &NfpUserInterface::Finalize>(),
            MakeServiceCommandMeta<NfpUserInterfaceCmd_ListDevices, &NfpUserInterface::ListDevices>(),
            MakeServiceCommandMeta<NfpUserInterfaceCmd_StartDetection, &NfpUserInterface::StartDetection>(),
            MakeServiceCommandMeta<NfpUserInterfaceCmd_StopDetection, &NfpUserInterface::StopDetection>(),
            MakeServiceCommandMeta<NfpUserInterfaceCmd_Mount, &NfpUserInterface::Mount>(),
            MakeServiceCommandMeta<NfpUserInterfaceCmd_Unmount, &NfpUserInterface::Unmount>(),
            MakeServiceCommandMeta<NfpUserInterfaceCmd_OpenApplicationArea, &NfpUserInterface::OpenApplicationArea>(),
            MakeServiceCommandMeta<NfpUserInterfaceCmd_GetApplicationArea, &NfpUserInterface::GetApplicationArea>(),
            MakeServiceCommandMeta<NfpUserInterfaceCmd_GetTagInfo, &NfpUserInterface::GetTagInfo>(),
            MakeServiceCommandMeta<NfpUserInterfaceCmd_GetRegisterInfo, &NfpUserInterface::GetRegisterInfo>(),
            MakeServiceCommandMeta<NfpUserInterfaceCmd_GetCommonInfo, &NfpUserInterface::GetCommonInfo>(),
            MakeServiceCommandMeta<NfpUserInterfaceCmd_GetModelInfo, &NfpUserInterface::GetModelInfo>(),
            MakeServiceCommandMeta<NfpUserInterfaceCmd_AttachActivateEvent, &NfpUserInterface::AttachActivateEvent>(),
            MakeServiceCommandMeta<NfpUserInterfaceCmd_AttachDeactivateEvent, &NfpUserInterface::AttachDeactivateEvent>(),
            MakeServiceCommandMeta<NfpUserInterfaceCmd_GetState, &NfpUserInterface::GetState>(),
            MakeServiceCommandMeta<NfpUserInterfaceCmd_GetDeviceState, &NfpUserInterface::GetDeviceState>(),
            MakeServiceCommandMeta<NfpUserInterfaceCmd_GetNpadId, &NfpUserInterface::GetNpadId>(),
            MakeServiceCommandMeta<NfpUserInterfaceCmd_GetApplicationAreaSize, &NfpUserInterface::GetApplicationAreaSize>(),            
            MakeServiceCommandMeta<NfpUserInterfaceCmd_AttachAvailabilityChangeEvent, &NfpUserInterface::AttachAvailabilityChangeEvent>(),
        };
};
