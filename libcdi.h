#pragma once

#define VC_EXTRALEAN
#include <windows.h>

#define CDI_VERSION "9.0.1"

enum CDI_ATA_BOOL
{
	CDI_BOOL_SSD = 0,
	CDI_BOOL_SSD_NVME,
	CDI_BOOL_SMART,
	CDI_BOOL_LBA48,
	CDI_BOOL_AAM,
	CDI_BOOL_APM,
	CDI_BOOL_NCQ,
	CDI_BOOL_NV_CACHE,
	CDI_BOOL_DEVSLP,
	CDI_BOOL_STREAMING,
	CDI_BOOL_GPL,
	CDI_BOOL_TRIM,
	CDI_BOOL_VOLATILE_WRITE_CACHE,
	CDI_BOOL_SMART_ENABLED,
	CDI_BOOL_AAM_ENABLED,
	CDI_BOOL_APM_ENABLED,
};

enum CDI_ATA_INT
{
	CDI_INT_DISK_ID = 0,
	CDI_INT_DISK_STATUS,
	CDI_INT_SCSI_PORT,
	CDI_INT_SCSI_TARGET_ID,
	CDI_INT_SCSI_BUS,
	CDI_INT_POWER_ON_HOURS,
	CDI_INT_TEMPERATURE,
	CDI_INT_TEMPERATURE_ALARM,
	CDI_INT_HOST_WRITES,
	CDI_INT_HOST_READS,
	CDI_INT_NAND_WRITES,
	CDI_INT_GB_ERASED,
	CDI_INT_WEAR_LEVELING_COUNT,
	CDI_INT_LIFE,
	CDI_INT_MAX_ATTRIBUTE,
};

enum CDI_ATA_DWORD
{
	CDI_DWORD_DISK_SIZE = 0,
	CDI_DWORD_LOGICAL_SECTOR_SIZE,
	CDI_DWORD_PHYSICAL_SECTOR_SIZE,
	CDI_DWORD_BUFFER_SIZE,
	CDI_DWORD_ATTR_COUNT,
	CDI_DWORD_POWER_ON_COUNT,
	CDI_DWORD_ROTATION_RATE,
	CDI_DWORD_DRIVE_LETTER,
	CDI_DWORD_DISK_STATUS,
	CDI_DWORD_DISK_VENDOR_ID, // SSD_VENDOR_NVME = 19
};

enum CDI_ATA_STRING
{
	CDI_STRING_SN,
	CDI_STRING_FIRMWARE,
	CDI_STRING_MODEL,
	CDI_STRING_DRIVE_MAP,
	CDI_STRING_TRANSFER_MODE_MAX,
	CDI_STRING_TRANSFER_MODE_CUR,
	CDI_STRING_INTERFACE,
	CDI_STRING_VERSION_MAJOR,
	CDI_STRING_VERSION_MINOR,
	CDI_STRING_PNP_ID,
};

enum CDI_DISK_STATUS
{
	CDI_DISK_STATUS_UNKNOWN = 0,
	CDI_DISK_STATUS_GOOD,
	CDI_DISK_STATUS_CAUTION,
	CDI_DISK_STATUS_BAD
};

#ifdef __cplusplus

typedef CAtaSmart CDI_SMART;

#else

typedef struct _CDI_SMART CDI_SMART;

__declspec(dllexport) CDI_SMART*	WINAPI cdi_create_smart(VOID);
__declspec(dllexport) VOID			WINAPI cdi_destroy_smart(CDI_SMART* ptr);
__declspec(dllexport) VOID			WINAPI cdi_init_smart(CDI_SMART* ptr, BOOL sort_drive_letter);
__declspec(dllexport) DWORD			WINAPI cdi_update_smart(CDI_SMART* ptr, INT index);
__declspec(dllexport) INT			WINAPI cdi_get_disk_count(CDI_SMART* ptr);

__declspec(dllexport) BOOL			WINAPI cdi_get_bool(CDI_SMART* ptr, INT index, enum CDI_ATA_BOOL attr);
__declspec(dllexport) INT			WINAPI cdi_get_int(CDI_SMART* ptr, INT index, enum CDI_ATA_INT attr);
__declspec(dllexport) DWORD			WINAPI cdi_get_dword(CDI_SMART* ptr, INT index, enum CDI_ATA_DWORD attr);
__declspec(dllexport) CHAR*			WINAPI cdi_get_string(CDI_SMART* ptr, INT index, enum CDI_ATA_STRING attr);
__declspec(dllexport) VOID			WINAPI cdi_free_string(CHAR* ptr);

__declspec(dllexport) CHAR* WINAPI cdi_get_smart_format(CDI_SMART* ptr, INT index);
__declspec(dllexport) CHAR*			WINAPI cdi_get_smart_value(CDI_SMART* ptr, INT index, INT attr);
__declspec(dllexport) INT			WINAPI cdi_get_smart_status(CDI_SMART* ptr, INT index, INT attr);

#endif
