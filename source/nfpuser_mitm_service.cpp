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

#include <mutex>
#include <switch.h>
#include "nfpuser_mitm_service.hpp"

extern FILE* g_logging_file;

void NfpUserMitmService::PostProcess(IMitmServiceObject *obj, IpcResponseContext *ctx) {
    fprintf(g_logging_file, "PostProcessing command %u\n", (u32)ctx->cmd_id);
    fflush(g_logging_file);
}

Result NfpUserMitmService::CreateUserInterface(Out<std::shared_ptr<NfpUserInterface>> out_storage) {
    fprintf(g_logging_file, "In NfpUserMitmService::CreateUserInterface\n");
    fflush(g_logging_file);

    std::shared_ptr<NfpUserInterface> storage = std::make_shared<NfpUserInterface>();
    out_storage.SetValue(std::move(storage));

    if (out_storage.IsDomain()) {
        fprintf(g_logging_file, "Is domain with objid %u\n", out_storage.GetObjectId());
        fflush(g_logging_file);

        out_storage.ChangeObjectId(2);
    }
    return 0;
}