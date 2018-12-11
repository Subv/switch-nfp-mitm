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
 
#include <cstdlib>
#include <cstdint>
#include <cstring>
#include <cstdio>
#include <atomic>
#include <malloc.h>

#include <switch.h>
#include <atmosphere.h>
#include <stratosphere.hpp>

#include "nfpuser_mitm_service.hpp"

extern "C" {
    extern u32 __start__;

    u32 __nx_applet_type = AppletType_None;

    #define INNER_HEAP_SIZE 0x20000
    size_t nx_inner_heap_size = INNER_HEAP_SIZE;
    char   nx_inner_heap[INNER_HEAP_SIZE];
    
    void __libnx_initheap(void);
    void __appInit(void);
    void __appExit(void);
}


void __libnx_initheap(void) {
	void*  addr = nx_inner_heap;
	size_t size = nx_inner_heap_size;

	/* Newlib */
	extern char* fake_heap_start;
	extern char* fake_heap_end;

	fake_heap_start = (char*)addr;
	fake_heap_end   = (char*)addr + size;
}

void __appInit(void) {
    Result rc;
    
    rc = smInitialize();
    if (R_FAILED(rc)) {
        fatalSimple(MAKERESULT(Module_Libnx, LibnxError_InitFail_SM));
    }
    
    rc = fsInitialize();
    if (R_FAILED(rc)) {
        fatalSimple(rc);
    }

    rc = fsdevMountSdmc();
    if (R_FAILED(rc)) {
        fatalSimple(rc);
    }
    
    //CheckAtmosphereVersion(CURRENT_ATMOSPHERE_VERSION);
}

void __appExit(void) {
    /* Cleanup services. */
    fsdevUnmountAll();
    fsExit();
    smExit();
}

struct NfpUserManagerOptions {
    static const size_t PointerBufferSize = 0x100;
    static const size_t MaxDomains = 4;
    static const size_t MaxDomainObjects = 0x100;
};

using NfpMitmManager = WaitableManager<NfpUserManagerOptions>;

std::atomic_bool g_key_combo_triggered = false;
IEvent* g_activate_event = nullptr;
FILE* g_logging_file = nullptr;

void HidLoop(void* arg) {
    while (true) {
        if (R_FAILED(hidInitialize())) {
            fatalSimple(MAKERESULT(Module_Libnx, LibnxError_InitFail_HID));
        }
        
        hidScanInput();
        auto keys = hidKeysDown(CONTROLLER_P1_AUTO);
        if (!g_key_combo_triggered && ((keys & (KEY_R | KEY_L)) == (KEY_R | KEY_L))) {
            fprintf(g_logging_file, "Key combo triggered\n");
            fflush(g_logging_file);
            g_key_combo_triggered = true;
            g_activate_event->Signal();
            // RebootToRcm();
        }
        
        hidExit();

        svcSleepThread(100000000);
    }
    svcExitThread();
}

int main(int argc, char **argv) {
    g_logging_file = fopen("nfp_log.log", "a");

    g_activate_event = CreateWriteOnlySystemEvent<true>();

    Thread hid_poller_thread = {0};
    consoleDebugInit(debugDevice_SVC);
    
    if (R_FAILED(threadCreate(&hid_poller_thread, &HidLoop, NULL, 0x4000, 0x15, -2))) {
        fatalSimple(MAKERESULT(Module_Libnx, 40));
        return -1;
    }
    if (R_FAILED(threadStart(&hid_poller_thread))) {
        fatalSimple(MAKERESULT(Module_Libnx, 41));
        return -1;
    }
        
    /* TODO: What's a good timeout value to use here? */
    auto server_manager = new NfpMitmManager(1);
        
    /* Create fsp-srv mitm. */
    AddMitmServerToManager<NfpUserMitmService>(server_manager, "nfp:user", 4);

    /* Loop forever, servicing our services. */
    server_manager->Process();

    fclose(g_logging_file);
    
    delete server_manager;

    return 0;
}

