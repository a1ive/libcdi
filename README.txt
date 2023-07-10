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
	VOID WINAPI cdi_init_smart(CDI_SMART* ptr, UINT64 flags);
		初始化 SMART 数据。在 cdi_create_smart 之后执行。遍历所有磁盘，收集 SMART 信息。
		参数：
			ptr		由 cdi_create_smart 获取的 CDI_SMART 结构体指针。
			flags	设置磁盘处理的各种选项，见下方 磁盘处理选项 部分。建议值为 0x01FBFF83。
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

磁盘处理选项
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
