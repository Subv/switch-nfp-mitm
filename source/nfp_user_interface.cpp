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
 
#include <atomic>
#include <cstdio>
#include <array>
#include <switch.h>

#include "nfp_user_interface.hpp"


extern std::atomic_bool g_key_combo_triggered;
extern IEvent* g_activate_event;
extern FILE* g_logging_file;

struct ModelInfo {
    std::array<u8, 0x8> amiibo_identification_block;
    u8 padding[0x38];
};
static_assert(sizeof(ModelInfo) == 0x40, "ModelInfo is an invalid size");

struct AmiiboFile {
    std::array<u8, 10> uuid;
    u8 padding[0x4a];
    ModelInfo model_info;
};
static_assert(sizeof(AmiiboFile) == 0x94, "AmiiboFile is an invalid size");

static AmiiboFile GetAmiibo() {
    AmiiboFile amiibo{};

    FILE* file = fopen("amiibo.bin", "rb");
    if (!file) {
        fatalSimple(MAKERESULT(Module_Libnx, 42));
    }
    fseek(file, 0, SEEK_END);
    long fsize = ftell(file);
    fseek(file, 0, SEEK_SET);

    fread(&amiibo, fsize, 1, file);
    fclose(file);
    return amiibo;
}

NfpUserInterface::NfpUserInterface() {
    fprintf(g_logging_file, "Creating NfpUserInterface\n");
    fflush(g_logging_file);

    deactivate_event = CreateWriteOnlySystemEvent<true>();
    availability_change_event = CreateWriteOnlySystemEvent<true>();
}

NfpUserInterface::~NfpUserInterface() {
    fprintf(g_logging_file, "Destroying NfpUserInterface\n");
    fflush(g_logging_file);

    delete deactivate_event;
    delete availability_change_event;
}

Result NfpUserInterface::Initialize(u64 unk, u64 unk2, PidDescriptor pid_desc, InBuffer<u8> buf) {
    fprintf(g_logging_file, "Calling initialize\n");
    fflush(g_logging_file);

    state = State::Initialized;
    device_state = DeviceState::Initialized;
    return 0;
}

Result NfpUserInterface::Finalize() {
    fprintf(g_logging_file, "Calling finalize\n");
    fflush(g_logging_file);
    state = State::NonInitialized;
    device_state = DeviceState::Finalized;
    return 0;
}

Result NfpUserInterface::GetState(Out<u32> out_state) {
    fprintf(g_logging_file, "In GetState\n");
    fflush(g_logging_file);
    out_state.SetValue(static_cast<u32>(state));
    return 0;
}

Result NfpUserInterface::ListDevices(OutBuffer<u8> buffer, Out<u32> size) {
    fprintf(g_logging_file, "In ListDevices\n");
    fflush(g_logging_file);
    memcpy(buffer.buffer, &device_handle, sizeof(device_handle));
    size.SetValue(1);
    return 0;
}

Result NfpUserInterface::GetNpadId(u64 handle, Out<u32> out_npad_id) {
    fprintf(g_logging_file, "In GetNpadId\n");
    fflush(g_logging_file);
    out_npad_id.SetValue(npad_id);
    return 0;
}

Result NfpUserInterface::AttachActivateEvent(u64 handle, Out<CopiedHandle> event) {
    fprintf(g_logging_file, "In AttachActivateEvent\n");
    fflush(g_logging_file);
    event.SetValue(g_activate_event->GetHandle());
    has_attached_handle = true;
    return 0;
}

Result NfpUserInterface::AttachDeactivateEvent(u64 handle, Out<CopiedHandle> event) {
    fprintf(g_logging_file, "In AttachDeactivateEvent\n");
    fflush(g_logging_file);
    event.SetValue(deactivate_event->GetHandle());
    return 0;
}

Result NfpUserInterface::StopDetection() {
    fprintf(g_logging_file, "In StopDetection\n");
    fflush(g_logging_file);
    switch (device_state) {
    case DeviceState::TagFound:
    case DeviceState::TagNearby:
        deactivate_event->Signal();
        device_state = DeviceState::Initialized;
        break;
    case DeviceState::SearchingForTag:
    case DeviceState::TagRemoved:
        device_state = DeviceState::Initialized;
        break;
    }
    return 0;
}

Result NfpUserInterface::GetDeviceState(Out<u32> out_state) {
    fprintf(g_logging_file, "In GetDeviceState\n");
    fflush(g_logging_file);
    if (g_key_combo_triggered && !has_attached_handle) {
        fprintf(g_logging_file, "Triggered GetDeviceState\n");
        fflush(g_logging_file);

        device_state = DeviceState::TagFound;
        g_key_combo_triggered = false;
        g_activate_event->Clear();
    }

    out_state.SetValue(static_cast<u32>(device_state));
    return 0;
}

Result NfpUserInterface::StartDetection() {
    fprintf(g_logging_file, "In StartDetection\n");
    fflush(g_logging_file);
    if (device_state == DeviceState::Initialized || device_state == DeviceState::TagRemoved) {
        device_state = DeviceState::SearchingForTag;
    }
    return 0;
}

Result NfpUserInterface::GetTagInfo(OutBuffer<u8> buffer) {
    auto amiibo = GetAmiibo();

    TagInfo tag_info{};
    tag_info.uuid = amiibo.uuid;
    tag_info.uuid_length = static_cast<u8>(tag_info.uuid.size());

    tag_info.protocol = 1; // TODO(ogniK): Figure out actual values
    tag_info.tag_type = 2;

    memcpy(buffer.buffer, &tag_info, sizeof(tag_info));
    return 0;
}

Result NfpUserInterface::Mount() {
    device_state = DeviceState::TagNearby;
    return 0;
}

Result NfpUserInterface::GetModelInfo(OutBuffer<u8> buffer) {
    auto amiibo = GetAmiibo();
    memcpy(buffer.buffer, &amiibo.model_info, sizeof(amiibo.model_info));
    return 0;
}

Result NfpUserInterface::Unmount() {
    device_state = DeviceState::TagFound;
    return 0;
}

Result NfpUserInterface::AttachAvailabilityChangeEvent(Out<CopiedHandle> event) {
    event.SetValue(availability_change_event->GetHandle());
    return 0;
}

Result NfpUserInterface::GetRegisterInfo() {
    return 0;
}

Result NfpUserInterface::GetCommonInfo(OutBuffer<u8> buffer) {
    CommonInfo common_info{};
    common_info.application_area_size = 0;
    memcpy(buffer.buffer, &common_info, sizeof(CommonInfo));
    return 0;
}

Result NfpUserInterface::OpenApplicationArea() {
    return 0;
}

Result NfpUserInterface::GetApplicationAreaSize(Out<u32> size) {
    size.SetValue(0);
    return 0;
}

Result NfpUserInterface::GetApplicationArea(Out<u32> unk) {
    unk.SetValue(0);
    return 0;
}