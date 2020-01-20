#ifndef _NCA_H_
#define _NCA_H_

#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>

#include "nx/pfs0.h"
#include "nx/romfs.h"
#include "nx/ncm.h"


#define NCA_HEADER_SIZE             0x400
#define NCA_SECTION_HEADER_SIZE     0x200

#define NCA_SECTOR_SIZE             0x200
#define NCA_XTS_SECTION_SIZE        0xC00
#define NCA_SECTION_TOTAL           0x4
#define MEDIA_REAL(x)((x * 0x200))

#define NCA0_MAGIC                  0x3041434E
#define NCA2_MAGIC                  0x3241434E
#define NCA3_MAGIC                  0x3341434E

#define NCA_PROGRAM_FIRST_SECTION_HEADER_BLOCK_OFFSET   0x400
#define NCA_PROGRAM_SECOND_SECTION_HEADER_BLOCK_OFFSET  0x600
#define NCA_PROGRAM_THIRD_SECTION_HEADER_BLOCK_OFFSET   0x800

#define NCA_PROGRAM_LOGO_OFFSET     0x8000
#define NCA_META_CNMT_OFFSET        0xC20


typedef enum
{
    NcaEncrpytMode_Decrypt,
    NcaEncrpytMode_Encrypt
} NcaEncrpytMode;

typedef enum
{
    NcaDistributionType_System      = 0x0,
    NcaDistributionType_GameCard    = 0x1
} NcaDistributionType;

typedef enum
{
    NcaContentType_Program      = 0x0,
    NcaContentType_Meta         = 0x1,
    NcaContentType_Control      = 0x2,
    NcaContentType_Manual       = 0x3,
    NcaContentType_Data         = 0x4,
    NcaContentType_PublicData   = 0x5
} NcaContentType;

typedef enum
{
    NcaOldKeyGeneration_100     = 0x0,
    NcaOldKeyGeneration_Unused  = 0x1,
    NcaOldKeyGeneration_300     = 0x2,
} NcaOldKeyGeneration;

typedef enum
{
    NcaKeyGeneration_301        = 0x3,
    NcaKeyGeneration_400        = 0x4,
    NcaKeyGeneration_500        = 0x5,
    NcaKeyGeneration_600        = 0x6,
    NcaKeyGeneration_620        = 0x7,
    NcaKeyGeneration_700        = 0x8,
    NcaKeyGeneration_810        = 0x9,
    NcaKeyGeneration_900        = 0x0A,
    NcaKeyGeneration_910        = 0x0B,
    NcaKeyGeneration_Invalid    = 0xFF
} NcaKeyGeneration;

typedef enum
{
    NcaKeyAreaEncryptionKeyIndex_Application    = 0x0,
    NcaKeyAreaEncryptionKeyIndex_Ocean          = 0x1,
    NcaKeyAreaEncryptionKeyIndex_System         = 0x2
} NcaKeyAreaEncryptionKeyIndex;

typedef enum
{
    NcaFileSystemType_RomFS = 0x0,
    NcaFileSystemType_PFS0  = 0x1
} NcaFileSystemType;

typedef enum
{
    NcaHashType_PFS0    = 0x2,
    NcaHashType_RomFS   = 0x3
} NcaHashType;

typedef enum
{
    NcaEncryptionType_None      = 0x1,
    NcaEncryptionType_AesCtrOld = 0x2,
    NcaEncryptionType_AesCtr    = 0x3,
    NcaEncryptionType_AesCtrEx  = 0x4
} NcaEncryptionType;

typedef struct
{
    uint32_t media_start_offset; // divided by 0x200.
    uint32_t media_end_offset;   // divided by 0x200.
    uint32_t _0x8;               // unkown.
    uint32_t _0xC;               // unkown.
} nca_section_table_entry_t;

typedef struct
{
    uint16_t version;           // always 2.
    uint8_t fs_type;            // see NcaFileSystemType.
    uint8_t hash_type;          // see NcaHashType.
    uint8_t encryption_type;    // see NcaEncryptionType.
    uint8_t _0x5[0x3];          // empty.
    union
    {
        pfs0_superblock_t pfs0_sb;
        romfs_superblock_t romfs_sb;
        // anything else?????
    };
    union
    {
        uint8_t section_ctr[0x8];
        struct
        {
            uint32_t section_ctr_low;
            uint32_t section_ctr_high;
        };
    };
    uint8_t _0x148[0xB8];       // empty.
} nca_section_header_t;

typedef struct
{
    uint8_t sha256[0x20];
} nca_section_header_hash_t;

typedef struct
{
    uint8_t area[0x10];
} nca_key_area_t;

typedef struct
{
    uint8_t rights_id[0x10];
} rights_id_t;

typedef struct
{
    uint8_t rsa_fixed_key[0x100];
    uint8_t rsa_npdm[0x100];        // key from npdm.
    uint32_t magic;
    uint8_t distribution_type;      // see NcaDistributionType.
    uint8_t content_type;           // see NcaContentType.
    uint8_t old_key_gen;            // see NcaOldKeyGeneration.
    uint8_t kaek_index;             // see NcaKeyAreaEncryptionKeyIndex.
    size_t size;
    uint64_t title_id;
    uint32_t context_id;
    uint32_t sdk_version;
    uint8_t key_gen;                // see NcaKeyGeneration.
    uint8_t _0x221[0xF];            // empty.
    rights_id_t rights_id;

    nca_section_table_entry_t section_table[0x4];
    nca_section_header_hash_t section_header_hash[0x4];
    nca_key_area_t key_area[0x4];

    uint8_t _0x340[0xC0];           // empty.

    nca_section_header_t section_header[0x4];
} nca_header_t;


//
bool nca_check_if_magic_valid(uint32_t magic);

//
const char *nca_return_key_gen_string(uint8_t key_gen);

//
uint16_t nca_return_key_gen_int(uint8_t key_gen);

//
bool nca_check_key_gen(uint8_t key_gen);

//
const char *nca_get_string_from_id(NcmContentId nca_id, char *out);

//
NcmContentId nca_get_id_from_string(const char *nca_in_string);


/*
*   nca header stuff.
*/

//
bool nca_decrypt_key_area(const nca_header_t *header, nca_key_area_t *out);

//
void nca_encrypt_header(nca_header_t *header);

//
void nca_decrypt_header(nca_header_t *header);

//
bool nca_get_header(FILE *fp, uint64_t offset, nca_header_t *header);

//
bool nca_get_header_decrypted(FILE *fp, uint64_t offset, nca_header_t *header);



/*
*   nca install stuff.
*/

//
int nca_read(void *in);

//
int nca_write(void *in);

//
bool nca_setup_placeholder(ncm_install_struct_t *out, size_t size, NcmContentId *content_id, NcmStorageId storage_id);

//
bool nca_start_install(NcmContentId content_id, NcmStorageId storage_id);

#endif