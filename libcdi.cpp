#include "stdafx.h"
#include "AtaSmart.h"
#include "NVMeInterpreter.h"
#include "libcdi.h"
#include "smartids.h"

//#pragma comment(lib, "pathcch.lib")

extern "C" CDI_SMART* WINAPI
cdi_create_smart()
{
	//SetDebugMode(1);
	//(void)CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
	return new CDI_SMART;
}

extern "C" VOID WINAPI
cdi_destroy_smart(CDI_SMART * ptr)
{
	delete ptr;
	//CoUninitialize();
}

inline BOOL
check_flag(UINT64 flags, UINT64 mask)
{
	return (flags & mask) ? TRUE : FALSE;
}

extern "C" VOID WINAPI
cdi_init_smart(CDI_SMART * ptr, UINT64 flags)
{
	BOOL change_disk = TRUE;

	ptr->FlagNoWakeUp = check_flag(flags, CDI_FLAG_NO_WAKEUP);
	ptr->SetAtaPassThroughSmart(check_flag(flags, CDI_FLAG_ATA_PASS_THROUGH));

	ptr->FlagNvidiaController = check_flag(flags, CDI_FLAG_ENABLE_NVIDIA);
	ptr->FlagMarvellController = check_flag(flags, CDI_FLAG_ENABLE_MARVELL);
	ptr->FlagUsbSat = check_flag(flags, CDI_FLAG_ENABLE_USB_SAT);
	ptr->FlagUsbSunplus = check_flag(flags, CDI_FLAG_ENABLE_USB_SUNPLUS);
	ptr->FlagUsbIodata = check_flag(flags, CDI_FLAG_ENABLE_USB_IODATA);
	ptr->FlagUsbLogitec = check_flag(flags, CDI_FLAG_ENABLE_USB_LOGITEC);
	ptr->FlagUsbProlific = check_flag(flags, CDI_FLAG_ENABLE_USB_PROLIFIC);
	ptr->FlagUsbJmicron = check_flag(flags, CDI_FLAG_ENABLE_USB_JMICRON);
	ptr->FlagUsbCypress = check_flag(flags, CDI_FLAG_ENABLE_USB_CYPRESS);
	ptr->FlagUsbMemory = check_flag(flags, CDI_FLAG_ENABLE_USB_MEMORY);
	ptr->FlagUsbNVMeJMicron3 = check_flag(flags, CDI_FLAG_ENABLE_NVME_JMICRON3);
	ptr->FlagUsbNVMeJMicron = check_flag(flags, CDI_FLAG_ENABLE_NVME_JMICRON);
	ptr->FlagUsbNVMeASMedia = check_flag(flags, CDI_FLAG_ENABLE_NVME_ASMEDIA);
	ptr->FlagUsbNVMeRealtek = check_flag(flags, CDI_FLAG_ENABLE_NVME_REALTEK);
	ptr->FlagMegaRAID = check_flag(flags, CDI_FLAG_ENABLE_MEGA_RAID);
	ptr->FlagIntelVROC = check_flag(flags, CDI_FLAG_ENABLE_INTEL_VROC);
	ptr->FlagUsbASM1352R = check_flag(flags, CDI_FLAG_ENABLE_ASM1352R);
	ptr->FlagAMD_RC2 = check_flag(flags, CDI_FLAG_ENABLE_AMD_RC2);

	ptr->Init(check_flag(flags, CDI_FLAG_USE_WMI),
		check_flag(flags, CDI_FLAG_ADVANCED_SEARCH),
		&change_disk,
		check_flag(flags, CDI_FLAG_WORKAROUND_HD204UI),
		check_flag(flags, CDI_FLAG_WORKAROUND_ADATA),
		check_flag(flags, CDI_FLAG_HIDE_NO_SMART),
		check_flag(flags, CDI_FLAG_SORT_DRIVE_LETTER));
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

extern "C" DWORD WINAPI
cdi_update_smart(CDI_SMART * ptr, INT index)
{
	return ptr->UpdateSmartInfo(index);
}

inline WCHAR* cs_to_wcs(const CString& str)
{
	size_t len = str.GetLength() + 1;
	auto ptr = (WCHAR*)CoTaskMemAlloc(len * sizeof(WCHAR));
	wcscpy_s(ptr, len, str.GetString());
	return ptr;
}

extern "C" INT WINAPI
cdi_get_disk_count(CDI_SMART * ptr)
{
	return (INT)ptr->vars.GetCount();
}

extern "C" BOOL WINAPI
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

extern "C" INT WINAPI
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

extern "C" DWORD WINAPI
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

extern "C" WCHAR* WINAPI
cdi_get_string(CDI_SMART * ptr, INT index, enum CDI_ATA_STRING attr)
{
	switch (attr)
	{
	case CDI_STRING_SN:
		return cs_to_wcs(ptr->vars[index].SerialNumber);
	case CDI_STRING_FIRMWARE:
		return cs_to_wcs(ptr->vars[index].FirmwareRev);
	case CDI_STRING_MODEL:
		return cs_to_wcs(ptr->vars[index].Model);
	case CDI_STRING_DRIVE_MAP:
		return cs_to_wcs(ptr->vars[index].DriveMap);
	case CDI_STRING_TRANSFER_MODE_MAX:
		return cs_to_wcs(ptr->vars[index].MaxTransferMode);
	case CDI_STRING_TRANSFER_MODE_CUR:
		return cs_to_wcs(ptr->vars[index].CurrentTransferMode);
	case CDI_STRING_INTERFACE:
		return cs_to_wcs(ptr->vars[index].Interface);
	case CDI_STRING_VERSION_MAJOR:
		return cs_to_wcs(ptr->vars[index].MajorVersion);
	case CDI_STRING_VERSION_MINOR:
		return cs_to_wcs(ptr->vars[index].MinorVersion);
	case CDI_STRING_PNP_ID:
		return cs_to_wcs(ptr->vars[index].PnpDeviceId);
	case CDI_STRING_SMART_KEY:
		return cs_to_wcs(ptr->vars[index].SmartKeyName);
	}
	return nullptr;
}

extern "C" VOID WINAPI
cdi_free_string(WCHAR* ptr)
{
	if (ptr)
		CoTaskMemFree(ptr);
}

extern "C" WCHAR* WINAPI
cdi_get_smart_format(CDI_SMART * ptr, INT index)
{
	CString fmt;
	if (ptr->vars[index].DiskVendorId == CAtaSmart::SSD_VENDOR_SANDFORCE)
	{
		if (ptr->vars[index].IsThresholdCorrect)
			fmt = _T("Cur Wor Thr RawValues(7)");
		else
			fmt = _T("Cur Wor --- RawValues(7)");
	}
	else if (ptr->vars[index].DiskVendorId == CAtaSmart::SSD_VENDOR_JMICRON
		&& ptr->vars[index].IsRawValues8)
		fmt = _T("Cur RawValues(8)");
	else if (ptr->vars[index].DiskVendorId == CAtaSmart::SSD_VENDOR_INDILINX)
		fmt = _T("RawValues(8)");
	else if (ptr->vars[index].DiskVendorId == CAtaSmart::SSD_VENDOR_NVME)
		fmt = _T("RawValues(7)");
	else
	{
		if (ptr->vars[index].IsThresholdCorrect)
			fmt = _T("Cur Wor Thr RawValues(6)");
		else
			fmt = _T("Cur Wor --- RawValues(6)");
	}
		
	return cs_to_wcs(fmt);
}

extern "C" BYTE WINAPI
cdi_get_smart_id(CDI_SMART * ptr, INT index, INT attr)
{
	return ptr->vars[index].Attribute[attr].Id;
}

extern "C" WCHAR* WINAPI
cdi_get_smart_value(CDI_SMART * ptr, INT index, INT attr, BOOL hex)
{
	CString cstr;
	UINT64 raw;
	SMART_ATTRIBUTE* data = &ptr->vars[index].Attribute[attr];

	if (ptr->vars[index].DiskVendorId == CAtaSmart::SSD_VENDOR_SANDFORCE)
	{
		raw = ((UINT64)data->Reserved << 48) +
			((UINT64)data->RawValue[5] << 40) +
			((UINT64)data->RawValue[4] << 32) +
			((UINT64)data->RawValue[3] << 24) +
			((UINT64)data->RawValue[2] << 16) +
			((UINT64)data->RawValue[1] << 8) +
			((UINT64)data->RawValue[0]);
		if (ptr->vars[index].IsThresholdCorrect)
			cstr.Format(hex ? _T("%3d %3d %3d %014llX") : _T("%3d %3d %3d %I64u"),
				data->CurrentValue, data->WorstValue, ptr->vars[index].Threshold[attr].ThresholdValue, raw);
		else
			cstr.Format(hex ? _T("%3d %3d --- %014llX") : _T("%3d %3d --- %I64u"),
				data->CurrentValue, data->WorstValue, raw);
	}
	else if (ptr->vars[index].DiskVendorId == CAtaSmart::SSD_VENDOR_JMICRON
		&& ptr->vars[index].IsRawValues8)
	{
		raw = ((UINT64)data->Reserved << 56) +
			((UINT64)data->RawValue[5] << 48) +
			((UINT64)data->RawValue[4] << 40) +
			((UINT64)data->RawValue[3] << 32) +
			((UINT64)data->RawValue[2] << 24) +
			((UINT64)data->RawValue[1] << 16) +
			((UINT64)data->RawValue[0] << 8) +
			((UINT64)data->WorstValue);
		cstr.Format(hex ? _T("%3d %016llX") : _T("%3d %I64u"),
			data->CurrentValue, raw);
	}
	else if (ptr->vars[index].DiskVendorId == CAtaSmart::SSD_VENDOR_INDILINX)
	{
		raw = ((UINT64)data->RawValue[5] << 56) +
			((UINT64)data->RawValue[4] << 48) +
			((UINT64)data->RawValue[3] << 40) +
			((UINT64)data->RawValue[2] << 32) +
			((UINT64)data->RawValue[1] << 24) +
			((UINT64)data->RawValue[0] << 16) +
			((UINT64)data->WorstValue << 8) +
			((UINT64)data->CurrentValue);
		cstr.Format(hex ? _T("%016llX") : _T("%I64u"), raw);
	}
	else if (ptr->vars[index].DiskVendorId == CAtaSmart::SSD_VENDOR_NVME)
	{
		raw = ((UINT64)data->Reserved << 48) +
			((UINT64)data->RawValue[5] << 40) +
			((UINT64)data->RawValue[4] << 32) +
			((UINT64)data->RawValue[3] << 24) +
			((UINT64)data->RawValue[2] << 16) +
			((UINT64)data->RawValue[1] << 8) +
			((UINT64)data->RawValue[0]);
		cstr.Format(hex ? _T("%014llX") : _T("%I64u"), raw);
	}
	else
	{
		raw = ((UINT64)data->RawValue[5] << 40) +
			((UINT64)data->RawValue[4] << 32) +
			((UINT64)data->RawValue[3] << 24) +
			((UINT64)data->RawValue[2] << 16) +
			((UINT64)data->RawValue[1] << 8) +
			((UINT64)data->RawValue[0]);
		if (ptr->vars[index].IsThresholdCorrect)
		{
			cstr.Format(hex ? _T("%3d %3d %3d %012llX") : _T("%3d %3d %3d %I64u"),
				data->CurrentValue, data->WorstValue, ptr->vars[index].Threshold[attr].ThresholdValue, raw);
		}
		else
		{
			cstr.Format(hex ? _T("%3d %3d --- %012llX") : _T("%3d %3d --- %I64u"),
				data->CurrentValue, data->WorstValue, raw);
		}
	}

	return cs_to_wcs(cstr);
}

extern "C" INT WINAPI
cdi_get_smart_status(CDI_SMART * ptr, INT index, INT attr)
{
	const BYTE attr_id = ptr->vars[index].Attribute[attr].Id;

	if (attr_id == 0x00 || attr_id == 0xFF)
		return CDI_DISK_STATUS_UNKNOWN;
	if (ptr->vars[index].AttributeStatus[attr] == CDI_DISK_STATUS_UNKNOWN)
		return CDI_DISK_STATUS_GOOD;
	return ptr->vars[index].AttributeStatus[attr];
}

extern "C" WCHAR * WINAPI
cdi_get_smart_name(CDI_SMART * ptr, INT index, BYTE id)
{
	for (size_t i = 0; i < ARRAYSIZE(SMART_NAMES); i++)
	{
		if (ptr->vars[index].SmartKeyName.CompareNoCase(SMART_NAMES[i].group) == 0)
		{
			for (size_t j = 0; j < SMART_NAMES[i].count; j++)
			{
				if (id == SMART_NAMES[i].name[j].id)
					return cs_to_wcs(SMART_NAMES[i].name[j].name);
			}
			break;
		}
	}
	return cs_to_wcs(_T("Vendor Specific"));
}
