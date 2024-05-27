#ifndef _SMARTINFO_H
#define	_SMARTINFO_H

#include "resource.h"

#include <devioctl.h>
#include <ntdddisk.h>

#define	LEN_SHORT_STRING	16
#define	LEN_STRING			256
#define	LEN_MID_STRING		1024
#define	LEN_MAX_STRING		4096

#define	LEN_BLOCK			256

//
#define INDEX_ATTRIB_INDEX									0
#define INDEX_ATTRIB_UNKNOWN1								1
#define INDEX_ATTRIB_UNKNOWN2								2
#define INDEX_ATTRIB_VALUE									3
#define INDEX_ATTRIB_WORST									4
#define INDEX_ATTRIB_RAW									5

//
enum COMMAND_TYPE
{
	CMD_TYPE_UNKNOWN = 0,
	CMD_TYPE_PHYSICAL_DRIVE,
	CMD_TYPE_SCSI_MINIPORT,
	CMD_TYPE_SILICON_IMAGE,
	CMD_TYPE_SAT,			// SAT = SCSI_ATA_TRANSLATION
	CMD_TYPE_SUNPLUS,
	CMD_TYPE_IO_DATA,
	CMD_TYPE_LOGITEC,
	CMD_TYPE_PROLIFIC,
	CMD_TYPE_JMICRON,
	CMD_TYPE_CYPRESS,
	CMD_TYPE_SAT_ASM1352R,	// AMS1352 2nd drive
	CMD_TYPE_CSMI,				// CSMI = Common Storage Management Interface
	CMD_TYPE_CSMI_PHYSICAL_DRIVE, // CSMI = Common Storage Management Interface 
	CMD_TYPE_WMI,
	CMD_TYPE_NVME_SAMSUNG,
	CMD_TYPE_NVME_INTEL,
	CMD_TYPE_NVME_STORAGE_QUERY,
	CMD_TYPE_NVME_JMICRON,
	CMD_TYPE_NVME_ASMEDIA,
	CMD_TYPE_NVME_REALTEK,
	CMD_TYPE_NVME_INTEL_RST,
	CMD_TYPE_MEGARAID,
	CMD_TYPE_DEBUG
};

//
typedef	struct
{
	int			id;
	int			critical;
	const WCHAR	*name;
	const WCHAR	*details;
}	ST_ATTRIBUTES;

//
#include <pshpack1.h>

//
//	Must Be 12 Bytes
typedef	struct
{
	BYTE	bIndex;
	/*
		Bit 0 - Warranty
		Bit 1 - Offline
		Bit 2 - Performance
		Bit 3 - Error rate

		Bit 4 - Event count 
		Bit 5 - Self-preservation
		Bits 6–15 - Reserved 
	 */
	WORD	wFlag;

	//	Current value of the attribute, normalized from 1 to 100 (0x01-0x64), if that has meaning for the attribute
	BYTE	bCurrent;

	//	Lowest normalized value of the attribute, from 1 to 100 (0x01-0x64), if that has meaning for the attribute
	BYTE	bWorst;

	//		32 bits of raw attribute data
	union {
		DWORD	dwData;
		struct 
		{
			WORD	wData0;
			WORD	wData1;
		};
	};

	//
	//		Attribute specific 
	WORD	wSpecific;

	//		Threshold
	BYTE	bThreshold;

}	ST_VARIABLES;

//
//	362 Bytes Structure
typedef	struct
{
	union{
		WORD			wLength;
		WORD			wRevision;
	};
	ST_VARIABLES	stVariables [ 30 ];
}	ST_VALUES;

#include <poppack.h>

//
#include <pshpack1.h>

typedef struct _IDENTIFY_DEVICE_DATA {
  struct {
    USHORT Reserved1 : 1;
    USHORT Retired3 : 1;
    USHORT ResponseIncomplete : 1;
    USHORT Retired2 : 3;
    USHORT FixedDevice : 1;
    USHORT RemovableMedia : 1;
    USHORT Retired1 : 7;
    USHORT DeviceType : 1;
  } GeneralConfiguration;
  USHORT NumCylinders;
  USHORT SpecificConfiguration;
  USHORT NumHeads;
  USHORT Retired1[2];
  USHORT NumSectorsPerTrack;
  USHORT VendorUnique1[3];
  UCHAR  SerialNumber[20];
  USHORT Retired2[2];
  USHORT Obsolete1;
  UCHAR  FirmwareRevision[8];
  UCHAR  ModelNumber[40];
  UCHAR  MaximumBlockTransfer;
  UCHAR  VendorUnique2;
  struct {
    USHORT FeatureSupported : 1;
    USHORT Reserved : 15;
  } TrustedComputing;
  struct {
    UCHAR  CurrentLongPhysicalSectorAlignment : 2;
    UCHAR  ReservedByte49 : 6;
    UCHAR  DmaSupported : 1;
    UCHAR  LbaSupported : 1;
    UCHAR  IordyDisable : 1;
    UCHAR  IordySupported : 1;
    UCHAR  Reserved1 : 1;
    UCHAR  StandybyTimerSupport : 1;
    UCHAR  Reserved2 : 2;
    USHORT ReservedWord50;
  } Capabilities;
  USHORT ObsoleteWords51[2];
  USHORT TranslationFieldsValid : 3;
  USHORT Reserved3 : 5;
  USHORT FreeFallControlSensitivity : 8;
  USHORT NumberOfCurrentCylinders;
  USHORT NumberOfCurrentHeads;
  USHORT CurrentSectorsPerTrack;
  ULONG  CurrentSectorCapacity;
  UCHAR  CurrentMultiSectorSetting;
  UCHAR  MultiSectorSettingValid : 1;
  UCHAR  ReservedByte59 : 3;
  UCHAR  SanitizeFeatureSupported : 1;
  UCHAR  CryptoScrambleExtCommandSupported : 1;
  UCHAR  OverwriteExtCommandSupported : 1;
  UCHAR  BlockEraseExtCommandSupported : 1;
  ULONG  UserAddressableSectors;
  USHORT ObsoleteWord62;
  USHORT MultiWordDMASupport : 8;
  USHORT MultiWordDMAActive : 8;
  USHORT AdvancedPIOModes : 8;
  USHORT ReservedByte64 : 8;
  USHORT MinimumMWXferCycleTime;
  USHORT RecommendedMWXferCycleTime;
  USHORT MinimumPIOCycleTime;
  USHORT MinimumPIOCycleTimeIORDY;
  struct {
    USHORT ZonedCapabilities : 2;
    USHORT NonVolatileWriteCache : 1;
    USHORT ExtendedUserAddressableSectorsSupported : 1;
    USHORT DeviceEncryptsAllUserData : 1;
    USHORT ReadZeroAfterTrimSupported : 1;
    USHORT Optional28BitCommandsSupported : 1;
    USHORT IEEE1667 : 1;
    USHORT DownloadMicrocodeDmaSupported : 1;
    USHORT SetMaxSetPasswordUnlockDmaSupported : 1;
    USHORT WriteBufferDmaSupported : 1;
    USHORT ReadBufferDmaSupported : 1;
    USHORT DeviceConfigIdentifySetDmaSupported : 1;
    USHORT LPSAERCSupported : 1;
    USHORT DeterministicReadAfterTrimSupported : 1;
    USHORT CFastSpecSupported : 1;
  } AdditionalSupported;
  USHORT ReservedWords70[5];
  USHORT QueueDepth : 5;
  USHORT ReservedWord75 : 11;
  struct {
    USHORT Reserved0 : 1;
    USHORT SataGen1 : 1;
    USHORT SataGen2 : 1;
    USHORT SataGen3 : 1;
    USHORT Reserved1 : 4;
    USHORT NCQ : 1;
    USHORT HIPM : 1;
    USHORT PhyEvents : 1;
    USHORT NcqUnload : 1;
    USHORT NcqPriority : 1;
    USHORT HostAutoPS : 1;
    USHORT DeviceAutoPS : 1;
    USHORT ReadLogDMA : 1;
    USHORT Reserved2 : 1;
    USHORT CurrentSpeed : 3;
    USHORT NcqStreaming : 1;
    USHORT NcqQueueMgmt : 1;
    USHORT NcqReceiveSend : 1;
    USHORT DEVSLPtoReducedPwrState : 1;
    USHORT Reserved3 : 8;
  } SerialAtaCapabilities;
  struct {
    USHORT Reserved0 : 1;
    USHORT NonZeroOffsets : 1;
    USHORT DmaSetupAutoActivate : 1;
    USHORT DIPM : 1;
    USHORT InOrderData : 1;
    USHORT HardwareFeatureControl : 1;
    USHORT SoftwareSettingsPreservation : 1;
    USHORT NCQAutosense : 1;
    USHORT DEVSLP : 1;
    USHORT HybridInformation : 1;
    USHORT Reserved1 : 6;
  } SerialAtaFeaturesSupported;
  struct {
    USHORT Reserved0 : 1;
    USHORT NonZeroOffsets : 1;
    USHORT DmaSetupAutoActivate : 1;
    USHORT DIPM : 1;
    USHORT InOrderData : 1;
    USHORT HardwareFeatureControl : 1;
    USHORT SoftwareSettingsPreservation : 1;
    USHORT DeviceAutoPS : 1;
    USHORT DEVSLP : 1;
    USHORT HybridInformation : 1;
    USHORT Reserved1 : 6;
  } SerialAtaFeaturesEnabled;
  USHORT MajorRevision;
  USHORT MinorRevision;
  struct {
    USHORT SmartCommands : 1;
    USHORT SecurityMode : 1;
    USHORT RemovableMediaFeature : 1;
    USHORT PowerManagement : 1;
    USHORT Reserved1 : 1;
    USHORT WriteCache : 1;
    USHORT LookAhead : 1;
    USHORT ReleaseInterrupt : 1;
    USHORT ServiceInterrupt : 1;
    USHORT DeviceReset : 1;
    USHORT HostProtectedArea : 1;
    USHORT Obsolete1 : 1;
    USHORT WriteBuffer : 1;
    USHORT ReadBuffer : 1;
    USHORT Nop : 1;
    USHORT Obsolete2 : 1;
    USHORT DownloadMicrocode : 1;
    USHORT DmaQueued : 1;
    USHORT Cfa : 1;
    USHORT AdvancedPm : 1;
    USHORT Msn : 1;
    USHORT PowerUpInStandby : 1;
    USHORT ManualPowerUp : 1;
    USHORT Reserved2 : 1;
    USHORT SetMax : 1;
    USHORT Acoustics : 1;
    USHORT BigLba : 1;
    USHORT DeviceConfigOverlay : 1;
    USHORT FlushCache : 1;
    USHORT FlushCacheExt : 1;
    USHORT WordValid83 : 2;
    USHORT SmartErrorLog : 1;
    USHORT SmartSelfTest : 1;
    USHORT MediaSerialNumber : 1;
    USHORT MediaCardPassThrough : 1;
    USHORT StreamingFeature : 1;
    USHORT GpLogging : 1;
    USHORT WriteFua : 1;
    USHORT WriteQueuedFua : 1;
    USHORT WWN64Bit : 1;
    USHORT URGReadStream : 1;
    USHORT URGWriteStream : 1;
    USHORT ReservedForTechReport : 2;
    USHORT IdleWithUnloadFeature : 1;
    USHORT WordValid : 2;
  } CommandSetSupport;
  struct {
    USHORT SmartCommands : 1;
    USHORT SecurityMode : 1;
    USHORT RemovableMediaFeature : 1;
    USHORT PowerManagement : 1;
    USHORT Reserved1 : 1;
    USHORT WriteCache : 1;
    USHORT LookAhead : 1;
    USHORT ReleaseInterrupt : 1;
    USHORT ServiceInterrupt : 1;
    USHORT DeviceReset : 1;
    USHORT HostProtectedArea : 1;
    USHORT Obsolete1 : 1;
    USHORT WriteBuffer : 1;
    USHORT ReadBuffer : 1;
    USHORT Nop : 1;
    USHORT Obsolete2 : 1;
    USHORT DownloadMicrocode : 1;
    USHORT DmaQueued : 1;
    USHORT Cfa : 1;
    USHORT AdvancedPm : 1;
    USHORT Msn : 1;
    USHORT PowerUpInStandby : 1;
    USHORT ManualPowerUp : 1;
    USHORT Reserved2 : 1;
    USHORT SetMax : 1;
    USHORT Acoustics : 1;
    USHORT BigLba : 1;
    USHORT DeviceConfigOverlay : 1;
    USHORT FlushCache : 1;
    USHORT FlushCacheExt : 1;
    USHORT Resrved3 : 1;
    USHORT Words119_120Valid : 1;
    USHORT SmartErrorLog : 1;
    USHORT SmartSelfTest : 1;
    USHORT MediaSerialNumber : 1;
    USHORT MediaCardPassThrough : 1;
    USHORT StreamingFeature : 1;
    USHORT GpLogging : 1;
    USHORT WriteFua : 1;
    USHORT WriteQueuedFua : 1;
    USHORT WWN64Bit : 1;
    USHORT URGReadStream : 1;
    USHORT URGWriteStream : 1;
    USHORT ReservedForTechReport : 2;
    USHORT IdleWithUnloadFeature : 1;
    USHORT Reserved4 : 2;
  } CommandSetActive;
  USHORT UltraDMASupport : 8;
  USHORT UltraDMAActive : 8;
  struct {
    USHORT TimeRequired : 15;
    USHORT ExtendedTimeReported : 1;
  } NormalSecurityEraseUnit;
  struct {
    USHORT TimeRequired : 15;
    USHORT ExtendedTimeReported : 1;
  } EnhancedSecurityEraseUnit;
  USHORT CurrentAPMLevel : 8;
  USHORT ReservedWord91 : 8;
  USHORT MasterPasswordID;
  USHORT HardwareResetResult;
  USHORT CurrentAcousticValue : 8;
  USHORT RecommendedAcousticValue : 8;
  USHORT StreamMinRequestSize;
  USHORT StreamingTransferTimeDMA;
  USHORT StreamingAccessLatencyDMAPIO;
  ULONG  StreamingPerfGranularity;
  ULONG  Max48BitLBA[2];
  USHORT StreamingTransferTime;
  USHORT DsmCap;
  struct {
    USHORT LogicalSectorsPerPhysicalSector : 4;
    USHORT Reserved0 : 8;
    USHORT LogicalSectorLongerThan256Words : 1;
    USHORT MultipleLogicalSectorsPerPhysicalSector : 1;
    USHORT Reserved1 : 2;
  } PhysicalLogicalSectorSize;
  USHORT InterSeekDelay;
  USHORT WorldWideName[4];
  USHORT ReservedForWorldWideName128[4];
  USHORT ReservedForTlcTechnicalReport;
  USHORT WordsPerLogicalSector[2];
  struct {
    USHORT ReservedForDrqTechnicalReport : 1;
    USHORT WriteReadVerify : 1;
    USHORT WriteUncorrectableExt : 1;
    USHORT ReadWriteLogDmaExt : 1;
    USHORT DownloadMicrocodeMode3 : 1;
    USHORT FreefallControl : 1;
    USHORT SenseDataReporting : 1;
    USHORT ExtendedPowerConditions : 1;
    USHORT Reserved0 : 6;
    USHORT WordValid : 2;
  } CommandSetSupportExt;
  struct {
    USHORT ReservedForDrqTechnicalReport : 1;
    USHORT WriteReadVerify : 1;
    USHORT WriteUncorrectableExt : 1;
    USHORT ReadWriteLogDmaExt : 1;
    USHORT DownloadMicrocodeMode3 : 1;
    USHORT FreefallControl : 1;
    USHORT SenseDataReporting : 1;
    USHORT ExtendedPowerConditions : 1;
    USHORT Reserved0 : 6;
    USHORT Reserved1 : 2;
  } CommandSetActiveExt;
  USHORT ReservedForExpandedSupportandActive[6];
  USHORT MsnSupport : 2;
  USHORT ReservedWord127 : 14;
  struct {
    USHORT SecuritySupported : 1;
    USHORT SecurityEnabled : 1;
    USHORT SecurityLocked : 1;
    USHORT SecurityFrozen : 1;
    USHORT SecurityCountExpired : 1;
    USHORT EnhancedSecurityEraseSupported : 1;
    USHORT Reserved0 : 2;
    USHORT SecurityLevel : 1;
    USHORT Reserved1 : 7;
  } SecurityStatus;
  USHORT ReservedWord129[31];
  struct {
    USHORT MaximumCurrentInMA : 12;
    USHORT CfaPowerMode1Disabled : 1;
    USHORT CfaPowerMode1Required : 1;
    USHORT Reserved0 : 1;
    USHORT Word160Supported : 1;
  } CfaPowerMode1;
  USHORT ReservedForCfaWord161[7];
  USHORT NominalFormFactor : 4;
  USHORT ReservedWord168 : 12;
  struct {
    USHORT SupportsTrim : 1;
    USHORT Reserved0 : 15;
  } DataSetManagementFeature;
  USHORT AdditionalProductID[4];
  USHORT ReservedForCfaWord174[2];
  USHORT CurrentMediaSerialNumber[30];
  struct {
    USHORT Supported : 1;
    USHORT Reserved0 : 1;
    USHORT WriteSameSuported : 1;
    USHORT ErrorRecoveryControlSupported : 1;
    USHORT FeatureControlSuported : 1;
    USHORT DataTablesSuported : 1;
    USHORT Reserved1 : 6;
    USHORT VendorSpecific : 4;
  } SCTCommandTransport;
  USHORT ReservedWord207[2];
  struct {
    USHORT AlignmentOfLogicalWithinPhysical : 14;
    USHORT Word209Supported : 1;
    USHORT Reserved0 : 1;
  } BlockAlignment;
  USHORT WriteReadVerifySectorCountMode3Only[2];
  USHORT WriteReadVerifySectorCountMode2Only[2];
  struct {
    USHORT NVCachePowerModeEnabled : 1;
    USHORT Reserved0 : 3;
    USHORT NVCacheFeatureSetEnabled : 1;
    USHORT Reserved1 : 3;
    USHORT NVCachePowerModeVersion : 4;
    USHORT NVCacheFeatureSetVersion : 4;
  } NVCacheCapabilities;
  USHORT NVCacheSizeLSW;
  USHORT NVCacheSizeMSW;
  USHORT NominalMediaRotationRate;
  USHORT ReservedWord218;
  struct {
    UCHAR NVCacheEstimatedTimeToSpinUpInSeconds;
    UCHAR Reserved;
  } NVCacheOptions;
  USHORT WriteReadVerifySectorCountMode : 8;
  USHORT ReservedWord220 : 8;
  USHORT ReservedWord221;
  struct {
    USHORT MajorVersion : 12;
    USHORT TransportType : 4;
  } TransportMajorVersion;
  USHORT TransportMinorVersion;
  USHORT ReservedWord224[6];
  ULONG  ExtendedNumberOfUserAddressableSectors[2];
  USHORT MinBlocksPerDownloadMicrocodeMode03;
  USHORT MaxBlocksPerDownloadMicrocodeMode03;
  USHORT ReservedWord236[19];
  USHORT Signature : 8;
  USHORT CheckSum : 8;
} IDENTIFY_DEVICE_DATA, *PIDENTIFY_DEVICE_DATA;

#include <poppack.h>


typedef struct
{
	BYTE  bDriverError;
	BYTE  bIDEStatus;
	BYTE  bReserved[2];
	DWORD dwReserved[2];
}	ST_DRIVERSTAT;

typedef struct
{
	DWORD			cBufferSize;
	ST_DRIVERSTAT	DriverStatus;
	BYTE			bBuffer[1];
}	ST_ATAOUTPARAM;

typedef struct
{
	BYTE			m_ucAttribIndex;
	WORD			m_wFlag;
	WORD			m_wData0;
	WORD			m_wData1;
	DWORD			m_dwData;
	WORD			m_wSpecific;
	BYTE			m_ucValue;
	BYTE			m_ucWorst;
	DWORD			m_bThreshold;
}	ST_SMART_INFO;

//
typedef struct
{
	GETVERSIONINPARAMS	m_stGetVersionInParams;
	WCHAR				m_csErrorString [ LEN_STRING ];
}	ST_DRIVE_INFO;

//
//	IOCTL_SCSI_MINIPORT
//
#include <pshpack1.h>

typedef struct
{
	union
	{
		SENDCMDINPARAMS		inp;
		SENDCMDOUTPARAMS	out;
	};
}	ST_SENDCMDPARAMS;

typedef struct 
{
	SRB_IO_CONTROL		srbc;

	ST_SENDCMDPARAMS	io;

	//
	BYTE data[512];

	//
}	ST_SCSI_MINIPORT_INP;

typedef struct 
{
	SRB_IO_CONTROL		srbc;

	ST_SENDCMDPARAMS	io;

	//
	BYTE data[512];

	//
}	ST_SCSI_MINIPORT_OUT;

/**
	IDEREGS	8 bytes
	UCHAR	bFeaturesReg;			// 0 : Used for specifying SMART "commands".
	UCHAR	bSectorCountReg;        // 1 : IDE sector count register
	UCHAR	bSectorNumberReg;       // 2 : IDE sector number register
	UCHAR	bCylLowReg;             // 3 : IDE low order cylinder value
	UCHAR	bCylHighReg;            // 4 : IDE high order cylinder value
	UCHAR	bDriveHeadReg;          // 5 : IDE drive/head register
	UCHAR	bCommandReg;            // 6 : Actual IDE command.
	UCHAR	bReserved;				// 7 : reserved for future use.  Must be zero.

 **/
//
typedef struct {
	struct {
		BYTE	opCode;
	} cdb0;
	struct {
		BYTE reserved : 1;
		BYTE protocol : 4;
		BYTE multipleCount : 3;
	} cdb1;
	struct {
		BYTE length : 2;
		BYTE byteBlock : 1;
		BYTE tDir : 1;
		BYTE reserved : 1;
		BYTE okCond : 1;
		BYTE offline : 2;
	} cdb2;
	IDEREGS	ideRegs;
} ST_CDB_10, *P_ST_CDB_10;

//
typedef struct {
	BYTE	cdb0;
	BYTE	cdb1;
	IDEREGS	ideRegs;
} ST_CDB_12_8, *P_ST_CDB_12_8;

//
typedef struct {
	BYTE	cdb0;
	BYTE	cdb1;
	BYTE	cdb2;
	IDEREGS	ideRegs;
} ST_CDB_12_9, *P_ST_CDB_12_9;

//
typedef struct {
	BYTE	cdb0;
	BYTE	cdb1;
	BYTE	cdb2;
	BYTE	cdb3;
	BYTE	cdb4;
	IDEREGS	ideRegs;
} ST_CDB_12_11, *P_ST_CDB_12_11;

typedef struct {
	BYTE	cdb0;
	BYTE	cdb1;
	BYTE	cdb2;
	BYTE	cdb3;
	BYTE	cdb4;
	BYTE	cdb5;
	IDEREGS	ideRegs;
} ST_CDB_16_12, *P_ST_CDB_16_12;

typedef struct {
	BYTE	cdb0;
	BYTE	cdb1;
	BYTE	cdb2;
	BYTE	cdb3;
	BYTE	cdb4;
	BYTE	cdb5;
	BYTE	cdb6;
	BYTE	cdb7;
	BYTE	cdb8;
	IDEREGS	ideRegs;
} ST_CDB_16_15, *P_ST_CDB_16_15;

//
//	SCSI_PASS_THROUGH
typedef struct {
	SCSI_PASS_THROUGH			ScsiPassThrough;
	ULONG						Filler;				// realign buffers to double word boundary
	UCHAR						SenseBuffer[32];
	UCHAR						DataBuffer[4096];
} ST_SCSI_PASS_THROUGH_BUF, *P_ST_SCSI_PASS_THROUGH_BUF;

typedef struct  {
	SCSI_PASS_THROUGH			ScsiPassThrough;
	ULONG						Filler;				// realign buffers to double word boundary
	UCHAR						SenseBuffer[24];
	UCHAR						DataBuffer[4096];
} ST_SCSI_PASS_THROUGH_BUF_24, *P_ST_SCSI_PASS_THROUGH_BUF_24;

//
//	SCSI_PASS_THROUGH_DIRECT
typedef struct {
	SCSI_PASS_THROUGH_DIRECT	ScsiPassThrough;
	ULONG						Filler;				// realign buffers to double word boundary
	UCHAR						SenseBuffer[32];
	UCHAR						DataBuffer[4096];
} ST_SCSI_PASS_THROUGH_DIR, *P_ST_SCSI_PASS_THROUGH_DIR;

typedef struct  {
	SCSI_PASS_THROUGH_DIRECT	ScsiPassThrough;
	ULONG						Filler;				// realign buffers to double word boundary
	UCHAR						SenseBuffer[24];
	UCHAR						DataBuffer[4096];
} ST_SCSI_PASS_THROUGH_DIR_24, *P_ST_SCSI_PASS_THROUGH_DIR_24;

#include <poppack.h>

//	Variant Values
union VARIANT_UNION
{
    LONGLONG llVal;
    LONG lVal;
    BYTE bVal;
    SHORT iVal;
    FLOAT fltVal;
    DOUBLE dblVal;
    VARIANT_BOOL boolVal;
    // _VARIANT_BOOL booleen;
    SCODE scode;
    CY cyVal;
    DATE date;
    BSTR bstrVal;
    IUnknown *punkVal;
    IDispatch *pdispVal;
    SAFEARRAY *parray;
    BYTE *pbVal;
    SHORT *piVal;
    LONG *plVal;
    LONGLONG *pllVal;
    FLOAT *pfltVal;
    DOUBLE *pdblVal;
    VARIANT_BOOL *pboolVal;
    // _VARIANT_BOOL *pbool;
    SCODE *pscode;
    CY *pcyVal;
    DATE *pdate;
    BSTR *pbstrVal;
    IUnknown **ppunkVal;
    IDispatch **ppdispVal;
    SAFEARRAY **pparray;
    VARIANT *pvarVal;
    PVOID byref;
    CHAR cVal;
    USHORT uiVal;
    ULONG ulVal;
    ULONGLONG ullVal;
    INT intVal;
    UINT uintVal;
    DECIMAL *pdecVal;
    CHAR *pcVal;
    USHORT *puiVal;
    ULONG *pulVal;
    ULONGLONG *pullVal;
    INT *pintVal;
    UINT *puintVal;
    struct __tagBRECORD
    {
		PVOID pvRecord;
		IRecordInfo *pRecInfo;
    } 	__VARIANT_NAME_4;
};

#endif