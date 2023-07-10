CrystalDiskInfo 动态链接库
	MIT license
	https://github.com/hiyohiyo/CrystalDiskInfo
	https://github.com/iTXTech/CrystalDiskInfoEmbedded
导出函数：
	CDI_SMART* WINAPI cdi_create_smart(VOID)
		创建 SMART 数据。加载 DLL 后，应首先调用此函数。
		参数：
		返回值：
			返回 CDI_SMART 结构体指针供其他函数使用。
			使用完毕后，请调用 cdi_destroy_smart 释放内存。
	VOID WINAPI cdi_destroy_smart(CDI_SMART* ptr);
		释放 SMART 数据。
		参数：
			ptr		由 cdi_create_smart 获取的 CDI_SMART 结构体指针。
		返回值：
	VOID WINAPI cdi_init_smart(CDI_SMART* ptr, BOOL use_wmi, BOOL advanced_disk_search, BOOL workaround_hd204ui, BOOL workaround_adata_ssd, BOOL hide_no_smart_disk, BOOL sort_drive_letter);
		初始化 SMART 数据。在 cdi_create_smart 之后执行。遍历所有磁盘，收集 SMART 信息。
		参数：
			ptr		由 cdi_create_smart 获取的 CDI_SMART 结构体指针。
			use_wmi					是否使用 WMI
			advanced_disk_search	是否启用高级磁盘搜索
			workaround_hd204ui		是否启用 HD204UI 特殊方案
			workaround_adata_ssd	是否启用 ADATA SSD 特殊方案
			hide_no_smart_disk		是否隐藏无 SMART 磁盘
			sort_drive_letter		是否按盘符顺序排列磁盘。
		返回值：
	DWORD WINAPI cdi_update_smart(CDI_SMART* ptr, INT index);
		更新硬盘的 SMART 信息。
		参数：
			ptr		由 cdi_create_smart 获取的 CDI_SMART 结构体指针。
			index	硬盘序号，从 0 开始，最大值为 cdi_get_disk_count 的返回值减 1。
		返回值：
			若磁盘的 SMART 信息无改动，则返回 0。
	INT WINAPI cdi_get_disk_count(CDI_SMART* ptr);
		获取磁盘计数。
		参数：
			ptr		由 cdi_create_smart 获取的 CDI_SMART 结构体指针。
		返回值：
			磁盘计数。
	BOOL WINAPI cdi_get_bool(CDI_SMART* ptr, INT index, enum CDI_ATA_BOOL attr);
		获取 BOOL 类型的磁盘属性。
		参数：
			ptr		由 cdi_create_smart 获取的 CDI_SMART 结构体指针。
			index	硬盘序号，从 0 开始，最大值为 cdi_get_disk_count 的返回值减 1。
			attr	磁盘属性，见下方 磁盘属性 (BOOL) 部分。
		返回值：
			BOOL 类型的对应磁盘属性。
	INT WINAPI cdi_get_int(CDI_SMART* ptr, INT index, enum CDI_ATA_INT attr);
		获取 INT 类型的磁盘属性。
		参数：
			ptr		由 cdi_create_smart 获取的 CDI_SMART 结构体指针。
			index	硬盘序号，从 0 开始，最大值为 cdi_get_disk_count 的返回值减 1。
			attr	磁盘属性，见下方 磁盘属性 (INT) 部分。
		返回值：
			INT 类型的对应磁盘属性。若返回值为负数，则表明该属性无效。
	DWORD WINAPI cdi_get_dword(CDI_SMART* ptr, INT index, enum CDI_ATA_DWORD attr);
		获取 DWORD 类型的磁盘属性。
		参数：
			ptr		由 cdi_create_smart 获取的 CDI_SMART 结构体指针。
			index	硬盘序号，从 0 开始，最大值为 cdi_get_disk_count 的返回值减 1。
			attr	磁盘属性，见下方 磁盘属性 (DWORD) 部分。
		返回值：
			DWORD 类型的对应磁盘属性。
	CHAR* WINAPI cdi_get_string(CDI_SMART* ptr, INT index, enum CDI_ATA_STRING attr);
		获取字符串类型的磁盘属性。
		参数：
			ptr		由 cdi_create_smart 获取的 CDI_SMART 结构体指针。
			index	硬盘序号，从 0 开始，最大值为 cdi_get_disk_count 的返回值减 1。
			attr	磁盘属性，见下方 磁盘属性 (CHAR*) 部分。
		返回值：
			字符串类型的对应磁盘属性。使用 cdi_free_string 释放内存。
	VOID WINAPI cdi_free_string(CHAR* ptr);
		释放字符串的内存。
		参数：
			ptr		字符串指针。
		返回值：
	CHAR* WINAPI cdi_get_smart_format(CDI_SMART* ptr, INT index);
		获取 SMART 数据格式。
		参数：
			ptr		由 cdi_create_smart 获取的 CDI_SMART 结构体指针。
			index	硬盘序号，从 0 开始，最大值为 cdi_get_disk_count 的返回值减 1。
		返回值：
			SMART 数据的格式。
			可能的返回值为 RawValues(6)，RawValues(8)，Cur RawValues(8)，Cur Wor --- RawValues(6)，Cur Wor Thr RawValues(6)，Cur Wor Thr RawValues(7)。
			使用 cdi_free_string 释放内存。
	BYTE WINAPI cdi_get_smart_id(CDI_SMART* ptr, INT index, INT attr);
		获取 SMART 属性的 ID。
		参数：
			ptr		由 cdi_create_smart 获取的 CDI_SMART 结构体指针。
			index	硬盘序号，从 0 开始，最大值为 cdi_get_disk_count 的返回值减 1。
			attr	SMART 属性序号，从 0 开始，最大值为 CDI_DWORD_ATTR_COUNT 属性的值减 1。
		返回值：
			SMART 属性的 ID。
	CHAR* WINAPI cdi_get_smart_value(CDI_SMART* ptr, INT index, INT attr);
		获取 SMART 属性的数据。
		参数：
			ptr		由 cdi_create_smart 获取的 CDI_SMART 结构体指针。
			index	硬盘序号，从 0 开始，最大值为 cdi_get_disk_count 的返回值减 1。
			attr	SMART 属性序号，从 0 开始，最大值为 CDI_DWORD_ATTR_COUNT 属性的值减 1。
		返回值：
			SMART 属性的数据。使用 cdi_free_string 释放内存。
	INT WINAPI cdi_get_smart_status(CDI_SMART* ptr, INT index, INT attr);
		获取 SMART 属性的状态。
		参数：
			ptr		由 cdi_create_smart 获取的 CDI_SMART 结构体指针。
			index	硬盘序号，从 0 开始，最大值为 cdi_get_disk_count 的返回值减 1。
			attr	SMART 属性序号，从 0 开始，最大值为 CDI_DWORD_ATTR_COUNT 属性的值减 1。
		返回值：
			SMART 属性的状态。请参考 磁盘健康状态 部分。

磁盘属性 (BOOL)：
	enum CDI_ATA_BOOL
	{
		CDI_BOOL_SSD = 0, // 是否为 SSD
		CDI_BOOL_SSD_NVME, // 是否为 NVMe SSD (>=Win10)
		CDI_BOOL_SMART, // 是否支持 SMART
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

磁盘属性 (INT)：
	enum CDI_ATA_INT
	{
		CDI_INT_DISK_ID = 0, // 磁盘 ID，即 \\.\PhysicalDriveX
		CDI_INT_DISK_STATUS, // 磁盘健康状态，请参考 磁盘健康状态 部分
		CDI_INT_SCSI_PORT,
		CDI_INT_SCSI_TARGET_ID,
		CDI_INT_SCSI_BUS,
		CDI_INT_POWER_ON_HOURS, // 通电时间 (小时)
		CDI_INT_TEMPERATURE, // 温度 (C)
		CDI_INT_TEMPERATURE_ALARM, // 警告温度 (C)
		CDI_INT_HOST_WRITES, // 主机总计写入 (GB)
		CDI_INT_HOST_READS, // 主机总计读取 (GB)
		CDI_INT_NAND_WRITES, // NAND 写入量 (GB)
		CDI_INT_GB_ERASED, // 擦取量 (GB)
		CDI_INT_WEAR_LEVELING_COUNT,
		CDI_INT_LIFE, // 健康度 (0% - 100%)
		CDI_INT_MAX_ATTRIBUTE,
	};

磁盘属性 (DWORD)：
	enum CDI_ATA_DWORD
	{
		CDI_DWORD_DISK_SIZE = 0, // 磁盘大小 (MB)
		CDI_DWORD_LOGICAL_SECTOR_SIZE, // 逻辑扇区大小
		CDI_DWORD_PHYSICAL_SECTOR_SIZE, // 物理扇区大小
		CDI_DWORD_BUFFER_SIZE, // 缓存大小 (B)
		CDI_DWORD_ATTR_COUNT, // SMART 属性总数
		CDI_DWORD_POWER_ON_COUNT, // 通电次数
		CDI_DWORD_ROTATION_RATE, // 转速 (RPM)
		CDI_DWORD_DRIVE_LETTER,
		CDI_DWORD_DISK_VENDOR_ID,
	};

磁盘属性 (CHAR*)：
	enum CDI_ATA_STRING
	{
		CDI_STRING_SN, // 序列号
		CDI_STRING_FIRMWARE, // 固件
		CDI_STRING_MODEL, // 型号
		CDI_STRING_DRIVE_MAP, // 盘符列表
		CDI_STRING_TRANSFER_MODE_MAX, // 最大传输模式
		CDI_STRING_TRANSFER_MODE_CUR, // 当前传输模式
		CDI_STRING_INTERFACE, // 接口 (如：NVM Express)
		CDI_STRING_VERSION_MAJOR,
		CDI_STRING_VERSION_MINOR,
		CDI_STRING_PNP_ID,
	};

磁盘健康状态：
	enum CDI_DISK_STATUS
	{
		CDI_DISK_STATUS_UNKNOWN = 0, // 未知
		CDI_DISK_STATUS_GOOD, // 良好
		CDI_DISK_STATUS_CAUTION, // 警告
		CDI_DISK_STATUS_BAD // 损坏
	};
