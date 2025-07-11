#include "imgui/imgui.h"
#include "imgui/imgui_impl_dx9.h"
#include "imgui/imgui_impl_win32.h"
#include <d3d9.h>
#include <tchar.h>
#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include <atomic>
#include <memory>
#include <sstream>
#include <functional>

#include "../libcdi/libcdi.h"
#include "version.h"
#include "resource.h"

#pragma comment(lib, "d3d9.lib")

class CdiDataManager;
struct AppContext;

// Custom deleter for CDI strings to ensure cdi_free_string is called.
struct CdiStringDeleter
{
	void operator()(WCHAR* p) const
	{
		if (p)
			cdi_free_string(p);
	}
};
using CdiStringPtr = std::unique_ptr<WCHAR, CdiStringDeleter>;

// RAII wrapper for getting a string from the CDI library.
CdiStringPtr GetCdiString(CDI_SMART* cdi, INT index, CDI_ATA_STRING attr)
{
	return CdiStringPtr(cdi_get_string(cdi, index, attr));
}

// Utility to convert a wide string (WCHAR*) to a UTF-8 std::string.
std::string WcsToUtf8(const WCHAR* wstr)
{
	if (!wstr)
		return "";
	int strSize = WideCharToMultiByte(CP_UTF8, 0, wstr, -1, NULL, 0, NULL, NULL);
	if (strSize == 0)
		return "";
	std::string str(strSize, 0);
	WideCharToMultiByte(CP_UTF8, 0, wstr, -1, &str[0], strSize, NULL, NULL);
	str.pop_back(); // Remove null terminator
	return str;
}

struct SmartAttribute
{
	BYTE id;
	std::string name;
	std::string valueDec;
	std::string valueHex;
	CDI_DISK_STATUS status;
};

struct DiskInfo
{
	// General Info
	bool isSsd = false;
	std::string model;
	std::string firmware;
	std::string serialNumber;
	std::string interfaceType;
	std::string transferMode;
	std::string driveMap;
	std::string majorVersion;
	std::string features;
	double diskSizeGB = 0.0;
	DWORD rotationRate = 0; // RPM
	DWORD bufferSizeKB = 0;

	// Status & Health
	CDI_DISK_STATUS healthStatus = CDI_DISK_STATUS_UNKNOWN;
	int temperature = -1; // Celsius
	int temperatureAlarm = -1; // Celsius
	int life = -1; // Percent for SSD

	// Usage Statistics
	int powerOnHours = -1;
	DWORD powerOnCount = 0;
	int hostReadsGB = -1;
	int hostWritesGB = -1;
	int nandWritesGB = -1;

	std::string smartFormat;
	std::vector<SmartAttribute> smartAttributes;
};

// Encapsulates the state and logic for fetching and managing disk data.
class CdiDataManager
{
public:
	CdiDataManager() = default;
	~CdiDataManager()
	{
		Stop(); // Ensure worker thread is stopped and joined.
	}

	// Deleted copy and move semantics to prevent accidental copying.
	CdiDataManager(const CdiDataManager&) = delete;
	CdiDataManager& operator=(const CdiDataManager&) = delete;
	CdiDataManager(CdiDataManager&&) = delete;
	CdiDataManager& operator=(CdiDataManager&&) = delete;

	void Start()
	{
		isRunning = true;
		workerThread = std::thread(&CdiDataManager::WorkerThread, this);
	}

	void Stop()
	{
		if (isRunning.exchange(false))
		{
			if (workerThread.joinable())
			{
				workerThread.join();
			}
		}
	}

	void RefreshDisk(int diskIndex)
	{
		updateDiskIndex = diskIndex;
	}

	// Provides safe, read-only access to the disk data.
	std::vector<DiskInfo> GetDiskInfo() const
	{
		std::lock_guard<std::mutex> lock(dataMutex);
		return diskInfo;
	}

	bool IsLoading() const
	{
		return isLoading;
	}
	bool IsDataReady() const
	{
		return dataReady;
	}

private:
	// The core worker function that runs on a separate thread.
	void WorkerThread()
	{
		(void)CoInitializeEx(NULL, COINIT_MULTITHREADED);

		// Custom deleter for the main CDI_SMART object.
		struct CdiSmartDeleter { void operator()(CDI_SMART* p) { if (p) cdi_destroy_smart(p); } };
		std::unique_ptr<CDI_SMART, CdiSmartDeleter> cdiSmart;

		cdiSmart.reset(cdi_create_smart());
		if (cdiSmart)
		{
			cdi_init_smart(cdiSmart.get(), CDI_FLAG_DEFAULT);

			{
				std::lock_guard<std::mutex> lock(dataMutex);
				int diskCount = cdi_get_disk_count(cdiSmart.get());
				diskInfo.assign(diskCount, DiskInfo{});
				for (int i = 0; i < diskCount; ++i)
				{
					UpdateDiskData(diskInfo[i], cdiSmart.get(), i);
				}
			}
		}
		dataReady = true;
		isLoading = false;

		while (isRunning)
		{
			int diskIndexToUpdate = updateDiskIndex.exchange(-1);
			if (diskIndexToUpdate >= 0)
			{
				isLoading = true;
				{
					std::lock_guard<std::mutex> lock(dataMutex);
					if (cdiSmart && diskIndexToUpdate < diskInfo.size())
					{
						cdi_update_smart(cdiSmart.get(), diskIndexToUpdate);
						UpdateDiskData(diskInfo[diskIndexToUpdate], cdiSmart.get(), diskIndexToUpdate);
					}
				}
				isLoading = false;
			}
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
		}
		CoUninitialize();
	}

	// Populates a DiskInfo struct with data for a given disk index.
	void UpdateDiskData(DiskInfo& disk, CDI_SMART* cdi, int diskIndex)
	{
		disk.isSsd = cdi_get_bool(cdi, diskIndex, CDI_BOOL_SSD);

		auto curTransferMode = GetCdiString(cdi, diskIndex, CDI_STRING_TRANSFER_MODE_CUR);
		auto maxTransferMode = GetCdiString(cdi, diskIndex, CDI_STRING_TRANSFER_MODE_MAX);
		std::string curModeStr = WcsToUtf8(curTransferMode.get());
		std::string maxModeStr = WcsToUtf8(maxTransferMode.get());

		if (!curModeStr.empty() && !maxModeStr.empty() && curModeStr != maxModeStr)
			disk.transferMode = curModeStr + " | " + maxModeStr;
		else
			disk.transferMode = !curModeStr.empty() ? curModeStr : maxModeStr;

		disk.model = WcsToUtf8(GetCdiString(cdi, diskIndex, CDI_STRING_MODEL).get());
		disk.firmware = WcsToUtf8(GetCdiString(cdi, diskIndex, CDI_STRING_FIRMWARE).get());
		disk.serialNumber = WcsToUtf8(GetCdiString(cdi, diskIndex, CDI_STRING_SN).get());
		disk.interfaceType = WcsToUtf8(GetCdiString(cdi, diskIndex, CDI_STRING_INTERFACE).get());

		disk.driveMap = WcsToUtf8(GetCdiString(cdi, diskIndex, CDI_STRING_DRIVE_MAP).get());
		disk.majorVersion = WcsToUtf8(GetCdiString(cdi, diskIndex, CDI_STRING_VERSION_MAJOR).get());
		disk.diskSizeGB = cdi_get_dword(cdi, diskIndex, CDI_DWORD_DISK_SIZE) / 1000.0;
		disk.rotationRate = cdi_get_dword(cdi, diskIndex, CDI_DWORD_ROTATION_RATE);
		disk.bufferSizeKB = cdi_get_dword(cdi, diskIndex, CDI_DWORD_BUFFER_SIZE) / 1024;

		disk.healthStatus = static_cast<CDI_DISK_STATUS>(cdi_get_int(cdi, diskIndex, CDI_INT_DISK_STATUS));
		disk.temperature = cdi_get_int(cdi, diskIndex, CDI_INT_TEMPERATURE);
		disk.temperatureAlarm = cdi_get_int(cdi, diskIndex, CDI_INT_TEMPERATURE_ALARM);
		disk.life = cdi_get_int(cdi, diskIndex, CDI_INT_LIFE);

		disk.powerOnHours = cdi_get_int(cdi, diskIndex, CDI_INT_POWER_ON_HOURS);
		disk.powerOnCount = cdi_get_dword(cdi, diskIndex, CDI_DWORD_POWER_ON_COUNT);
		disk.hostReadsGB = cdi_get_int(cdi, diskIndex, CDI_INT_HOST_READS);
		disk.hostWritesGB = cdi_get_int(cdi, diskIndex, CDI_INT_HOST_WRITES);
		disk.nandWritesGB = cdi_get_int(cdi, diskIndex, CDI_INT_NAND_WRITES);

		std::stringstream featuresSS;
		if (cdi_get_bool(cdi, diskIndex, CDI_BOOL_SMART)) featuresSS << "S.M.A.R.T. ";
		if (cdi_get_bool(cdi, diskIndex, CDI_BOOL_AAM)) featuresSS << "AAM ";
		if (cdi_get_bool(cdi, diskIndex, CDI_BOOL_APM)) featuresSS << "APM ";
		if (cdi_get_bool(cdi, diskIndex, CDI_BOOL_NCQ)) featuresSS << "NCQ ";
		if (cdi_get_bool(cdi, diskIndex, CDI_BOOL_TRIM)) featuresSS << "TRIM ";
		if (cdi_get_bool(cdi, diskIndex, CDI_BOOL_GPL)) featuresSS << "GPL ";
		if (cdi_get_bool(cdi, diskIndex, CDI_BOOL_VOLATILE_WRITE_CACHE)) featuresSS << "VolatileWriteCache ";
		disk.features = featuresSS.str();

		disk.smartFormat = WcsToUtf8(CdiStringPtr(cdi_get_smart_format(cdi, diskIndex)).get());
		disk.smartAttributes.clear();
		DWORD attr_count = cdi_get_dword(cdi, diskIndex, CDI_DWORD_ATTR_COUNT);
		disk.smartAttributes.reserve(attr_count);
		for (DWORD i = 0; i < attr_count; ++i)
		{
			BYTE id = cdi_get_smart_id(cdi, diskIndex, i);
			if (id == 0) continue;

			SmartAttribute attr;
			attr.id = id;
			attr.status = static_cast<CDI_DISK_STATUS>(cdi_get_smart_status(cdi, diskIndex, i));
			attr.name = WcsToUtf8(CdiStringPtr(cdi_get_smart_name(cdi, diskIndex, id)).get());
			attr.valueDec = WcsToUtf8(CdiStringPtr(cdi_get_smart_value(cdi, diskIndex, i, FALSE)).get());
			attr.valueHex = WcsToUtf8(CdiStringPtr(cdi_get_smart_value(cdi, diskIndex, i, TRUE)).get());
			disk.smartAttributes.push_back(std::move(attr));
		}
	}

	// --- Member Variables ---
	mutable std::mutex dataMutex;
	std::vector<DiskInfo> diskInfo;
	std::thread workerThread;

	std::atomic<bool> isRunning = false;
	std::atomic<bool> isLoading = true;
	std::atomic<bool> dataReady = false;
	std::atomic<int> updateDiskIndex = -1; // -1 = idle, >= 0 = index to update
};

struct GraphicsContext
{
	LPDIRECT3D9 pD3D = nullptr;
	LPDIRECT3DDEVICE9 pd3dDevice = nullptr;
	D3DPRESENT_PARAMETERS d3dpp = {};
};

struct AppContext
{
	GraphicsContext graphics;
	CdiDataManager cdiManager;
	float mainScale = 1.0f;
	int selectedDisk = 0;
	bool showDemoWindow = false;
	bool smartWindowOpen = true;
	bool saveIniFile = false;
	bool showAboutWindow = false;
	bool showHexValues = false;

	// For handling device reset
	bool deviceLost = false;
	UINT resizeWidth = 0;
	UINT resizeHeight = 0;
};

void RenderMenuBar(AppContext& context)
{
	if (ImGui::BeginMenuBar())
	{
		if (ImGui::BeginMenu("File"))
		{
			bool isLoading = context.cdiManager.IsLoading();
			if (ImGui::MenuItem("Refresh", nullptr, false, !isLoading))
			{
				context.cdiManager.RefreshDisk(context.selectedDisk);
			}
			ImGui::MenuItem("Show Hex Values", nullptr, &context.showHexValues);
			
			ImGui::MenuItem("Save configuration", nullptr, &context.saveIniFile);

			ImGui::Separator();
			if (ImGui::MenuItem("Exit"))
			{
				context.smartWindowOpen = false;
			}
			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Help"))
		{
			if (ImGui::MenuItem("Debug"))
			{
				context.showDemoWindow = true;
			}
			if (ImGui::MenuItem("About"))
			{
				context.showAboutWindow = true;
			}
			ImGui::EndMenu();
		}

		ImGui::EndMenuBar();
	}
}

ImVec4 GetStatusColor(CDI_DISK_STATUS status, float alpha = 1.0f)
{
	switch (status)
	{
	case CDI_DISK_STATUS_GOOD:
		return ImVec4(0.12f, 0.78f, 0.59f, alpha); // A friendly Teal/Cyan
	case CDI_DISK_STATUS_CAUTION:
		return ImVec4(0.99f, 0.65f, 0.18f, alpha); // A clear Orange/Amber
	case CDI_DISK_STATUS_BAD:
		return ImVec4(0.84f, 0.24f, 0.24f, alpha); // A softer Red
	default:
		return ImVec4(0.5f, 0.5f, 0.5f, alpha); // Neutral Gray
	}
}

void RenderStatusIndicator(ImDrawList* drawList, CDI_DISK_STATUS status, ImVec2 centerPos, float radius)
{
	ImVec4 colorVec = GetStatusColor(status);
	ImU32 statusColorU32 = ImGui::ColorConvertFloat4ToU32(colorVec);
	drawList->AddCircleFilled(centerPos, radius, statusColorU32);
}

void ShowStatusIndicator(CDI_DISK_STATUS status)
{
	ImDrawList* drawList = ImGui::GetWindowDrawList();
	float radius = ImGui::GetTextLineHeight() * 0.4f;
	ImVec2 p = ImGui::GetCursorScreenPos();
	ImVec2 circleCenter = ImVec2(p.x + radius + 1, p.y + ImGui::GetTextLineHeight() * 0.5f);
	RenderStatusIndicator(drawList, status, circleCenter, radius);
	ImGui::Dummy(ImVec2(radius * 2 + ImGui::GetStyle().ItemSpacing.x, 0));
}

void RenderInfoBox(float scale, const char* label, const char* value, const char* subValue, CDI_DISK_STATUS status)
{
	ImGui::PushID(label);

	ImGui::BeginGroup();
	ImGui::Text(" %s", label);
	ImGui::Spacing();

	ImVec4 color = GetStatusColor(status);

	ImGui::PushStyleColor(ImGuiCol_Button, color);
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, color);
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, color);
	ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 0.0f);

	ImGui::Button("##infobox", ImVec2(100 * scale, 65 * scale));

	ImGui::PopStyleVar();
	ImGui::PopStyleColor(3);

	ImVec2 pMin = ImGui::GetItemRectMin();
	ImVec2 pMax = ImGui::GetItemRectMax();
	ImDrawList* drawList = ImGui::GetWindowDrawList();

	ImVec2 valueTextSize = ImGui::CalcTextSize(value);
	ImVec2 subValueTextSize = ImGui::CalcTextSize(subValue);
	ImVec2 centerPos = ImVec2(pMin.x + (pMax.x - pMin.x) * 0.5f, pMin.y + (pMax.y - pMin.y) * 0.5f);
	ImVec2 valuePos = ImVec2(centerPos.x - valueTextSize.x * 0.5f, centerPos.y - (valueTextSize.y + subValueTextSize.y) * 0.5f);
	ImVec2 subValuePos = ImVec2(centerPos.x - subValueTextSize.x * 0.5f, valuePos.y + valueTextSize.y);

	// Use black text for high contrast on all background colors.
	ImU32 textColor = IM_COL32(0, 0, 0, 255);

	drawList->AddText(valuePos, textColor, value);
	if (subValue && *subValue)
	{
		drawList->AddText(subValuePos, textColor, subValue);
	}

	ImGui::EndGroup();
	ImGui::PopID();
}

void RenderAttributePair(const char* label, const std::string& value)
{
	ImGui::Text("%s", label);
	ImGui::PushID(label);
	ImGui::InputText("##value", (char*)value.c_str(), value.length() + 1, ImGuiInputTextFlags_ReadOnly);

	// Show a tooltip with the full value when the item is hovered.
	if (ImGui::IsItemHovered() && !value.empty())
	{
		ImGui::SetTooltip("%s", value.c_str());
	}

	ImGui::PopID();
}

void RenderDiskButton(int diskIndex, int& selectedDisk, const DiskInfo& disk)
{
	ImGuiStyle& style = ImGui::GetStyle();
	ImGui::PushID(diskIndex);

	bool isSelected = (selectedDisk == diskIndex);
	if (isSelected)
	{
		ImGui::PushStyleColor(ImGuiCol_Button, style.Colors[ImGuiCol_ButtonActive]);
	}

	float buttonHeight = ImGui::GetTextLineHeightWithSpacing() * 2 + style.FramePadding.y * 2;
	if (ImGui::Button("##disk_button", ImVec2(160.0f, buttonHeight)))
	{
		selectedDisk = diskIndex;
	}

	if (isSelected)
	{
		ImGui::PopStyleColor(1);
	}

	ImDrawList* drawList = ImGui::GetWindowDrawList();
	ImVec2 pMin = ImGui::GetItemRectMin();
	ImVec2 pMax = ImGui::GetItemRectMax();

	std::string diskIdText = "Disk " + std::to_string(diskIndex) + ":";

	ImVec2 textLine1Size = ImGui::CalcTextSize(diskIdText.c_str());
	ImVec2 line1Pos(pMin.x + style.FramePadding.x, pMin.y + style.FramePadding.y);

	drawList->AddText(line1Pos, ImGui::GetColorU32(ImGuiCol_Text), diskIdText.c_str());

	float radius = ImGui::GetTextLineHeight() * 0.4f;
	ImVec2 indicatorCenter(line1Pos.x + textLine1Size.x + style.ItemSpacing.x + radius, line1Pos.y + ImGui::GetTextLineHeight() * 0.5f);
	RenderStatusIndicator(drawList, disk.healthStatus, indicatorCenter, radius);

	ImVec2 line2Pos(pMin.x + style.FramePadding.x, pMin.y + style.FramePadding.y + ImGui::GetTextLineHeightWithSpacing());
	drawList->PushClipRect(pMin, pMax, true);
	drawList->AddText(line2Pos, ImGui::GetColorU32(ImGuiCol_Text), disk.model.c_str());
	drawList->PopClipRect();

	if (ImGui::IsItemHovered())
	{
		const char* healthStr = cdi_get_health_status(disk.healthStatus);
		ImGui::SetTooltip("Model: %s\nHealth: %s", disk.model.c_str(), healthStr);
	}

	ImGui::PopID();
}

void RenderSmartTable(AppContext& context, const std::string smartFormat, const std::vector<SmartAttribute>& smartAttributes)
{
	ImGuiTableFlags tableFlags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY | ImGuiTableFlags_Resizable;

	if (ImGui::BeginTable("SMART Data", 4, tableFlags))
	{
		float statusColumnWidth = ImGui::GetTextLineHeight() * 1.5f; // A sensible default width
		float idColumnWidth = ImGui::CalcTextSize("FF").x + ImGui::GetStyle().CellPadding.x * 2.0f;

		// Column 1: For the status indicator. It has a hidden header ("##...").
		ImGui::TableSetupColumn("##Status", ImGuiTableColumnFlags_WidthFixed, statusColumnWidth);
		// Column 2: For the ID.
		ImGui::TableSetupColumn("ID", ImGuiTableColumnFlags_WidthFixed, idColumnWidth);
		// Column 3 & 4: Stretch by default.
		ImGui::TableSetupColumn("Attribute Name", ImGuiTableColumnFlags_WidthStretch);
		ImGui::TableSetupColumn(smartFormat.c_str(), ImGuiTableColumnFlags_WidthStretch);

		// Use the standard headers row, which correctly handles resizable column separators.
		// It will correctly display "ID", "Attribute Name", and "Value", and leave the first header blank.
		ImGui::TableHeadersRow();

		// --- Table Body ---
		for (const auto& attr : smartAttributes)
		{
			ImGui::TableNextRow();

			// Column 1: Status Indicator
			ImGui::TableSetColumnIndex(0);
			ShowStatusIndicator(attr.status);

			// Column 2: ID
			ImGui::TableSetColumnIndex(1);
			ImGui::Text("%02X", attr.id);

			// Column 3: Attribute Name
			ImGui::TableSetColumnIndex(2);
			ImGui::Text("%s", attr.name.c_str());

			// Column 4: Value
			ImGui::TableSetColumnIndex(3);
			const std::string& valueToShow = context.showHexValues ? attr.valueHex : attr.valueDec;
			ImGui::Text("%s", valueToShow.c_str());
		}
		ImGui::EndTable();
	}
}

void RenderDiskDetails(AppContext& context, const DiskInfo& disk)
{
	// --- Header (Model & Size) ---
	ImGui::SetWindowFontScale(2.0f);
	ImGui::Text("%s", disk.model.c_str());
	ImGui::SameLine();
	ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), ": %.1f GB", disk.diskSizeGB);
	ImGui::SetWindowFontScale(1.0f);
	ImGui::Separator();

	// --- Main Layout (Left: Status boxes, Right: Attributes) ---
	if (ImGui::BeginTable("MainLayout", 2, ImGuiTableFlags_None))
	{
		ImGui::TableSetupColumn("StatusColumn", ImGuiTableColumnFlags_WidthFixed, 110.0f * context.mainScale);
		ImGui::TableSetupColumn("AttributesColumn", ImGuiTableColumnFlags_WidthStretch);
		ImGui::TableNextRow();

		// --- LEFT COLUMN ---
		ImGui::TableSetColumnIndex(0);
		{
			std::string lifeStr = (disk.life >= 0) ? std::to_string(disk.life) + " %" : "";
			RenderInfoBox(context.mainScale, "Health Status", cdi_get_health_status(disk.healthStatus), lifeStr.c_str(), disk.healthStatus);
			ImGui::Spacing();
			if (disk.temperature >= 0)
			{
				int alarmThreshold = (disk.temperatureAlarm > 0) ? disk.temperatureAlarm : 60;
				CDI_DISK_STATUS tempStatus = (disk.temperature >= alarmThreshold) ? CDI_DISK_STATUS_CAUTION : CDI_DISK_STATUS_GOOD;
				RenderInfoBox(context.mainScale, "Temperature", (std::to_string(disk.temperature) + " C").c_str(), "", tempStatus);
			}
		}

		// --- RIGHT COLUMN ---
		ImGui::TableSetColumnIndex(1);
		{
			// Prepare all display strings
			std::string powerOnCountStr = (disk.powerOnCount > 0) ? std::to_string(disk.powerOnCount) + " times" : "---";
			std::string powerOnHoursStr = (disk.powerOnHours >= 0) ? std::to_string(disk.powerOnHours) + " hours" : "---";
			std::string hostReadsStr = (disk.hostReadsGB >= 0) ? std::to_string(disk.hostReadsGB) + " GB" : "---";
			std::string hostWritesStr = (disk.hostWritesGB >= 0) ? std::to_string(disk.hostWritesGB) + " GB" : "---";
			std::string nandWritesStr = (disk.nandWritesGB >= 0) ? std::to_string(disk.nandWritesGB) + " GB" : "---";
			std::string rotationRateStr = (disk.rotationRate > 1) ? std::to_string(disk.rotationRate) + " RPM" : "---";
			std::string bufferSizeStr = (disk.bufferSizeKB > 0) ? std::to_string(disk.bufferSizeKB) + " KB" : "---";

			float columnWidth = ImGui::GetContentRegionAvail().x * 0.5f - ImGui::GetStyle().ItemSpacing.x * 0.5f;

			// Left column of attributes
			ImGui::BeginGroup();
			ImGui::PushItemWidth(columnWidth);
			RenderAttributePair("Firmware", disk.firmware);
			RenderAttributePair("Serial Number", disk.serialNumber);
			RenderAttributePair("Interface", disk.interfaceType);
			RenderAttributePair("Transfer Mode", disk.transferMode);
			RenderAttributePair("Drive Letter", disk.driveMap);
			ImGui::PopItemWidth();
			ImGui::EndGroup();

			ImGui::SameLine();

			// Right column of attributes
			ImGui::BeginGroup();
			ImGui::PushItemWidth(columnWidth);
			if (disk.isSsd)
			{
				RenderAttributePair("Total Host Reads", hostReadsStr);
				RenderAttributePair("Total Host Writes", hostWritesStr);
				if (disk.nandWritesGB >= 0)
					RenderAttributePair("Total NAND Writes", nandWritesStr);
				else
					RenderAttributePair("---", "---");
			}
			else
			{
				RenderAttributePair("Buffer Size", bufferSizeStr);
				RenderAttributePair("Rotation Rate", rotationRateStr);
				RenderAttributePair("---", "---");
			}
			RenderAttributePair("Power On Count", powerOnCountStr);
			RenderAttributePair("Power On Hours", powerOnHoursStr);
			ImGui::PopItemWidth();
			ImGui::EndGroup();
		}
		ImGui::EndTable();
	}

	// --- BOTTOM SECTION (Standard, Features) ---
	ImGui::PushItemWidth(-1.0f);
	RenderAttributePair("Standard", disk.majorVersion);
	RenderAttributePair("Features", disk.features);
	ImGui::PopItemWidth();
	ImGui::Separator();

	// --- Buttons ---
	if (ImGui::Button("Refresh"))
	{
		if (!context.cdiManager.IsLoading())
		{
			context.cdiManager.RefreshDisk(context.selectedDisk);
		}
	}
	ImGui::SameLine();
	ImGui::Checkbox("Show Hex Values", &context.showHexValues);
	ImGui::Separator();

	// --- SMART Data Table ---
	RenderSmartTable(context, disk.smartFormat, disk.smartAttributes);
}

void RenderLoadingWindow(AppContext& context)
{
	if (!context.cdiManager.IsLoading())
		return;

	ImVec2 parentWindowPos = ImGui::GetWindowPos();
	ImVec2 parentWindowSize = ImGui::GetWindowSize();
	ImVec2 parentCenter = ImVec2(parentWindowPos.x + parentWindowSize.x * 0.5f, parentWindowPos.y + parentWindowSize.y * 0.5f);

	ImGui::SetNextWindowPos(parentCenter, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

	ImGui::OpenPopup("Loading...");
	if (ImGui::BeginPopupModal("Loading...", NULL, ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::Text("Loading disk information, please wait...");
		ImGui::EndPopup();
	}
}

void RenderAboutWindow(AppContext& context)
{
	if (!context.showAboutWindow)
		return;

	const ImGuiViewport* mainViewport = ImGui::GetMainViewport();
	ImGui::SetNextWindowPos(mainViewport->GetWorkCenter(), ImGuiCond_FirstUseEver);

	if (!ImGui::Begin("About", &context.showAboutWindow, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoCollapse))
	{
		ImGui::End();
		return;
	}

	ImGui::SetWindowFontScale(2.0f);
	ImGui::Text(IMCDI_NAME);
	ImGui::SetWindowFontScale(1.0f);
	ImGui::SameLine();
	ImGui::Text("v%s", cdi_get_version());
	ImGui::Separator();
	ImGui::Spacing();

	ImGui::TextWrapped("A simple disk S.M.A.R.T. information utility built with Dear ImGui, using the libcdi.");
	ImGui::Spacing();

	ImGui::Text("Copyright " IMCDI_COPYRIGHT);
	ImGui::TextLinkOpenURL("https://github.com/a1ive/libcdi");
	ImGui::Text("Based on CrystalDiskInfo by hiyohiyo:");
	ImGui::TextLinkOpenURL("https://github.com/hiyohiyo/CrystalDiskInfo");
	ImGui::Spacing();
	ImGui::Separator();
	ImGui::Spacing();

	float buttonWidth = 120;
	ImGui::SetCursorPosX((ImGui::GetWindowWidth() - buttonWidth) * 0.5f);
	if (ImGui::Button("OK", ImVec2(buttonWidth, 0)))
		context.showAboutWindow = false;

	ImGui::End();
}

bool CreateDeviceD3D(HWND hWnd, GraphicsContext& context)
{
	if ((context.pD3D = Direct3DCreate9(D3D_SDK_VERSION)) == nullptr)
		return false;

	ZeroMemory(&context.d3dpp, sizeof(context.d3dpp));
	context.d3dpp.Windowed = TRUE;
	context.d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	context.d3dpp.BackBufferFormat = D3DFMT_UNKNOWN;
	context.d3dpp.EnableAutoDepthStencil = TRUE;
	context.d3dpp.AutoDepthStencilFormat = D3DFMT_D16;
	context.d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_ONE;
	if (context.pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd, D3DCREATE_HARDWARE_VERTEXPROCESSING, &context.d3dpp, &context.pd3dDevice) < 0)
		return false;
	return true;
}

void CleanupDeviceD3D(GraphicsContext& context)
{
	if (context.pd3dDevice)
	{
		context.pd3dDevice->Release();
		context.pd3dDevice = nullptr;
	}
	if (context.pD3D)
	{
		context.pD3D->Release();
		context.pD3D = nullptr;
	}
}

void ResetDevice(AppContext& context)
{
	ImGui_ImplDX9_InvalidateDeviceObjects();
	HRESULT hr = context.graphics.pd3dDevice->Reset(&context.graphics.d3dpp);
	if (hr == D3DERR_INVALIDCALL)
		IM_ASSERT(0);
	ImGui_ImplDX9_CreateDeviceObjects();
}

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
		return true;

	AppContext* context = reinterpret_cast<AppContext*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
	if (!context)
	{
		return DefWindowProc(hWnd, msg, wParam, lParam);
	}

	switch (msg)
	{
	case WM_SIZE:
		if (wParam == SIZE_MINIMIZED)
			return 0;
		context->resizeWidth = (UINT)LOWORD(lParam);
		context->resizeHeight = (UINT)HIWORD(lParam);
		return 0;
	case WM_SYSCOMMAND:
		if ((wParam & 0xfff0) == SC_KEYMENU)
			return 0;
		break;
	case WM_DESTROY:
		::PostQuitMessage(0);
		return 0;
	}
	return ::DefWindowProcW(hWnd, msg, wParam, lParam);
}

int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nShowCmd)
{
	AppContext context;

	ImGui_ImplWin32_EnableDpiAwareness();
	context.mainScale = ImGui_ImplWin32_GetDpiScaleForMonitor(::MonitorFromPoint(POINT{ 0, 0 }, MONITOR_DEFAULTTOPRIMARY));

	WNDCLASSW wc =
	{
		CS_CLASSDC,
		WndProc,
		0L,
		0L,
		hInstance,
		LoadIconW(hInstance, MAKEINTRESOURCEW(IDI_ICON_MAIN)),
		LoadCursorW(NULL, IDC_ARROW),
		nullptr,
		nullptr,
		L"" IMCDI_NAME
	};
	::RegisterClassW(&wc);

	HWND hWnd = ::CreateWindowW(
		wc.lpszClassName,
		L"" IMCDI_NAME,
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		(int)(1024 * context.mainScale),
		(int)(768 * context.mainScale),
		nullptr,
		nullptr,
		wc.hInstance,
		nullptr
	);

	// Associate the AppContext pointer with the window handle.
	SetWindowLongPtrW(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(&context));

	if (!CreateDeviceD3D(hWnd, context.graphics))
	{
		CleanupDeviceD3D(context.graphics);
		::UnregisterClassW(wc.lpszClassName, wc.hInstance);
		return 1;
	}

	::ShowWindow(hWnd, SW_HIDE);
	::UpdateWindow(hWnd);

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

	ImGui::StyleColorsDark();
	ImGuiStyle& style = ImGui::GetStyle();
	if (context.mainScale > 1.0f)
	{
		style.ScaleAllSizes(context.mainScale);
	}
	style.WindowRounding = 0.0f;
	style.Colors[ImGuiCol_WindowBg].w = 1.0f;

	ImGui_ImplWin32_Init(hWnd);
	ImGui_ImplDX9_Init(context.graphics.pd3dDevice);

	context.cdiManager.Start();

	ImVec4 clearColor = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

	// Main loop
	bool done = false;
	while (!done)
	{
		MSG msg;
		while (::PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE))
		{
			::TranslateMessage(&msg);
			::DispatchMessage(&msg);
			if (msg.message == WM_QUIT)
				done = true;
		}
		if (done)
			break;

		if (context.deviceLost)
		{
			HRESULT hr = context.graphics.pd3dDevice->TestCooperativeLevel();
			if (hr == D3DERR_DEVICELOST) { ::Sleep(10); continue; }
			if (hr == D3DERR_DEVICENOTRESET) ResetDevice(context);
			context.deviceLost = false;
		}
		if (context.resizeWidth != 0 && context.resizeHeight != 0)
		{
			context.graphics.d3dpp.BackBufferWidth = context.resizeWidth;
			context.graphics.d3dpp.BackBufferHeight = context.resizeHeight;
			context.resizeWidth = context.resizeHeight = 0;
			ResetDevice(context);
		}

		ImGui_ImplDX9_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();

		io.IniFilename = context.saveIniFile ? "imcdi.ini" : nullptr;
		
		if (context.showDemoWindow)
			ImGui::ShowDemoWindow(&context.showDemoWindow);

		if (context.smartWindowOpen)
		{
			ImGui::SetNextWindowSize(ImVec2(700, 750), ImGuiCond_FirstUseEver);
			ImGui::Begin("SMART Information", &context.smartWindowOpen, ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoCollapse);

			RenderMenuBar(context);

			// Activate once
			static std::once_flag focusOnceFlag;
			if (context.cdiManager.IsDataReady())
			{
				std::call_once(focusOnceFlag, []()
					{
						ImGuiViewport* viewport = ImGui::GetWindowViewport();
						if (viewport && viewport->PlatformHandle)
						{
							HWND hImGuiWnd = (HWND)viewport->PlatformHandle;
							::SetForegroundWindow(hImGuiWnd);
							::SetActiveWindow(hImGuiWnd);
						}
					});
			}

			if (!context.cdiManager.IsDataReady())
			{
				ImGui::Text("Initializing...");
			}
			else
			{
				auto diskInfo = context.cdiManager.GetDiskInfo();
				int diskCount = static_cast<int>(diskInfo.size());

				if (diskCount == 0)
				{
					ImGui::Text("No disks found.");
				}
				else
				{
					float buttonHeight = ImGui::GetTextLineHeightWithSpacing() * 2 + style.FramePadding.y * 2;
					float containerHeight = buttonHeight + style.WindowPadding.y * 2.0f + style.ScrollbarSize;

					ImGui::BeginChild("DiskSelector", ImVec2(0, containerHeight), true, ImGuiWindowFlags_HorizontalScrollbar);
					for (int i = 0; i < diskCount; ++i)
					{
						RenderDiskButton(i, context.selectedDisk, diskInfo[i]);
						if (i < diskCount - 1)
							ImGui::SameLine();
					}
					ImGui::EndChild();
					ImGui::Separator();

					if (context.selectedDisk >= 0 && context.selectedDisk < diskCount)
					{
						RenderDiskDetails(context, diskInfo[context.selectedDisk]);
					}
				}
			}

			RenderLoadingWindow(context);

			ImGui::End();
		}

		if (!context.smartWindowOpen)
		{
			done = true;
		}

		RenderAboutWindow(context);

		ImGui::EndFrame();
		context.graphics.pd3dDevice->SetRenderState(D3DRS_ZENABLE, FALSE);
		context.graphics.pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
		context.graphics.pd3dDevice->SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE);
		D3DCOLOR clearColDx = D3DCOLOR_RGBA(
			(int)(clearColor.x * clearColor.w * 255.0f),
			(int)(clearColor.y * clearColor.w * 255.0f),
			(int)(clearColor.z * clearColor.w * 255.0f),
			(int)(clearColor.w * 255.0f)
		);
		context.graphics.pd3dDevice->Clear(0, nullptr, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, clearColDx, 1.0f, 0);
		if (context.graphics.pd3dDevice->BeginScene() >= 0)
		{
			ImGui::Render();
			ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
			context.graphics.pd3dDevice->EndScene();
		}

		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
		}

		HRESULT result = context.graphics.pd3dDevice->Present(nullptr, nullptr, nullptr, nullptr);
		if (result == D3DERR_DEVICELOST)
			context.deviceLost = true;
	}

	// --- Cleanup ---
	ImGui_ImplDX9_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

	CleanupDeviceD3D(context.graphics);
	::DestroyWindow(hWnd);
	::UnregisterClassW(wc.lpszClassName, wc.hInstance);

	return 0;
}
