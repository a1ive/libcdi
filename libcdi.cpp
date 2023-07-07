#include "stdafx.h"
#include "AtaSmart.h"
#include "NVMeInterpreter.h"
#include "UtilityFx.h"
#include "libcdi.h"

#pragma comment(lib, "pathcch.lib")

extern "C" __declspec(dllexport) CDI_SMART* WINAPI
cdi_create_smart()
{
	auto ata = new CDI_SMART;
	//SetDebugMode(1);
	(void)CoInitializeEx(0, COINIT_APARTMENTTHREADED);
	ata->SetAtaPassThroughSmart(TRUE);

	ata->FlagNvidiaController = TRUE;
	ata->FlagMarvellController = TRUE;
	ata->FlagUsbSat = TRUE;
	ata->FlagUsbSunplus = TRUE;
	ata->FlagUsbIodata = TRUE;
	ata->FlagUsbLogitec = TRUE;
	ata->FlagUsbProlific = TRUE;
	ata->FlagUsbJmicron = TRUE;
	ata->FlagUsbCypress = TRUE;
	ata->FlagUsbMemory = TRUE;
	ata->FlagUsbNVMeJMicron3 = TRUE;
	ata->FlagUsbNVMeJMicron = TRUE;
	ata->FlagUsbNVMeASMedia = TRUE;
	ata->FlagUsbNVMeRealtek = TRUE;
	ata->FlagMegaRAID = TRUE;
	ata->FlagIntelVROC = TRUE;
	ata->FlagUsbASM1352R = TRUE;

	return ata;
}

extern "C" __declspec(dllexport) VOID WINAPI
cdi_destroy_smart(CDI_SMART * ptr)
{
	if (ptr)
	{
		delete ptr;
		ptr = nullptr;
	}
	CoUninitialize();
}

extern "C" __declspec(dllexport) VOID WINAPI
cdi_init_smart(CDI_SMART * ptr, BOOL sort_drive_letter)
{
	BOOL change_disk = TRUE;
	ptr->Init(TRUE, TRUE, &change_disk, FALSE, FALSE, FALSE, sort_drive_letter);
	for (INT i = 0; i < ptr->vars.GetCount(); i++)
	{
		if (ptr->vars[i].IsSsd)
			ptr->vars[i].AlarmTemperature = 60;
		else
			ptr->vars[i].AlarmTemperature = 50;
		ptr->vars[i].AlarmHealthStatus = 1;

		ptr->vars[i].Threshold05 = 1;
		ptr->vars[i].ThresholdC5 = 1;
		ptr->vars[i].ThresholdC6 = 1;
		ptr->vars[i].ThresholdFF = 10;

		ptr->vars[i].DiskStatus = ptr->CheckDiskStatus(i);
	}
}

extern "C" __declspec(dllexport) DWORD WINAPI
cdi_update_smart(CDI_SMART * ptr, INT index)
{
	return ptr->UpdateSmartInfo(index);
}

CHAR* cs_to_str(CString str)
{
	auto len = str.GetLength() + 1;
	auto buff = new TCHAR[len];
	memcpy(buff, (LPCTSTR)str, sizeof(TCHAR) * (len));
	auto charBuf = (CHAR*)CoTaskMemAlloc(len);
	size_t converted;
	wcstombs_s(&converted, charBuf, len, buff, len);
	delete[] buff;
	return charBuf;
}

extern "C" __declspec(dllexport) INT WINAPI
cdi_get_disk_count(CDI_SMART * ptr)
{
	return (INT)ptr->vars.GetCount();
}

extern "C" __declspec(dllexport) BOOL WINAPI
cdi_get_bool(CDI_SMART * ptr, INT index, enum CDI_ATA_BOOL attr)
{
	switch (attr)
	{
	case CDI_BOOL_SSD:
		return ptr->vars[index].IsSsd;
	case CDI_BOOL_SSD_NVME:
		return (ptr->vars[index].DiskVendorId == CAtaSmart::SSD_VENDOR_NVME) ? TRUE : FALSE;
	case CDI_BOOL_SMART:
		return ptr->vars[index].IsSmartSupported;
	case CDI_BOOL_LBA48:
		return ptr->vars[index].IsLba48Supported;
	case CDI_BOOL_AAM:
		return ptr->vars[index].IsAamSupported;
	case CDI_BOOL_APM:
		return ptr->vars[index].IsApmSupported;
	case CDI_BOOL_NCQ:
		return ptr->vars[index].IsNcqSupported;
	case CDI_BOOL_NV_CACHE:
		return ptr->vars[index].IsNvCacheSupported;
	case CDI_BOOL_DEVSLP:
		return ptr->vars[index].IsDeviceSleepSupported;
	case CDI_BOOL_STREAMING:
		return ptr->vars[index].IsStreamingSupported;
	case CDI_BOOL_GPL:
		return ptr->vars[index].IsGplSupported;
	case CDI_BOOL_TRIM:
		return ptr->vars[index].IsTrimSupported;
	case CDI_BOOL_VOLATILE_WRITE_CACHE:
		return ptr->vars[index].IsVolatileWriteCachePresent;
	case CDI_BOOL_SMART_ENABLED:
		return ptr->vars[index].IsSmartEnabled;
	case CDI_BOOL_AAM_ENABLED:
		return ptr->vars[index].IsAamEnabled;
	case CDI_BOOL_APM_ENABLED:
		return ptr->vars[index].IsApmEnabled;
	}
	return FALSE;
}

extern "C" __declspec(dllexport) INT WINAPI
cdi_get_int(CDI_SMART * ptr, INT index, enum CDI_ATA_INT attr)
{
	switch (attr)
	{
	case CDI_INT_DISK_ID:
		return ptr->vars[index].PhysicalDriveId;
	case CDI_INT_DISK_STATUS:
		return (INT)ptr->vars[index].DiskStatus;
	case CDI_INT_SCSI_PORT:
		return ptr->vars[index].ScsiPort;
	case CDI_INT_SCSI_TARGET_ID:
		return ptr->vars[index].ScsiTargetId;
	case CDI_INT_SCSI_BUS:
		return ptr->vars[index].ScsiBus;
	case CDI_INT_POWER_ON_HOURS:
	{
		INT div = 1;
		INT num = ptr->vars[index].MeasuredPowerOnHours;
		if (num > 0)
		{
			if (ptr->vars[index].MeasuredTimeUnitType == CAtaSmart::POWER_ON_MINUTES && ptr->vars[index].IsMaxtorMinute)
				div = 60;
		}
		else
		{
			num = ptr->vars[index].DetectedPowerOnHours;
			if (num > 0 && ptr->vars[index].DetectedTimeUnitType == CAtaSmart::POWER_ON_MINUTES && ptr->vars[index].IsMaxtorMinute)
				div = 60;
		}
		return num / div;
	}
	case CDI_INT_TEMPERATURE:
		return ptr->vars[index].Temperature;
	case CDI_INT_TEMPERATURE_ALARM:
		return ptr->vars[index].AlarmTemperature;
	case CDI_INT_HOST_WRITES:
		return ptr->vars[index].HostWrites;
	case CDI_INT_HOST_READS:
		return ptr->vars[index].HostReads;
	case CDI_INT_NAND_WRITES:
		return ptr->vars[index].NandWrites;
	case CDI_INT_GB_ERASED:
		return ptr->vars[index].GBytesErased;
	case CDI_INT_WEAR_LEVELING_COUNT:
		return ptr->vars[index].WearLevelingCount;
	case CDI_INT_LIFE:
		return ptr->vars[index].Life;
	case CDI_INT_MAX_ATTRIBUTE:
		return CAtaSmart::MAX_ATTRIBUTE;
	}
	return -1;
}

extern "C" __declspec(dllexport) DWORD WINAPI
cdi_get_dword(CDI_SMART * ptr, INT index, enum CDI_ATA_DWORD attr)
{
	switch (attr)
	{
	case CDI_DWORD_DISK_SIZE:
		return ptr->vars[index].TotalDiskSize;
	case CDI_DWORD_LOGICAL_SECTOR_SIZE:
		return ptr->vars[index].LogicalSectorSize;
	case CDI_DWORD_PHYSICAL_SECTOR_SIZE:
		return ptr->vars[index].PhysicalSectorSize;
	case CDI_DWORD_BUFFER_SIZE:
		return ptr->vars[index].BufferSize;
	case CDI_DWORD_ATTR_COUNT:
		return ptr->vars[index].AttributeCount;
	case CDI_DWORD_POWER_ON_COUNT:
		return ptr->vars[index].PowerOnCount;
	case CDI_DWORD_ROTATION_RATE:
		return ptr->vars[index].NominalMediaRotationRate;
	case CDI_DWORD_DRIVE_LETTER:
		return ptr->vars[index].DriveLetterMap;
	case CDI_DWORD_DISK_VENDOR_ID:
		return ptr->vars[index].DiskVendorId;
	}
	return 0;
}

extern "C" __declspec(dllexport) CHAR* WINAPI
cdi_get_string(CDI_SMART * ptr, INT index, enum CDI_ATA_STRING attr)
{
	switch (attr)
	{
	case CDI_STRING_SN:
		return cs_to_str(ptr->vars[index].SerialNumber);
	case CDI_STRING_FIRMWARE:
		return cs_to_str(ptr->vars[index].FirmwareRev);
	case CDI_STRING_MODEL:
		return cs_to_str(ptr->vars[index].Model);
	case CDI_STRING_DRIVE_MAP:
		return cs_to_str(ptr->vars[index].DriveMap);
	case CDI_STRING_TRANSFER_MODE_MAX:
		return cs_to_str(ptr->vars[index].MaxTransferMode);
	case CDI_STRING_TRANSFER_MODE_CUR:
		return cs_to_str(ptr->vars[index].CurrentTransferMode);
	case CDI_STRING_INTERFACE:
		return cs_to_str(ptr->vars[index].Interface);
	case CDI_STRING_VERSION_MAJOR:
		return cs_to_str(ptr->vars[index].MajorVersion);
	case CDI_STRING_VERSION_MINOR:
		return cs_to_str(ptr->vars[index].MinorVersion);
	case CDI_STRING_PNP_ID:
		return cs_to_str(ptr->vars[index].PnpDeviceId);
	}
	return NULL;
}

extern "C" __declspec(dllexport) VOID WINAPI
cdi_free_string(CHAR* ptr)
{
	if (ptr)
		CoTaskMemFree(ptr);
}

extern "C" __declspec(dllexport) CHAR* WINAPI
cdi_get_smart_format(CDI_SMART * ptr, INT index)
{
	CString fmt;
	if (ptr->vars[index].DiskVendorId == CAtaSmart::SSD_VENDOR_NVME)
		fmt = _T("RawValues(6)");
	else if (ptr->vars[index].IsRawValues8)
	{
		if (ptr->vars[index].DiskVendorId == CAtaSmart::SSD_VENDOR_JMICRON)
			fmt = _T("Cur RawValues(8)");
		else
			fmt = _T("RawValues(8)");
	}
	else if (ptr->vars[index].IsRawValues7)
		fmt = _T("Cur Wor Thr RawValues(7)");
	else
	{
		if (ptr->vars[index].IsThresholdCorrect)
			fmt = _T("Cur Wor Thr RawValues(6)");
		else
			fmt = _T("Cur Wor --- RawValues(6)");
	}
		
	return cs_to_str(fmt);
}

extern "C" __declspec(dllexport) BYTE WINAPI
cdi_get_smart_id(CDI_SMART * ptr, INT index, INT attr)
{
	return ptr->vars[index].Attribute[attr].Id;
}

extern "C" __declspec(dllexport) CHAR* WINAPI
cdi_get_smart_value(CDI_SMART * ptr, INT index, INT attr)
{
	CString cstr;
	if (ptr->vars[index].DiskVendorId == CAtaSmart::SSD_VENDOR_NVME)
	{
		cstr.Format(_T("%02X%02X%02X%02X%02X%02X"),
			ptr->vars[index].Attribute[attr].RawValue[5],
			ptr->vars[index].Attribute[attr].RawValue[4],
			ptr->vars[index].Attribute[attr].RawValue[3],
			ptr->vars[index].Attribute[attr].RawValue[2],
			ptr->vars[index].Attribute[attr].RawValue[1],
			ptr->vars[index].Attribute[attr].RawValue[0]
		);
	}
	else if (ptr->vars[index].DiskVendorId == CAtaSmart::SSD_VENDOR_SANDFORCE)
	{
		cstr.Format(_T("%3d %3d %3d %02X%02X%02X%02X%02X%02X%02X"),
			ptr->vars[index].Attribute[attr].CurrentValue,
			ptr->vars[index].Attribute[attr].WorstValue,
			ptr->vars[index].Threshold[attr].ThresholdValue,
			ptr->vars[index].Attribute[attr].Reserved,
			ptr->vars[index].Attribute[attr].RawValue[5],
			ptr->vars[index].Attribute[attr].RawValue[4],
			ptr->vars[index].Attribute[attr].RawValue[3],
			ptr->vars[index].Attribute[attr].RawValue[2],
			ptr->vars[index].Attribute[attr].RawValue[1],
			ptr->vars[index].Attribute[attr].RawValue[0]
		);
	}
	// For JMicron 60x
	else if (ptr->vars[index].DiskVendorId == CAtaSmart::SSD_VENDOR_JMICRON && ptr->vars[index].IsRawValues8)
	{
		cstr.Format(_T("%3d %02X%02X%02X%02X%02X%02X%02X%02X"),
			ptr->vars[index].Attribute[attr].CurrentValue,
			ptr->vars[index].Attribute[attr].Reserved,
			ptr->vars[index].Attribute[attr].RawValue[5],
			ptr->vars[index].Attribute[attr].RawValue[4],
			ptr->vars[index].Attribute[attr].RawValue[3],
			ptr->vars[index].Attribute[attr].RawValue[2],
			ptr->vars[index].Attribute[attr].RawValue[1],
			ptr->vars[index].Attribute[attr].RawValue[0],
			ptr->vars[index].Attribute[attr].WorstValue
		);
	}
	else if (ptr->vars[index].DiskVendorId == CAtaSmart::SSD_VENDOR_INDILINX)
	{
		cstr.Format(_T("%02X%02X%02X%02X%02X%02X%02X%02X"),
			ptr->vars[index].Attribute[attr].RawValue[5],
			ptr->vars[index].Attribute[attr].RawValue[4],
			ptr->vars[index].Attribute[attr].RawValue[3],
			ptr->vars[index].Attribute[attr].RawValue[2],
			ptr->vars[index].Attribute[attr].RawValue[1],
			ptr->vars[index].Attribute[attr].RawValue[0],
			ptr->vars[index].Attribute[attr].WorstValue,
			ptr->vars[index].Attribute[attr].CurrentValue
		);
	}
	else
	{
		if (ptr->vars[index].IsThresholdCorrect)
		{
			cstr.Format(_T("%3d %3d %3d %02X%02X%02X%02X%02X%02X"),
				ptr->vars[index].Attribute[attr].CurrentValue,
				ptr->vars[index].Attribute[attr].WorstValue,
				ptr->vars[index].Threshold[attr].ThresholdValue,
				ptr->vars[index].Attribute[attr].RawValue[5],
				ptr->vars[index].Attribute[attr].RawValue[4],
				ptr->vars[index].Attribute[attr].RawValue[3],
				ptr->vars[index].Attribute[attr].RawValue[2],
				ptr->vars[index].Attribute[attr].RawValue[1],
				ptr->vars[index].Attribute[attr].RawValue[0]
			);
		}
		else
		{
			cstr.Format(_T("%3d %3d --- %02X%02X%02X%02X%02X%02X"),
				ptr->vars[index].Attribute[attr].CurrentValue,
				ptr->vars[index].Attribute[attr].WorstValue,
				ptr->vars[index].Attribute[attr].RawValue[5],
				ptr->vars[index].Attribute[attr].RawValue[4],
				ptr->vars[index].Attribute[attr].RawValue[3],
				ptr->vars[index].Attribute[attr].RawValue[2],
				ptr->vars[index].Attribute[attr].RawValue[1],
				ptr->vars[index].Attribute[attr].RawValue[0]
			);
		}
	}
	return cs_to_str(cstr);
}

extern "C" __declspec(dllexport) INT WINAPI
cdi_get_smart_status(CDI_SMART * ptr, INT index, INT attr)
{
	BYTE attr_status[CAtaSmart::MAX_ATTRIBUTE]{};
	TCHAR attr_text[CAtaSmart::MAX_ATTRIBUTE][5][32]{};
	ptr->CorrectDiskAttributeStatus(index, attr_status, 0, attr_text);
	return (INT)attr_status[attr];
}
