
#define VC_EXTRALEAN
#include <windows.h>
#include <stdio.h>

#include "../libcdi/libcdi.h"
#pragma comment(lib, "libcdi.lib")

int main(int argc, char* argv[])
{
	INT i, count;
	CDI_SMART* smart;

	(void)CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
	smart = cdi_create_smart();

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
				cdi_get_health_status(cdi_get_int(smart, i, CDI_INT_DISK_STATUS)), d);
		else
			printf("\tHealth status: %s\n",
				cdi_get_health_status(cdi_get_int(smart, i, CDI_INT_DISK_STATUS)));

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
				cdi_get_health_status(cdi_get_smart_status(smart, i, j)),
				str,
				cdi_get_smart_name(smart, i, id));
			cdi_free_string(str);
		}
	}

	cdi_destroy_smart(smart);
	CoUninitialize();
	return 0;
}
