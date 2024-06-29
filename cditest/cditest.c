
#define VC_EXTRALEAN
#include <windows.h>
#include <stdio.h>

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
#define CDI_FLAG_ADVANCED_SEARCH		(1ULL << 1) // TRUE
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

/* CDI_FLAG_USE_WMI | 
 * CDI_FLAG_ATA_PASS_THROUGH |
 * CDI_FLAG_ENABLE_NVIDIA |
 * CDI_FLAG_ENABLE_MARVELL |
 * CDI_FLAG_ENABLE_USB_SAT |
 * CDI_FLAG_ENABLE_USB_SUNPLUS |
 * CDI_FLAG_ENABLE_USB_IODATA |
 * CDI_FLAG_ENABLE_USB_LOGITEC |
 * CDI_FLAG_ENABLE_USB_PROLIFIC |
 * CDI_FLAG_ENABLE_USB_JMICRON |
 * CDI_FLAG_ENABLE_USB_CYPRESS |
 * CDI_FLAG_ENABLE_USB_MEMORY |
 * CDI_FLAG_ENABLE_NVME_JMICRON |
 * CDI_FLAG_ENABLE_NVME_ASMEDIA |
 * CDI_FLAG_ENABLE_NVME_REALTEK |
 * CDI_FLAG_ENABLE_MEGA_RAID |
 * CDI_FLAG_ENABLE_INTEL_VROC |
 * CDI_FLAG_ENABLE_ASM1352R
*/
#define CDI_FLAG_DEFAULT 0x01FBFF81ULL

typedef struct _CDI_SMART CDI_SMART;

CONST CHAR*	(WINAPI *cdi_get_version)(VOID);

CDI_SMART*	(WINAPI *cdi_create_smart)(VOID);
VOID		(WINAPI *cdi_destroy_smart)(CDI_SMART* ptr);
VOID		(WINAPI *cdi_init_smart)(CDI_SMART* ptr, UINT64 flags);
DWORD		(WINAPI *cdi_update_smart)(CDI_SMART* ptr, INT index);
INT			(WINAPI *cdi_get_disk_count)(CDI_SMART* ptr);

BOOL		(WINAPI *cdi_get_bool)(CDI_SMART* ptr, INT index, enum CDI_ATA_BOOL attr);
INT			(WINAPI *cdi_get_int)(CDI_SMART* ptr, INT index, enum CDI_ATA_INT attr);
DWORD		(WINAPI *cdi_get_dword)(CDI_SMART* ptr, INT index, enum CDI_ATA_DWORD attr);
WCHAR*		(WINAPI *cdi_get_string)(CDI_SMART* ptr, INT index, enum CDI_ATA_STRING attr);
VOID		(WINAPI *cdi_free_string)(WCHAR* ptr);

WCHAR*		(WINAPI *cdi_get_smart_format)(CDI_SMART* ptr, INT index);
BYTE		(WINAPI *cdi_get_smart_id)(CDI_SMART* ptr, INT index, INT attr);
WCHAR*		(WINAPI *cdi_get_smart_value)(CDI_SMART* ptr, INT index, INT attr, BOOL hex);
INT			(WINAPI *cdi_get_smart_status)(CDI_SMART* ptr, INT index, INT attr);
WCHAR*		(WINAPI *cdi_get_smart_name)(CDI_SMART* ptr, INT index, BYTE id);

static inline LPCSTR
get_health_status(enum CDI_DISK_STATUS status)
{
	switch (status)
	{
	case CDI_DISK_STATUS_GOOD:
		return "Good";
	case CDI_DISK_STATUS_CAUTION:
		return "Caution";
	case CDI_DISK_STATUS_BAD:
		return "Bad";
	}
	return "Unknown";
}

static inline FARPROC
get_dll_func(HMODULE dll, LPCSTR func)
{
	FARPROC ptr = GetProcAddress(dll, func);
	if (ptr == NULL)
	{
		printf("Cannot find %s in libcdi.dll\n", func);
		exit(-1);
	}
	return ptr;
}

static HMODULE
load_cdi(void)
{
	HMODULE dll = LoadLibraryW(L"libcdi.dll");
	if (dll == NULL)
	{
		printf("Cannot load libcdi.dll\n");
		exit(-1);
	}

	*(FARPROC*)&cdi_get_version = get_dll_func(dll, "cdi_get_version");

	*(FARPROC*)&cdi_create_smart = get_dll_func(dll, "cdi_create_smart");
	*(FARPROC*)&cdi_destroy_smart = get_dll_func(dll, "cdi_destroy_smart");
	*(FARPROC*)&cdi_init_smart = get_dll_func(dll, "cdi_init_smart");
	*(FARPROC*)&cdi_update_smart = get_dll_func(dll, "cdi_update_smart");
	*(FARPROC*)&cdi_get_disk_count = get_dll_func(dll, "cdi_get_disk_count");

	*(FARPROC*)&cdi_get_bool = get_dll_func(dll, "cdi_get_bool");
	*(FARPROC*)&cdi_get_int = get_dll_func(dll, "cdi_get_int");
	*(FARPROC*)&cdi_get_dword = get_dll_func(dll, "cdi_get_dword");
	*(FARPROC*)&cdi_get_string = get_dll_func(dll, "cdi_get_string");
	*(FARPROC*)&cdi_free_string = get_dll_func(dll, "cdi_free_string");

	*(FARPROC*)&cdi_get_smart_format = get_dll_func(dll, "cdi_get_smart_format");
	*(FARPROC*)&cdi_get_smart_id = get_dll_func(dll, "cdi_get_smart_id");
	*(FARPROC*)&cdi_get_smart_value = get_dll_func(dll, "cdi_get_smart_value");
	*(FARPROC*)&cdi_get_smart_status = get_dll_func(dll, "cdi_get_smart_status");
	*(FARPROC*)&cdi_get_smart_name = get_dll_func(dll, "cdi_get_smart_name");

	(void)CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
	return dll;
}

static void
unload_cdi(HMODULE dll)
{
	cdi_get_version = NULL;

	cdi_create_smart = NULL;
	cdi_destroy_smart = NULL;
	cdi_init_smart = NULL;
	cdi_update_smart = NULL;
	cdi_get_disk_count = NULL;

	cdi_get_bool = NULL;
	cdi_get_int = NULL;
	cdi_get_dword = NULL;
	cdi_get_string = NULL;
	cdi_free_string = NULL;

	cdi_get_smart_format = NULL;
	cdi_get_smart_id = NULL;
	cdi_get_smart_value = NULL;
	cdi_get_smart_status = NULL;
	cdi_get_smart_name = NULL;

	CoUninitialize();
	FreeLibrary(dll);
}

int main(int argc, char* argv[])
{
	INT i, count;
	HMODULE dll = load_cdi();
	CDI_SMART* smart = cdi_create_smart();

	printf("CDI v%s\n", cdi_get_version());

	cdi_init_smart(smart, CDI_FLAG_DEFAULT);
	count = cdi_get_disk_count(smart);
	printf("Disk count: %d\n", count);

	for (i = 0; i < count; i++)
	{
		INT d;
		DWORD n;
		WCHAR* str;
		BOOL ssd;
		BYTE id;
		printf("\n");
		cdi_update_smart(smart, i);
		printf("\\\\.\\PhysicalDrive%d\n", cdi_get_int(smart, i, CDI_INT_DISK_ID));

		ssd = cdi_get_bool(smart, i, CDI_BOOL_SSD);
		printf("\tSSD: %s\n", ssd ? "Yes" : "No");

		str = cdi_get_string(smart, i, CDI_STRING_DRIVE_MAP);
		printf("\tDrive letters: %ls\n", str);
		cdi_free_string(str);

		str = cdi_get_string(smart, i, CDI_STRING_MODEL);
		printf("\tModel: %ls\n", str);
		cdi_free_string(str);

		str = cdi_get_string(smart, i, CDI_STRING_SN);
		printf("\tSerial: %ls\n", str);
		cdi_free_string(str);

		str = cdi_get_string(smart, i, CDI_STRING_FIRMWARE);
		printf("\tFirmware: %ls\n", str);
		cdi_free_string(str);

		str = cdi_get_string(smart, i, CDI_STRING_INTERFACE);
		printf("\tInterface: %ls\n", str);
		cdi_free_string(str);

		str = cdi_get_string(smart, i, CDI_STRING_TRANSFER_MODE_CUR);
		printf("\tCurrent transfer mode: %ls\n", str);
		cdi_free_string(str);

		str = cdi_get_string(smart, i, CDI_STRING_TRANSFER_MODE_MAX);
		printf("\tMax transfer mode: %ls\n", str);
		cdi_free_string(str);

		d = cdi_get_int(smart, i, CDI_INT_LIFE);
		if (d >= 0)
			printf("\tHealth status: %s (%d%%)\n",
				get_health_status(cdi_get_int(smart, i, CDI_INT_DISK_STATUS)), d);
		else
			printf("\tHealth status: %s\n",
				get_health_status(cdi_get_int(smart, i, CDI_INT_DISK_STATUS)));

		printf("\tTemperature: %d (C)\n", cdi_get_int(smart, i, CDI_INT_TEMPERATURE));

		str = cdi_get_smart_format(smart, i);
		printf("\tID  Status %-24ls Name\n", str);
		cdi_free_string(str);

		n = cdi_get_dword(smart, i, CDI_DWORD_ATTR_COUNT);
		for (INT j = 0; j < (INT)n; j++)
		{
			str = cdi_get_smart_value(smart, i, j, FALSE);
			id = cdi_get_smart_id(smart, i, j);
			printf("\t%02X %7s %-24ls %ls\n",
				id,
				get_health_status(cdi_get_smart_status(smart, i, j)),
				str,
				cdi_get_smart_name(smart, i, id));
			cdi_free_string(str);
		}
	}

	cdi_destroy_smart(smart);
	unload_cdi(dll);
	return 0;
}
