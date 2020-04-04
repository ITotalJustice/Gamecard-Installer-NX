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
    NcaContentType_PublicData   = 0x5,
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
    NcaHashType_Auto                    = 0x0,
    NcaHashType_HierarchicalSha256      = 0x2,
    NcaHashType_HierarchicalIntegrity   = 0x3
} NcaHashType;

typedef enum
{
    NcaEncryptionType_Auto      = 0x0,
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
} NcaSectionTableEntry_t;

typedef struct
{
    uint64_t offset;
    uint64_t size;
    uint32_t magic; // BKTR
    uint8_t _0x14[0x4];
    uint8_t _0x18[0x4];
    uint8_t _0x1C[0x4];
} PatchInfo_t;

typedef struct
{
    uint16_t version;           // always 2.
    uint8_t fs_type;            // see NcaFileSystemType.
    uint8_t hash_type;          // see NcaHashType.
    uint8_t encryption_type;    // see NcaEncryptionType.
    uint8_t _0x5[0x3];          // empty.
    union
    {
        uint8_t section_data[0xF8];
        Pfs0Superblock_t pfs0_sb;
        RomfsSuperblock_t romfs_sb;
        // anything else?????
    };
    PatchInfo_t patch_info1;
    PatchInfo_t patch_info2;
    union
    {
        uint64_t section_ctr;
        struct
        {
            uint32_t section_ctr_low;
            uint32_t section_ctr_high;
        };
    };
    uint8_t sparse_info[0x30];
    uint8_t _0x178[0x88];       /* Padding. */
} NcaFsHeader_t;

typedef struct
{
    uint8_t sha256[0x20];
} NcaSectionHeaderHash_t;

typedef struct
{
    uint8_t area[0x10];
} NcaKeyArea_t;

typedef struct
{
    uint8_t rights_id[0x10];
} RightsId_t;

typedef struct
{
    uint64_t id;        // belonging ID.
    NcaKeyArea_t key;   // the actual key.
} NcaKeySlot_t;

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
    uint8_t header_1_sig_key_gen;
    uint8_t _0x222[0xE];            // empty.
    FsRightsId rights_id;

    NcaSectionTableEntry_t section_table[NCA_SECTION_TOTAL];
    NcaSectionHeaderHash_t section_header_hash[NCA_SECTION_TOTAL];
    NcaKeyArea_t key_area[NCA_SECTION_TOTAL];

    uint8_t _0x340[0xC0];           // empty.

    NcaFsHeader_t section_header[NCA_SECTION_TOTAL];
} NcaHeader_t;


//
bool nca_check_if_magic_valid(uint32_t magic);

//
const char *nca_get_magic_string(uint32_t magic);
const char *nca_get_distribution_type_string(NcaDistributionType type);
const char *nca_get_content_type_string(NcaContentType type);
const char *nca_get_keak_index_string(NcaKeyAreaEncryptionKeyIndex index);
const char *nca_get_file_system_type_string(NcaFileSystemType type);
const char *nca_get_hash_type_string(NcaHashType type);
const char *nca_get_encryption_type_string(NcaEncryptionType type);
const char *nca_return_key_gen_string(uint8_t key_gen);

//
uint16_t nca_return_key_gen_int(uint8_t key_gen);
bool nca_check_key_gen(uint8_t key_gen);

//
const char *nca_get_string_from_id(const NcmContentId *nca_id, char *out);
const NcmContentId nca_get_id_from_string(const char *nca_in_string);


bool nca_set_keyslot(uint64_t id, const uint8_t *key);
NcaKeySlot_t *nca_get_keyslot(void);
NcaKeyArea_t *nca_get_keyslot_key(void);
uint64_t nca_get_keyslot_id(void);


/*
*   nca header stuff.
*/

//
void nca_update_lower_ctr(uint8_t *ctr, uint64_t offset);
void nca_update_upper_ctr(uint8_t *ctr, uint64_t offset);

//
bool nca_decrypt_keak(const NcaHeader_t *header, NcaKeyArea_t *out);
bool nca_encrypt_keak(NcaHeader_t *header, const NcaKeyArea_t *decrypted_key, uint8_t key_gen);

//
bool nca_encrypt_header(const NcaHeader_t *in_header, NcaHeader_t *out_header);
bool nca_decrypt_header(const NcaHeader_t *in_header, NcaHeader_t *out_header);

//
bool nca_get_header(FILE *fp, uint64_t offset, NcaHeader_t *header);
bool nca_get_header_decrypted(FILE *fp, uint64_t offset, NcaHeader_t *header);



/*
*   nca install stuff.
*/

//
bool nca_setup_placeholder(NcmInstall_t *out, size_t size, NcmContentId *content_id, NcmStorageId storage_id);

//
bool nca_start_install(const char *name, const NcmContentId *content_id, uint64_t offset, NcmStorageId storage_id, FILE *fp);


/*
*   Debug
*/

//
void nca_print_header(const NcaHeader_t *header);
void nca_print_key_area(const NcaKeyArea_t *keak);

#endif