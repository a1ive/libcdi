
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

typedef struct _CDI_SMART CDI_SMART;

CDI_SMART*	(WINAPI *cdi_create_smart)(VOID);
VOID		(WINAPI *cdi_destroy_smart)(CDI_SMART* ptr);
VOID		(WINAPI *cdi_init_smart)(CDI_SMART* ptr, BOOL sort_drive_letter);
DWORD		(WINAPI *cdi_update_smart)(CDI_SMART* ptr, INT index);
INT			(WINAPI *cdi_get_disk_count)(CDI_SMART* ptr);

BOOL		(WINAPI *cdi_get_bool)(CDI_SMART* ptr, INT index, enum CDI_ATA_BOOL attr);
INT			(WINAPI *cdi_get_int)(CDI_SMART* ptr, INT index, enum CDI_ATA_INT attr);
DWORD		(WINAPI *cdi_get_dword)(CDI_SMART* ptr, INT index, enum CDI_ATA_DWORD attr);
CHAR*		(WINAPI *cdi_get_string)(CDI_SMART* ptr, INT index, enum CDI_ATA_STRING attr);
VOID		(WINAPI *cdi_free_string)(CHAR* ptr);

CHAR*		(WINAPI* cdi_get_smart_format)(CDI_SMART* ptr, INT index);
CHAR*		(WINAPI *cdi_get_smart_value)(CDI_SMART* ptr, INT index, INT attr);
INT			(WINAPI *cdi_get_smart_status)(CDI_SMART* ptr, INT index, INT attr);

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

static HMODULE
load_cdi(void)
{
	HMODULE dll = LoadLibraryW(L"libcdi.dll");
	if (dll == NULL)
	{
		printf("Cannot load libcdi.dll\n");
		exit(-1);
	}

	*(FARPROC*)&cdi_create_smart = GetProcAddress(dll, "cdi_create_smart");
	*(FARPROC*)&cdi_destroy_smart = GetProcAddress(dll, "cdi_destroy_smart");
	*(FARPROC*)&cdi_init_smart = GetProcAddress(dll, "cdi_init_smart");
	*(FARPROC*)&cdi_update_smart = GetProcAddress(dll, "cdi_update_smart");
	*(FARPROC*)&cdi_get_disk_count = GetProcAddress(dll, "cdi_get_disk_count");

	*(FARPROC*)&cdi_get_bool = GetProcAddress(dll, "cdi_get_bool");
	*(FARPROC*)&cdi_get_int = GetProcAddress(dll, "cdi_get_int");
	*(FARPROC*)&cdi_get_dword = GetProcAddress(dll, "cdi_get_dword");
	*(FARPROC*)&cdi_get_string = GetProcAddress(dll, "cdi_get_string");
	*(FARPROC*)&cdi_free_string = GetProcAddress(dll, "cdi_free_string");

	*(FARPROC*)&cdi_get_smart_format = GetProcAddress(dll, "cdi_get_smart_format");
	*(FARPROC*)&cdi_get_smart_value = GetProcAddress(dll, "cdi_get_smart_value");
	*(FARPROC*)&cdi_get_smart_status = GetProcAddress(dll, "cdi_get_smart_status");

	if (cdi_create_smart == NULL ||
		cdi_destroy_smart == NULL ||
		cdi_init_smart == NULL ||
		cdi_update_smart == NULL ||
		cdi_get_disk_count == NULL ||
		cdi_get_bool == NULL ||
		cdi_get_int == NULL ||
		cdi_get_dword == NULL ||
		cdi_get_string == NULL ||
		cdi_free_string == NULL ||
		cdi_get_smart_format == NULL ||
		cdi_get_smart_value == NULL ||
		cdi_get_smart_status == NULL)
	{
		printf("Cannot find functions in libcdi.dll\n");
		exit(-1);
	}

	return dll;
}

static void
unload_cdi(HMODULE dll)
{
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
	cdi_get_smart_value = NULL;
	cdi_get_smart_status = NULL;

	FreeLibrary(dll);
}

int main(int argc, char* argv[])
{
	INT i, count;
	HMODULE dll = load_cdi();
	CDI_SMART* smart = cdi_create_smart();

	cdi_init_smart(smart, TRUE);
	count = cdi_get_disk_count(smart);
	printf("Disk count: %d\n", count);

	for (i = 0; i < count; i++)
	{
		INT d;
		CHAR* str;
		BOOL ssd;
		printf("\n");
		cdi_update_smart(smart, i);
		printf("DISK%d\n", cdi_get_int(smart, i, CDI_INT_DISK_ID));

		ssd = cdi_get_bool(smart, i, CDI_BOOL_SSD);
		printf("\tSSD: %s\n", ssd ? "Yes" : "No");

		str = cdi_get_string(smart, i, CDI_STRING_DRIVE_MAP);
		printf("\tDrive letters: %s\n", str);
		cdi_free_string(str);

		str = cdi_get_string(smart, i, CDI_STRING_MODEL);
		printf("\tModel: %s\n", str);
		cdi_free_string(str);

		str = cdi_get_string(smart, i, CDI_STRING_SN);
		printf("\tSerial: %s\n", str);
		cdi_free_string(str);

		str = cdi_get_string(smart, i, CDI_STRING_FIRMWARE);
		printf("\tFirmware: %s\n", str);
		cdi_free_string(str);

		str = cdi_get_string(smart, i, CDI_STRING_INTERFACE);
		printf("\tInterface: %s\n", str);
		cdi_free_string(str);

		str = cdi_get_string(smart, i, CDI_STRING_TRANSFER_MODE_CUR);
		printf("\tCurrent transfer mode: %s\n", str);
		cdi_free_string(str);

		str = cdi_get_string(smart, i, CDI_STRING_TRANSFER_MODE_MAX);
		printf("\tMax transfer mode: %s\n", str);
		cdi_free_string(str);

		d = cdi_get_int(smart, i, CDI_INT_LIFE);
		if (d >= 0)
			printf("\tHealth status: %s (%d%%)\n",
				get_health_status(cdi_get_int(smart, i, CDI_INT_DISK_STATUS)), d);
		else
			printf("\tHealth status: %s\n",
				get_health_status(cdi_get_int(smart, i, CDI_INT_DISK_STATUS)));

		printf("\tTemperature: %d (C)\n", cdi_get_int(smart, i, CDI_INT_TEMPERATURE));
	}

	cdi_destroy_smart(smart);
	unload_cdi(dll);
	return 0;
}
