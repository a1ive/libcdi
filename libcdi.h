#pragma once

#define VC_EXTRALEAN
#include <windows.h>

#define CDI_VERSION "9.1.1"

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
	CDI_STRING_SMART_KEY,
};

enum CDI_DISK_STATUS
{
	CDI_DISK_STATUS_UNKNOWN = 0,
	CDI_DISK_STATUS_GOOD,
	CDI_DISK_STATUS_CAUTION,
	CDI_DISK_STATUS_BAD
};

#define CDI_FLAG_USE_WMI				(1ULL << 0) // TRUE
#define CDI_FLAG_ADVANCED_SEARCH		(1ULL << 1) // FALSE
#define CDI_FLAG_WORKAROUND_HD204UI		(1ULL << 2) // FALSE
#define CDI_FLAG_WORKAROUND_ADATA		(1ULL << 3) // FALSE
#define CDI_FLAG_HIDE_NO_SMART			(1ULL << 4) // FALSE
#define CDI_FLAG_SORT_DRIVE_LETTER		(1ULL << 5) // FALSE
#define CDI_FLAG_NO_WAKEUP				(1ULL << 6) // FALSE
#define CDI_FLAG_ATA_PASS_THROUGH		(1ULL << 7) // TRUE

#define CDI_FLAG_ENABLE_NVIDIA			(1ULL << 8)  // TRUE
#define CDI_FLAG_ENABLE_MARVELL			(1ULL << 9)  // TRUE
#define CDI_FLAG_ENABLE_USB_SAT			(1ULL << 10) // TRUE
#define CDI_FLAG_ENABLE_USB_SUNPLUS		(1ULL << 11) // TRUE
#define CDI_FLAG_ENABLE_USB_IODATA		(1ULL << 12) // TRUE
#define CDI_FLAG_ENABLE_USB_LOGITEC		(1ULL << 13) // TRUE
#define CDI_FLAG_ENABLE_USB_PROLIFIC	(1ULL << 14) // TRUE
#define CDI_FLAG_ENABLE_USB_JMICRON		(1ULL << 15) // TRUE
#define CDI_FLAG_ENABLE_USB_CYPRESS		(1ULL << 16) // TRUE
#define CDI_FLAG_ENABLE_USB_MEMORY		(1ULL << 17) // TRUE
#define CDI_FLAG_ENABLE_NVME_JMICRON3	(1ULL << 18) // FALSE
#define CDI_FLAG_ENABLE_NVME_JMICRON	(1ULL << 19) // TRUE
#define CDI_FLAG_ENABLE_NVME_ASMEDIA	(1ULL << 20) // TRUE
#define CDI_FLAG_ENABLE_NVME_REALTEK	(1ULL << 21) // TRUE
#define CDI_FLAG_ENABLE_MEGA_RAID		(1ULL << 22) // TRUE
#define CDI_FLAG_ENABLE_INTEL_VROC		(1ULL << 23) // TRUE
#define CDI_FLAG_ENABLE_ASM1352R		(1ULL << 24) // TRUE
#define CDI_FLAG_ENABLE_AMD_RC2			(1ULL << 25) // FALSE
//#define CDI_FLAG_ENABLE_JMS56X			(1ULL << 26) // FALSE
//#define CDI_FLAG_ENABLE_JMB39X			(1ULL << 27) // FALSE

#define CDI_FLAG_DEFAULT \
	( \
		CDI_FLAG_USE_WMI | \
		CDI_FLAG_ATA_PASS_THROUGH | \
		CDI_FLAG_ENABLE_NVIDIA | \
		CDI_FLAG_ENABLE_MARVELL | \
		CDI_FLAG_ENABLE_USB_SAT | \
		CDI_FLAG_ENABLE_USB_SUNPLUS | \
		CDI_FLAG_ENABLE_USB_IODATA | \
		CDI_FLAG_ENABLE_USB_LOGITEC | \
		CDI_FLAG_ENABLE_USB_PROLIFIC | \
		CDI_FLAG_ENABLE_USB_JMICRON | \
		CDI_FLAG_ENABLE_USB_CYPRESS | \
		CDI_FLAG_ENABLE_USB_MEMORY | \
		CDI_FLAG_ENABLE_NVME_JMICRON | \
		CDI_FLAG_ENABLE_NVME_ASMEDIA | \
		CDI_FLAG_ENABLE_NVME_REALTEK | \
		CDI_FLAG_ENABLE_MEGA_RAID | \
		CDI_FLAG_ENABLE_INTEL_VROC | \
		CDI_FLAG_ENABLE_ASM1352R \
	)

#ifdef __cplusplus

typedef CAtaSmart CDI_SMART;

#else

typedef struct _CDI_SMART CDI_SMART;

CDI_SMART*	WINAPI cdi_create_smart(VOID);
VOID		WINAPI cdi_destroy_smart(CDI_SMART* ptr);
VOID		WINAPI cdi_init_smart(CDI_SMART* ptr, UINT64 flags);
DWORD		WINAPI cdi_update_smart(CDI_SMART* ptr, INT index);
INT			WINAPI cdi_get_disk_count(CDI_SMART* ptr);

BOOL		WINAPI cdi_get_bool(CDI_SMART* ptr, INT index, enum CDI_ATA_BOOL attr);
INT			WINAPI cdi_get_int(CDI_SMART* ptr, INT index, enum CDI_ATA_INT attr);
DWORD		WINAPI cdi_get_dword(CDI_SMART* ptr, INT index, enum CDI_ATA_DWORD attr);
WCHAR*		WINAPI cdi_get_string(CDI_SMART* ptr, INT index, enum CDI_ATA_STRING attr);
VOID		WINAPI cdi_free_string(WCHAR* ptr);

WCHAR*		WINAPI cdi_get_smart_format(CDI_SMART* ptr, INT index);
BYTE		WINAPI cdi_get_smart_id(CDI_SMART* ptr, INT index, INT attr);
WCHAR*		WINAPI cdi_get_smart_value(CDI_SMART* ptr, INT index, INT attr, BOOL hex);
INT			WINAPI cdi_get_smart_status(CDI_SMART* ptr, INT index, INT attr);
WCHAR*		WINAPI cdi_get_smart_name(CDI_SMART* ptr, INT index, BYTE id);

#endif
