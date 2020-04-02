#include <stdio.h>
#include <stdbool.h>
#include <switch.h>

#include "nx/es.h"
#include "util/log.h"


// globals.
Service g_es_service = {0};


bool es_start_service()
{
    Result rc = smGetService(&g_es_service, "es");
    if (R_FAILED(rc))
    {
        write_log("failed to start es service %s\n", __func__);
        return false;
    }
    return true;
}

void es_close_service()
{
    serviceClose(&g_es_service);
}

bool es_import_tik_and_cert(void const *tik_buf, size_t tik_size, void const *cert_buf, size_t cert_size)
{
    Result rc = serviceDispatch(&g_es_service, 1,
        .buffer_attrs = { SfBufferAttr_HipcMapAlias | SfBufferAttr_In, SfBufferAttr_HipcMapAlias | SfBufferAttr_In },
        .buffers = { { tik_buf, tik_size }, { cert_buf, cert_size } });

    if (R_FAILED(rc))
    {
        write_log("failed to import tik / cert\n");
        return false;
    }

    return true;
}

// not implemented //
bool es_import_ticket_certificate_set(void const *cert_buf, size_t cert_size)
{
    Result rc = serviceDispatch(&g_es_service, 2,
        .buffer_attrs = { SfBufferAttr_HipcMapAlias | SfBufferAttr_In },
        .buffers = { { cert_buf, cert_size } });

    if (R_FAILED(rc))
    {
        write_log("failed to import tik / cert\n");
        return false;
    }

    return true;
}

bool es_delete_common_tik(const FsRightsId *rights_id)
{
    Result rc = serviceDispatch(&g_es_service, 3,
        .buffer_attrs = { SfBufferAttr_HipcMapAlias | SfBufferAttr_In },
        .buffers = { { rights_id, sizeof(FsRightsId) } });

    if (R_FAILED(rc))
    {
        write_log("failed to delete common tik\n");
        return false;
    }

    return true;
}

bool es_delete_personalised_tik(const FsRightsId *rights_id)
{
    Result rc = serviceDispatch(&g_es_service, 4,
        .buffer_attrs = { SfBufferAttr_HipcMapAlias | SfBufferAttr_In },
        .buffers = { { rights_id, sizeof(FsRightsId) } });

    if (R_FAILED(rc))
    {
        write_log("failed to delete personalised tik\n");
        return false;
    }

    return true;
}

bool es_delete_all_common_tik(void)
{
    Result rc = serviceDispatch(&g_es_service, 5, SfOutHandleAttr_None);
    if (R_FAILED(rc))
    {
        write_log("failed to delete all common tickets\n");
        return false;
    }
    return true;
}

bool es_delete_all_personalised_tik(void)
{
    Result rc = serviceDispatch(&g_es_service, 6, SfOutHandleAttr_None);
    if (R_FAILED(rc))
    {
        write_log("failed to delete all personalised tickets\n");
        return false;
    }
    return true;
}

bool es_delete_all_personalised_tik_ex()
{
    Result rc = serviceDispatch(&g_es_service, 7, SfOutHandleAttr_None);
    if (R_FAILED(rc))
    {
        write_log("failed to delete all personalised tickets\n");
        return false;
    }
    return true;
}

uint32_t es_count_common_tik(void)
{
    uint32_t out_total = 0;
    Result rc = serviceDispatchOut(&g_es_service, 9, out_total, SfOutHandleAttr_None);
    if (R_FAILED(rc))
    {
        write_log("failed to count common tickets\n");
        return 0;
    }
    return out_total;
}

uint32_t es_count_personailsed_tik(void)
{
    uint32_t out_total = 0;
    Result rc = serviceDispatchOut(&g_es_service, 10, out_total, SfOutHandleAttr_None);
    if (R_FAILED(rc))
    {
        write_log("failed to count personalized tickets\n");
        return 0;
    }
    return out_total;
}

bool es_list_common_tik(FsRightsId *out, uint32_t count)
{
    uint32_t out_total = 0;
    uint64_t buffer_idk = 0;

    Result rc = serviceDispatchInOut(&g_es_service, 11, buffer_idk, out_total,
        .buffer_attrs = { SfBufferAttr_HipcMapAlias | SfBufferAttr_Out },
        .buffers = { { out, count * sizeof(FsRightsId) } });
        
    if (R_FAILED(rc))
    {
        write_log("failed to list common tickets\n");
        return false;
    }

    return true;
}

bool es_list_personalised_tik(FsRightsId *out, uint32_t count)
{
    uint32_t out_total = 0;
    uint64_t buffer_idk = 0;

    Result rc = serviceDispatchInOut(&g_es_service, 12, buffer_idk, out_total,
        .buffer_attrs = { SfBufferAttr_HipcMapAlias | SfBufferAttr_Out },
        .buffers = { { out, count * sizeof(FsRightsId) } });

    if (R_FAILED(rc))
    {
        write_log("failed to list personalised tickets\n");
        return false;
    }

    return true;
}

uint32_t es_get_common_tik_size(const FsRightsId *rights_id)
{
    uint32_t out_size = 0;
    uint64_t buffer_idk = 0;

    Result rc = serviceDispatchInOut(&g_es_service, 14, buffer_idk, out_size,
        .buffer_attrs = { SfBufferAttr_HipcMapAlias | SfBufferAttr_Out },
        .buffers = { { rights_id, sizeof(FsRightsId) } });

    if (R_FAILED(rc))
    {
        write_log("failed to get common tik size\n");
        return 0;
    }

    return out_size;
}

bool es_get_common_tik_data(void *out, size_t out_size, const FsRightsId *rights_id)
{
    uint64_t buffer_idk = 0;

    Result rc = serviceDispatchInOut(&g_es_service, 16, rights_id, buffer_idk,
        .buffer_attrs = { SfBufferAttr_HipcMapAlias | SfBufferAttr_Out },
        .buffers = { { out, out_size } });

    if (R_FAILED(rc))
    {
        write_log("failed to get common ticket data\n");
        return false;
    }

    return true;
}