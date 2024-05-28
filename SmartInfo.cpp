// SmartInfo.cpp : définit le point d'entrée pour l'application console.
//

#include "stdafx.h"

//
#define _NTSCSI_USER_MODE_
#include <devioctl.h>
#include <ntdddisk.h>
#include <ntddscsi.h>
#include <ntddstor.h>
#include <scsi.h>

#include <comdef.h>
#include <Wbemidl.h>
#pragma comment(lib, "wbemuuid.lib")

#include "SmartInfo.h"

#include <windows.h>
#include <SetupAPI.h>

#include <string.h>
#include <stdlib.h>

#include <initguid.h>
#include <Usbiodef.h>

#include <locale.h>

#include "AutomaticVersionHeader.h"

#include "PrintRoutine.h"
#include "strstr.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#ifndef _wsizeof
#define _wsizeof(text) sizeof(text)/sizeof(WCHAR)
#endif // !_wsizeof

static  WCHAR   szErrorText [ LEN_MAX_STRING ];

#define DRIVE_HEAD_REG  0xA0

static  ST_DRIVE_INFO               DriveInfo;
static  IDENTIFY_DEVICE_DATA        SectorInfo;
static  ST_SMART_INFO               SmartInfo;
static  ST_SCSI_MINIPORT_INP        ScsiMiniPortInp;
static  ST_SCSI_MINIPORT_OUT        ScsiMiniPortOut;
static  STORAGE_PROPERTY_QUERY      StoragePropertyQuery;
static  BYTE                        StorageDeviceDescriptor [ 4096 ];
static  STORAGE_DEVICE_DESCRIPTOR   *pStorageDeviceDescriptor   = (STORAGE_DEVICE_DESCRIPTOR*) StorageDeviceDescriptor;
static  STORAGE_DEVICE_NUMBER       StorageDeviceNumber;
static  STORAGE_PREDICT_FAILURE     StoragePredictFailure;

static  ST_SCSI_PASS_THROUGH_BUF    ScsiPassThroughWithBuffer;

static  BYTE                        DriveInformation [ 4096 ];
static  DRIVE_LAYOUT_INFORMATION_EX *pDriveInformation = (DRIVE_LAYOUT_INFORMATION_EX *)DriveInformation;

static  BYTE                        DriveGeometry [ 4096 ];
static  DISK_GEOMETRY_EX            *pDriveGeometry = (DISK_GEOMETRY_EX *)DriveGeometry;

static  SENDCMDINPARAMS             SendCmdInParams;
static  SENDCMDOUTPARAMS            SendCmdOutParams;

static  BYTE                        GetMediaTypes [ sizeof(GET_MEDIA_TYPES)+15*sizeof(DEVICE_MEDIA_INFO)];
static  PGET_MEDIA_TYPES            pGetMediaTypes = (PGET_MEDIA_TYPES) GetMediaTypes;

static  char                        AtaIdentifyRequest[sizeof(ATA_PASS_THROUGH_EX)+IDENTIFY_BUFFER_SIZE]    = {0};
static  PATA_PASS_THROUGH_EX        pAtaIdentifyRequest = (PATA_PASS_THROUGH_EX) AtaIdentifyRequest;

static char                         SmartIdentifyResponse[sizeof(SENDCMDOUTPARAMS)+IDENTIFY_BUFFER_SIZE-1]  = {0};

static  SCSI_ADDRESS                ScsiAddress = {0};
static  BYTE                        ScsiAdapterBusInfo [ sizeof (SCSI_ADAPTER_BUS_INFO) + sizeof(SCSI_BUS_DATA) * 32 ];
static  SCSI_ADAPTER_BUS_INFO       *pScsiAdapterBusInfo = (SCSI_ADAPTER_BUS_INFO *)ScsiAdapterBusInfo;
static  IO_SCSI_CAPABILITIES        IoScsiCapabilities = {0};

//
#define                     LOW_VALUE           1
#define                     HIGH_VALUE          5
#define                     CRITICAL_VALUE      10

static  bool                Low             = false;
static  bool                High            = false;
static  bool                Critical        = false;

static  bool                ListDevice      = false;
static  bool                ListDeviceBrief = false;
static  bool                ListDeviceFull  = false;
static  bool                ListDevice32    = false;
static  bool                AddField        = false;
static  bool                ListFields      = false;

static  bool                ListAllDevice   = false;

static  bool                ListAllInt      = false;
static  bool                ListCdRomInt    = false;
static  bool                ListDiskInt     = false;
static  bool                ListFloppyInt   = false;
static  bool                ListPartInt     = false;
static  bool                ListPortInt     = false;
static  bool                ListHubInt      = false;
static  bool                ListUsbInt      = false;
static  bool                ListVolumeInt   = false;

static  bool                ListSomeInt     = false;

static  bool                ListProperties  = false;
static  bool                ListGUID        = false;

static  bool                Locale          = false;

static  bool                NonZero         = false;
static  bool                NoDetail        = false;
static  bool                NoHeader        = false;
static  bool                ShowError       = false;
static  bool                ShowStruct      = false;
static  bool                FullDetail      = false;

static  bool                ShowDevice      = false;
static  bool                ShowMedia       = false;
static  bool                ShowDrive       = false;
static  bool                ShowGeometry    = false;
static  bool                ShowStorage     = false;
static  bool                ShowScsi        = false;
static  bool                ShowPredict     = false;


static  bool                DebugMode       = false;
static  bool                Dump            = false;

static  bool                Flag            = false;

static  bool                Smart           = false;

static  bool                Pause           = false;

static  bool                PhysicalDisk    = true;
static  bool                ScsiDisk        = false;

static  bool                AtaMode         = false;

static  size_t              MaxNameLen      = 0;

static  _locale_t           LocaleType      = NULL;
static  WCHAR               LocaleString [ LEN_STRING ];

//
static  WCHAR               TextInfo [ LEN_MAX_STRING ];

//
static  WCHAR               DeviceName[MAX_PATH]    = {0};

//
#define                     DEVICE_MAX      256
static  int                 DevicesCount    = 0;
static  BYTE                DevicesNumber [ DEVICE_MAX ];
static  WCHAR               *DevicesName [ DEVICE_MAX ];

//
#define                     LEN_DUMP_LINE       128
static WCHAR                szDumpLineW [ LEN_DUMP_LINE ];
static CHAR                 szDumpLineA [ LEN_DUMP_LINE ];

//
#define                     MAX_ATTRIBUTES  256
static ST_SMART_INFO        SmartInfos [ MAX_ATTRIBUTES ];

//
#define _casereturn1(value) case value : return #value;
#define _casereturn2(value1,value2) case value1 : return #value2;

#define _wcasereturn1(value)    case value : return L#value;
#define _wcasereturn2(value)    case value : return FormatUpperString ( L#value );

//
struct TableDiskDriveStruct
{
    WCHAR   *name;
    int     width;
    bool    numeric;        // Default true will be changed
};

//
TableDiskDriveStruct    TableDiskDriveAll [] =
{
    { L"Availability", 0, true },
    { L"BytesPerSector", 0, true },
    { L"Capabilities", 0, true },
    { L"CapabilityDescriptions", 0, true },
    { L"Caption", 0, true },
    { L"CompressionMethod", 0, true },
    { L"ConfigManagerErrorCode", 0, true },
    { L"ConfigManagerUserConfig", 0, true },
    { L"DefaultBlockSize", 0, true },
    { L"Description", 0, true },
    { L"DeviceID", 0, true },
    { L"ErrorCleared", 0, true },
    { L"ErrorDescription", 0, true },
    { L"ErrorMethodology", 0, true },
    { L"Index", 0, true },
    { L"InstallDate", 0, true },
    { L"InterfaceType", 0, true },
    { L"LastErrorCode", 0, true },
    { L"Manufacturer", 0, true },
    { L"MaxBlockSize", 0, true },
    { L"MaxMediaSize", 0, true },
    { L"MediaLoaded", 0, true },
    { L"MediaType", 0, true },
    { L"MinBlockSize", 0, true },
    { L"Model", 0, true },
    { L"Name", 0, true },
    { L"NeedsCleaning", 0, true },
    { L"NumberOfMediaSupported", 0, true },
    { L"Partitions", 0, true },
    { L"PNPDeviceID", 0, true },
    { L"PowerManagementCapabilities", 0, true },
    { L"PowerManagementSupported", 0, true },
    { L"SCSIBus", 0, true },
    { L"SCSILogicalUnit", 0, true },
    { L"SCSIPort", 0, true },
    { L"SCSITargetId", 0, true },
    { L"SectorsPerTrack", 0, true },
    { L"Signature", 0, true },
    { L"Size", 0, true },
    { L"Status", 0, true },
    { L"StatusInfo", 0, true },
    { L"SystemName", 0, true },
    { L"TotalCylinders", 0, true },
    { L"TotalHeads", 0, true },
    { L"TotalSectors", 0, true },
    { L"TotalTracks", 0, true },
    { L"TracksPerCylinder", 0, true },
    { NULL, NULL, true },
};

//
static const int iTableDiskDriveUsedMax = 256;
static int iTableDiskDriveUsedCount     = 0;

TableDiskDriveStruct    TableDiskDriveUsed [iTableDiskDriveUsedMax];

//
ST_ATTRIBUTES       AttributeUnknown =
{
    -1,
    -1,
    L"Unknown Name",
    L"Unknown Details"
};

//
//  https://en.wikipedia.org/wiki/S.M.A.R.T.
//
static ST_ATTRIBUTES        Attributes [] =
{
    {
        1,
        CRITICAL_VALUE,
        L"Raw Read Error Rate",
        L"This attribute value depends of read errors, disk surface condition and indicates the rate of hardware read errors that occurred when reading data from a disk surface. Lower values indicate that there is a problem with either disk surface or read/write heads"
    },

    {
        2,
        HIGH_VALUE,
        L"Throughput Performance",
        L"Overall (general) throughput performance of a hard disk drive. If the value of this attribute is deceasing there is a high probability of troubles with your disk.",
    },

    {
        3,
        LOW_VALUE,
        L"Spin Up Time",
        L"Average time of spindle spin up (from zero RPM (Revolutions Per Minute) to fully operational). Active SMART shows the raw value of this attribute in milliseconds or seconds."
    },

    {
        4,
        0,
        L"Start/Stop Count",
        L"This raw value of this attribute is a count of hard disk spindle start/stop cycles"
    },

    {
        5,
        CRITICAL_VALUE,
        L"Reallocated Sectors Count",
        L"Count of reallocated sectors. When the hard drive finds a read/write/verification error, it marks this sector as reallocated and transfers data to a special reserved area (spare area). \
        This process is also known as remapping and reallocated sectors are called remaps. This is why, on a modern hard disks, you can not see bad blocks while testing the surface - all bad blocks are hidden in reallocated sectors. However, the more sectors that are reallocated, the more a sudden decrease (up to 10% and more) can be noticed in the disk read/write speed."
    },

    {
        6,
        0,
        L"Read Channel Margin",
        L"Margin of a channel while reading data. The function of this attribute is not specified."
    },

    {
        7,
        0,
        L"Seek Error Rate",
        L"Rate of seek errors of the magnetic heads. If there is a failure in the mechanical positioning system, a servo damage or a thermal widening of the hard disk, seek errors arise. More seek errors indicates a worsening condition of a disk surface and the disk mechanical subsystem."
    },

    {
        8,
        HIGH_VALUE,
        L"Seek Time Performance",
        L"Average performance of seek operations of the magnetic heads. If this attribute is decreasing, it is a sign of problems in the hard disk drive mechanical subsystem."
    },

    {
        9,
        0,
        L"Power-On Hours",
        L"Count of hours in power-on state. The raw value of this attribute shows total count of hours (or minutes, or seconds, depending on manufacturer) in power-on state. A decrease of this attribute value to the critical level (threshold) indicates a decrease of the MTBF (Mean Time Between Failures). \
        However, in reality, even if the MTBF value falls to zero, it does not mean that the MTBF resource is completely exhausted and the drive will not function normally."
    },

    {
        10,
        CRITICAL_VALUE,
        L"Spin Retry Count",
        L"Count of retry of spin start attempts. This attribute stores a total count of the spin start attempts to reach the fully operational speed (under the condition that the first attempt was unsuccessful). A decrease of this attribute value is a sign of problems in the hard disk mechanical subsystem."
    },

    {
        11,
        LOW_VALUE,
        L"Recalibration Retries",
        L"This attribute indicates the number of times recalibration was requested (under the condition that the first attempt was unsuccessful). A decrease of this attribute value is a sign of problems in the hard disk mechanical subsystem."
    },

    {
        12,
        0,
        L"Device Power Cycle Count",
        L"This attribute indicates the count of full hard Ddisk power on/off cycles."
    },

    {
        13,
        LOW_VALUE,
        L"Soft Read Error Rate",
        L"This is the rate of program read errors occurring when reading data from a disk surface."
    },

    {
        22,
        HIGH_VALUE,
        L"Current Helium Level",
        L"Specific to He8 drives from HGST. This value measures the helium inside of the drive specific to this manufacturer. It is a pre-fail attribute that trips once the drive detects that the internal environment is out of specification"
    },

    {
        160,
        0,
        L"Uncorrectable Sector Count",
        L""
    },

    {
        161,
        0,
        L"Valid Spare Blocks ",
        L""
    },

    {
        163,
        0,
        L"Initial Invalid Blocks",
        L""
    },

    {
        164,
        0,
        L"Total TLC Erase Count ",
        L""
    },

    {
        165,
        0,
        L"Maximum TLC Erase Count",
        L""
    },

    {
        166,
        0,
        L"Minimum TLC Erase Count",
        L""
    },

    {
        167,
        0,
        L"Average TLC Erase Count",
        L""
    },

    {
        169,
        0,
        L"Percentage Lifetime Remaining",
        L""
    },

    {
        171,
        0,
        L"SSD Program Fail Count",
        L""
    },

    {
        172,
        0,
        L"SSD Erase Fail Count",
        L""
    },

    {
        173,
        0,
        L"SSD Wear Leveling Count",
        L""
    },

    {
        174,
        0,
        L"Unexpected Power Loss Count",
        L""
    },

    {
        175,
        0,
        L"Power Loss Protection Failure",
        L""
    },

    {
        176,
        0,
        L"Erase Fail Count",
        L""
    },

    {
        177,
        0,
        L"Wear Range Delta",
        L""
    },

    {
        178,
        0,
        L"Used Reserved Block Count",
        L""
    },

    {
        179,
        0,
        L"Used Reserved Block Count Total",
        L""
    },

    {
        180,
        0,
        L"Unused Reserved Block Count Total",
        L""
    },

    {
        181,
        LOW_VALUE,
        L"Program Fail Count Total or Non-4K Aligned Access Count",
        L""
    },

    {
        182,
        0,
        L"Erase Fail Count",
        L""
    },

    {
        183,
        LOW_VALUE,
        L"SATA Downshift Error Count or Runtime Bad Block",
        L""
    },

    {
        184,
        CRITICAL_VALUE,
        L"End-to-End error / IOEDC",
        L""
    },

    {
        185,
        0,
        L"Head Stability",
        L""
    },

    {
        186,
        0,
        L"Induced Op-Vibration Detection",
        L""
    },

    {
        187,
        CRITICAL_VALUE,
        L"Reported Uncorrectable Errors",
        L""
    },

    {
        188,
        CRITICAL_VALUE,
        L"Command Timeout",
        L""
    },

    {
        189,
        LOW_VALUE,
        L"High Fly Writes",
        L""
    },

    {
        190,
        0,
        L"Temperature Difference or Airflow Temperature",
        L""
    },

    {
        191,
        LOW_VALUE,
        L"G-sense Error Rate",
        L""
    },

    {
        192,
        LOW_VALUE,
        L"Power-off Retract Count",
        L"Number of power-off or emergency retract cycles"
    },

    {
        193,
        LOW_VALUE,
        L"Load/Unload Cycle Count",
        L"Count of load/unload cycles into Landing Zone position. For more info see the Head Load/Unload Technology description."
    },

    {
        193,
        0,
        L"Load Cycle Count or Load/Unload Cycle Count (Fujitsu)",
        L""
    },

    {
        194,
        LOW_VALUE,
        L"Temperature",
        L"Hard disk drive temperature. The raw value of this attribute shows built-in heat sensor registrations (in degrees centigrade)."
    },

    {
        195,
        0,
        L"Hardware ECC Recovered",
        L""
    },

    {
        196,
        CRITICAL_VALUE,
        L"Reallocation Event Count",
        L"Count of remap operations (transfering data from a bad sector to a special reserved disk area - spare area). \
        The raw value of this attribute shows the total number of attempts to transfer data from reallocated sectors to a spare area. Unsuccessful attempts are counted as well as successful."
    },

    {
        197,
        CRITICAL_VALUE,
        L"Current Pending Sector Count",
        L"Current count of unstable sectors (waiting for remapping). The raw value of this attribute indicates the total number of sectors waiting for remapping. Later, when some of these sectors are read successfully, the value is decreased. If errors still occur when reading some sector, the hard drive will try to restore the data, transfer it to the reserved disk area (spare area) and mark this sector as remapped. If this attribute value remains at zero, it indicates that the quality of the corresponding surface area is low."
    },

    {
        198,
        CRITICAL_VALUE,
        L"Uncorrectable Sector Count",
        L"Quantity of uncorrectable errors. The raw value of this attribute indicates the total number of uncorrectable errors when reading/writing a sector. A rise in the value of this attribute indicates that there are evident defects of the disk surface and/or there are problems in the hard disk drive mechanical subsystem."
    },

    {
        199,
        LOW_VALUE,
        L"UltraDMA CRC Error Count",
        L"The total quantity of CRC errors during UltraDMA mode. The raw value of this attribute indicates the number of errors found during data transfer in UltraDMA mode by ICRC (Interface CRC)."
    },

    {
        200,
        LOW_VALUE,
        L"Write Error Rate (Multi Zone Error Rate)",
        L"Write data errors rate. This attribute indicates the total number of errors found when writing a sector. The higher the raw value, the worse the disk surface condition and/or mechanical subsystem is."
    },

    {
        201,
        CRITICAL_VALUE,
        L"Soft Read Error Rate or TA Counter Detected",
        L""
    },

    {
        202,
        LOW_VALUE,
        L"Data Address Mark errors or TA Counter Increased",
        L""
    },

    {
        203,
        LOW_VALUE,
        L"Run Out Cancel",
        L""
    },

    {
        204,
        LOW_VALUE,
        L"Soft ECC Correction",
        L""
    },

    {
        205,
        LOW_VALUE,
        L"Thermal Asperity Rate",
        L""
    },

    {
        206,
        0,
        L"Flying Height",
        L""
    },

    {
        207,
        LOW_VALUE,
        L"Spin High Current",
        L""
    },

    {
        208,
        0,
        L"Spin Buzz",
        L""
    },

    {
        209,
        0,
        L"Offline Seek Performance",
        L""
    },

    {
        210,
        0,
        L"Vibration During Write",
        L""
    },

    {
        220,
        LOW_VALUE,
        L"Disk Shift",
        L"Distance the disk has shifted relative to the spindle (usually due to shock or temperature). Unit of measure is unknown"
    },

    {
        221,
        LOW_VALUE,
        L"G-Sense Error Rate",
        L"Rate of errors occurring as a result of impact loads. This attribute stores an indication of a shock-sensitive sensor, that is, the total quantity of errors occurring as a result of internal impact loads (dropping drive, wrong installation, etc.)."
    },

    {
        224,
        LOW_VALUE,
        L"Load Friction",
        L"Resistance caused by friction in mechanical parts while operating"
    },

    {
        225,
        LOW_VALUE,
        L"Load/Unload Cycle Count",
        L"Total count of load cycles[36] Some drives use 193 (0xC1) for Load Cycle Count instead. See Description for 193 for significance of this number. "
    },

    {
        227,
        LOW_VALUE,
        L"Torque Amplification Count",
        L"Count of attempts to compensate for platter speed variations"
    },

    {
        228,
        0,
        L"Power-Off Retract Count",
        L"This attribute shows a count of the number of times the drive was powered down."
    },

    {
        220,
        CRITICAL_VALUE,
        L"Disk Shift",
        L"Shift of disks towards spindle. The raw value of this attribute indicates how much the disk has shifted. Unit measure is unknown. For more info see G-Force Protection technology description (click to learn more on Seagate website ). \
        NOTE: Shift of disks is possible as a result of a strong shock or a fall, or for other reasons."
    },

    {
        222,
        0,
        L"Loaded Hours",
        L"Loading on magnetic heads actuator caused by the general operating time. Only time when the actuator was in the operating position is counted."
    },

    {
        223,
        0,
        L"Load/Unload Retry Count",
        L"Loading on magnetic heads actuator caused by numerous recurrences of operations like: reading, recording, positioning of heads, etc. Only the time when heads were in the operating position is counted."
    },

    {
        224,
        0,
        L"Load Friction",
        L"Loading on magnetic heads actuator caused by friction in mechanical parts of the store. Only the time when heads were in the operating position is counted."
    },

    {
        226,
        0,
        L"Load-in Time",
        L"Total time of loading on the magnetic heads actuator. This attribute indicates total time in which the drive was under load (on the assumption that the magnetic heads were in operating mode and out of the parking area)."
    },

    {
        227,
        0,
        L"Torque Amplification Count",
        L"Count of efforts of the rotating moment of a drive."
    },

    {
        230,
        0,
        L"GMR Head Amplitude",
        L"Amplitude of heads trembling (GMR-head) in running mode."
    },


    {
        235,
        0,
        L"Good Block Count AND System(Free) Block Count",
        L""
    },


    {
        240,
        0,
        L"Head Flying Hours or 'Transfer Error Rate' (Fujitsu)",
        L""
    },

    {
        241,
        0,
        L"Total LBAs Written",
        L""
    },

    {
        242,
        0,
        L"Total LBAs Read",
        L""
    },

    {
        243,
        0,
        L"Total LBAs Written Expanded",
        L""
    },

    {
        244,
        0,
        L"Total LBAs Read Expanded",
        L""
    },

    {
        245,
        0,
        L"Max Erase Count",
        L""
    },

    {
        246,
        0,
        L"Cumulative host sectors written",
        L""
    },

    {
        247,
        0,
        L"Number of NAND pages of data written by the host",
        L""
    },

    {
        248,
        0,
        L"Number of NAND pages written by the FTL",
        L""
    },

    {
        249,
        0,
        L"NAND Writes (1GiB)",
        L""
    },

    {
        250,
        LOW_VALUE,
        L"Read Error Retry Rate",
        L""
    },

    {
        254,
        LOW_VALUE,
        L"Count of Free Fall Events detected",
        L""
    },

};

//
//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
int CompareIgnoreCase ( WCHAR one, WCHAR two )
{
    if ( toupper ( one ) == toupper ( two ) )
    {
        return 0;
    }
    else if ( toupper ( one ) < toupper ( two ) )
    {
        return -1;
    }
    else if ( toupper ( one ) > toupper ( two ) )
    {
        return 1;
    }

    return 2;
}

//
//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
int CompareIMin  ( WCHAR *pText, WCHAR *pKey, int minimum )
{
    int count       = 0;

    while ( *pKey != L'\0' && *pText != L'\0' )
    {
        int iRes = CompareIgnoreCase ( *pText, *pKey );
        if ( iRes != 0 )
        {
            return iRes;
        }
        pText++;
        pKey++;
        count++;
    }

    if ( count < minimum )
    {
        return -1;
    }

    return 0;
}

//
//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
int CompareIChr ( WCHAR *pText, WCHAR *pKey, WCHAR chr )
{
    int count       = 0;
    int minimum     = -1;

    while ( *pKey != L'\0' && *pText != L'\0' )
    {
        if ( *pKey == chr )
        {
            minimum = count;
            pKey++;
        }
        else
        {
            int iRes = CompareIgnoreCase ( *pText, *pKey );
            if ( iRes != 0 )
            {
                return iRes;
            }
            pText++;
            pKey++;
            count++;
        }
    }

    if ( count < minimum )
    {
        return -1;
    }

    return 0;
}

//
//====================================================================================
//
//====================================================================================
BOOL IsHexa ( const BYTE octet )
{
    if ( octet >= '0' && octet <= '9' )
    {
        return TRUE;
    }
    if ( octet >= 'A' && octet <= 'F' )
    {
        return TRUE;
    }
    if ( octet >= 'a' && octet <= 'f' )
    {
        return TRUE;
    }
    return FALSE;
}

//
//====================================================================================
//
//====================================================================================
BOOL IsUSASCII ( const BYTE octet )
{
    if ( octet >= ' ' && octet <= 0x7e )
    {
        return TRUE;
    }
    return FALSE;
}

//
///////////////////////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////////////////////
void SwitchBytes ( BYTE *octets, size_t len )
{
    for ( size_t i = 0; i < len; i = i + 2 )
    {
        BYTE temp           = octets [ i ];
        octets [ i ]        = octets [ i + 1 ];
        octets [ i + 1 ]    = temp;
    }
}


//
//====================================================================================
//
//====================================================================================
BOOL IsHexa ( const BYTE *pText )
{
    //
    //  Odd
    if ( ! ( strlen( ( char * ) pText )  % 2 == 0 ) )
    {
        return FALSE;
    }

    //
    while ( *pText != '\0' )
    {
        if ( ! IsHexa ( *pText ) )
        {
            return FALSE;
        }
        pText++;
    }

    return TRUE;
}

//
//====================================================================================
//
//====================================================================================
BOOL IsUSASCII ( const BYTE *pText )
{
    //
    while ( *pText != '\0' )
    {
        if ( ! IsUSASCII ( *pText ) )
        {
            return FALSE;
        }
        pText++;
    }

    return TRUE;
}

//
//====================================================================================
//
//====================================================================================
BYTE HexaToBin ( const BYTE hexa )
{
    char octet = '\0';
    if ( hexa >= '0' && hexa <= '9' )
    {
        octet = hexa - '0';
    }
    else if ( hexa >= 'A' && hexa <= 'F' )
    {
        octet = hexa - 'A' + 10;
    }
    else if ( hexa >= L'a' && hexa <= 'f' )
    {
        octet = hexa - 'a' + 10;
    }
    return octet;
}

//
//====================================================================================
//
//====================================================================================
BYTE HexaToBin ( const BYTE hexa [ 2 ] )
{
    BYTE octet = ( HexaToBin ( hexa [ 0 ] ) << 4 ) | HexaToBin ( hexa [ 1 ] );
    return octet;
}

//
//====================================================================================
//
//====================================================================================
BOOL HexaToBin ( BYTE *pText, BYTE *pBytes, size_t iSizeInBytes )
{
    ZeroMemory ( pBytes, iSizeInBytes );
    size_t iSize = strlen( ( char * ) pText);

    //  Return Size Must be at least the string size plus one
    if ( iSizeInBytes < iSize + 1 )
    {
        return FALSE;
    }

    if ( IsHexa ( pText ) )
    {
        size_t index = 0;
        while ( *pText != '\0' && index < iSizeInBytes )
        {
            pBytes [ index ] = HexaToBin ( pText );
            index++;
            pText = pText + 2;
        }

        return TRUE;
    }

    return FALSE;
}


//
//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
BOOL Is_WinXP_SP2_or_Later ()
{
   OSVERSIONINFOEX osvi;
   DWORDLONG dwlConditionMask = 0;
   int op=VER_GREATER_EQUAL;

   // Initialize the OSVERSIONINFOEX structure.

   ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));
   osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
   osvi.dwMajorVersion = 5;
   osvi.dwMinorVersion = 1;
   osvi.wServicePackMajor = 2;
   osvi.wServicePackMinor = 0;

   // Initialize the condition mask.

   VER_SET_CONDITION( dwlConditionMask, VER_MAJORVERSION, op );
   VER_SET_CONDITION( dwlConditionMask, VER_MINORVERSION, op );
   VER_SET_CONDITION( dwlConditionMask, VER_SERVICEPACKMAJOR, op );
   VER_SET_CONDITION( dwlConditionMask, VER_SERVICEPACKMINOR, op );

   // Perform the test.

   return VerifyVersionInfo(
      &osvi,
      VER_MAJORVERSION | VER_MINORVERSION |
      VER_SERVICEPACKMAJOR | VER_SERVICEPACKMINOR,
      dwlConditionMask);
}

//
//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
BOOL Is_Win7_SP1_or_Later ()
{
   OSVERSIONINFOEX osvi;
   DWORDLONG dwlConditionMask = 0;
   int op=VER_GREATER_EQUAL;

   // Initialize the OSVERSIONINFOEX structure.

   ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));
   osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
   osvi.dwMajorVersion = 6;
   osvi.dwMinorVersion = 1;
   osvi.wServicePackMajor = 1;
   osvi.wServicePackMinor = 0;

   // Initialize the condition mask.

   VER_SET_CONDITION( dwlConditionMask, VER_MAJORVERSION, op );
   VER_SET_CONDITION( dwlConditionMask, VER_MINORVERSION, op );
   VER_SET_CONDITION( dwlConditionMask, VER_SERVICEPACKMAJOR, op );
   VER_SET_CONDITION( dwlConditionMask, VER_SERVICEPACKMINOR, op );

   // Perform the test.

   return VerifyVersionInfo(
      &osvi,
      VER_MAJORVERSION | VER_MINORVERSION |
      VER_SERVICEPACKMAJOR | VER_SERVICEPACKMINOR,
      dwlConditionMask);
}

//
//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
BOOL Is_Win8_or_Later ()
{
   OSVERSIONINFOEX osvi;
   DWORDLONG dwlConditionMask = 0;
   int op=VER_GREATER_EQUAL;

   // Initialize the OSVERSIONINFOEX structure.

   ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));
   osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
   osvi.dwMajorVersion = 6;
   osvi.dwMinorVersion = 2;
   osvi.wServicePackMajor = 0;
   osvi.wServicePackMinor = 0;

   // Initialize the condition mask.

   VER_SET_CONDITION( dwlConditionMask, VER_MAJORVERSION, op );
   VER_SET_CONDITION( dwlConditionMask, VER_MINORVERSION, op );
   VER_SET_CONDITION( dwlConditionMask, VER_SERVICEPACKMAJOR, op );
   VER_SET_CONDITION( dwlConditionMask, VER_SERVICEPACKMINOR, op );

   // Perform the test.

   return VerifyVersionInfo(
      &osvi,
      VER_MAJORVERSION | VER_MINORVERSION |
      VER_SERVICEPACKMAJOR | VER_SERVICEPACKMINOR,
      dwlConditionMask);
}

//
//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
const WCHAR *OSVersion()
{
    BOOL bRet                   = FALSE;
    DWORD dwFlags               = FILE_VER_GET_NEUTRAL;
    DWORD dwHandle              = NULL;
    const WCHAR *pKernel32dll   = L"kernel32.dll";
    static WCHAR szVersion [ LEN_STRING ];
    ZeroMemory ( szVersion, sizeof(szVersion));

    //
    DWORD dwSize = GetFileVersionInfoSizeEx ( dwFlags, pKernel32dll, &dwHandle );
    if ( dwSize > 0 && dwSize  < LEN_MAX_STRING )
    {
        BYTE *pByteVersion = ( BYTE * ) malloc ( dwSize );

        dwFlags     = FILE_VER_GET_NEUTRAL;
        dwHandle    = NULL;
        BOOL bRet = GetFileVersionInfoEx(FILE_VER_GET_NEUTRAL, pKernel32dll, dwHandle, dwSize, pByteVersion );
        if ( bRet )
        {
            UINT size   = 0;
            void *ptr   = nullptr;
            bRet = VerQueryValue ( pByteVersion, L"\\", &ptr, &size );
            VS_FIXEDFILEINFO *pFixed = ( VS_FIXEDFILEINFO * ) ptr;

            swprintf_s (
                szVersion, _wsizeof(szVersion), L"%d.%d.%d.%d",
                HIWORD(pFixed->dwFileVersionMS), LOWORD(pFixed->dwFileVersionMS),
                HIWORD(pFixed->dwFileVersionLS), LOWORD(pFixed->dwFileVersionLS) );
        }   
        free ( pByteVersion );
    }

    return szVersion;
}
//
//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
WCHAR *GetLastErrorText  (DWORD dwLastError )
{
    ZeroMemory ( szErrorText, sizeof (szErrorText) );

    switch ( dwLastError )
    {
    case ERROR_INVALID_FUNCTION :
        wcscpy_s ( szErrorText, sizeof(szErrorText) / sizeof(WCHAR), L"Error Invalid Function" );
        break;
    case ERROR_INVALID_DATA :
        wcscpy_s ( szErrorText, sizeof(szErrorText) / sizeof(WCHAR), L"Error Invalid Data" );
        break;
    case ERROR_NOT_SUPPORTED :
        wcscpy_s ( szErrorText, sizeof(szErrorText) / sizeof(WCHAR), L"Error Not Supported" );
        break;
    case ERROR_FILE_NOT_FOUND :
        wcscpy_s ( szErrorText, sizeof(szErrorText) / sizeof(WCHAR), L"File Not Found" );
        break;
    case ERROR_PATH_NOT_FOUND :
        wcscpy_s ( szErrorText, sizeof(szErrorText) / sizeof(WCHAR), L"Path Not Found" );
        break;
    case ERROR_ACCESS_DENIED :
        wcscpy_s ( szErrorText, sizeof(szErrorText) / sizeof(WCHAR), L"Access Denied" );
        break;
    case ERROR_INVALID_ACCESS :
        wcscpy_s ( szErrorText, sizeof(szErrorText) / sizeof(WCHAR), L"Invalid Access" );
        break;
    case ERROR_WRITE_PROTECT :
        wcscpy_s ( szErrorText, sizeof(szErrorText) / sizeof(WCHAR), L"Write Protect" );
        break;
    case ERROR_SHARING_VIOLATION :
        wcscpy_s ( szErrorText, sizeof(szErrorText) / sizeof(WCHAR), L"Sharing Violation" );
        break;
    case  ERROR_INVALID_USER_BUFFER :
        wcscpy_s ( szErrorText, sizeof(szErrorText) / sizeof(WCHAR), L"Invalid User Buffer" );
        break;
    case  ERROR_NOT_ENOUGH_MEMORY :
        wcscpy_s ( szErrorText, sizeof(szErrorText) / sizeof(WCHAR), L"Not Enough Memory" );
        break;
    case  ERROR_OPERATION_ABORTED :
        wcscpy_s ( szErrorText, sizeof(szErrorText) / sizeof(WCHAR), L"Operation Aborted" );
        break;
    case  ERROR_NOT_ENOUGH_QUOTA :
        wcscpy_s ( szErrorText, sizeof(szErrorText) / sizeof(WCHAR), L"Not Enough Quota" );
        break;
    case  ERROR_IO_PENDING :
        wcscpy_s ( szErrorText, sizeof(szErrorText) / sizeof(WCHAR), L"IO Pending" );
        break;
    case ERROR_INVALID_PARAMETER :
        wcscpy_s ( szErrorText, sizeof(szErrorText) / sizeof(WCHAR), L"Invalid Parameter" );
        break;
    case ERROR_INVALID_NAME :
        wcscpy_s ( szErrorText, sizeof(szErrorText) / sizeof(WCHAR), L"Invalid Name" );
        break;
    case ERROR_FILE_EXISTS :
        wcscpy_s ( szErrorText, sizeof(szErrorText) / sizeof(WCHAR), L"File Exists" );
        break;
    case FVE_E_LOCKED_VOLUME :
        wcscpy_s ( szErrorText, sizeof(szErrorText) / sizeof(WCHAR), L"Locked Volume" );
        break;
    case ERROR_TOO_MANY_SEM_REQUESTS :
        wcscpy_s ( szErrorText, sizeof(szErrorText) / sizeof(WCHAR), L"Too Many Sem Requests" );
        break;
    case ERROR_INSUFFICIENT_BUFFER :
        wcscpy_s ( szErrorText, sizeof(szErrorText) / sizeof(WCHAR), L"Insufficient Buffer" );
        break;
    case ERROR_NO_MORE_ITEMS :
        wcscpy_s ( szErrorText, sizeof(szErrorText) / sizeof(WCHAR), L"No More Items" );
        break;
    default :
        swprintf_s ( szErrorText, sizeof(szErrorText) / sizeof(WCHAR), L"Unknown Error 0x%08lx", dwLastError );
        break;
    }

    return szErrorText;
}

//
//====================================================================================
//
//====================================================================================
WCHAR *GetLastErrorText ()
{
    DWORD dwLastError = GetLastError();
    return GetLastErrorText ( dwLastError) ;
}

//
//====================================================================================
//
//====================================================================================
WCHAR *FormatUpperString ( const WCHAR *pText )
{
    static WCHAR    szText [ LEN_STRING ];
    ZeroMemory( szText, sizeof(szText) );

    int i = 0;
    bool bToUpper = TRUE;
    while ( *pText != L'\0' )
    {
        if ( *pText == L'_' )
        {
            bToUpper = TRUE;
        }
        //  To Uppercase and not upper
        else if ( bToUpper && islower ( *pText ) )
        {
            szText [ i ] = toupper ( *pText );
            i++;
            bToUpper = FALSE;
        }
        //  To Lowercase and not lower
        else if ( ! bToUpper && isupper ( *pText ) )
        {
            szText [ i ] = tolower ( *pText );
            i++;
        }
        else
        {
            szText [ i ] = *pText;
            i++;
            bToUpper = FALSE;
        }
        pText++;
    }
    return szText;
}

//
//====================================================================================
//
//====================================================================================
const char *DetectionTypeToString ( DETECTION_TYPE detectionType )
{
    switch ( detectionType )
    {
        _casereturn2(DetectNone,None)
        _casereturn2(DetectInt13,Int13)
        _casereturn2(DetectExInt13,ExInt13)
    }
    return "Detect_???";
}


//
//====================================================================================
//
//====================================================================================
const char *PartitionStyleToString ( PARTITION_STYLE partitionStyle )
{
    switch ( partitionStyle )
    {
        _casereturn2(PARTITION_STYLE_MBR,MBR)
        _casereturn2(PARTITION_STYLE_GPT,GPT)
        _casereturn2(PARTITION_STYLE_RAW,RAW)
    }
    return "STYLE_???";
}

//
//====================================================================================
//
//====================================================================================
const char *PartitionTypeToString ( UCHAR partitionType )
{

    switch ( partitionType )
    {
        _casereturn2(PARTITION_ENTRY_UNUSED,UNUSED)
        _casereturn2(PARTITION_FAT_12,FAT_12)
        _casereturn2(PARTITION_XENIX_1,XENIX_1)
        _casereturn2(PARTITION_XENIX_2,XENIX_2)
        _casereturn2(PARTITION_FAT_16,)
        _casereturn2(PARTITION_EXTENDED,FAT_16)
        _casereturn2(PARTITION_HUGE,HUGE)
        _casereturn2(PARTITION_IFS,IFS)
        _casereturn2(PARTITION_OS2BOOTMGR,OS2BOOTMGR)
        _casereturn2(PARTITION_FAT32,FAT32)
        _casereturn2(PARTITION_FAT32_XINT13,XINT13)
        _casereturn2(PARTITION_XINT13,XINT13)
        _casereturn2(PARTITION_XINT13_EXTENDED,XINT13_EXTENDED)
        _casereturn2(PARTITION_PREP,PREP)
        _casereturn2(PARTITION_LDM,LDM)
        _casereturn2(PARTITION_UNIX,UNIX)
        _casereturn2(PARTITION_SPACES,SPACES)
    }

static char szPartitionType [ LEN_STRING ];
    sprintf_s ( szPartitionType, sizeof(szPartitionType), "TYPE_0x%02x", partitionType );
    return szPartitionType;
}

//
//====================================================================================
//
//====================================================================================
const WCHAR *BusTypeToString ( STORAGE_BUS_TYPE busType )
{
    switch ( busType )
    {
        _wcasereturn1(BusTypeUnknown)
        _wcasereturn1(BusTypeScsi)
        _wcasereturn1(BusTypeAtapi)
        _wcasereturn1(BusTypeAta)
        _wcasereturn1(BusType1394)
        _wcasereturn1(BusTypeSsa)
        _wcasereturn1(BusTypeFibre)
        _wcasereturn1(BusTypeUsb)
        _wcasereturn1(BusTypeRAID)
        _wcasereturn1(BusTypeiScsi)
        _wcasereturn1(BusTypeSas)
        _wcasereturn1(BusTypeSata)
        _wcasereturn1(BusTypeSd)
        _wcasereturn1(BusTypeMmc)
        _wcasereturn1(BusTypeVirtual)
        _wcasereturn1(BusTypeFileBackedVirtual)
        _wcasereturn1(BusTypeSpaces)
        _wcasereturn1(BusTypeMax)
        _wcasereturn1(BusTypeMaxReserved)
    }

    return L"BusType_???";
}

//
//====================================================================================
//
//====================================================================================
const WCHAR *MediaTypeToString ( STORAGE_MEDIA_TYPE mediaType )
{
    switch ( mediaType )
    {
        _wcasereturn1(Unknown)                // Format is unknown
        _wcasereturn1(F5_1Pt2_512)            // 5.25") 1.2MB)  512 bytes/sector
        _wcasereturn1(F3_1Pt44_512)           // 3.5")  1.44MB) 512 bytes/sector
        _wcasereturn1(F3_2Pt88_512)           // 3.5")  2.88MB) 512 bytes/sector
        _wcasereturn1(F3_20Pt8_512)           // 3.5")  20.8MB) 512 bytes/sector
        _wcasereturn1(F3_720_512)             // 3.5")  720KB)  512 bytes/sector
        _wcasereturn1(F5_360_512)             // 5.25") 360KB)  512 bytes/sector
        _wcasereturn1(F5_320_512)             // 5.25") 320KB)  512 bytes/sector
        _wcasereturn1(F5_320_1024)            // 5.25") 320KB)  1024 bytes/sector
        _wcasereturn1(F5_180_512)             // 5.25") 180KB)  512 bytes/sector
        _wcasereturn1(F5_160_512)             // 5.25") 160KB)  512 bytes/sector
        _wcasereturn1(RemovableMedia)         // Removable media other than floppy
        _wcasereturn1(FixedMedia)             // Fixed hard disk media
        _wcasereturn1(F3_120M_512)            // 3.5") 120M Floppy
        _wcasereturn1(F3_640_512)             // 3.5" )  640KB)  512 bytes/sector
        _wcasereturn1(F5_640_512)             // 5.25")  640KB)  512 bytes/sector
        _wcasereturn1(F5_720_512)             // 5.25")  720KB)  512 bytes/sector
        _wcasereturn1(F3_1Pt2_512)            // 3.5" )  1.2Mb)  512 bytes/sector
        _wcasereturn1(F3_1Pt23_1024)          // 3.5" )  1.23Mb) 1024 bytes/sector
        _wcasereturn1(F5_1Pt23_1024)          // 5.25")  1.23MB) 1024 bytes/sector
        _wcasereturn1(F3_128Mb_512)           // 3.5" MO 128Mb   512 bytes/sector
        _wcasereturn1(F3_230Mb_512)           // 3.5" MO 230Mb   512 bytes/sector
        _wcasereturn1(F8_256_128)             // 8")     256KB)  128 bytes/sector
        _wcasereturn1(F3_200Mb_512)           // 3.5")   200M Floppy (HiFD)
        _wcasereturn1(F3_240M_512)            // 3.5")   240Mb Floppy (HiFD)
        _wcasereturn1(F3_32M_512)              // 3.5")   32Mb Floppy

        _wcasereturn1(DDS_4mm)
        _wcasereturn1(MiniQic)                   // Tape - miniQIC Tape
        _wcasereturn1(Travan)                    // Tape - Travan TR-1)2)3)...
        _wcasereturn1(QIC)                       // Tape - QIC
        _wcasereturn1(MP_8mm)                    // Tape - 8mm Exabyte Metal Particle
        _wcasereturn1(AME_8mm)                   // Tape - 8mm Exabyte Advanced Metal Evap
        _wcasereturn1(AIT1_8mm)                  // Tape - 8mm Sony AIT
        _wcasereturn1(DLT)                       // Tape - DLT Compact IIIxt) IV
        _wcasereturn1(NCTP)                      // Tape - Philips NCTP
        _wcasereturn1(IBM_3480)                  // Tape - IBM 3480
        _wcasereturn1(IBM_3490E)                 // Tape - IBM 3490E
        _wcasereturn1(IBM_Magstar_3590)          // Tape - IBM Magstar 3590
        _wcasereturn1(IBM_Magstar_MP)            // Tape - IBM Magstar MP
        _wcasereturn1(STK_DATA_D3)               // Tape - STK Data D3
        _wcasereturn1(SONY_DTF)                  // Tape - Sony DTF
        _wcasereturn1(DV_6mm)                    // Tape - 6mm Digital Video
        _wcasereturn1(DMI)                       // Tape - Exabyte DMI and compatibles
        _wcasereturn1(SONY_D2)                   // Tape - Sony D2S and D2L
        _wcasereturn1(CLEANER_CARTRIDGE)         // Cleaner - All Drive types that support Drive Cleaners
        _wcasereturn1(CD_ROM)                    // Opt_Disk - CD
        _wcasereturn1(CD_R)                      // Opt_Disk - CD-Recordable (Write Once)
        _wcasereturn1(CD_RW)                     // Opt_Disk - CD-Rewriteable
        _wcasereturn1(DVD_ROM)                   // Opt_Disk - DVD-ROM
        _wcasereturn1(DVD_R)                     // Opt_Disk - DVD-Recordable (Write Once)
        _wcasereturn1(DVD_RW)                    // Opt_Disk - DVD-Rewriteable
        _wcasereturn1(MO_3_RW)                   // Opt_Disk - 3.5" Rewriteable MO Disk
        _wcasereturn1(MO_5_WO)                   // Opt_Disk - MO 5.25" Write Once
        _wcasereturn1(MO_5_RW)                   // Opt_Disk - MO 5.25" Rewriteable (not LIMDOW)
        _wcasereturn1(MO_5_LIMDOW)               // Opt_Disk - MO 5.25" Rewriteable (LIMDOW)
        _wcasereturn1(PC_5_WO)                   // Opt_Disk - Phase Change 5.25" Write Once Optical
        _wcasereturn1(PC_5_RW)                   // Opt_Disk - Phase Change 5.25" Rewriteable
        _wcasereturn1(PD_5_RW)                   // Opt_Disk - PhaseChange Dual Rewriteable
        _wcasereturn1(ABL_5_WO)                  // Opt_Disk - Ablative 5.25" Write Once Optical
        _wcasereturn1(PINNACLE_APEX_5_RW)        // Opt_Disk - Pinnacle Apex 4.6GB Rewriteable Optical
        _wcasereturn1(SONY_12_WO)                // Opt_Disk - Sony 12" Write Once
        _wcasereturn1(PHILIPS_12_WO)             // Opt_Disk - Philips/LMS 12" Write Once
        _wcasereturn1(HITACHI_12_WO)             // Opt_Disk - Hitachi 12" Write Once
        _wcasereturn1(CYGNET_12_WO)              // Opt_Disk - Cygnet/ATG 12" Write Once
        _wcasereturn1(KODAK_14_WO)               // Opt_Disk - Kodak 14" Write Once
        _wcasereturn1(MO_NFR_525)                // Opt_Disk - Near Field Recording (Terastor)
        _wcasereturn1(NIKON_12_RW)               // Opt_Disk - Nikon 12" Rewriteable
        _wcasereturn1(IOMEGA_ZIP)                // Mag_Disk - Iomega Zip
        _wcasereturn1(IOMEGA_JAZ)                // Mag_Disk - Iomega Jaz
        _wcasereturn1(SYQUEST_EZ135)             // Mag_Disk - Syquest EZ135
        _wcasereturn1(SYQUEST_EZFLYER)           // Mag_Disk - Syquest EzFlyer
        _wcasereturn1(SYQUEST_SYJET)             // Mag_Disk - Syquest SyJet
        _wcasereturn1(AVATAR_F2)                 // Mag_Disk - 2.5" Floppy
        _wcasereturn1(MP2_8mm)                   // Tape - 8mm Hitachi
        _wcasereturn1(DST_S)                     // Ampex DST Small Tapes
        _wcasereturn1(DST_M)                     // Ampex DST Medium Tapes
        _wcasereturn1(DST_L)                     // Ampex DST Large Tapes
        _wcasereturn1(VXATape_1)                 // Ecrix 8mm Tape
        _wcasereturn1(VXATape_2)                 // Ecrix 8mm Tape
        _wcasereturn1(STK_9840)                  // STK 9840
        _wcasereturn1(LTO_Ultrium)               // IBM) HP) Seagate LTO Ultrium
        _wcasereturn1(LTO_Accelis)               // IBM) HP) Seagate LTO Accelis
        _wcasereturn1(DVD_RAM)                   // Opt_Disk - DVD-RAM
        _wcasereturn1(AIT_8mm)                   // AIT2 or higher
        _wcasereturn1(ADR_1)                     // OnStream ADR Mediatypes
        _wcasereturn1(ADR_2)
        _wcasereturn1(STK_9940)                  // STK 9940
        _wcasereturn1(SAIT)                      // SAIT Tapes
        _wcasereturn1(VXATape)                    // VXA (Ecrix 8mm) Tape
    }

    return L"MediaType_???";
}
//
//====================================================================================
//
//====================================================================================
const WCHAR *DeviceTypeToString ( ULONG deviceType )
{
    switch ( deviceType )
    {
        _wcasereturn2(FILE_DEVICE_BEEP)
        _wcasereturn2(FILE_DEVICE_CD_ROM)
        _wcasereturn2(FILE_DEVICE_CD_ROM_FILE_SYSTEM)
        _wcasereturn2(FILE_DEVICE_CONTROLLER)
        _wcasereturn2(FILE_DEVICE_DATALINK)
        _wcasereturn2(FILE_DEVICE_DFS)
        _wcasereturn2(FILE_DEVICE_DISK)
        _wcasereturn2(FILE_DEVICE_DISK_FILE_SYSTEM)
        _wcasereturn2(FILE_DEVICE_FILE_SYSTEM)
        _wcasereturn2(FILE_DEVICE_INPORT_PORT)
        _wcasereturn2(FILE_DEVICE_KEYBOARD)
        _wcasereturn2(FILE_DEVICE_MAILSLOT)
        _wcasereturn2(FILE_DEVICE_MIDI_IN)
        _wcasereturn2(FILE_DEVICE_MIDI_OUT)
        _wcasereturn2(FILE_DEVICE_MOUSE)
        _wcasereturn2(FILE_DEVICE_MULTI_UNC_PROVIDER)
        _wcasereturn2(FILE_DEVICE_NAMED_PIPE)
        _wcasereturn2(FILE_DEVICE_NETWORK)
        _wcasereturn2(FILE_DEVICE_NETWORK_BROWSER)
        _wcasereturn2(FILE_DEVICE_NETWORK_FILE_SYSTEM)
        _wcasereturn2(FILE_DEVICE_NULL)
        _wcasereturn2(FILE_DEVICE_PARALLEL_PORT)
        _wcasereturn2(FILE_DEVICE_PHYSICAL_NETCARD)
        _wcasereturn2(FILE_DEVICE_PRINTER)
        _wcasereturn2(FILE_DEVICE_SCANNER)
        _wcasereturn2(FILE_DEVICE_SERIAL_MOUSE_PORT)
        _wcasereturn2(FILE_DEVICE_SERIAL_PORT)
        _wcasereturn2(FILE_DEVICE_SCREEN)
        _wcasereturn2(FILE_DEVICE_SOUND)
        _wcasereturn2(FILE_DEVICE_STREAMS)
        _wcasereturn2(FILE_DEVICE_TAPE)
        _wcasereturn2(FILE_DEVICE_TAPE_FILE_SYSTEM)
        _wcasereturn2(FILE_DEVICE_TRANSPORT)
        _wcasereturn2(FILE_DEVICE_UNKNOWN)
        _wcasereturn2(FILE_DEVICE_VIDEO)
        _wcasereturn2(FILE_DEVICE_VIRTUAL_DISK)
        _wcasereturn2(FILE_DEVICE_WAVE_IN)
        _wcasereturn2(FILE_DEVICE_WAVE_OUT)
        _wcasereturn2(FILE_DEVICE_8042_PORT)
        _wcasereturn2(FILE_DEVICE_NETWORK_REDIRECTOR)
        _wcasereturn2(FILE_DEVICE_BATTERY)
        _wcasereturn2(FILE_DEVICE_BUS_EXTENDER)
        _wcasereturn2(FILE_DEVICE_MODEM)
        _wcasereturn2(FILE_DEVICE_VDM)
        _wcasereturn2(FILE_DEVICE_MASS_STORAGE)
        _wcasereturn2(FILE_DEVICE_SMB)
        _wcasereturn2(FILE_DEVICE_KS)
        _wcasereturn2(FILE_DEVICE_CHANGER)
        _wcasereturn2(FILE_DEVICE_SMARTCARD)
        _wcasereturn2(FILE_DEVICE_ACPI)
        _wcasereturn2(FILE_DEVICE_DVD)
        _wcasereturn2(FILE_DEVICE_FULLSCREEN_VIDEO)
        _wcasereturn2(FILE_DEVICE_DFS_FILE_SYSTEM)
        _wcasereturn2(FILE_DEVICE_DFS_VOLUME)
        _wcasereturn2(FILE_DEVICE_SERENUM)
        _wcasereturn2(FILE_DEVICE_TERMSRV)
        _wcasereturn2(FILE_DEVICE_KSEC)
        _wcasereturn2(FILE_DEVICE_FIPS)
        _wcasereturn2(FILE_DEVICE_INFINIBAND)
        _wcasereturn2(FILE_DEVICE_VMBUS)
        _wcasereturn2(FILE_DEVICE_CRYPT_PROVIDER)
        _wcasereturn2(FILE_DEVICE_WPD)
        _wcasereturn2(FILE_DEVICE_BLUETOOTH)
        _wcasereturn2(FILE_DEVICE_MT_COMPOSITE)
        _wcasereturn2(FILE_DEVICE_MT_TRANSPORT)
        _wcasereturn2(FILE_DEVICE_BIOMETRIC)
        _wcasereturn2(FILE_DEVICE_PMI)
        _wcasereturn2(FILE_DEVICE_EHSTOR)
        _wcasereturn2(FILE_DEVICE_DEVAPI)
        _wcasereturn2(FILE_DEVICE_GPIO)
        _wcasereturn2(FILE_DEVICE_USBEX)
        _wcasereturn2(FILE_DEVICE_CONSOLE)
        _wcasereturn2(FILE_DEVICE_NFP)
        _wcasereturn2(FILE_DEVICE_SYSENV)
    }

    return FormatUpperString(L"FILE_DEVICE_???");
}

//
//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
BOOL ShowDetailLineA ( CHAR pSeparator, CHAR *pLabel, CHAR *format, ... )
{
    PrintNormalA ( "S.M.A.R.T. %c ", pSeparator );
    PrintNormalA ( "%-16s : ", pLabel );

    //
    va_list vaList;
    va_start(vaList, format);
    int result = PrintVA ( LocaleMode, true, stdout, format, vaList );
    va_end(vaList);

    //
    return TRUE;
}


//
//////////////////////////////////////////////////////////////////////////////
//  Dump Buffer
//////////////////////////////////////////////////////////////////////////////
void DumpBuffer ( CHAR pSeparator, CHAR *pLabel, BYTE *pBuffer, int iSize, BOOL bOffset, BOOL bHexa, BOOL bAscii )
{
    int iStep = 16;

    //
    if ( pBuffer != NULL && iSize > 0 )
    {
        for ( int i = 0; i < iSize; i = i + iStep )
        {
            //
            ZeroMemory ( szDumpLineA, sizeof ( szDumpLineA ) );

            //
            if ( bOffset )
            {
                sprintf_s ( szDumpLineA + strlen(szDumpLineA), sizeof(szDumpLineA) - strlen(szDumpLineA), "%04x : ", i );
            }
            for ( int j = i; j < i + iStep; j++ )
            {
                if ( j < iSize )
                {
                    sprintf_s ( szDumpLineA + strlen(szDumpLineA), sizeof(szDumpLineA) - strlen(szDumpLineA), "%02x ", pBuffer [ j ] );
                }
                else
                {
                    strcat_s ( szDumpLineA, sizeof(szDumpLineA), "__ " );
                }
            }

            if ( bAscii )
            {
                strcat_s ( szDumpLineA, sizeof(szDumpLineA), "- " );
                for ( int j = i; j < i + iStep; j++ )
                {
                    //
                    if ( j < iSize && pBuffer [ j ] >= ' ' && pBuffer [ j ] < 0x7f )
                    {
                        sprintf_s ( szDumpLineA + strlen(szDumpLineA), sizeof(szDumpLineA) - strlen(szDumpLineA), "%c", pBuffer [ j ] );
                    }
                    else
                    {
                        strcat_s ( szDumpLineA, sizeof(szDumpLineA), "." );
                    }
                }
            }

            ShowDetailLineA ( pSeparator, pLabel, "%s\n" , szDumpLineA );
        }
    }
}


//
///////////////////////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////////////////////
ST_ATTRIBUTES *FindAttribute ( int id )
{
    for ( int i = 0; i < sizeof(Attributes)/sizeof(ST_ATTRIBUTES); i++ )
    {
        ST_ATTRIBUTES *pAttribute = & ( Attributes [ i ] );
        if ( pAttribute->id == id )
        {
            return pAttribute;
        }
    }

    return &AttributeUnknown;
}

//
///////////////////////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////////////////////
const size_t FindAttributeNameMaxLen ( )
{
    size_t len = 0;
    for ( int i = 0; i < sizeof(Attributes)/sizeof(ST_ATTRIBUTES); i++ )
    {
        ST_ATTRIBUTES *pAttribute = & ( Attributes [ i ] );
        if ( wcslen ( pAttribute->name ) > len )
        {
            len = wcslen ( pAttribute->name );
        }
    }

    return len;
}

//
///////////////////////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////////////////////
const WCHAR *FindAttributeName ( int id )
{
    for ( int i = 0; i < sizeof(Attributes)/sizeof(ST_ATTRIBUTES); i++ )
    {
        ST_ATTRIBUTES *pAttribute = & ( Attributes [ i ] );
        if ( pAttribute->id == id )
        {
            return pAttribute->name;
        }
    }

    return L"Unknown";
}

//
///////////////////////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////////////////////
const WCHAR *FindAttributeDetails ( int id )
{
    for ( int i = 0; i < sizeof(Attributes)/sizeof(ST_ATTRIBUTES); i++ )
    {
        ST_ATTRIBUTES *pAttribute = & ( Attributes [ i ] );
        if ( pAttribute->id == id )
        {
            return pAttribute->details;
        }
    }

    return L"Unknown";
}

//
///////////////////////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////////////////////
void SetIdeRegsZero ( IDEREGS *pIdeRegs )
{
    pIdeRegs->bFeaturesReg      = 0;
    pIdeRegs->bSectorCountReg   = 0;
    pIdeRegs->bSectorNumberReg  = 0;
    pIdeRegs->bCylLowReg        = 0;
    pIdeRegs->bCylHighReg       = 0;
    pIdeRegs->bDriveHeadReg     = 0;
    pIdeRegs->bCommandReg       = 0;
}

//
///////////////////////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////////////////////
void SetIdeRegsDefault ( IDEREGS *pIdeRegs )
{
    pIdeRegs->bFeaturesReg      = 0;
    pIdeRegs->bSectorCountReg   = 1;
    pIdeRegs->bSectorNumberReg  = 1;
    pIdeRegs->bCylLowReg        = SMART_CYL_LOW;
    pIdeRegs->bCylHighReg       = SMART_CYL_HI;
    pIdeRegs->bDriveHeadReg     = DRIVE_HEAD_REG;
    pIdeRegs->bCommandReg       = 0;
}

//
///////////////////////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////////////////////
void SetAtaIdentify ( IDEREGS *pIdeRegs )
{
    SetIdeRegsDefault ( pIdeRegs );
    pIdeRegs->bCommandReg       = ID_CMD;
}

//
///////////////////////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////////////////////
void SetSmartIdentify ( IDEREGS *pIdeRegs )
{
    SetIdeRegsDefault ( pIdeRegs );
    pIdeRegs->bCommandReg       = ID_CMD;
}

//
///////////////////////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////////////////////
void SetScsiIdentify ( IDEREGS *pIdeRegs )
{
    SetIdeRegsDefault ( pIdeRegs );
    pIdeRegs->bCommandReg       = ID_CMD;
}

//
///////////////////////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////////////////////
void SetReturnSmartStatus ( IDEREGS *pIdeRegs )
{
    SetIdeRegsDefault ( pIdeRegs );
    pIdeRegs->bFeaturesReg      = RETURN_SMART_STATUS;
    pIdeRegs->bCommandReg       = SMART_CMD;
}

//
///////////////////////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////////////////////
void SetReadAttributes ( IDEREGS *pIdeRegs )
{
    SetIdeRegsDefault ( pIdeRegs );
    pIdeRegs->bFeaturesReg      = READ_ATTRIBUTES;
    pIdeRegs->bCommandReg       = SMART_CMD;
}

//
///////////////////////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////////////////////
void SetReadThresholds ( IDEREGS *pIdeRegs )
{
    SetIdeRegsDefault ( pIdeRegs );
    pIdeRegs->bFeaturesReg      = READ_THRESHOLDS;
    pIdeRegs->bCommandReg       = SMART_CMD;
}

//
///////////////////////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////////////////////
void SetEnableStart ( IDEREGS *pIdeRegs )
{
    SetIdeRegsDefault ( pIdeRegs );
    pIdeRegs->bFeaturesReg      = ENABLE_SMART;
    pIdeRegs->bCommandReg       = SMART_CMD;
}

//
///////////////////////////////////////////////////////////////////////////////
//  https://stackoverflow.com/questions/19520949/how-to-use-ioctl-scsi-miniport-via-deviceiocontrol-from-c-net
///////////////////////////////////////////////////////////////////////////////
BOOL ScsiMiniPortCommands ( HANDLE hDevice, ULONG code )
{
    BOOL            bRet    = FALSE;
    DWORD           dwRet   = 0;
    int             size    = sizeof(SRB_IO_CONTROL);
    WCHAR           *label  = L"";

    ZeroMemory ( &ScsiMiniPortInp, sizeof(ScsiMiniPortInp) );
    ZeroMemory ( &ScsiMiniPortOut, sizeof(ScsiMiniPortOut) );

    int dataSize = 0;
    if ( code == IOCTL_SCSI_MINIPORT_IDENTIFY )
    {
        label       = L"IOCTL_SCSI_MINIPORT_IDENTIFY";
        dataSize    = IDENTIFY_BUFFER_SIZE;
        SetScsiIdentify ( &ScsiMiniPortInp.io.inp.irDriveRegs );
    }
    else if ( code == IOCTL_SCSI_MINIPORT_SMART_VERSION )
    {
        label       = L"IOCTL_SCSI_MINIPORT_SMART_VERSION";
        dataSize    = READ_ATTRIBUTE_BUFFER_SIZE;
        ScsiMiniPortInp.io.inp.irDriveRegs.bCommandReg      = SMART_CMD;
    }
    else if ( code == IOCTL_SCSI_MINIPORT_ENABLE_SMART )
    {
        label       = L"IOCTL_SCSI_MINIPORT_ENABLE_SMART";
        dataSize    = READ_ATTRIBUTE_BUFFER_SIZE;
        SetEnableStart ( &ScsiMiniPortInp.io.inp.irDriveRegs );
    }
    else if ( code == IOCTL_SCSI_MINIPORT_READ_SMART_ATTRIBS )
    {
        label       = L"IOCTL_SCSI_MINIPORT_READ_SMART_ATTRIBS";
        dataSize    = READ_ATTRIBUTE_BUFFER_SIZE;
        SetReadAttributes ( &ScsiMiniPortInp.io.inp.irDriveRegs );
    }
    else if ( code == IOCTL_SCSI_MINIPORT_READ_SMART_THRESHOLDS )
    {
        label       = L"IOCTL_SCSI_MINIPORT_READ_SMART_THRESHOLDS";
        dataSize    = READ_THRESHOLD_BUFFER_SIZE;
        SetReadThresholds ( &ScsiMiniPortInp.io.inp.irDriveRegs );
    }
    else if ( code == IOCTL_SCSI_MINIPORT_RETURN_STATUS )
    {
        label       = L"IOCTL_SCSI_MINIPORT_RETURN_STATUS";
        dataSize    = sizeof ( IDEREGS );
        SetReturnSmartStatus ( &ScsiMiniPortInp.io.inp.irDriveRegs );
    }
    else
    {
        if ( ShowError )
        {
            PrintStderrW ( L"Error - Invalid Code %lu\n", code );
        }
        return FALSE;
    }

    //  Input SRBC
    ScsiMiniPortInp.srbc.HeaderLength                       = sizeof(SRB_IO_CONTROL);
    memcpy(ScsiMiniPortInp.srbc.Signature, IOCTL_MINIPORT_SIGNATURE_SCSIDISK, strlen(IOCTL_MINIPORT_SIGNATURE_SCSIDISK));
    ScsiMiniPortInp.srbc.Timeout                            = 5; // seconds
    ScsiMiniPortInp.srbc.ControlCode                        = code;
    ScsiMiniPortInp.srbc.Length                             = sizeof(ST_SENDCMDPARAMS) - 1 + dataSize;

    //  Inp
    ScsiMiniPortInp.io.inp.cBufferSize                      = dataSize;
    ScsiMiniPortInp.io.inp.bDriveNumber                     = ScsiAddress.TargetId;

    //  Output SRBC
    ScsiMiniPortOut.srbc.HeaderLength                       = sizeof(SRB_IO_CONTROL);
    memcpy(ScsiMiniPortOut.srbc.Signature, IOCTL_MINIPORT_SIGNATURE_SCSIDISK, strlen(IOCTL_MINIPORT_SIGNATURE_SCSIDISK));
    ScsiMiniPortOut.srbc.Timeout                            = 5; // seconds
    ScsiMiniPortOut.srbc.Length                             = sizeof(ST_SENDCMDPARAMS) - 1 + dataSize;
    ScsiMiniPortOut.srbc.ControlCode                        = code;
    ScsiMiniPortOut.io.out.cBufferSize                      = dataSize;
    
    bRet =
        DeviceIoControl (
            hDevice, IOCTL_SCSI_MINIPORT,
            &ScsiMiniPortInp, sizeof(SRB_IO_CONTROL) + sizeof(SENDCMDINPARAMS) - 1,
            &ScsiMiniPortOut, sizeof(SRB_IO_CONTROL) + sizeof(SENDCMDOUTPARAMS)  - 1 + dataSize,
            &dwRet, NULL );
    if ( ! bRet && ShowError )
    {
        dwRet = GetLastError();
        PrintStderrW ( L"Error - DeviceIoControl IOCTL_SCSI_MINIPORT(%s) Fails - Cause : 0x%lx - %s\n", label, GetLastError(), GetLastErrorText() );
    }

    return bRet;
}

//
///////////////////////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////////////////////
void PrintAttributes ()
{
    WCHAR           szFlag [ LEN_SHORT_STRING ];

    //
    PrintNormalW ( L"%*s %4s %c: %6s %5s %5s %16s (%8s %8s) %8s %5s\n",
        MaxNameLen, L"Name",
        L"Num", L'I', L"Flag", L"Val", L"Low", L"Data",  L"Data0", L"Data1", L"Specific", L"Thr" );
    for ( int i = 0;  i < MAX_ATTRIBUTES; i++ )
    {
        //
        //  Display Smart Info
        SmartInfo = SmartInfos [ i ];
        if ( SmartInfo.m_ucAttribIndex != 0 )
        {
            ST_ATTRIBUTES *pAttribute = FindAttribute ( SmartInfo.m_ucAttribIndex );
            bool show = true;
            if ( Low && ( pAttribute->critical < LOW_VALUE ) )
            {
                show = false;
            }

            if ( High && ( pAttribute->critical < HIGH_VALUE ) )
            {
                show = false;
            }

            if ( Critical && ( pAttribute->critical < CRITICAL_VALUE ) )
            {
                show = false;
            }

            if ( NonZero && ( SmartInfo.m_dwData == 0 ) )
            {
                show = false;
            }

            WCHAR indicator = L' ';
            if ( pAttribute->critical == LOW_VALUE )
            {
                indicator = L'L';
            }
            else if ( pAttribute->critical == HIGH_VALUE )
            {
                indicator = L'H';
            }
            else if ( pAttribute->critical == CRITICAL_VALUE )
            {
                indicator = L'C';
            }

            if ( show )
            {
                ZeroMemory ( szFlag, sizeof(szFlag) );
                if ( SmartInfo.m_wFlag & 0x01 )
                {
                    szFlag [ 0 ] = L'W';
                }
                else
                {
                    szFlag [ 0 ] = L'-';
                }

                if ( SmartInfo.m_wFlag & 0x02 )
                {
                    szFlag [ 1 ] = L'O';
                }
                else
                {
                    szFlag [ 1 ] = L'-';
                }

                if ( SmartInfo.m_wFlag & 0x04 )
                {
                    szFlag [ 2 ] = L'P';
                }
                else
                {
                    szFlag [ 2 ] = L'-';
                }

                if ( SmartInfo.m_wFlag & 0x08 )
                {
                    szFlag [ 3 ] = L'R';
                }
                else
                {
                    szFlag [ 3 ] = L'-';
                }

                if ( SmartInfo.m_wFlag & 0x10 )
                {
                    szFlag [ 4 ] = L'C';
                }
                else
                {
                    szFlag [ 4 ] = L'-';
                }

                if ( SmartInfo.m_wFlag & 0x20 )
                {
                    szFlag [ 5 ] = L'S';
                }
                else
                {
                    szFlag [ 5 ] = L'-';
                }

#if 0
                PrintNormalW ( L"%*s #%3d %c: %06x %5d %5d %16lu (%8u %8u) %8u %5d\n",
                    MaxNameLen, pAttribute->name,
                    SmartInfo.m_ucAttribIndex, indicator,
                    SmartInfo.m_wFlag,
                    SmartInfo.m_ucValue, SmartInfo.m_ucWorst,
                    SmartInfo.m_dwData, SmartInfo.m_wData0, SmartInfo.m_wData1, SmartInfo.m_wSpecific,
                    SmartInfo.m_bThreshold );
#else
                PrintNormalW ( L"%*s #%3d %c: %6s %5d %5d %16lu (%8u %8u) %8u %5d\n",
                    MaxNameLen, pAttribute->name,
                    SmartInfo.m_ucAttribIndex, indicator,
                    szFlag,
                    SmartInfo.m_ucValue, SmartInfo.m_ucWorst,
                    SmartInfo.m_dwData, SmartInfo.m_wData0, SmartInfo.m_wData1, SmartInfo.m_wSpecific,
                    SmartInfo.m_bThreshold );
#endif
            }
        }
    }
}

//
///////////////////////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////////////////////
BOOL ReadSMARTAttributes( HANDLE hDevice )
{
    ZeroMemory ( &SendCmdInParams, sizeof(SendCmdInParams) );
    DWORD           dwRet = 0;
    BOOL            bRet = FALSE;
    BYTE            szAttributes [ sizeof(ST_ATAOUTPARAM) + READ_ATTRIBUTE_BUFFER_SIZE - 1 ];
    PBYTE           pSmartValues;

    const WCHAR *pLabelName = L"--- SMART Attributes";

    //
    //  Reset Smart Info
    memset ( SmartInfos, 0, sizeof ( SmartInfos ) );
    memset ( &SmartInfo, 0, sizeof ( SmartInfo ) );

    //
    SendCmdInParams.cBufferSize                     = READ_ATTRIBUTE_BUFFER_SIZE;
    SendCmdInParams.bDriveNumber                    = 0;

    SetReadAttributes ( &SendCmdInParams.irDriveRegs );

    bRet =
        DeviceIoControl (
            hDevice, SMART_RCV_DRIVE_DATA,
            &SendCmdInParams, sizeof(SendCmdInParams),
            szAttributes, sizeof(ST_ATAOUTPARAM) + READ_ATTRIBUTE_BUFFER_SIZE - 1,
            &dwRet, NULL );
    if ( bRet )
    {
        ST_ATAOUTPARAM  *pOutputAttributes = (ST_ATAOUTPARAM*)szAttributes;
        pSmartValues = (PBYTE)(pOutputAttributes->bBuffer);

        if ( DebugMode )
        {
            PrintNormalW ( L"%*s : %u\n", MaxNameLen + 6, L"Driver Error", pOutputAttributes->DriverStatus.bDriverError );
            PrintNormalW ( L"%*s : %u\n", MaxNameLen + 6, L"IDE Status", pOutputAttributes->DriverStatus.bIDEStatus );
            PrintNormalW ( L"%*s : %lu\n", MaxNameLen + 6, L"Buffer Size", pOutputAttributes->cBufferSize );
        }

        if ( Dump )
        {
            DumpBuffer ( ' ', "DATA", pSmartValues, READ_ATTRIBUTE_BUFFER_SIZE, true, true, true );
        }

        //
        ST_VALUES *pstValues = (ST_VALUES *) pSmartValues;

        for ( UCHAR ucT1 = 0; ucT1 < 30; ++ucT1 )
        {
            if ( pstValues->stVariables [ ucT1 ].bIndex != 0 )
            {
                SmartInfo.m_ucAttribIndex   = pstValues->stVariables [ ucT1 ].bIndex;
                SmartInfo.m_ucValue         = pstValues->stVariables [ ucT1 ].bCurrent;
                SmartInfo.m_ucWorst         = pstValues->stVariables [ ucT1 ].bWorst;
                SmartInfo.m_wFlag           = pstValues->stVariables [ ucT1 ].wFlag;

                SmartInfo.m_wData0          = pstValues->stVariables [ ucT1 ].wData0;
                SmartInfo.m_wData1          = pstValues->stVariables [ ucT1 ].wData1;
                SmartInfo.m_dwData          = pstValues->stVariables [ ucT1 ].dwData;
                SmartInfo.m_wSpecific       = pstValues->stVariables [ ucT1 ].wSpecific;

                SmartInfo.m_bThreshold      = MAXDWORD;

                SmartInfos [ SmartInfo.m_ucAttribIndex ] = SmartInfo;
            }
        }
    }
    else if ( ShowError )
    {
        dwRet = GetLastError();
        PrintStderrW ( L"Error - DeviceIoControl SMART_RCV_DRIVE_DATA Fails - Cause : 0x%lx - %s\n", GetLastError(), GetLastErrorText() );
    }

    SetReadThresholds ( &SendCmdInParams.irDriveRegs );

    SendCmdInParams.cBufferSize                 = READ_THRESHOLD_BUFFER_SIZE; // Is same as attrib size
    bRet =
        DeviceIoControl (
            hDevice, SMART_RCV_DRIVE_DATA,
            &SendCmdInParams, sizeof(SendCmdInParams),
            szAttributes, sizeof(ST_ATAOUTPARAM) + READ_ATTRIBUTE_BUFFER_SIZE - 1,
            &dwRet, NULL );
    if ( bRet )
    {
        ST_ATAOUTPARAM  *pOutputAttributes = (ST_ATAOUTPARAM*)szAttributes;
        pSmartValues = (PBYTE)( pOutputAttributes->bBuffer );

        if ( Dump )
        {
            DumpBuffer ( ' ', "THRE", pSmartValues, READ_ATTRIBUTE_BUFFER_SIZE, true, true, true );
        }

        //
        ST_VALUES *pstValues        = (ST_VALUES *) pSmartValues;

        for ( UCHAR ucT1 = 0; ucT1 < 30; ++ucT1 )
        {
            if ( pstValues->stVariables [ ucT1 ].bIndex != 0 )
            {
                SmartInfos [ pstValues->stVariables [ ucT1 ].bIndex ].m_bThreshold  = pstValues->stVariables [ ucT1 ].bThreshold;
            }
        }
    }
    else if ( ShowError )
    {
        PrintStderrW ( L"Error - DeviceIoControl SMART_RCV_DRIVE_DATA Fails - Cause : 0x%lx - %s\n", GetLastError(), GetLastErrorText() );
    }
    else
    {
        PrintNormalW ( L"%*s : %s\n", MaxNameLen + 6, pLabelName, GetLastErrorText() );
    }

    PrintAttributes ();

    return bRet;
}

//
///////////////////////////////////////////////////////////////////////////////
//  https://www.irig106.org/wiki/ch10_handbook:data_retrieval
///////////////////////////////////////////////////////////////////////////////
BOOL ScsiPassThruAttributes ( HANDLE hDevice, COMMAND_TYPE type, UCHAR target, BOOL bPrint )
{
    BOOL            bRet    = FALSE;
    DWORD           dwRet   = 0;

    PBYTE           pSmartValues;

    const WCHAR *pLabelName = L"--- SCSI Attributes";

    //
    //  Reset Smart Info
    memset ( SmartInfos, 0, sizeof ( SmartInfos ) );
    memset ( &SmartInfo, 0, sizeof ( SmartInfo ) );

    //
    ZeroMemory ( &ScsiPassThroughWithBuffer, sizeof(ScsiPassThroughWithBuffer) );

    ScsiPassThroughWithBuffer.ScsiPassThrough.Length                = sizeof(SCSI_PASS_THROUGH);
    ScsiPassThroughWithBuffer.ScsiPassThrough.PathId                = 0;
    ScsiPassThroughWithBuffer.ScsiPassThrough.TargetId              = 0;
    ScsiPassThroughWithBuffer.ScsiPassThrough.Lun                   = 0;
    ScsiPassThroughWithBuffer.ScsiPassThrough.SenseInfoLength       = 24;
    ScsiPassThroughWithBuffer.ScsiPassThrough.DataIn                = SCSI_IOCTL_DATA_IN;
    ScsiPassThroughWithBuffer.ScsiPassThrough.DataTransferLength    = READ_ATTRIBUTE_BUFFER_SIZE;
    ScsiPassThroughWithBuffer.ScsiPassThrough.TimeOutValue          = 2;
    ScsiPassThroughWithBuffer.ScsiPassThrough.DataBufferOffset      = offsetof(ST_SCSI_PASS_THROUGH_BUF, DataBuffer);
    ScsiPassThroughWithBuffer.ScsiPassThrough.SenseInfoOffset       = offsetof(ST_SCSI_PASS_THROUGH_BUF, SenseBuffer);

    if ( type == CMD_TYPE_SAT )
    {
        ScsiPassThroughWithBuffer.ScsiPassThrough.CdbLength = 12;
        ScsiPassThroughWithBuffer.ScsiPassThrough.Cdb[0]    = 0xA1;//ATA PASS THROUGH(12) OPERATION CODE(A1h)
        ScsiPassThroughWithBuffer.ScsiPassThrough.Cdb[1]    = (4 << 1) | 0; //MULTIPLE_COUNT=0,PROTOCOL=4(PIO Data-In),Reserved
        ScsiPassThroughWithBuffer.ScsiPassThrough.Cdb[2]    = (1 << 3) | (1 << 2) | 2;//OFF_LINE=0,CK_COND=0,Reserved=0,T_DIR=1(ToDevice),BYTE_BLOCK=1,T_LENGTH=2
        
        SetReadAttributes ( (IDEREGS *)&ScsiPassThroughWithBuffer.ScsiPassThrough.Cdb[3] );
        ScsiPassThroughWithBuffer.ScsiPassThrough.Cdb[8] = target;
    }
    else
    {
        return FALSE;
    }

    DWORD length = offsetof(ST_SCSI_PASS_THROUGH_BUF, DataBuffer) + ScsiPassThroughWithBuffer.ScsiPassThrough.DataTransferLength;

    bRet = DeviceIoControl (
        hDevice, IOCTL_SCSI_PASS_THROUGH,
        &ScsiPassThroughWithBuffer, sizeof(SCSI_PASS_THROUGH),
        &ScsiPassThroughWithBuffer, length, &dwRet, NULL );

    //
    //  Success
    if ( bRet )
    {
        pSmartValues = (PBYTE)(ScsiPassThroughWithBuffer.DataBuffer);

        if ( Dump )
        {
            DumpBuffer ( ' ', "SDAT", ScsiPassThroughWithBuffer.SenseBuffer, sizeof(ScsiPassThroughWithBuffer.SenseBuffer), true, true, true );
            DumpBuffer ( ' ', "DATA", pSmartValues, READ_ATTRIBUTE_BUFFER_SIZE, true, true, true );
        }

        //
        ST_VALUES *pstValues = (ST_VALUES *) pSmartValues;

        for ( UCHAR ucT1 = 0; ucT1 < 30; ++ucT1 )
        {
            if ( pstValues->stVariables [ ucT1 ].bIndex != 0 )
            {
                SmartInfo.m_ucAttribIndex   = pstValues->stVariables [ ucT1 ].bIndex;
                SmartInfo.m_ucValue         = pstValues->stVariables [ ucT1 ].bCurrent;
                SmartInfo.m_ucWorst         = pstValues->stVariables [ ucT1 ].bWorst;
                SmartInfo.m_wFlag           = pstValues->stVariables [ ucT1 ].wFlag;

                SmartInfo.m_wData0          = pstValues->stVariables [ ucT1 ].wData0;
                SmartInfo.m_wData1          = pstValues->stVariables [ ucT1 ].wData1;
                SmartInfo.m_dwData          = pstValues->stVariables [ ucT1 ].dwData;
                SmartInfo.m_wSpecific       = pstValues->stVariables [ ucT1 ].wSpecific;

                SmartInfo.m_bThreshold      = MAXDWORD;

                SmartInfos [ SmartInfo.m_ucAttribIndex ] = SmartInfo;
            }
        }
    }

    //
    //  Error
    else if ( ShowError )
    {
        dwRet = GetLastError();
        PrintStderrW ( L"Error - DeviceIoControl IOCTL_SCSI_PASS_THROUGH Fails - Cause : 0x%lx - %s\n", GetLastError(), GetLastErrorText() );
    }

    //  Threshold
    ZeroMemory ( &ScsiPassThroughWithBuffer, sizeof(ScsiPassThroughWithBuffer) );

    ScsiPassThroughWithBuffer.ScsiPassThrough.Length                = sizeof(SCSI_PASS_THROUGH);
    ScsiPassThroughWithBuffer.ScsiPassThrough.PathId                = 0;
    ScsiPassThroughWithBuffer.ScsiPassThrough.TargetId              = 0;
    ScsiPassThroughWithBuffer.ScsiPassThrough.Lun                   = 0;
    ScsiPassThroughWithBuffer.ScsiPassThrough.SenseInfoLength       = 24;
    ScsiPassThroughWithBuffer.ScsiPassThrough.DataIn                = SCSI_IOCTL_DATA_IN;
    ScsiPassThroughWithBuffer.ScsiPassThrough.DataTransferLength    = READ_THRESHOLD_BUFFER_SIZE;
    ScsiPassThroughWithBuffer.ScsiPassThrough.TimeOutValue          = 2;
    ScsiPassThroughWithBuffer.ScsiPassThrough.DataBufferOffset      = offsetof(ST_SCSI_PASS_THROUGH_BUF, DataBuffer);
    ScsiPassThroughWithBuffer.ScsiPassThrough.SenseInfoOffset       = offsetof(ST_SCSI_PASS_THROUGH_BUF, SenseBuffer);

    if (type == CMD_TYPE_SAT)
    {
        ScsiPassThroughWithBuffer.ScsiPassThrough.CdbLength = 12;
        ScsiPassThroughWithBuffer.ScsiPassThrough.Cdb[0]    = 0xA1;//ATA PASS THROUGH(12) OPERATION CODE(A1h)
        ScsiPassThroughWithBuffer.ScsiPassThrough.Cdb[1]    = (4 << 1) | 0; //MULTIPLE_COUNT=0,PROTOCOL=4(PIO Data-In),Reserved
        ScsiPassThroughWithBuffer.ScsiPassThrough.Cdb[2]    = (1 << 3) | (1 << 2) | 2;//OFF_LINE=0,CK_COND=0,Reserved=0,T_DIR=1(ToDevice),BYTE_BLOCK=1,T_LENGTH=2
        
        SetReadThresholds ( (IDEREGS *)&ScsiPassThroughWithBuffer.ScsiPassThrough.Cdb[3] );
        ScsiPassThroughWithBuffer.ScsiPassThrough.Cdb[8] = target;
    }
    else
    {
        return FALSE;
    }

    length = offsetof(ST_SCSI_PASS_THROUGH_BUF, DataBuffer) + ScsiPassThroughWithBuffer.ScsiPassThrough.DataTransferLength;

    bRet = DeviceIoControl(hDevice, IOCTL_SCSI_PASS_THROUGH,
        &ScsiPassThroughWithBuffer, sizeof(SCSI_PASS_THROUGH),
        &ScsiPassThroughWithBuffer, length, &dwRet, NULL);

    //
    //  Success
    if ( bRet )
    {
        pSmartValues = (PBYTE)(ScsiPassThroughWithBuffer.DataBuffer);

        if ( Dump )
        {
            DumpBuffer ( ' ', "STHR", ScsiPassThroughWithBuffer.SenseBuffer, sizeof(ScsiPassThroughWithBuffer.SenseBuffer), true, true, true );
            DumpBuffer ( ' ', "THRE", pSmartValues, READ_ATTRIBUTE_BUFFER_SIZE, true, true, true );
        }

        //
        ST_VALUES *pstValues        = (ST_VALUES *) pSmartValues;

        for ( UCHAR ucT1 = 0; ucT1 < 30; ++ucT1 )
        {
            if ( pstValues->stVariables [ ucT1 ].bIndex != 0 )
            {
                SmartInfos [ pstValues->stVariables [ ucT1 ].bIndex ].m_bThreshold  = pstValues->stVariables [ ucT1 ].bThreshold;
            }
        }
    }

    //  Error
    else if ( ShowError )
    {
        dwRet = GetLastError();
        PrintStderrW ( L"Error - DeviceIoControl IOCTL_SCSI_PASS_THROUGH Fails - Cause : 0x%lx - %s\n", GetLastError(), GetLastErrorText() );
    }
    else
    {
        PrintNormalW ( L"%*s : %s\n", MaxNameLen + 6, pLabelName, GetLastErrorText() );
    }

    //
    PrintAttributes ();

    return bRet;
}

//
///////////////////////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////////////////////
void PrintGeneralConfiguration ( PIDENTIFY_DEVICE_DATA pIdentify, char *pText )
{
    if ( pIdentify->GeneralConfiguration.DeviceType != 0 )
    {
        PrintNormalA ( "%*s - %s\n", MaxNameLen + 6, pText, "DeviceType" );
    }
    if ( pIdentify->GeneralConfiguration.FixedDevice != 0 )
    {
        PrintNormalA ( "%*s - %s\n", MaxNameLen + 6, pText, "FixedDevice" );
    }
    if ( pIdentify->GeneralConfiguration.RemovableMedia != 0 )
    {
        PrintNormalA ( "%*s - %s\n", MaxNameLen + 6, pText, "RemovableMedia" );
    }
    if ( pIdentify->GeneralConfiguration.ResponseIncomplete != 0 )
    {
        PrintNormalA ( "%*s - %s\n", MaxNameLen + 6, pText, "ResponseIncomplete " );
    }
}

//
///////////////////////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////////////////////
void PrintTrustComputing ( PIDENTIFY_DEVICE_DATA pIdentify, char *pText )
{
    if ( pIdentify->TrustedComputing.FeatureSupported != 0 )
    {
        PrintNormalA ( "%*s - %s\n", MaxNameLen + 6, pText, "FeatureSupported" );
    }
}

//
///////////////////////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////////////////////
void PrintCapabilities ( PIDENTIFY_DEVICE_DATA pIdentify, char *pText )
{
    PrintNormalA ( "%*s - %s : %u\n", MaxNameLen + 6, pText, "PhysicalSectorAlignment", pIdentify->Capabilities.CurrentLongPhysicalSectorAlignment );

    if ( pIdentify->Capabilities.DmaSupported != 0 )
    {
        PrintNormalA ( "%*s - %s\n", MaxNameLen + 6, pText, "DmaSupported" );
    }
    if ( pIdentify->Capabilities.IordyDisable != 0 )
    {
        PrintNormalA ( "%*s - %s\n", MaxNameLen + 6, pText, "IordyDisable" );
    }
    if ( pIdentify->Capabilities.IordySupported != 0 )
    {
        PrintNormalA ( "%*s - %s\n", MaxNameLen + 6, pText, "IordySupported" );
    }
    if ( pIdentify->Capabilities.LbaSupported != 0 )
    {
        PrintNormalA ( "%*s - %s\n", MaxNameLen + 6, pText, "LbaSupported" );
    }
    if ( pIdentify->Capabilities.StandybyTimerSupport != 0 )
    {
        PrintNormalA ( "%*s - %s\n", MaxNameLen + 6, pText, "StandybyTimerSupport" );
    }
    
}

//
///////////////////////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////////////////////
void PrintAdditionalSupported ( PIDENTIFY_DEVICE_DATA pIdentify, char *pText )
{
    if ( pIdentify->AdditionalSupported.CFastSpecSupported != 0 )
    {
        PrintNormalA ( "%*s - %s\n", MaxNameLen + 6, pText, "CFastSpecSupported" );
    }
    if ( pIdentify->AdditionalSupported.DeterministicReadAfterTrimSupported != 0 )
    {
        PrintNormalA ( "%*s - %s\n", MaxNameLen + 6, pText, "DeterministicReadAfterTrimSupported" );
    }
    if ( pIdentify->AdditionalSupported.DeviceConfigIdentifySetDmaSupported != 0 )
    {
        PrintNormalA ( "%*s - %s\n", MaxNameLen + 6, pText, "DeviceConfigIdentifySetDmaSupported" );
    }
    if ( pIdentify->AdditionalSupported.DeviceEncryptsAllUserData != 0 )
    {
        PrintNormalA ( "%*s - %s\n", MaxNameLen + 6, pText, "DeviceEncryptsAllUserData" );
    }
    if ( pIdentify->AdditionalSupported.DownloadMicrocodeDmaSupported != 0 )
    {
        PrintNormalA ( "%*s - %s\n", MaxNameLen + 6, pText, "DownloadMicrocodeDmaSupported" );
    }
    if ( pIdentify->AdditionalSupported.ExtendedUserAddressableSectorsSupported != 0 )
    {
        PrintNormalA ( "%*s - %s\n", MaxNameLen + 6, pText, "ExtendedUserAddressableSectorsSupported" );
    }
    if ( pIdentify->AdditionalSupported.IEEE1667 != 0 )
    {
        PrintNormalA ( "%*s - %s\n", MaxNameLen + 6, pText, "IEEE1667" );
    }
    if ( pIdentify->AdditionalSupported.LPSAERCSupported != 0 )
    {
        PrintNormalA ( "%*s - %s\n", MaxNameLen + 6, pText, "LPSAERCSupported" );
    }
    if ( pIdentify->AdditionalSupported.NonVolatileWriteCache != 0 )
    {
        PrintNormalA ( "%*s - %s\n", MaxNameLen + 6, pText, "NonVolatileWriteCache" );
    }
    if ( pIdentify->AdditionalSupported.Optional28BitCommandsSupported != 0 )
    {
        PrintNormalA ( "%*s - %s\n", MaxNameLen + 6, pText, "Optional28BitCommandsSupported" );
    }
    if ( pIdentify->AdditionalSupported.ReadBufferDmaSupported != 0 )
    {
        PrintNormalA ( "%*s - %s\n", MaxNameLen + 6, pText, "ReadBufferDmaSupported" );
    }
    if ( pIdentify->AdditionalSupported.ReadZeroAfterTrimSupported != 0 )
    {
        PrintNormalA ( "%*s - %s\n", MaxNameLen + 6, pText, "ReadZeroAfterTrimSupported" );
    }
    if ( pIdentify->AdditionalSupported.SetMaxSetPasswordUnlockDmaSupported != 0 )
    {
        PrintNormalA ( "%*s - %s\n", MaxNameLen + 6, pText, "SetMaxSetPasswordUnlockDmaSupported" );
    }
    if ( pIdentify->AdditionalSupported.WriteBufferDmaSupported != 0 )
    {
        PrintNormalA ( "%*s - %s\n", MaxNameLen + 6, pText, "WriteBufferDmaSupported" );
    }
    if ( pIdentify->AdditionalSupported.ZonedCapabilities != 0 )
    {
        PrintNormalA ( "%*s - %s\n", MaxNameLen + 6, pText, "ZonedCapabilities" );
    }
}

//
///////////////////////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////////////////////
void PrintSerialAtaCapabilities ( PIDENTIFY_DEVICE_DATA pIdentify, char *pText )
{
    PrintNormalA ( "%*s - %s : %u\n", MaxNameLen + 6, pText, "CurrentSpeed", pIdentify->SerialAtaCapabilities.CurrentSpeed );
    if ( pIdentify->SerialAtaCapabilities.DeviceAutoPS != 0 )
    {
        PrintNormalA ( "%*s - %s\n", MaxNameLen + 6, pText, "DeviceAutoPS" );
    }
    if ( pIdentify->SerialAtaCapabilities.DEVSLPtoReducedPwrState != 0 )
    {
        PrintNormalA ( "%*s - %s\n", MaxNameLen + 6, pText, "DEVSLPtoReducedPwrState" );
    }
    if ( pIdentify->SerialAtaCapabilities.HIPM != 0 )
    {
        PrintNormalA ( "%*s - %s\n", MaxNameLen + 6, pText, "HIPM" );
    }
    if ( pIdentify->SerialAtaCapabilities.HostAutoPS != 0 )
    {
        PrintNormalA ( "%*s - %s\n", MaxNameLen + 6, pText, "HostAutoPS" );
    }
    if ( pIdentify->SerialAtaCapabilities.NCQ != 0 )
    {
        PrintNormalA ( "%*s - %s\n", MaxNameLen + 6, pText, "NCQ" );
    }
    if ( pIdentify->SerialAtaCapabilities.NcqPriority != 0 )
    {
        PrintNormalA ( "%*s - %s\n", MaxNameLen + 6, pText, "NcqPriority" );
    }
    if ( pIdentify->SerialAtaCapabilities.NcqQueueMgmt != 0 )
    {
        PrintNormalA ( "%*s - %s\n", MaxNameLen + 6, pText, "NcqQueueMgmt" );
    }
    if ( pIdentify->SerialAtaCapabilities.NcqReceiveSend != 0 )
    {
        PrintNormalA ( "%*s - %s\n", MaxNameLen + 6, pText, "NcqReceiveSend" );
    }
    if ( pIdentify->SerialAtaCapabilities.NcqStreaming != 0 )
    {
        PrintNormalA ( "%*s - %s\n", MaxNameLen + 6, pText, "NcqStreaming" );
    }
    if ( pIdentify->SerialAtaCapabilities.NcqUnload != 0 )
    {
        PrintNormalA ( "%*s - %s\n", MaxNameLen + 6, pText, "NcqUnload" );
    }
    if ( pIdentify->SerialAtaCapabilities.PhyEvents != 0 )
    {
        PrintNormalA ( "%*s - %s\n", MaxNameLen + 6, pText, "PhyEvents" );
    }
    if ( pIdentify->SerialAtaCapabilities.ReadLogDMA != 0 )
    {
        PrintNormalA ( "%*s - %s\n", MaxNameLen + 6, pText, "ReadLogDMA" );
    }
    if ( pIdentify->SerialAtaCapabilities.SataGen1 != 0 )
    {
        PrintNormalA ( "%*s - %s\n", MaxNameLen + 6, pText, "SataGen1" );
    }
    if ( pIdentify->SerialAtaCapabilities.SataGen2 != 0 )
    {
        PrintNormalA ( "%*s - %s\n", MaxNameLen + 6, pText, "SataGen2" );
    }
    if ( pIdentify->SerialAtaCapabilities.SataGen3 != 0 )
    {
        PrintNormalA ( "%*s - %s\n", MaxNameLen + 6, pText, "SataGen3" );
    }
}

//
///////////////////////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////////////////////
void PrintSerialAtaFeaturesSupported ( PIDENTIFY_DEVICE_DATA pIdentify, char *pText )
{
    if ( pIdentify->SerialAtaFeaturesSupported.DEVSLP != 0 )
    {
        PrintNormalA ( "%*s - %s\n", MaxNameLen + 6, pText, "DEVSLP" );
    }
    if ( pIdentify->SerialAtaFeaturesSupported.DIPM != 0 )
    {
        PrintNormalA ( "%*s - %s\n", MaxNameLen + 6, pText, "DIPM" );
    }
    if ( pIdentify->SerialAtaFeaturesSupported.DmaSetupAutoActivate != 0 )
    {
        PrintNormalA ( "%*s - %s\n", MaxNameLen + 6, pText, "DmaSetupAutoActivate" );
    }
    if ( pIdentify->SerialAtaFeaturesSupported.HardwareFeatureControl != 0 )
    {
        PrintNormalA ( "%*s - %s\n", MaxNameLen + 6, pText, "HardwareFeatureControl" );
    }
    if ( pIdentify->SerialAtaFeaturesSupported.HybridInformation != 0 )
    {
        PrintNormalA ( "%*s - %s\n", MaxNameLen + 6, pText, "HybridInformation" );
    }
    if ( pIdentify->SerialAtaFeaturesSupported.InOrderData != 0 )
    {
        PrintNormalA ( "%*s - %s\n", MaxNameLen + 6, pText, "InOrderData" );
    }
    if ( pIdentify->SerialAtaFeaturesSupported.NCQAutosense != 0 )
    {
        PrintNormalA ( "%*s - %s\n", MaxNameLen + 6, pText, "NCQAutosense" );
    }
    if ( pIdentify->SerialAtaFeaturesSupported.NonZeroOffsets != 0 )
    {
        PrintNormalA ( "%*s - %s\n", MaxNameLen + 6, pText, "NonZeroOffsets" );
    }
    if ( pIdentify->SerialAtaFeaturesSupported.SoftwareSettingsPreservation != 0 )
    {
        PrintNormalA ( "%*s - %s\n", MaxNameLen + 6, pText, "SoftwareSettingsPreservation" );
    }
}

//
///////////////////////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////////////////////
void PrintSerialAtaFeaturesEnabled ( PIDENTIFY_DEVICE_DATA pIdentify, char *pText )
{
    if ( pIdentify->SerialAtaFeaturesEnabled.DEVSLP != 0 )
    {
        PrintNormalA ( "%*s - %s\n", MaxNameLen + 6, pText, "DEVSLP" );
    }
    if ( pIdentify->SerialAtaFeaturesEnabled.DIPM != 0 )
    {
        PrintNormalA ( "%*s - %s\n", MaxNameLen + 6, pText, "DIPM" );
    }
    if ( pIdentify->SerialAtaFeaturesEnabled.DmaSetupAutoActivate != 0 )
    {
        PrintNormalA ( "%*s - %s\n", MaxNameLen + 6, pText, "DmaSetupAutoActivate" );
    }
    if ( pIdentify->SerialAtaFeaturesEnabled.HardwareFeatureControl != 0 )
    {
        PrintNormalA ( "%*s - %s\n", MaxNameLen + 6, pText, "HardwareFeatureControl" );
    }
    if ( pIdentify->SerialAtaFeaturesEnabled.HybridInformation != 0 )
    {
        PrintNormalA ( "%*s - %s\n", MaxNameLen + 6, pText, "HybridInformation" );
    }
    if ( pIdentify->SerialAtaFeaturesEnabled.InOrderData != 0 )
    {
        PrintNormalA ( "%*s - %s\n", MaxNameLen + 6, pText, "InOrderData" );
    }
    if ( pIdentify->SerialAtaFeaturesEnabled.NonZeroOffsets != 0 )
    {
        PrintNormalA ( "%*s - %s\n", MaxNameLen + 6, pText, "NonZeroOffsets" );
    }
    if ( pIdentify->SerialAtaFeaturesEnabled.SoftwareSettingsPreservation != 0 )
    {
        PrintNormalA ( "%*s - %s\n", MaxNameLen + 6, pText, "SoftwareSettingsPreservation" );
    }
}

//
///////////////////////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////////////////////
void PrintCommandSetSupport ( PIDENTIFY_DEVICE_DATA pIdentify, char *pText )
{
    if ( pIdentify->CommandSetSupport.Acoustics != 0 )
    {
        PrintNormalA ( "%*s - %s\n", MaxNameLen + 6, pText, "Acoustics" );
    }
    if ( pIdentify->CommandSetSupport.AdvancedPm != 0 )
    {
        PrintNormalA ( "%*s - %s\n", MaxNameLen + 6, pText, "AdvancedPm" );
    }
    if ( pIdentify->CommandSetSupport.BigLba != 0 )
    {
        PrintNormalA ( "%*s - %s\n", MaxNameLen + 6, pText, "BigLba" );
    }
    if ( pIdentify->CommandSetSupport.Cfa != 0 )
    {
        PrintNormalA ( "%*s - %s\n", MaxNameLen + 6, pText, "Cfa" );
    }
    if ( pIdentify->CommandSetSupport.DeviceConfigOverlay != 0 )
    {
        PrintNormalA ( "%*s - %s\n", MaxNameLen + 6, pText, "DeviceConfigOverlay" );
    }
    if ( pIdentify->CommandSetSupport.DeviceReset != 0 )
    {
        PrintNormalA ( "%*s - %s\n", MaxNameLen + 6, pText, "DeviceReset" );
    }
    if ( pIdentify->CommandSetSupport.DmaQueued != 0 )
    {
        PrintNormalA ( "%*s - %s\n", MaxNameLen + 6, pText, "DmaQueued" );
    }
    if ( pIdentify->CommandSetSupport.DownloadMicrocode != 0 )
    {
        PrintNormalA ( "%*s - %s\n", MaxNameLen + 6, pText, "DownloadMicrocode" );
    }
    if ( pIdentify->CommandSetSupport.FlushCache != 0 )
    {
        PrintNormalA ( "%*s - %s\n", MaxNameLen + 6, pText, "FlushCache" );
    }
    if ( pIdentify->CommandSetSupport.FlushCacheExt != 0 )
    {
        PrintNormalA ( "%*s - %s\n", MaxNameLen + 6, pText, "FlushCacheExt" );
    }
    if ( pIdentify->CommandSetSupport.GpLogging != 0 )
    {
        PrintNormalA ( "%*s - %s\n", MaxNameLen + 6, pText, "GpLogging" );
    }
    if ( pIdentify->CommandSetSupport.HostProtectedArea != 0 )
    {
        PrintNormalA ( "%*s - %s\n", MaxNameLen + 6, pText, "HostProtectedArea" );
    }
    if ( pIdentify->CommandSetSupport.IdleWithUnloadFeature != 0 )
    {
        PrintNormalA ( "%*s - %s\n", MaxNameLen + 6, pText, "IdleWithUnloadFeature" );
    }
    if ( pIdentify->CommandSetSupport.LookAhead != 0 )
    {
        PrintNormalA ( "%*s - %s\n", MaxNameLen + 6, pText, "LookAhead" );
    }
    if ( pIdentify->CommandSetSupport.ManualPowerUp != 0 )
    {
        PrintNormalA ( "%*s - %s\n", MaxNameLen + 6, pText, "ManualPowerUp" );
    }
    if ( pIdentify->CommandSetSupport.MediaCardPassThrough != 0 )
    {
        PrintNormalA ( "%*s - %s\n", MaxNameLen + 6, pText, "MediaCardPassThrough" );
    }
    if ( pIdentify->CommandSetSupport.MediaSerialNumber != 0 )
    {
        PrintNormalA ( "%*s - %s\n", MaxNameLen + 6, pText, "MediaSerialNumber" );
    }
    if ( pIdentify->CommandSetSupport.Msn != 0 )
    {
        PrintNormalA ( "%*s - %s\n", MaxNameLen + 6, pText, "Msn" );
    }
    if ( pIdentify->CommandSetSupport.Nop != 0 )
    {
        PrintNormalA ( "%*s - %s\n", MaxNameLen + 6, pText, "Nop" );
    }
    if ( pIdentify->CommandSetSupport.PowerManagement != 0 )
    {
        PrintNormalA ( "%*s - %s\n", MaxNameLen + 6, pText, "PowerManagement" );
    }
    if ( pIdentify->CommandSetSupport.PowerUpInStandby!= 0 )
    {
        PrintNormalA ( "%*s - %s\n", MaxNameLen + 6, pText, "PowerUpInStandby" );
    }
    if ( pIdentify->CommandSetSupport.ReadBuffer != 0 )
    {
        PrintNormalA ( "%*s - %s\n", MaxNameLen + 6, pText, "ReadBuffer" );
    }
    if ( pIdentify->CommandSetSupport.ReleaseInterrupt != 0 )
    {
        PrintNormalA ( "%*s - %s\n", MaxNameLen + 6, pText, "ReleaseInterrupt" );
    }
    if ( pIdentify->CommandSetSupport.RemovableMediaFeature != 0 )
    {
        PrintNormalA ( "%*s - %s\n", MaxNameLen + 6, pText, "RemovableMediaFeature" );
    }
    if ( pIdentify->CommandSetSupport.SecurityMode != 0 )
    {
        PrintNormalA ( "%*s - %s\n", MaxNameLen + 6, pText, "SecurityMode" );
    }
    if ( pIdentify->CommandSetSupport.ServiceInterrupt != 0 )
    {
        PrintNormalA ( "%*s - %s\n", MaxNameLen + 6, pText, "ServiceInterrupt" );
    }
    if ( pIdentify->CommandSetSupport.SetMax != 0 )
    {
        PrintNormalA ( "%*s - %s\n", MaxNameLen + 6, pText, "SetMax" );
    }
    if ( pIdentify->CommandSetSupport.SmartCommands != 0 )
    {
        PrintNormalA ( "%*s - %s\n", MaxNameLen + 6, pText, "SmartCommands" );
    }
    if ( pIdentify->CommandSetSupport.SmartErrorLog != 0 )
    {
        PrintNormalA ( "%*s - %s\n", MaxNameLen + 6, pText, "SmartErrorLog" );
    }
    if ( pIdentify->CommandSetSupport.SmartSelfTest != 0 )
    {
        PrintNormalA ( "%*s - %s\n", MaxNameLen + 6, pText, "SmartSelfTest" );
    }
    if ( pIdentify->CommandSetSupport.StreamingFeature != 0 )
    {
        PrintNormalA ( "%*s - %s\n", MaxNameLen + 6, pText, "StreamingFeature" );
    }
    if ( pIdentify->CommandSetSupport.URGReadStream != 0 )
    {
        PrintNormalA ( "%*s - %s\n", MaxNameLen + 6, pText, "URGReadStream" );
    }
    if ( pIdentify->CommandSetSupport.URGWriteStream != 0 )
    {
        PrintNormalA ( "%*s - %s\n", MaxNameLen + 6, pText, "URGWriteStream" );
    }
    if ( pIdentify->CommandSetSupport.WordValid != 0 )
    {
        PrintNormalA ( "%*s - %s\n", MaxNameLen + 6, pText, "WordValid" );
    }
    if ( pIdentify->CommandSetSupport.WordValid83 != 0 )
    {
        PrintNormalA ( "%*s - %s\n", MaxNameLen + 6, pText, "WordValid83" );
    }
    if ( pIdentify->CommandSetSupport.WriteBuffer != 0 )
    {
        PrintNormalA ( "%*s - %s\n", MaxNameLen + 6, pText, "WriteBuffer" );
    }
    if ( pIdentify->CommandSetSupport.WriteCache != 0 )
    {
        PrintNormalA ( "%*s - %s\n", MaxNameLen + 6, pText, "WriteCache" );
    }
    if ( pIdentify->CommandSetSupport.WriteFua != 0 )
    {
        PrintNormalA ( "%*s - %s\n", MaxNameLen + 6, pText, "WriteFua" );
    }
    if ( pIdentify->CommandSetSupport.WriteQueuedFua != 0 )
    {
        PrintNormalA ( "%*s - %s\n", MaxNameLen + 6, pText, "WriteQueuedFua" );
    }
    if ( pIdentify->CommandSetSupport.WWN64Bit != 0 )
    {
        PrintNormalA ( "%*s - %s\n", MaxNameLen + 6, pText, "WWN64Bit" );
    }
}

//
///////////////////////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////////////////////
void PrintPhysicalLogicalSectorSize ( PIDENTIFY_DEVICE_DATA pIdentify, char *pText )
{
    if ( pIdentify->PhysicalLogicalSectorSize.LogicalSectorLongerThan256Words != 0 )
    {
        PrintNormalA ( "%*s - %s\n", MaxNameLen + 6, pText, "LogicalSectorLongerThan256Words" );
    }
    if ( pIdentify->PhysicalLogicalSectorSize.LogicalSectorsPerPhysicalSector != 0 )
    {
        PrintNormalA ( "%*s - %s : %d\n", MaxNameLen + 6, pText, "LogicalSectorsPerPhysicalSector", pIdentify->PhysicalLogicalSectorSize.LogicalSectorsPerPhysicalSector );
    }
    if ( pIdentify->PhysicalLogicalSectorSize.MultipleLogicalSectorsPerPhysicalSector != 0 )
    {
        PrintNormalA ( "%*s - %s\n", MaxNameLen + 6, pText, "MultipleLogicalSectorsPerPhysicalSector" );
    }

}

//
///////////////////////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////////////////////
void PrintSecurityStatus ( PIDENTIFY_DEVICE_DATA pIdentify, char *pText )
{
    if ( pIdentify->SecurityStatus.EnhancedSecurityEraseSupported != 0 )
    {
        PrintNormalA ( "%*s - %s\n", MaxNameLen + 6, pText, "EnhancedSecurityEraseSupported" );
    }
    if ( pIdentify->SecurityStatus.SecurityCountExpired != 0 )
    {
        PrintNormalA ( "%*s - %s\n", MaxNameLen + 6, pText, "SecurityCountExpired" );
    }
    if ( pIdentify->SecurityStatus.SecurityEnabled != 0 )
    {
        PrintNormalA ( "%*s - %s\n", MaxNameLen + 6, pText, "SecurityEnabled" );
    }
    if ( pIdentify->SecurityStatus.SecurityFrozen != 0 )
    {
        PrintNormalA ( "%*s - %s\n", MaxNameLen + 6, pText, "SecurityFrozen" );
    }
    if ( pIdentify->SecurityStatus.SecurityLevel != 0 )
    {
        PrintNormalA ( "%*s - %s\n", MaxNameLen + 6, pText, "SecurityLevel" );
    }
    if ( pIdentify->SecurityStatus.SecurityLocked != 0 )
    {
        PrintNormalA ( "%*s - %s\n", MaxNameLen + 6, pText, "SecurityLocked" );
    }
    if ( pIdentify->SecurityStatus.SecuritySupported != 0 )
    {
        PrintNormalA ( "%*s - %s\n", MaxNameLen + 6, pText, "SecuritySupported" );
    }
}

//
///////////////////////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////////////////////
void PrintCfaPowerMode1 ( PIDENTIFY_DEVICE_DATA pIdentify, char *pText )
{
    if ( pIdentify->CfaPowerMode1.CfaPowerMode1Disabled != 0 )
    {
        PrintNormalA ( "%*s - %s\n", MaxNameLen + 6, pText, "CfaPowerMode1Disabled" );
    }
    if ( pIdentify->CfaPowerMode1.CfaPowerMode1Required != 0 )
    {
        PrintNormalA ( "%*s - %s\n", MaxNameLen + 6, pText, "CfaPowerMode1Required" );
    }
    if ( pIdentify->CfaPowerMode1.MaximumCurrentInMA != 0 )
    {
        PrintNormalA ( "%*s - %s : %d\n", MaxNameLen + 6, pText, "MaximumCurrentInMA", pIdentify->CfaPowerMode1.MaximumCurrentInMA );
    }
    if ( pIdentify->CfaPowerMode1.Word160Supported != 0 )
    {
        PrintNormalA ( "%*s - %s\n", MaxNameLen + 6, pText, "Word160Supported" );
    }
}

//
///////////////////////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////////////////////
void PrintDataSetManagementFeature ( PIDENTIFY_DEVICE_DATA pIdentify, char *pText )
{
    if ( pIdentify->DataSetManagementFeature.SupportsTrim != 0 )
    {
        PrintNormalA ( "%*s - %s\n", MaxNameLen + 6, pText, "SupportsTrim" );
    }
}

//
///////////////////////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////////////////////
void PrintBlockAlignment ( PIDENTIFY_DEVICE_DATA pIdentify, char *pText )
{
    if ( pIdentify->BlockAlignment.AlignmentOfLogicalWithinPhysical != 0 )
    {
        PrintNormalA ( "%*s - %s : %d\n", MaxNameLen + 6, pText, "LogicalSectorLongerThan256Words", pIdentify->BlockAlignment.AlignmentOfLogicalWithinPhysical );
    }
    if ( pIdentify->BlockAlignment.Word209Supported != 0 )
    {
        PrintNormalA ( "%*s - %s\n", MaxNameLen + 6, pText, "Word209Supported" );
    }
}

//
///////////////////////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////////////////////
void PrintNVCacheCapabilities ( PIDENTIFY_DEVICE_DATA pIdentify, char *pText )
{
    if ( pIdentify->NVCacheCapabilities.NVCacheFeatureSetEnabled != 0 )
    {
        PrintNormalA ( "%*s - %s\n", MaxNameLen + 6, pText, "NVCacheFeatureSetEnabled" );
    }
    if ( pIdentify->NVCacheCapabilities.NVCacheFeatureSetVersion != 0 )
    {
        PrintNormalA ( "%*s - %s\n", MaxNameLen + 6, pText, "NVCacheFeatureSetVersion" );
    }
    if ( pIdentify->NVCacheCapabilities.NVCachePowerModeEnabled != 0 )
    {
        PrintNormalA ( "%*s - %s\n", MaxNameLen + 6, pText, "NVCachePowerModeEnabled" );
    }
    if ( pIdentify->NVCacheCapabilities.NVCachePowerModeVersion != 0 )
    {
        PrintNormalA ( "%*s - %s : %d\n", MaxNameLen + 6, pText, "NVCachePowerModeVersion", pIdentify->NVCacheCapabilities.NVCachePowerModeVersion );
    }
}

//
///////////////////////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////////////////////
void PrintNVCacheOptions ( PIDENTIFY_DEVICE_DATA pIdentify, char *pText )
{
    if ( pIdentify->NVCacheOptions.NVCacheEstimatedTimeToSpinUpInSeconds != 0 )
    {
        PrintNormalA ( "%*s - %s : %u\n", MaxNameLen + 6, pText, "NVCacheEstimatedTimeToSpinUpInSeconds", pIdentify->NVCacheOptions.NVCacheEstimatedTimeToSpinUpInSeconds );
    }
}

//
///////////////////////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////////////////////
void PrintCommandSetActive ( PIDENTIFY_DEVICE_DATA pIdentify, char *pText )
{
    if ( pIdentify->CommandSetActive.Acoustics != 0 )
    {
        PrintNormalA ( "%*s - %s\n", MaxNameLen + 6, pText, "Acoustics" );
    }
    if ( pIdentify->CommandSetActive.AdvancedPm != 0 )
    {
        PrintNormalA ( "%*s - %s\n", MaxNameLen + 6, pText, "AdvancedPm" );
    }
    if ( pIdentify->CommandSetActive.BigLba != 0 )
    {
        PrintNormalA ( "%*s - %s\n", MaxNameLen + 6, pText, "BigLba" );
    }
    if ( pIdentify->CommandSetActive.Cfa != 0 )
    {
        PrintNormalA ( "%*s - %s\n", MaxNameLen + 6, pText, "Cfa" );
    }
    if ( pIdentify->CommandSetActive.DeviceConfigOverlay != 0 )
    {
        PrintNormalA ( "%*s - %s\n", MaxNameLen + 6, pText, "DeviceConfigOverlay" );
    }
    if ( pIdentify->CommandSetActive.DeviceReset != 0 )
    {
        PrintNormalA ( "%*s - %s\n", MaxNameLen + 6, pText, "DeviceReset" );
    }
    if ( pIdentify->CommandSetActive.DmaQueued != 0 )
    {
        PrintNormalA ( "%*s - %s\n", MaxNameLen + 6, pText, "DmaQueued" );
    }
    if ( pIdentify->CommandSetActive.DownloadMicrocode != 0 )
    {
        PrintNormalA ( "%*s - %s\n", MaxNameLen + 6, pText, "DownloadMicrocode" );
    }
    if ( pIdentify->CommandSetActive.FlushCache != 0 )
    {
        PrintNormalA ( "%*s - %s\n", MaxNameLen + 6, pText, "FlushCache" );
    }
    if ( pIdentify->CommandSetActive.FlushCacheExt != 0 )
    {
        PrintNormalA ( "%*s - %s\n", MaxNameLen + 6, pText, "FlushCacheExt" );
    }
    if ( pIdentify->CommandSetActive.GpLogging != 0 )
    {
        PrintNormalA ( "%*s - %s\n", MaxNameLen + 6, pText, "GpLogging" );
    }
    if ( pIdentify->CommandSetActive.HostProtectedArea != 0 )
    {
        PrintNormalA ( "%*s - %s\n", MaxNameLen + 6, pText, "HostProtectedArea" );
    }
    if ( pIdentify->CommandSetActive.IdleWithUnloadFeature != 0 )
    {
        PrintNormalA ( "%*s - %s\n", MaxNameLen + 6, pText, "IdleWithUnloadFeature" );
    }
    if ( pIdentify->CommandSetActive.LookAhead != 0 )
    {
        PrintNormalA ( "%*s - %s\n", MaxNameLen + 6, pText, "LookAhead" );
    }
    if ( pIdentify->CommandSetActive.ManualPowerUp != 0 )
    {
        PrintNormalA ( "%*s - %s\n", MaxNameLen + 6, pText, "ManualPowerUp" );
    }
    if ( pIdentify->CommandSetActive.MediaCardPassThrough != 0 )
    {
        PrintNormalA ( "%*s - %s\n", MaxNameLen + 6, pText, "MediaCardPassThrough" );
    }
    if ( pIdentify->CommandSetActive.MediaSerialNumber != 0 )
    {
        PrintNormalA ( "%*s - %s\n", MaxNameLen + 6, pText, "MediaSerialNumber" );
    }
    if ( pIdentify->CommandSetActive.Msn != 0 )
    {
        PrintNormalA ( "%*s - %s\n", MaxNameLen + 6, pText, "Msn" );
    }
    if ( pIdentify->CommandSetActive.Nop != 0 )
    {
        PrintNormalA ( "%*s - %s\n", MaxNameLen + 6, pText, "Nop" );
    }
    if ( pIdentify->CommandSetActive.PowerManagement != 0 )
    {
        PrintNormalA ( "%*s - %s\n", MaxNameLen + 6, pText, "PowerManagement" );
    }
    if ( pIdentify->CommandSetActive.PowerUpInStandby!= 0 )
    {
        PrintNormalA ( "%*s - %s\n", MaxNameLen + 6, pText, "PowerUpInStandby" );
    }
    if ( pIdentify->CommandSetActive.ReadBuffer != 0 )
    {
        PrintNormalA ( "%*s - %s\n", MaxNameLen + 6, pText, "ReadBuffer" );
    }
    if ( pIdentify->CommandSetActive.ReleaseInterrupt != 0 )
    {
        PrintNormalA ( "%*s - %s\n", MaxNameLen + 6, pText, "ReleaseInterrupt" );
    }
    if ( pIdentify->CommandSetActive.RemovableMediaFeature != 0 )
    {
        PrintNormalA ( "%*s - %s\n", MaxNameLen + 6, pText, "RemovableMediaFeature" );
    }
    if ( pIdentify->CommandSetActive.SecurityMode != 0 )
    {
        PrintNormalA ( "%*s - %s\n", MaxNameLen + 6, pText, "SecurityMode" );
    }
    if ( pIdentify->CommandSetActive.ServiceInterrupt != 0 )
    {
        PrintNormalA ( "%*s - %s\n", MaxNameLen + 6, pText, "ServiceInterrupt" );
    }
    if ( pIdentify->CommandSetActive.SetMax != 0 )
    {
        PrintNormalA ( "%*s - %s\n", MaxNameLen + 6, pText, "SetMax" );
    }
    if ( pIdentify->CommandSetActive.SmartCommands != 0 )
    {
        PrintNormalA ( "%*s - %s\n", MaxNameLen + 6, pText, "SmartCommands" );
    }
    if ( pIdentify->CommandSetActive.SmartErrorLog != 0 )
    {
        PrintNormalA ( "%*s - %s\n", MaxNameLen + 6, pText, "SmartErrorLog" );
    }
    if ( pIdentify->CommandSetActive.SmartSelfTest != 0 )
    {
        PrintNormalA ( "%*s - %s\n", MaxNameLen + 6, pText, "SmartSelfTest" );
    }
    if ( pIdentify->CommandSetActive.StreamingFeature != 0 )
    {
        PrintNormalA ( "%*s - %s\n", MaxNameLen + 6, pText, "StreamingFeature" );
    }
    if ( pIdentify->CommandSetActive.URGReadStream != 0 )
    {
        PrintNormalA ( "%*s - %s\n", MaxNameLen + 6, pText, "URGReadStream" );
    }
    if ( pIdentify->CommandSetActive.URGWriteStream != 0 )
    {
        PrintNormalA ( "%*s - %s\n", MaxNameLen + 6, pText, "URGWriteStream" );
    }
    if ( pIdentify->CommandSetActive.WriteBuffer != 0 )
    {
        PrintNormalA ( "%*s - %s\n", MaxNameLen + 6, pText, "WriteBuffer" );
    }
    if ( pIdentify->CommandSetActive.WriteCache != 0 )
    {
        PrintNormalA ( "%*s - %s\n", MaxNameLen + 6, pText, "WriteCache" );
    }
    if ( pIdentify->CommandSetActive.WriteFua != 0 )
    {
        PrintNormalA ( "%*s - %s\n", MaxNameLen + 6, pText, "WriteFua" );
    }
    if ( pIdentify->CommandSetActive.WriteQueuedFua != 0 )
    {
        PrintNormalA ( "%*s - %s\n", MaxNameLen + 6, pText, "WriteQueuedFua" );
    }
    if ( pIdentify->CommandSetActive.WWN64Bit != 0 )
    {
        PrintNormalA ( "%*s - %s\n", MaxNameLen + 6, pText, "WWN64Bit" );
    }
}

//
///////////////////////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////////////////////
void PrintSCTCommandTransport ( PIDENTIFY_DEVICE_DATA pIdentify, char *pText )
{
    if ( pIdentify->SCTCommandTransport.DataTablesSuported != 0 )
    {
        PrintNormalA ( "%*s - %s\n", MaxNameLen + 6, pText, "DataTablesSuported" );
    }
    if ( pIdentify->SCTCommandTransport.ErrorRecoveryControlSupported != 0 )
    {
        PrintNormalA ( "%*s - %s\n", MaxNameLen + 6, pText, "ErrorRecoveryControlSupported" );
    }
    if ( pIdentify->SCTCommandTransport.FeatureControlSuported != 0 )
    {
        PrintNormalA ( "%*s - %s\n", MaxNameLen + 6, pText, "FeatureControlSuported" );
    }
    if ( pIdentify->SCTCommandTransport.VendorSpecific != 0 )
    {
        PrintNormalA ( "%*s - %s : %d\n", MaxNameLen + 6, pText, "VendorSpecific", pIdentify->SCTCommandTransport.VendorSpecific );
    }
    if ( pIdentify->SCTCommandTransport.WriteSameSuported != 0 )
    {
        PrintNormalA ( "%*s - %s\n", MaxNameLen + 6, pText, "WriteSameSuported" );
    }
}

//
///////////////////////////////////////////////////////////////////////////////
//  
///////////////////////////////////////////////////////////////////////////////
void PrintCommandSetSupportExt ( PIDENTIFY_DEVICE_DATA pIdentify, char *pText )
{
    if ( pIdentify->CommandSetSupportExt.DownloadMicrocodeMode3 != 0 )
    {
        PrintNormalA ( "%*s - %s\n", MaxNameLen + 6, pText, "DownloadMicrocodeMode3" );
    }
    if ( pIdentify->CommandSetSupportExt.ExtendedPowerConditions != 0 )
    {
        PrintNormalA ( "%*s - %s\n", MaxNameLen + 6, pText, "ExtendedPowerConditions" );
    }
    if ( pIdentify->CommandSetSupportExt.FreefallControl != 0 )
    {
        PrintNormalA ( "%*s - %s\n", MaxNameLen + 6, pText, "FreefallControl" );
    }
    if ( pIdentify->CommandSetSupportExt.ReadWriteLogDmaExt != 0 )
    {
        PrintNormalA ( "%*s - %s\n", MaxNameLen + 6, pText, "ReadWriteLogDmaExt" );
    }
    if ( pIdentify->CommandSetSupportExt.ReservedForDrqTechnicalReport != 0 )
    {
        PrintNormalA ( "%*s - %s\n", MaxNameLen + 6, pText, "ReservedForDrqTechnicalReport" );
    }
    if ( pIdentify->CommandSetSupportExt.SenseDataReporting != 0 )
    {
        PrintNormalA ( "%*s - %s\n", MaxNameLen + 6, pText, "SenseDataReporting" );
    }
    if ( pIdentify->CommandSetSupportExt.WordValid != 0 )
    {
        PrintNormalA ( "%*s - %s\n", MaxNameLen + 6, pText, "WordValid" );
    }
    if ( pIdentify->CommandSetSupportExt.WriteReadVerify != 0 )
    {
        PrintNormalA ( "%*s - %s\n", MaxNameLen + 6, pText, "WriteReadVerify" );
    }
    if ( pIdentify->CommandSetSupportExt.WriteUncorrectableExt != 0 )
    {
        PrintNormalA ( "%*s - %s\n", MaxNameLen + 6, pText, "WriteUncorrectableExt" );
    }
}

//
///////////////////////////////////////////////////////////////////////////////
//  PrintCommandSetSupportExt
///////////////////////////////////////////////////////////////////////////////
void PrintCommandSetActiveExt ( PIDENTIFY_DEVICE_DATA pIdentify, char *pText )
{
    if ( pIdentify->CommandSetActiveExt.DownloadMicrocodeMode3 != 0 )
    {
        PrintNormalA ( "%*s - %s\n", MaxNameLen + 6, pText, "DownloadMicrocodeMode3" );
    }
    if ( pIdentify->CommandSetActiveExt.ExtendedPowerConditions != 0 )
    {
        PrintNormalA ( "%*s - %s\n", MaxNameLen + 6, pText, "ExtendedPowerConditions" );
    }
    if ( pIdentify->CommandSetActiveExt.FreefallControl != 0 )
    {
        PrintNormalA ( "%*s - %s\n", MaxNameLen + 6, pText, "FreefallControl" );
    }
    if ( pIdentify->CommandSetActiveExt.ReadWriteLogDmaExt != 0 )
    {
        PrintNormalA ( "%*s - %s\n", MaxNameLen + 6, pText, "ReadWriteLogDmaExt" );
    }
    if ( pIdentify->CommandSetActiveExt.ReservedForDrqTechnicalReport != 0 )
    {
        PrintNormalA ( "%*s - %s\n", MaxNameLen + 6, pText, "ReservedForDrqTechnicalReport" );
    }
    if ( pIdentify->CommandSetActiveExt.SenseDataReporting != 0 )
    {
        PrintNormalA ( "%*s - %s\n", MaxNameLen + 6, pText, "SenseDataReporting" );
    }
    if ( pIdentify->CommandSetActiveExt.WriteReadVerify != 0 )
    {
        PrintNormalA ( "%*s - %s\n", MaxNameLen + 6, pText, "WriteReadVerify" );
    }
    if ( pIdentify->CommandSetActiveExt.WriteUncorrectableExt != 0 )
    {
        PrintNormalA ( "%*s - %s\n", MaxNameLen + 6, pText, "WriteUncorrectableExt" );
    }
}

//
///////////////////////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////////////////////
void PrintCurrentMediaSerialNumber ( PIDENTIFY_DEVICE_DATA pIdentify, char *pText )
{
    int iCountNonZero = 0;
    for ( int i = 0; i < 30; i++ )
    {
        if ( pIdentify->CurrentMediaSerialNumber [ i ] != 0 )
        {
            iCountNonZero++;
        }
    }

    if ( iCountNonZero == 0 )
    {
        return;
    }

    PrintNormalA ( "%*s - ", MaxNameLen + 6, pText );
    for ( int i = 0; i < 30; i++ )
    {
        if ( pIdentify->CurrentMediaSerialNumber [ i ] != 0 )
        {
            PrintNormalW ( L"%04x ", pIdentify->CurrentMediaSerialNumber [ i ] );
        }
    }
    PrintNormalA ( "\n" );
}

//
///////////////////////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////////////////////
char *SeparatedNumberL ( unsigned long long largeInt, char separator = ',' )
{
    static char szTemp1 [ LEN_STRING ];
    return SeparatedNumberA ( szTemp1, sizeof(szTemp1), largeInt, separator );
}

//
///////////////////////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////////////////////
char *SeparatedNumberL ( LARGE_INTEGER largeInt, char separator = ',' )
{
    static char szTemp2 [ LEN_STRING ];
    return SeparatedNumberA ( szTemp2, sizeof(szTemp2), largeInt, separator );
}

//
///////////////////////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////////////////////
char *HumanReadableL ( unsigned long long largeInt )
{
    static char szTemp3 [ LEN_STRING ];
    return HumanReadableA ( szTemp3, sizeof(szTemp3), largeInt );
}

//
///////////////////////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////////////////////
char *HumanReadableL ( LARGE_INTEGER largeInt )
{
    static char szTemp4 [ LEN_STRING ];
    return HumanReadableA ( szTemp4, sizeof(szTemp4), largeInt );
}

//
///////////////////////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////////////////////
void PrintIdentify ( PIDENTIFY_DEVICE_DATA pIdentify )
{
    char szTemp [ LEN_STRING ];

    memset ( szTemp, 0, sizeof(szTemp) );
    memcpy ( szTemp, pIdentify->ModelNumber, sizeof(pIdentify->ModelNumber) );
    PrintNormalA ( "%*s : %s\n", MaxNameLen + 6, "Model Number", szTemp );

    memset ( szTemp, 0, sizeof(szTemp) );
    memcpy ( szTemp, pIdentify->SerialNumber, sizeof(pIdentify->SerialNumber) );
    PrintNormalA ( "%*s : %s\n", MaxNameLen + 6, "Serial Number", szTemp );

    memset ( szTemp, 0, sizeof(szTemp) );
    memcpy ( szTemp, pIdentify->FirmwareRevision, sizeof(pIdentify->FirmwareRevision) );
    PrintNormalA ( "%*s : %s\n", MaxNameLen + 6, "Firmware Rev", szTemp );

    //
    if ( ! NoDetail )
    {
        PrintNormalA ( "%*s : 0x%04x\n", MaxNameLen + 6, "General Configuration", pIdentify->GeneralConfiguration );
        if ( FullDetail )
        {
            PrintGeneralConfiguration ( pIdentify, "" );
        }
        PrintNormalA ( "%*s : %s\n", MaxNameLen + 6, "Num Cylinders", SeparatedNumberL ( pIdentify->NumCylinders ) );
        PrintNormalA ( "%*s : %u\n", MaxNameLen + 6, "Specific Configuration", pIdentify->SpecificConfiguration );
        PrintNormalA ( "%*s : %u\n", MaxNameLen + 6, "Num Heads", pIdentify->NumHeads );
        PrintNormalA ( "%*s : %u\n", MaxNameLen + 6, "Num Sectors Per Track", pIdentify->NumSectorsPerTrack );
        PrintNormalA ( "%*s : 0x%04x\n", MaxNameLen + 6, "Vendor Unique 0", pIdentify->VendorUnique1 [ 0 ] );
        if ( FullDetail )
        {
            PrintNormalA ( "%*s : 0x%04x\n", MaxNameLen + 6, "Vendor Unique 1", pIdentify->VendorUnique1 [ 1 ] );
            PrintNormalA ( "%*s : 0x%04x\n", MaxNameLen + 6, "Vendor Unique 2", pIdentify->VendorUnique1 [ 2 ] );
        }
        PrintNormalA ( "%*s : %u\n", MaxNameLen + 6, "Maximum Block Transfer", pIdentify->MaximumBlockTransfer );
        PrintNormalA ( "%*s : %u\n", MaxNameLen + 6, "Vendor Unique 2", pIdentify->VendorUnique2 );
        PrintNormalA ( "%*s : 0x%04x\n", MaxNameLen + 6, "Trusted Computing", pIdentify->TrustedComputing );
        if ( FullDetail )
        {
            PrintTrustComputing ( pIdentify, "" );
        }
        PrintNormalA ( "%*s : 0x%04x\n", MaxNameLen + 6, "Capabilities", pIdentify->Capabilities );
        if ( FullDetail )
        {
            PrintCapabilities ( pIdentify, "" );
        }
        PrintNormalA ( "%*s : %u\n", MaxNameLen + 6, "Translation Fields Valid", pIdentify->TranslationFieldsValid );
        PrintNormalA ( "%*s : %u\n", MaxNameLen + 6, "Free Fall Control Sensitivity", pIdentify->FreeFallControlSensitivity );
        PrintNormalA ( "%*s : %s\n", MaxNameLen + 6, "Number Of Current Cylinders", SeparatedNumberL ( pIdentify->NumberOfCurrentCylinders ) );
        PrintNormalA ( "%*s : %u\n", MaxNameLen + 6, "Number Of Current Heads", pIdentify->NumberOfCurrentHeads );
        PrintNormalA ( "%*s : %u\n", MaxNameLen + 6, "Current Sectors Per Track", pIdentify->CurrentSectorsPerTrack );
        PrintNormalA ( "%*s : %s\n", MaxNameLen + 6, "Current Sector Capacity", SeparatedNumberL ( pIdentify->CurrentSectorCapacity ) );
        PrintNormalA ( "%*s : %u\n", MaxNameLen + 6, "Current Multi Sector Setting", pIdentify->CurrentMultiSectorSetting );
        PrintNormalA ( "%*s : %u\n", MaxNameLen + 6, "Multi Sector Setting Valid", pIdentify->MultiSectorSettingValid );
        PrintNormalA ( "%*s : %u\n", MaxNameLen + 6, "Sanitize Feature Supported", pIdentify->SanitizeFeatureSupported );
        PrintNormalA ( "%*s : %u\n", MaxNameLen + 6, "Crypto Scramble Ext Command Supported", pIdentify->CryptoScrambleExtCommandSupported  );
        PrintNormalA ( "%*s : %u\n", MaxNameLen + 6, "Overwrite Ext Command Supported", pIdentify->OverwriteExtCommandSupported );
        PrintNormalA ( "%*s : %u\n", MaxNameLen + 6, "Block Erase Ext Command Supported", pIdentify->BlockEraseExtCommandSupported );
        PrintNormalA ( "%*s : %s\n", MaxNameLen + 6, "User Addressable Sectors", SeparatedNumberL ( pIdentify->UserAddressableSectors ) );
        PrintNormalA ( "%*s : %u\n", MaxNameLen + 6, "Multi Word DMA Support", pIdentify->MultiWordDMASupport );
        PrintNormalA ( "%*s : %u\n", MaxNameLen + 6, "Multi Word DMA Active", pIdentify->MultiWordDMAActive );
        PrintNormalA ( "%*s : %u\n", MaxNameLen + 6, "Advanced PIO Modes", pIdentify->AdvancedPIOModes );
        PrintNormalA ( "%*s : %u\n", MaxNameLen + 6, "Minimum MWXfer Cycle Time", pIdentify->MinimumMWXferCycleTime );
        PrintNormalA ( "%*s : %u\n", MaxNameLen + 6, "Recommended MWXfer Cycle Time", pIdentify->RecommendedMWXferCycleTime );
        PrintNormalA ( "%*s : %u\n", MaxNameLen + 6, "Minimum PIO Cycle Time", pIdentify->MinimumPIOCycleTime );
        PrintNormalA ( "%*s : %u\n", MaxNameLen + 6, "Minimum PIO Cycle Time IORDY", pIdentify->MinimumPIOCycleTimeIORDY );
        PrintNormalA ( "%*s : 0x%04x\n", MaxNameLen + 6, "Additional Supported", pIdentify->AdditionalSupported );
        if ( FullDetail )
        {
            PrintAdditionalSupported ( pIdentify, "" );
        }
        PrintNormalA ( "%*s : %u\n", MaxNameLen + 6, "Queue Depth", pIdentify->QueueDepth );
        PrintNormalA ( "%*s : 0x%08x\n", MaxNameLen + 6, "Serial Ata Capabilities", pIdentify->SerialAtaCapabilities );
        if ( FullDetail )
        {
            PrintSerialAtaCapabilities ( pIdentify, "" );
        }
        PrintNormalA ( "%*s : 0x%08x\n", MaxNameLen + 6, "Serial Ata Features Supported", pIdentify->SerialAtaFeaturesSupported );
        if ( FullDetail )
        {
            PrintSerialAtaFeaturesSupported ( pIdentify, "" );
        }
        PrintNormalA ( "%*s : 0x%08x\n", MaxNameLen + 6, "Serial Ata Features Enabled", pIdentify->SerialAtaFeaturesEnabled );
        if ( FullDetail )
        {
            PrintSerialAtaFeaturesEnabled ( pIdentify, "" );
        }
        PrintNormalA ( "%*s : %u\n", MaxNameLen + 6, "Major Revision", pIdentify->MajorRevision );
        PrintNormalA ( "%*s : %u\n", MaxNameLen + 6, "Minor Revision", pIdentify->MinorRevision );
        PrintNormalA ( "%*s : 0x%08x\n", MaxNameLen + 6, "Command Set Support", pIdentify->CommandSetSupport );
        if ( FullDetail )
        {
            PrintCommandSetSupport ( pIdentify, "" );
        }
        PrintNormalA ( "%*s : 0x%08x\n", MaxNameLen + 6, "Command Set Active", pIdentify->CommandSetActive );
        if ( FullDetail )
        {
            PrintCommandSetActive ( pIdentify, "" );
        }
        PrintNormalA ( "%*s : %u\n", MaxNameLen + 6, "Ultra DMA Support", pIdentify->UltraDMASupport );
        PrintNormalA ( "%*s : %u\n", MaxNameLen + 6, "Ultra DMA Active", pIdentify->UltraDMAActive );
        PrintNormalA ( "%*s : %s : %u\n", MaxNameLen + 6, "Normal Security Erase Unit", "TimeRequired", pIdentify->NormalSecurityEraseUnit.TimeRequired );
        PrintNormalA ( "%*s : %s : %u\n", MaxNameLen + 6, "Normal Security Erase Unit", "ExtendedTimeReported", pIdentify->NormalSecurityEraseUnit.ExtendedTimeReported );
        PrintNormalA ( "%*s : %s : %u\n", MaxNameLen + 6, "Enhanced Security Erase Unit", "TimeRequired", pIdentify->EnhancedSecurityEraseUnit.TimeRequired );
        PrintNormalA ( "%*s : %s : %u\n", MaxNameLen + 6, "Enhanced Security Erase Unit", "ExtendedTimeReported", pIdentify->EnhancedSecurityEraseUnit.ExtendedTimeReported );
        PrintNormalA ( "%*s : %u\n", MaxNameLen + 6, "Current APM Level", pIdentify->CurrentAPMLevel );
        PrintNormalA ( "%*s : %u\n", MaxNameLen + 6, "Master Password ID", pIdentify->MasterPasswordID );
        PrintNormalA ( "%*s : %u\n", MaxNameLen + 6, "Hardware Reset Result", pIdentify->HardwareResetResult );
        PrintNormalA ( "%*s : %u\n", MaxNameLen + 6, "Current Acoustic Value", pIdentify->CurrentAcousticValue );
        PrintNormalA ( "%*s : %u\n", MaxNameLen + 6, "Recommended Acoustic Value", pIdentify->RecommendedAcousticValue );
        PrintNormalA ( "%*s : %u\n", MaxNameLen + 6, "Stream Min Request Size", pIdentify->StreamMinRequestSize );
        PrintNormalA ( "%*s : %u\n", MaxNameLen + 6, "Streaming Transfer Time DMA", pIdentify->StreamingTransferTimeDMA );
        PrintNormalA ( "%*s : %u\n", MaxNameLen + 6, "Streaming Access Latency DMA PIO", pIdentify->StreamingAccessLatencyDMAPIO );
        PrintNormalA ( "%*s : %lu\n", MaxNameLen + 6, "Streaming Perf Granularity", pIdentify->StreamingPerfGranularity );

        LARGE_INTEGER uLargeInt;
        uLargeInt.LowPart   = pIdentify->Max48BitLBA[0];
        uLargeInt.HighPart  = pIdentify->Max48BitLBA[1];

        PrintNormalA ( "%*s : %s\n", MaxNameLen + 6, "Max 48Bit LBA", SeparatedNumberL ( uLargeInt )  );
        PrintNormalA ( "%*s : %u\n", MaxNameLen + 6, "Streaming Transfer Time", pIdentify->StreamingTransferTime );
        PrintNormalA ( "%*s : %u\n", MaxNameLen + 6, "Dsm Cap", pIdentify->DsmCap );
        PrintNormalA ( "%*s : 0x%04x\n", MaxNameLen + 6, "Physical Logical Sector Size", pIdentify->PhysicalLogicalSectorSize );
        if ( FullDetail )
        {
            PrintPhysicalLogicalSectorSize ( pIdentify, "" );
        }

        PrintNormalA ( "%*s : %u\n", MaxNameLen + 6, "Inter Seek Delay", pIdentify->InterSeekDelay );
        PrintNormalA ( "%*s : 0x%04x\n", MaxNameLen + 6, "World Wide Name 0", pIdentify->WorldWideName [ 0 ] );
        if ( FullDetail )
        {
            PrintNormalA ( "%*s : 0x%04x\n", MaxNameLen + 6, "World Wide Name 1", pIdentify->WorldWideName [ 1 ] );
            PrintNormalA ( "%*s : 0x%04x\n", MaxNameLen + 6, "World Wide Name 2", pIdentify->WorldWideName [ 2 ] );
            PrintNormalA ( "%*s : 0x%04x\n", MaxNameLen + 6, "World Wide Name 3", pIdentify->WorldWideName [ 3 ] );
        }
        PrintNormalA ( "%*s : %u\n", MaxNameLen + 6, "Words Per Logical Sector 0", pIdentify->WordsPerLogicalSector [ 0 ]);
        PrintNormalA ( "%*s : %u\n", MaxNameLen + 6, "Words Per Logical Sector 1", pIdentify->WordsPerLogicalSector [ 1 ]);
        PrintNormalA ( "%*s : 0x%04x\n", MaxNameLen + 6, "Command Set Support Ext", pIdentify->CommandSetSupportExt );
        if ( FullDetail )
        {
            PrintCommandSetSupportExt ( pIdentify, "" );
        }
        PrintNormalA ( "%*s : 0x%04x\n", MaxNameLen + 6, "Command Set Active Ext", pIdentify->CommandSetActiveExt );
        if ( FullDetail )
        {
            PrintCommandSetActiveExt ( pIdentify, "" );
        }
        PrintNormalA ( "%*s : %u\n", MaxNameLen + 6, "Msn Support", pIdentify->MsnSupport );
        PrintNormalA ( "%*s : 0x%04x\n", MaxNameLen + 6, "Security Status", pIdentify->SecurityStatus );
        if ( FullDetail )
        {
            PrintSecurityStatus ( pIdentify, "" );
        }
        PrintNormalA ( "%*s : 0x%04x\n", MaxNameLen + 6, "Cfa Power Mode1", pIdentify->CfaPowerMode1 );
        if ( FullDetail )
        {
            PrintCfaPowerMode1 ( pIdentify, "" );
        }
        PrintNormalA ( "%*s : %u\n", MaxNameLen + 6, "Nominal Form Factor", pIdentify->NominalFormFactor );
        PrintNormalA ( "%*s : 0x%04x\n", MaxNameLen + 6, "Data Set Management Feature", pIdentify->DataSetManagementFeature );
        if ( FullDetail )
        {
            PrintDataSetManagementFeature ( pIdentify, "" );
        }
        PrintNormalA ( "%*s : 0x%04x\n", MaxNameLen + 6, "Additional Product ID 0", pIdentify->AdditionalProductID [ 0 ]);
        if ( FullDetail )
        {
            PrintNormalA ( "%*s : 0x%04x\n", MaxNameLen + 6, "Additional Product ID 1", pIdentify->AdditionalProductID [ 1 ]);
            PrintNormalA ( "%*s : 0x%04x\n", MaxNameLen + 6, "Additional Product ID 2", pIdentify->AdditionalProductID [ 2 ]);
            PrintNormalA ( "%*s : 0x%04x\n", MaxNameLen + 6, "Additional Product ID 3", pIdentify->AdditionalProductID [ 3 ]);
        }
        PrintNormalA ( "%*s : %u\n", MaxNameLen + 6, "Current Media Serial Number", pIdentify->CurrentMediaSerialNumber [ 0 ] );
        if ( FullDetail )
        {
            PrintCurrentMediaSerialNumber ( pIdentify, "" );
        }
        PrintNormalA ( "%*s : 0x%04x\n", MaxNameLen + 6, "SCT Command Transport", pIdentify->SCTCommandTransport );
        if ( FullDetail )
        {
            PrintSCTCommandTransport ( pIdentify, "" );
        }
        PrintNormalA ( "%*s : 0x%04x\n", MaxNameLen + 6, "Block Alignment", pIdentify->BlockAlignment );
        if ( FullDetail )
        {
            PrintBlockAlignment ( pIdentify, "" );
        }
        PrintNormalA ( "%*s : %u %u\n", MaxNameLen + 6, "Write Read Verify Sector Count Mode3 Only",
            pIdentify->WriteReadVerifySectorCountMode3Only [ 0 ], pIdentify->WriteReadVerifySectorCountMode3Only [ 1 ] );
        PrintNormalA ( "%*s : %u %u\n", MaxNameLen + 6, "Write Read Verify Sector Count Mode2 Only",
            pIdentify->WriteReadVerifySectorCountMode2Only [ 0 ], pIdentify->WriteReadVerifySectorCountMode2Only [ 1 ]);
        PrintNormalA ( "%*s : 0x%04x\n", MaxNameLen + 6, "NV Cache Capabilities", pIdentify->NVCacheCapabilities );
        if ( FullDetail )
        {
            PrintNVCacheCapabilities ( pIdentify, "" );
        }
        PrintNormalA ( "%*s : %u\n", MaxNameLen + 6, "NV Cache Size LSW", pIdentify->NVCacheSizeLSW );
        PrintNormalA ( "%*s : %u\n", MaxNameLen + 6, "NV Cache Size MSW", pIdentify->NVCacheSizeMSW );
        PrintNormalA ( "%*s : %u\n", MaxNameLen + 6, "Nominal Media Rotation Rate", pIdentify->NominalMediaRotationRate );
        PrintNormalA ( "%*s : 0x%04x\n", MaxNameLen + 6, "NV Cache Options", pIdentify->NVCacheOptions );
        if ( FullDetail )
        {
            PrintNVCacheOptions ( pIdentify, "" );
        }
        PrintNormalA ( "%*s : %u\n", MaxNameLen + 6, "Write Read Verify Sector Count Mode", pIdentify->WriteReadVerifySectorCountMode );
        PrintNormalA ( "%*s : Major : %u Type : %u\n", MaxNameLen + 6, "Transport Major Version",
            pIdentify->TransportMajorVersion.MajorVersion, pIdentify->TransportMajorVersion.TransportType );
        PrintNormalA ( "%*s : %u\n", MaxNameLen + 6, "Transport Minor Version", pIdentify->TransportMinorVersion );

        uLargeInt.LowPart   = pIdentify->ExtendedNumberOfUserAddressableSectors [ 0 ];
        uLargeInt.HighPart  = pIdentify->ExtendedNumberOfUserAddressableSectors [ 1 ];
        PrintNormalA ( "%*s : %s\n", MaxNameLen + 6, "Extended Number Of User Addressable Sectors", SeparatedNumberL ( uLargeInt ) );
        PrintNormalA ( "%*s : %u\n", MaxNameLen + 6, "Min Blocks Per Download Microcode Mode 03", pIdentify->MinBlocksPerDownloadMicrocodeMode03 );
        PrintNormalA ( "%*s : %u\n", MaxNameLen + 6, "Max Blocks Per Download Microcode Mode 03", pIdentify->MaxBlocksPerDownloadMicrocodeMode03 );
        PrintNormalA ( "%*s : 0x%04x\n", MaxNameLen + 6, "Signature", pIdentify->Signature );
        PrintNormalA ( "%*s : 0x%04x\n", MaxNameLen + 6, "CheckSum", pIdentify->CheckSum );

    }

    if ( Dump )
    {
        DumpBuffer ( ' ', "SECTOR", (BYTE *) pIdentify, sizeof ( IDENTIFY_DEVICE_DATA ), true, true, true );
    }

}

//
///////////////////////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////////////////////
BOOL ScsiPassThruIdentify ( HANDLE hDevice, COMMAND_TYPE type, UCHAR target )
{
    BOOL            bRet    = FALSE;
    DWORD           dwRet   = 0;

    const WCHAR *pLabelName = L"--- SCSI Identify";

    ZeroMemory ( &ScsiPassThroughWithBuffer, sizeof(ScsiPassThroughWithBuffer) );

    ScsiPassThroughWithBuffer.ScsiPassThrough.Length = sizeof(SCSI_PASS_THROUGH);
    ScsiPassThroughWithBuffer.ScsiPassThrough.PathId = 0;
    ScsiPassThroughWithBuffer.ScsiPassThrough.TargetId = 0;
    ScsiPassThroughWithBuffer.ScsiPassThrough.Lun = 0;
    ScsiPassThroughWithBuffer.ScsiPassThrough.SenseInfoLength = 24;
    ScsiPassThroughWithBuffer.ScsiPassThrough.DataIn = SCSI_IOCTL_DATA_IN;
    ScsiPassThroughWithBuffer.ScsiPassThrough.DataTransferLength = IDENTIFY_BUFFER_SIZE;
    ScsiPassThroughWithBuffer.ScsiPassThrough.TimeOutValue = 2;
    ScsiPassThroughWithBuffer.ScsiPassThrough.DataBufferOffset = offsetof(ST_SCSI_PASS_THROUGH_BUF, DataBuffer);
    ScsiPassThroughWithBuffer.ScsiPassThrough.SenseInfoOffset = offsetof(ST_SCSI_PASS_THROUGH_BUF, SenseBuffer);

    if (type == CMD_TYPE_SAT)
    {
        ScsiPassThroughWithBuffer.ScsiPassThrough.CdbLength = 12;
        ScsiPassThroughWithBuffer.ScsiPassThrough.Cdb[0]    = 0xA1;//ATA PASS THROUGH(12) OPERATION CODE(A1h)
        ScsiPassThroughWithBuffer.ScsiPassThrough.Cdb[1]    = (4 << 1) | 0; //MULTIPLE_COUNT=0,PROTOCOL=4(PIO Data-In),Reserved
        ScsiPassThroughWithBuffer.ScsiPassThrough.Cdb[2]    = (1 << 3) | (1 << 2) | 2;//OFF_LINE=0,CK_COND=0,Reserved=0,T_DIR=1(ToDevice),BYTE_BLOCK=1,T_LENGTH=2
        
        SetScsiIdentify ( (IDEREGS *)&ScsiPassThroughWithBuffer.ScsiPassThrough.Cdb[3] );
        ScsiPassThroughWithBuffer.ScsiPassThrough.Cdb[8]    = target;
    }
    else
    {
        return FALSE;
    }

    DWORD length = offsetof(ST_SCSI_PASS_THROUGH_BUF, DataBuffer) + ScsiPassThroughWithBuffer.ScsiPassThrough.DataTransferLength;

    bRet = DeviceIoControl ( hDevice, IOCTL_SCSI_PASS_THROUGH,
        &ScsiPassThroughWithBuffer, sizeof(SCSI_PASS_THROUGH),
        &ScsiPassThroughWithBuffer, length, &dwRet, NULL);

    //
    //  Success
    if ( bRet && ( dwRet == length ) )
    {
        // CopyMemory ( &m_stDrivesInfo[bDeviceNumber].m_stInfo, ( (char *) szOutput ) +16, sizeof(ST_IDSECTOR) );
        CopyMemory ( &SectorInfo, ( (char *) ScsiPassThroughWithBuffer.DataBuffer ), sizeof(IDENTIFY_DEVICE_DATA) );

        SwitchBytes ( SectorInfo.ModelNumber, sizeof(SectorInfo.ModelNumber) );
        SwitchBytes ( SectorInfo.SerialNumber, sizeof(SectorInfo.SerialNumber) );
        SwitchBytes ( SectorInfo.FirmwareRevision, sizeof(SectorInfo.FirmwareRevision) );

        //
        //  Display Drive Info
        if ( ! NoDetail )
        {
            //
            //  Display Drive Info
            PrintNormalW ( L"%*s : ---------------\n", MaxNameLen + 6, pLabelName );

            PrintIdentify ( &SectorInfo );
        }
    }

    //  Error
    else if ( ShowError )
    {
        dwRet = GetLastError();
        PrintStderrW ( L"Error - DeviceIoControl IOCTL_SCSI_PASS_THROUGH Fails - Cause : 0x%lx - %s\n", GetLastError(), GetLastErrorText() );
        bRet = false;
    }
    else if ( ! NoDetail )
    {
        PrintNormalW ( L"%*s : ---------------\n", MaxNameLen + 6, pLabelName, GetLastErrorText() );
        bRet = false;
    }
    else
    {
        bRet = false;
    }

    return bRet;
}


//
///////////////////////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////////////////////
BOOL AtaIdentify ( HANDLE hDevice )
{
    BOOL bRet                           = FALSE;
    DWORD dwRet                         = 0;

    const WCHAR *pLabelName = L"--- ATA Identify";

    ZeroMemory ( AtaIdentifyRequest, sizeof(AtaIdentifyRequest) );

    pAtaIdentifyRequest->Length                 = sizeof(ATA_PASS_THROUGH_EX);
    pAtaIdentifyRequest->AtaFlags               = ATA_FLAGS_DATA_IN | ATA_FLAGS_DRDY_REQUIRED;
    pAtaIdentifyRequest->DataBufferOffset       = sizeof( ATA_PASS_THROUGH_EX );
    pAtaIdentifyRequest->DataTransferLength     = sizeof( IDENTIFY_DEVICE_DATA );
    pAtaIdentifyRequest->TimeOutValue           = 2;        // In Seconds

    IDEREGS *ir                                 = (IDEREGS *) pAtaIdentifyRequest->CurrentTaskFile;
    SetAtaIdentify ( ir );

    bRet =
        DeviceIoControl(
            hDevice, IOCTL_ATA_PASS_THROUGH,
            pAtaIdentifyRequest, sizeof(ATA_PASS_THROUGH_EX)+IDENTIFY_BUFFER_SIZE,
            pAtaIdentifyRequest, sizeof(ATA_PASS_THROUGH_EX)+IDENTIFY_BUFFER_SIZE,
            &dwRet, NULL );
    //
    //  Success
    if ( bRet && ( dwRet >= sizeof(ATA_PASS_THROUGH_EX) ) )
    {
        // CopyMemory ( &m_stDrivesInfo[bDeviceNumber].m_stInfo, ( (char *) szOutput ) +16, sizeof(ST_IDSECTOR) );
        CopyMemory ( &SectorInfo, ( (char *) AtaIdentifyRequest ) + sizeof(ATA_PASS_THROUGH_EX), sizeof(IDENTIFY_DEVICE_DATA) );

        SwitchBytes ( SectorInfo.ModelNumber, sizeof(SectorInfo.ModelNumber) );
        SwitchBytes ( SectorInfo.SerialNumber, sizeof(SectorInfo.SerialNumber) );
        SwitchBytes ( SectorInfo.FirmwareRevision, sizeof(SectorInfo.FirmwareRevision) );

        //
        //  Display Drive Info
        if ( ! NoDetail )
        {
            //
            //  Display Drive Info
            PrintNormalW ( L"%*s : ---------------\n", MaxNameLen + 6, pLabelName );

            PrintIdentify ( &SectorInfo );
        }
    }

    //
    //  Error
    else if ( ShowError )
    {
        dwRet = GetLastError();
        PrintStderrW ( L"Error - DeviceIoControl IOCTL_ATA_PASS_THROUGH Fails - Cause : 0x%lx - %s\n", GetLastError(), GetLastErrorText() );
        bRet = FALSE;
    }

    //
    else if ( ! NoDetail )
    {
        PrintNormalW ( L"%*s : %s\n", MaxNameLen + 6, pLabelName, GetLastErrorText() );
        bRet = FALSE;
    }

    else
    {
        bRet = FALSE;
    }

    return bRet;
}

//
///////////////////////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////////////////////
BOOL SmartIdentify ( HANDLE hDevice )
{
    BOOL bRet                           = FALSE;
    DWORD dwRet                         = 0;

    const WCHAR *pLabelName = L"--- SMART Identify";

    ZeroMemory ( &SendCmdInParams, sizeof(SendCmdInParams) );

    ZeroMemory ( &SmartIdentifyResponse, sizeof(SmartIdentifyResponse) );

    SendCmdInParams.cBufferSize                     = IDENTIFY_BUFFER_SIZE;
    SendCmdInParams.bDriveNumber                    = 0;

    SetSmartIdentify ( &SendCmdInParams.irDriveRegs );

    bRet =
        DeviceIoControl (
            hDevice, SMART_RCV_DRIVE_DATA,
            &SendCmdInParams, sizeof(SendCmdInParams)-1,
            SmartIdentifyResponse, sizeof(SENDCMDOUTPARAMS)+IDENTIFY_BUFFER_SIZE-1, &dwRet, NULL );
    //
    //  Success
    if ( bRet && ( dwRet >= sizeof(SENDCMDOUTPARAMS) ) )
    {
        // CopyMemory ( &m_stDrivesInfo[bDeviceNumber].m_stInfo, ( (char *) szOutput ) +16, sizeof(ST_IDSECTOR) );
        CopyMemory ( &SectorInfo, ( (char *) SmartIdentifyResponse ) + sizeof(SENDCMDOUTPARAMS) - 1, sizeof(IDENTIFY_DEVICE_DATA) );

        SwitchBytes ( SectorInfo.ModelNumber, sizeof(SectorInfo.ModelNumber) );
        SwitchBytes ( SectorInfo.SerialNumber, sizeof(SectorInfo.SerialNumber) );
        SwitchBytes ( SectorInfo.FirmwareRevision, sizeof(SectorInfo.FirmwareRevision) );

        //
        if ( ! NoDetail )
        {
            //
            //  Display Drive Info
            PrintNormalW ( L"%*s : ---------------\n", MaxNameLen + 6, pLabelName );

            //
            if ( Smart )
            {
                PrintNormalW ( L"%*s : %u\n", MaxNameLen + 6, L"SMART Version", DriveInfo.m_stGetVersionInParams.bVersion );
                PrintNormalW ( L"%*s : %u\n", MaxNameLen + 6, L"SMART Revision", DriveInfo.m_stGetVersionInParams.bRevision );
            }

            //
            PrintIdentify ( &SectorInfo );
        }
    }

    //
    //  Error
    else if ( ShowError )
    {
        dwRet = GetLastError();
        PrintStderrW ( L"Error - DeviceIoControl SMART_RCV_DRIVE_DATA Fails - Cause : 0x%lx - %s\n", GetLastError(), GetLastErrorText() );
        bRet = FALSE;
    }
    else
    {
        if ( ! NoDetail )
        {
            PrintNormalW ( L"%*s : %s\n", MaxNameLen + 6, pLabelName, GetLastErrorText() );
        }
        bRet = FALSE;
    }
    return bRet;
}

//
///////////////////////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////////////////////
BOOL IsSmartEnabled ( HANDLE hDevice )
{
    ZeroMemory ( &SendCmdInParams, sizeof(SendCmdInParams) );
    ZeroMemory ( &SendCmdOutParams, sizeof(SendCmdOutParams) );

    DWORD dwRet                         = 0;
    BOOL bRet                           = FALSE;

    SendCmdInParams.cBufferSize                     = 0;
    SendCmdInParams.bDriveNumber                    = 0;
    SetEnableStart ( &SendCmdInParams.irDriveRegs );

    //
    bRet =
        DeviceIoControl (
            hDevice, SMART_SEND_DRIVE_COMMAND,
            &SendCmdInParams, sizeof(SendCmdInParams) - 1,
            &SendCmdOutParams, sizeof(SendCmdOutParams) - 1,
            &dwRet, NULL );
    //
    //  Success
    if ( bRet )
    {
        return bRet;
    }

    //  Error
    else if ( ShowError )
    {
        dwRet = GetLastError();
        PrintStderrW ( L"Error - DeviceIoControl SMART_SEND_DRIVE_COMMAND Fails - Cause : 0x%lx - %s\n", GetLastError(), GetLastErrorText() );
        swprintf_s ( DriveInfo.m_csErrorString, _wsizeof(DriveInfo.m_csErrorString), L"Error %lu in reading SMART Enabled flag", dwRet );
    }

    return bRet;
}

//
///////////////////////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////////////////////
BOOL GetScsiAddress ( HANDLE hDevice )
{
    DWORD       dwRet;
    BOOL        bRet;

    const WCHAR *pLabelName = L"--- SCSI Address";

    ZeroMemory ( &ScsiAddress, sizeof(ScsiAddress) );
    bRet =
        DeviceIoControl(hDevice, IOCTL_SCSI_GET_ADDRESS,
            NULL, 0,
            &ScsiAddress, sizeof(ScsiAddress),
            &dwRet, NULL);

    //
    //  Success
    if ( bRet )
    {
        if ( ! NoHeader && ShowScsi )
        {
            PrintNormalW ( L"%*s : ---------------\n", MaxNameLen + 6, pLabelName );
            PrintNormalW ( L"%*s : %u\n", MaxNameLen + 6, L"Lun", ScsiAddress.Lun );
            PrintNormalW ( L"%*s : %u\n", MaxNameLen + 6, L"Path Id", ScsiAddress.PathId );
            PrintNormalW ( L"%*s : %u\n", MaxNameLen + 6, L"Port Number", ScsiAddress.PortNumber );
            PrintNormalW ( L"%*s : %u\n", MaxNameLen + 6, L"Target Id", ScsiAddress.TargetId );
        }
    }

    //  Error
    else if ( ShowError )
    {
        PrintStderrW ( L"Error - DeviceIoControl IOCTL_SCSI_GET_ADDRESS Fails - Cause : 0x%lx - %s\n", GetLastError(), GetLastErrorText() );
    }
    else if ( ! NoHeader && ShowScsi )
    {
        PrintNormalW ( L"%*s : %s\n", MaxNameLen + 6, pLabelName, GetLastErrorText() );
    }

    return bRet;
}

//
///////////////////////////////////////////////////////////////////////////////
//  https://www.irig106.org/wiki/ch10_handbook:data_retrieval
///////////////////////////////////////////////////////////////////////////////
BOOL GetScsiGetInquiryData ( HANDLE hDevice )
{
    DWORD       dwRet;
    BOOL        bRet;

    const WCHAR *pLabelName = L"--- SCSI Inquiry Data";

    ZeroMemory ( ScsiAdapterBusInfo, sizeof(ScsiAdapterBusInfo) );
    bRet =
        DeviceIoControl(hDevice, IOCTL_SCSI_GET_INQUIRY_DATA,
            NULL, 0,
            &ScsiAdapterBusInfo, sizeof(ScsiAdapterBusInfo),
            &dwRet, NULL);

    //
    //  Success
    if ( bRet )
    {
        if ( ! NoHeader && ShowScsi )
        {
            PrintNormalW ( L"%*s : ---------------\n", MaxNameLen + 6, pLabelName );
            PrintNormalW ( L"%*s : %u\n", MaxNameLen + 6, L"Number Of Buses", pScsiAdapterBusInfo->NumberOfBuses );
            for ( int i = 0; i < pScsiAdapterBusInfo->NumberOfBuses; i++ )
            {
                PrintNormalW ( L"%*s : %u\n", MaxNameLen + 6, L"Initiator Bus Id", pScsiAdapterBusInfo->BusData [ i ].InitiatorBusId );
                PrintNormalW ( L"%*s : %lu\n", MaxNameLen + 6, L"Inquiry Data Offset", pScsiAdapterBusInfo->BusData [ i ].InquiryDataOffset );
                PrintNormalW ( L"%*s : %u\n", MaxNameLen + 6, L"Number Of Logical Units", pScsiAdapterBusInfo->BusData [ i ].NumberOfLogicalUnits );
            }

            if ( Dump )
            {
                DumpBuffer ( ' ', "BUS", ScsiAdapterBusInfo, sizeof(ScsiAdapterBusInfo), true, true, true );
            }
        }
    }

    //  Error
    else if ( ShowError )
    {
        PrintStderrW ( L"Error - DeviceIoControl IOCTL_SCSI_GET_INQUIRY_DATA Fails - Cause : 0x%lx - %s\n", GetLastError(), GetLastErrorText() );
    }
    else if ( ! NoHeader && ShowScsi )
    {
        PrintNormalW ( L"%*s : %s\n", MaxNameLen + 6, pLabelName, GetLastErrorText() );
    }

    return bRet;
}

//
///////////////////////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////////////////////
BOOL GetScsiGetGetCapabilities ( HANDLE hDevice )
{
    DWORD       dwRet;
    BOOL        bRet;

    const WCHAR *pLabelName = L"--- SCSI Capabilities";

    ZeroMemory ( &IoScsiCapabilities, sizeof(IoScsiCapabilities) );
    bRet =
        DeviceIoControl(hDevice, IOCTL_SCSI_GET_CAPABILITIES,
            NULL, 0,
            &IoScsiCapabilities, sizeof(IoScsiCapabilities),
            &dwRet, NULL);

    //
    //  Success
    if ( bRet )
    {
        if ( ! NoHeader && ShowScsi )
        {
            PrintNormalW ( L"%*s : ---------------\n", MaxNameLen + 6, pLabelName );
            PrintNormalW ( L"%*s : %u\n", MaxNameLen + 6, L"Length", IoScsiCapabilities.Length );
            PrintNormalW ( L"%*s : %u\n", MaxNameLen + 6, L"Adapter Scans Down", IoScsiCapabilities.AdapterScansDown );
            PrintNormalW ( L"%*s : %lu\n", MaxNameLen + 6, L"Adapter Uses Pio", IoScsiCapabilities.AdapterUsesPio );
            PrintNormalW ( L"%*s : %lu\n", MaxNameLen + 6, L"Alignment Mask", IoScsiCapabilities.AlignmentMask );
            PrintNormalW ( L"%*s : %lu\n", MaxNameLen + 6, L"Maximum Physical Pages", IoScsiCapabilities.MaximumPhysicalPages );
            PrintNormalA ( "%*s : %s\n", MaxNameLen + 6, "Maximum Transfer Length", SeparatedNumberL ( IoScsiCapabilities.MaximumTransferLength ) );
            PrintNormalW ( L"%*s : %lu\n", MaxNameLen + 6, L"Supported Asynchronous Events", IoScsiCapabilities.SupportedAsynchronousEvents );
            PrintNormalW ( L"%*s : %u\n", MaxNameLen + 6, L"Tagged Queuing", IoScsiCapabilities.TaggedQueuing );
        }
    }

    //  Error
    else if ( ShowError )
    {
        PrintStderrW ( L"Error - DeviceIoControl IOCTL_SCSI_GET_CAPABILITIES Fails - Cause : 0x%lx - %s\n", GetLastError(), GetLastErrorText() );
    }
    else if ( ! NoHeader && ShowScsi )
    {
        PrintNormalW ( L"%*s : %s\n", MaxNameLen + 6, pLabelName, GetLastErrorText() );
    }

    return bRet;
}

//
///////////////////////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////////////////////
BOOL GetDeviceNumber ( HANDLE hDevice )
{
    DWORD       dwRet;
    BOOL        bRet;

    const WCHAR *pLabelName = L"--- Device Number";

    ZeroMemory ( &StorageDeviceNumber, sizeof(StorageDeviceNumber) );

    bRet =
        DeviceIoControl(hDevice, IOCTL_STORAGE_GET_DEVICE_NUMBER,
            NULL, 0,
            &StorageDeviceNumber, sizeof(StorageDeviceNumber),
            &dwRet, NULL);

    //
    //  Success
    if ( bRet )
    {
        if ( ! NoHeader && ShowDevice )
        {
            PrintNormalW ( L"%*s : ---------------\n", MaxNameLen + 6, pLabelName );
            PrintNormalW ( L"%*s : %lu\n", MaxNameLen + 6, L"Device Number", StorageDeviceNumber.DeviceNumber );
            PrintNormalW ( L"%*s : %s\n", MaxNameLen + 6, L"Device Type", DeviceTypeToString ( StorageDeviceNumber.DeviceType ) );
            PrintNormalW ( L"%*s : %lu\n", MaxNameLen + 6, L"Partition Number", StorageDeviceNumber.PartitionNumber );
        }
    }

    //  Error
    else if ( ShowError )
    {
        PrintStderrW ( L"Error - DeviceIoControl IOCTL_STORAGE_GET_DEVICE_NUMBER Fails - Cause : 0x%lx - %s\n", GetLastError(), GetLastErrorText() );
    }
    else if ( ! NoHeader && ShowDevice )
    {
        PrintNormalW ( L"%*s : %s\n", MaxNameLen + 6, pLabelName, GetLastErrorText() );
    }

    return bRet;
}

//
///////////////////////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////////////////////
BOOL GetStorage ( HANDLE hDevice )
{
    DWORD       dwRet;
    BOOL        bRet;

    const WCHAR *pLabelName = L"--- Storage";

    //
    ZeroMemory ( StorageDeviceDescriptor, sizeof(StorageDeviceDescriptor) );
    ZeroMemory ( &StoragePropertyQuery, sizeof(StoragePropertyQuery) );

    StoragePropertyQuery.PropertyId = StorageDeviceProperty;
    StoragePropertyQuery.QueryType  = PropertyStandardQuery;

    bRet =
        DeviceIoControl(hDevice, IOCTL_STORAGE_QUERY_PROPERTY,
            &StoragePropertyQuery, sizeof(STORAGE_PROPERTY_QUERY),
            StorageDeviceDescriptor, sizeof(StorageDeviceDescriptor),
            &dwRet, NULL);

    //
    //  Success
    if ( bRet )
    {
        if ( ! NoHeader && ShowStorage )
        {
            PrintNormalW ( L"%*s : ---------------\n", MaxNameLen + 6, pLabelName );
            PrintNormalW ( L"%*s : %s\n", MaxNameLen + 6, L"Bus Type", BusTypeToString ( pStorageDeviceDescriptor->BusType ) );
        
            PrintNormalW ( L"%*s : %02x\n", MaxNameLen + 6, L"Scsi 2 Device Type", pStorageDeviceDescriptor->DeviceType );

            if ( pStorageDeviceDescriptor->ProductIdOffset )
            {
                char *model = (char*)pStorageDeviceDescriptor + pStorageDeviceDescriptor->ProductIdOffset;
                PrintNormalA ( "%*s : %s\n", MaxNameLen + 6, "Model", model);
            }

            if ( pStorageDeviceDescriptor->ProductRevisionOffset )
            {
                char *firmware = (char*)pStorageDeviceDescriptor + pStorageDeviceDescriptor->ProductRevisionOffset;
                PrintNormalA ( "%*s : %s\n", MaxNameLen + 6, "Firmware", firmware);
            }

            if ( pStorageDeviceDescriptor->SerialNumberOffset )
            {
                BYTE *serialnumber = (BYTE *)pStorageDeviceDescriptor + pStorageDeviceDescriptor->SerialNumberOffset;
                PrintNormalA ( "%*s : %s\n", MaxNameLen + 6, "Serial Number", serialnumber);
                if ( IsHexa ( serialnumber ) )
                {
                    BYTE szTemp [ LEN_STRING ];
                    HexaToBin ( serialnumber, szTemp, sizeof(szTemp) );
                    if ( IsUSASCII ( szTemp ) )
                    {
                        SwitchBytes ( szTemp, sizeof(szTemp)  );
                        PrintNormalA ( "%*s : %s\n", MaxNameLen + 6, "Serial Number", szTemp);
                    }
                }
            }
        }       
    }

    //  Error
    else if ( ShowError )
    {
        PrintStderrW ( L"Error - DeviceIoControl IOCTL_STORAGE_QUERY_PROPERTY Fails - Cause : 0x%lx - %s\n", GetLastError(), GetLastErrorText() );
    }
    else if ( ! NoHeader && ShowStorage )
    {
        PrintNormalW ( L"%*s : %s\n", MaxNameLen + 6, pLabelName, GetLastErrorText() );
    }

    return bRet;
}

//
///////////////////////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////////////////////
BOOL GetStoragePredictFailure ( HANDLE hDevice )
{
    DWORD       dwRet;
    BOOL        bRet;

    const WCHAR *pLabelName = L"--- Predict Failure";

    //
    ZeroMemory ( &StoragePredictFailure, sizeof(StoragePredictFailure) );

    //
    //  Success
    bRet =
        DeviceIoControl(hDevice, IOCTL_STORAGE_PREDICT_FAILURE,
            NULL, 0,
            &StoragePredictFailure, sizeof(StoragePredictFailure),
            &dwRet, NULL);
    if ( bRet )
    {
        if ( ! NoHeader && ShowPredict )
        {
            PrintNormalW ( L"%*s : ---------------\n", MaxNameLen + 6, pLabelName );
            PrintNormalA ( "%*s : %lu\n", MaxNameLen + 6, "Predict Failure", StoragePredictFailure.PredictFailure );
        }
    }

    //  Error
    else if ( ShowError )
    {
        PrintStderrW ( L"Error - DeviceIoControl IOCTL_STORAGE_PREDICT_FAILURE Fails - Cause : 0x%lx - %s\n", GetLastError(), GetLastErrorText() );
    }
    else if ( ! NoHeader && ShowPredict )
    {
        PrintNormalW ( L"%*s : %s\n", MaxNameLen + 6, pLabelName, GetLastErrorText() );
    }

    return bRet;
}

//
///////////////////////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////////////////////
WCHAR *GuidToString(GUID *pGuid)
{
    static WCHAR szTemp [ LEN_STRING ];
    swprintf_s ( szTemp, _wsizeof(szTemp),
        L"{%08lX-%04hX-%04hX-%02hhX%02hhX-%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX}",
        pGuid->Data1, pGuid->Data2, pGuid->Data3,
        pGuid->Data4[0], pGuid->Data4[1], pGuid->Data4[2], pGuid->Data4[3],
        pGuid->Data4[4], pGuid->Data4[5], pGuid->Data4[6], pGuid->Data4[7] );
    return szTemp;
}

//
///////////////////////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////////////////////
WCHAR *GuidToString(WCHAR *pText, GUID *pGuid)
{
    static WCHAR szTemp [ LEN_STRING ];
    swprintf_s ( szTemp, _wsizeof(szTemp),
        L"%s={%08lX-%04hX-%04hX-%02hhX%02hhX-%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX}",
        pText,
        pGuid->Data1, pGuid->Data2, pGuid->Data3,
        pGuid->Data4[0], pGuid->Data4[1], pGuid->Data4[2], pGuid->Data4[3],
        pGuid->Data4[4], pGuid->Data4[5], pGuid->Data4[6], pGuid->Data4[7] );
    return szTemp;
}

//
///////////////////////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////////////////////
BOOL GetDiskGeometry ( HANDLE hDevice )
{
    DWORD       dwRet;
    BOOL        bRet;

    const WCHAR *pLabelName = L"--- Drive Geometry";

    //
    ZeroMemory ( DriveGeometry, sizeof(DriveGeometry) );

    bRet =
        DeviceIoControl(hDevice, IOCTL_DISK_GET_DRIVE_GEOMETRY_EX,
            NULL, 0,
            &DriveGeometry, sizeof(DriveGeometry),
            &dwRet, NULL);

    //
    //  Success
    if ( bRet )
    {
        if ( ! NoHeader && ShowGeometry )
        {
            PrintNormalW ( L"%*s : ---------------\n", MaxNameLen + 6, pLabelName );
            PrintNormalA ( "%*s : %s (%s)\n", MaxNameLen + 6, "Disk Size", SeparatedNumberL ( pDriveGeometry->DiskSize ), HumanReadableL ( pDriveGeometry->DiskSize ) );
            PrintNormalA ( "%*s : %lu\n", MaxNameLen + 6, "Bytes Per Sector", pDriveGeometry->Geometry.BytesPerSector );
            PrintNormalA ( "%*s : %s\n", MaxNameLen + 6, "Cylinders", SeparatedNumberL ( pDriveGeometry->Geometry.Cylinders ) );
            PrintNormalA ( "%*s : %lu\n", MaxNameLen + 6, "Sectors Per Track", pDriveGeometry->Geometry.SectorsPerTrack );
            PrintNormalA ( "%*s : %lu\n", MaxNameLen + 6, "Tracks Per Cylinder", pDriveGeometry->Geometry.TracksPerCylinder );

            PDISK_DETECTION_INFO pDiskDetectionInfo = DiskGeometryGetDetect ( pDriveGeometry );
            PrintNormalA ( "%*s : %s\n", MaxNameLen + 6, "Detection Type", DetectionTypeToString ( pDiskDetectionInfo->DetectionType ) );
            switch ( pDiskDetectionInfo->DetectionType )
            {
                case DetectInt13 :
                {
                    PrintNormalA ( "%*s : %u\n", MaxNameLen + 6, "Drive Select", pDiskDetectionInfo->Int13.DriveSelect );
                    PrintNormalA ( "%*s : %s\n", MaxNameLen + 6, "Max Cylinders", SeparatedNumberL ( pDiskDetectionInfo->Int13.MaxCylinders ) );
                    PrintNormalA ( "%*s : %u\n", MaxNameLen + 6, "Max Heads", pDiskDetectionInfo->Int13.MaxHeads );
                    PrintNormalA ( "%*s : %u\n", MaxNameLen + 6, "Number Drives", pDiskDetectionInfo->Int13.NumberDrives );
                    PrintNormalA ( "%*s : %u\n", MaxNameLen + 6, "Sectors Per Track", pDiskDetectionInfo->Int13.SectorsPerTrack );
                    break;
                }
                case DetectExInt13 :
                {
                    PrintNormalA ( "%*s : %u\n", MaxNameLen + 6, "Ex Buffer Size", pDiskDetectionInfo->ExInt13.ExBufferSize );
                    PrintNormalA ( "%*s : %s\n", MaxNameLen + 6, "Ex Cylinders", SeparatedNumberL ( pDiskDetectionInfo->ExInt13.ExCylinders ) );
                    PrintNormalA ( "%*s : %u\n", MaxNameLen + 6, "Ex Flags", pDiskDetectionInfo->ExInt13.ExFlags );
                    PrintNormalA ( "%*s : %u\n", MaxNameLen + 6, "Ex Heads", pDiskDetectionInfo->ExInt13.ExHeads );
                    PrintNormalA ( "%*s : %u\n", MaxNameLen + 6, "Ex Sector Size", pDiskDetectionInfo->ExInt13.ExSectorSize );
                    PrintNormalA ( "%*s : %s\n", MaxNameLen + 6, "Ex Sectors Per Drive", SeparatedNumberL ( pDiskDetectionInfo->ExInt13.ExSectorsPerDrive ) );
                    PrintNormalA ( "%*s : %u\n", MaxNameLen + 6, "Ex Sectors Per Track", pDiskDetectionInfo->ExInt13.ExSectorsPerTrack );
                    break;
                }
            }

            PDISK_PARTITION_INFO pDiskPartitionInfo = DiskGeometryGetPartition ( pDriveGeometry );
            PrintNormalA ( "%*s : %lu\n", MaxNameLen + 6, "Size Of Partition Info", pDiskPartitionInfo->SizeOfPartitionInfo );
            PrintNormalA ( "%*s : %s\n", MaxNameLen + 6, "PartitionStyle", PartitionStyleToString ( pDiskPartitionInfo->PartitionStyle ) );
            switch ( pDiskPartitionInfo->PartitionStyle )
            {
                case PARTITION_STYLE_MBR :
                {
                    PrintNormalA ( "%*s : 0x%08lx\n", MaxNameLen + 6, "Signature", pDiskPartitionInfo->Mbr.Signature );
                    PrintNormalA ( "%*s : 0x%08lx\n", MaxNameLen + 6, "CheckSum", pDiskPartitionInfo->Mbr.CheckSum );
                    break;
                }
                case PARTITION_STYLE_GPT :
                {
                    PrintNormalW ( L"%*s : %s\n", MaxNameLen + 6, L"Disk Id", GuidToString ( &pDiskPartitionInfo->Gpt.DiskId ) );
                    break;
                }
            }
        }
    }

    //  Error
    else if ( ShowError )
    {
        PrintStderrW ( L"Error - DeviceIoControl IOCTL_DISK_GET_DRIVE_LAYOUT_EX Fails - Cause : 0x%lx - %s\n", GetLastError(), GetLastErrorText() );
    }
    else if ( ! NoHeader && ShowGeometry )
    {
        PrintNormalW ( L"%*s : %s\n", MaxNameLen + 6, pLabelName, GetLastErrorText() );
    }

    return bRet;
}

//
///////////////////////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////////////////////
BOOL GetDiskPartition ( HANDLE hDevice )
{
    DWORD       dwRet;
    BOOL        bRet;

    const WCHAR *pLabelName = L"--- Drive Information";

    //
    ZeroMemory ( DriveInformation, sizeof(DriveInformation) );

    bRet =
        DeviceIoControl(hDevice, IOCTL_DISK_GET_DRIVE_LAYOUT_EX,
            NULL, 0,
            &DriveInformation, sizeof(DriveInformation),
            &dwRet, NULL);

    //
    //  Success
    if ( bRet )
    {
        if ( ! NoHeader && ShowDrive )
        {
            PrintNormalW ( L"%*s : ---------------\n", MaxNameLen + 6, pLabelName );
            PARTITION_STYLE style = (PARTITION_STYLE)pDriveInformation->PartitionStyle;
            PrintNormalA ( "%*s : %s\n", MaxNameLen + 6, "Partition Style", PartitionStyleToString ( style ) );
            PrintNormalA ( "%*s : %lu\n", MaxNameLen + 6, "Partition Count", pDriveInformation->PartitionCount );
            switch ( style )
            {
                case PARTITION_STYLE_MBR :
                {
                    PrintNormalA ( "%*s : 0x%08lx\n", MaxNameLen + 6, "Signature", pDriveInformation->Mbr.Signature );
                    break;
                }
                case PARTITION_STYLE_GPT :
                {
                    PrintNormalW ( L"%*s : %s\n", MaxNameLen + 6, L"Disk Id", GuidToString ( &pDriveInformation->Gpt.DiskId ) );
                    PrintNormalA ( "%*s : %lu\n", MaxNameLen + 6, "Max Partition Count", pDriveInformation->Gpt.MaxPartitionCount );
                    PrintNormalA ( "%*s : %s\n", MaxNameLen + 6, "Starting Usable Offset", SeparatedNumberL ( pDriveInformation->Gpt.StartingUsableOffset ) );
                    PrintNormalA ( "%*s : %s (%s)\n", MaxNameLen + 6, "Usable Length", SeparatedNumberL ( pDriveInformation->Gpt.UsableLength ), HumanReadableL ( pDriveInformation->Gpt.UsableLength ) );
                    break;
                }
            }

            //
            for ( size_t i = 0; i <  pDriveInformation->PartitionCount; i++ )
            {
                if ( pDriveInformation->PartitionEntry [ i ].PartitionNumber >= 0 )
                {
                    PrintNormalA ( "%*s : %lu\n", MaxNameLen + 6, "#### Partition Number", pDriveInformation->PartitionEntry [ i ].PartitionNumber );
                    style = (PARTITION_STYLE) pDriveInformation->PartitionEntry [ i ].PartitionStyle;
                    PrintNormalA ( "%*s : %s\n", MaxNameLen + 6, "Partition Number", PartitionStyleToString( style ) );
                    PrintNormalA ( "%*s : %s\n", MaxNameLen + 6, "Starting Offset", SeparatedNumberL ( pDriveInformation->PartitionEntry [ i ].StartingOffset ) );
                    PrintNormalA ( "%*s : %s (%s)\n", MaxNameLen + 6, "Partition Length", SeparatedNumberL ( pDriveInformation->PartitionEntry [ i ].PartitionLength ), HumanReadableL ( pDriveInformation->PartitionEntry [ i ].PartitionLength ) );
                    switch ( style )
                    {
                        case PARTITION_STYLE_MBR :
                        {
                            PrintNormalA ( "%*s : %u\n", MaxNameLen + 6, "Boot Indicator", pDriveInformation->PartitionEntry [ i ].Mbr.BootIndicator );
                            PrintNormalA ( "%*s : %s\n", MaxNameLen + 6, "Hidden Sectors", SeparatedNumberL ( pDriveInformation->PartitionEntry [ i ].Mbr.HiddenSectors ) );
                            PrintNormalA ( "%*s : %s\n", MaxNameLen + 6, "Partition Type", PartitionTypeToString ( pDriveInformation->PartitionEntry [ i ].Mbr.PartitionType ) );
                            PrintNormalA ( "%*s : %u\n", MaxNameLen + 6, "Recognized Partition", pDriveInformation->PartitionEntry [ i ].Mbr.RecognizedPartition );
                            break;
                        }
                        case PARTITION_STYLE_GPT :
                        {
                            PrintNormalW ( L"%*s : %s\n", MaxNameLen + 6, L"Name", pDriveInformation->PartitionEntry [ i ].Gpt.Name );
                            PrintNormalW ( L"%*s : %s\n", MaxNameLen + 6, L"Partition Id", GuidToString ( & pDriveInformation->PartitionEntry [ i ].Gpt.PartitionId ) );
                            PrintNormalW ( L"%*s : %s\n", MaxNameLen + 6, L"Partition Type", GuidToString ( & pDriveInformation->PartitionEntry [ i ].Gpt.PartitionType ) );
                            break;
                        }
                    }
                }
            }
        }
    }

    //  Error
    else if ( ShowError )
    {
        PrintStderrW ( L"Error - DeviceIoControl IOCTL_DISK_GET_DRIVE_LAYOUT_EX Fails - Cause : 0x%lx - %s\n", GetLastError(), GetLastErrorText() );
    }
    else if ( ! NoHeader && ShowDrive )
    {
        PrintNormalW ( L"%*s : %s\n", MaxNameLen + 6, pLabelName, GetLastErrorText() );
    }

    return bRet;
}

//
///////////////////////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////////////////////
BOOL GetMediaTypesEx ( HANDLE hDevice )
{
    DWORD       dwRet;
    BOOL        bRet;

    const WCHAR *pLabelName = L"--- Media Types";

    //
    ZeroMemory ( GetMediaTypes, sizeof(GetMediaTypes) );
    
    //
    bRet =
        DeviceIoControl(hDevice, IOCTL_STORAGE_GET_MEDIA_TYPES_EX,
            NULL, 0,
            &GetMediaTypes, sizeof(GetMediaTypes),
            &dwRet, NULL);

    //
    //  Success
    if ( bRet )
    {
        if ( ! NoHeader && ShowMedia )
        {
            PrintNormalW ( L"%*s : ---------------\n", MaxNameLen + 6, pLabelName );
            PrintNormalW ( L"%*s : %s\n", MaxNameLen + 6, L"Device Type", DeviceTypeToString ( pGetMediaTypes->DeviceType ) );
            PrintNormalW ( L"%*s : %lu\n", MaxNameLen + 6, L"Media Info Count", pGetMediaTypes->MediaInfoCount );
            if ( pGetMediaTypes->DeviceType == FILE_DEVICE_DISK )
            {
                for ( unsigned i = 0; i < pGetMediaTypes->MediaInfoCount; i++ )
                {
                    PrintNormalW ( L"%*s : %s\n", MaxNameLen + 6, L"Media Type", MediaTypeToString ( pGetMediaTypes->MediaInfo [ i ].DeviceSpecific.DiskInfo.MediaType ) );
                    PrintNormalA ( "%*s : %s\n", MaxNameLen + 6, "Cylinders", SeparatedNumberL ( pGetMediaTypes->MediaInfo [ i ].DeviceSpecific.DiskInfo.Cylinders ) );
                    PrintNormalW ( L"%*s : %lu\n", MaxNameLen + 6, L"Tracks Per Cylinder", pGetMediaTypes->MediaInfo [ i ].DeviceSpecific.DiskInfo.TracksPerCylinder );
                    PrintNormalW ( L"%*s : %lu\n", MaxNameLen + 6, L"Sectors Per Track", pGetMediaTypes->MediaInfo [ i ].DeviceSpecific.DiskInfo.SectorsPerTrack );
                    PrintNormalW ( L"%*s : %lu\n", MaxNameLen + 6, L"Bytes Per Sector", pGetMediaTypes->MediaInfo [ i ].DeviceSpecific.DiskInfo.BytesPerSector );
                    PrintNormalW ( L"%*s : %lu\n", MaxNameLen + 6, L"Number Media Sides", pGetMediaTypes->MediaInfo [ i ].DeviceSpecific.DiskInfo.NumberMediaSides );
                    //  MEDIA_READ_WRITE MEDIA_CURRENTLY_MOUNTED
                    PrintNormalW ( L"%*s : %04lx\n", MaxNameLen + 6, L"Media Characteristics", pGetMediaTypes->MediaInfo [ i ].DeviceSpecific.DiskInfo.MediaCharacteristics );
                }
            }
        }
    }

    //  Error
    else if ( ShowError )
    {
        PrintStderrW ( L"Error - DeviceIoControl IOCTL_STORAGE_GET_MEDIA_TYPES_EX Fails - Cause : 0x%lx - %s\n", GetLastError(), GetLastErrorText() );
    }
    else if ( ! NoHeader && ShowMedia )
    {
        PrintNormalW ( L"%*s : %s\n", MaxNameLen + 6, pLabelName, GetLastErrorText() );
    }

    return bRet;
}

//
///////////////////////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////////////////////
BOOL SmartGetVersion ( HANDLE hDevice )
{
    BOOL bRet                           = FALSE;
    DWORD dwRet                         = 0;

    ZeroMemory ( &DriveInfo.m_stGetVersionInParams, sizeof(DriveInfo.m_stGetVersionInParams) );

    bRet =
        DeviceIoControl (
            hDevice, SMART_GET_VERSION,
            NULL, 0,
            &DriveInfo.m_stGetVersionInParams, sizeof(GETVERSIONINPARAMS),
            &dwRet, NULL );
    if ( ! bRet && ShowError )
    {
        PrintStderrW ( L"Error - DeviceIoControl SMART_GET_VERSION Fails - Cause : 0x%lx - %s\n", GetLastError(), GetLastErrorText() );
    }

    return bRet;
}

//
///////////////////////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////////////////////
BOOL ReadSMARTInfo ( WCHAR *pName, BYTE bDeviceNumber )
{
    HANDLE hDevice                      = NULL;
    BOOL bRet                           = FALSE;
    DWORD dwRet                         = 0;
    
    if ( pName != NULL )
    {
        if ( *pName == '\\' )
        {
            wsprintf ( DeviceName, L"%s", pName );
        }
        else
        {
            wsprintf ( DeviceName, L"\\\\.\\%s", pName );
        }
    }
    else
    {
        if ( PhysicalDisk )
        {
            wsprintf ( DeviceName, L"\\\\.\\PHYSICALDRIVE%u", bDeviceNumber );
        }
        else if ( ScsiDisk )
        {
            wsprintf ( DeviceName, L"\\\\.\\SCSI%u:", bDeviceNumber );
        }
    }

    //
    DWORD dwDesiredAccess   = GENERIC_READ | GENERIC_WRITE;
    DWORD dwShareMode       = FILE_SHARE_READ | FILE_SHARE_WRITE;

    //
    hDevice = CreateFile ( DeviceName, dwDesiredAccess, dwShareMode, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_SYSTEM, NULL );
    if( hDevice != INVALID_HANDLE_VALUE )
    {
        PrintNormalW ( L"%*s : %s\n", MaxNameLen + 6, L"Device Name", DeviceName );

        bRet = GetDeviceNumber ( hDevice );
        if ( bRet )
        {
            bDeviceNumber = ( BYTE ) StorageDeviceNumber.DeviceNumber;
        }

        //
        bRet = GetMediaTypesEx ( hDevice );

        //
        bRet = GetStorage ( hDevice );

        //
        bRet = GetDiskPartition ( hDevice );

        //
        bRet = GetDiskGeometry ( hDevice );

        //
        bRet = GetScsiAddress ( hDevice );

        //
        bRet = GetScsiGetGetCapabilities ( hDevice );

        //
        if ( pStorageDeviceDescriptor->BusType == BusTypeAta || pStorageDeviceDescriptor->BusType == BusTypeSata )
        {
            //
            bRet = GetScsiGetInquiryData ( hDevice );

            //
            bRet = GetStoragePredictFailure ( hDevice );

            //
            bRet = SmartGetVersion ( hDevice );

            //
            if ( bRet )
            {           
                if ( ( DriveInfo.m_stGetVersionInParams.fCapabilities & CAP_SMART_CMD ) == CAP_SMART_CMD )
                {
                    if ( IsSmartEnabled ( hDevice ) )
                    {
                        //
                        //  Collect Drive Info
                        if ( ! AtaMode )
                        {
                            bRet = SmartIdentify ( hDevice );
                        }
                        else
                        {
                            bRet = AtaIdentify ( hDevice );
                        }

                        //
                        //  Read Smart Attributes
                        bRet = ReadSMARTAttributes ( hDevice );
                    }
                    else if ( ShowError )
                    {
                        PrintStderrW ( L"Error - Smart Is Disabled\n" );
                    }
                }
                else if ( ShowError )
                {
                    PrintStderrW ( L"Error - Smart Command not Available\n" );
                }
            }
        }
        else if ( pStorageDeviceDescriptor->BusType == BusTypeUsb )
        {
            bRet = ScsiPassThruIdentify ( hDevice, CMD_TYPE_SAT, 0xA0 /* ScsiAddress.TargetId */ );
            if ( ! bRet )
            {
                // bRet = ScsiPassThruIdentify ( hDevice, CMD_TYPE_SAT, 0xB0 /* ScsiAddress.TargetId */ );
            }
            bRet = ScsiPassThruAttributes ( hDevice, CMD_TYPE_SAT, ScsiAddress.TargetId, TRUE );
        }
        else if ( ShowError )
        {
            PrintStderrW ( L"Error - Bus Type is not treated\n" );
        }

        //
        CloseHandle(hDevice);
    }
    else if ( ShowError )
    {
        PrintStderrW ( L"Error - CreateFile %s Fails - Cause : 0x%lx - %s\n", DeviceName, GetLastError(), GetLastErrorText() );
        return FALSE;
    }
    else
    {
        PrintNormalW ( L"%*s : %s : %s\n", MaxNameLen + 6, L"Device Name", DeviceName, GetLastErrorText() );
        return FALSE;
    }

    return bRet;
}


//
//====================================================================================
//  Print Help
//====================================================================================
void ShowFlagInfo ( int iArgCount, WCHAR* pArgValues[] )
{
    int iWidth = 30;

    PrintHelpLine ( iWidth, PROGRAM_NAME_P, PROGRAM_DATE_F, PROGRAM_VERSION );

    PrintHelpLine ( );
    PrintHelpLine ( iWidth, L"", L"Bit 0 - W = Warranty" );
    PrintHelpLine ( iWidth, L"", L"Bit 1 - O = Offline" );
    PrintHelpLine ( iWidth, L"", L"Bit 2 - P = Performance" );
    PrintHelpLine ( iWidth, L"", L"Bit 3 - R = Error rate" );

    PrintHelpLine ( iWidth, L"", L"Bit 4 - C = Event count" );
    PrintHelpLine ( iWidth, L"", L"Bit 5 - S = Self-preservation" );

}

//
//====================================================================================
//  Print Help
//====================================================================================
void PrintVersion ( int iArgCount, WCHAR* pArgValues[] )
{
    int iWidth = 30;
    PrintHelpLine ( iWidth, PROGRAM_NAME_P, PROGRAM_DATE_F, PROGRAM_VERSION );
    const WCHAR *pVersion = OSVersion ( );
    PrintHelpLine ( iWidth, L"OS Version", pVersion );
}

//
///////////////////////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////////////////////
void AddToTable ( WCHAR *pName )
{
    for ( int i = 0; i < sizeof(TableDiskDriveAll) / sizeof ( TableDiskDriveStruct ); i++ )
    {
        if ( TableDiskDriveAll [ i ].name == NULL )
        {
            return;
        }

        //
        BOOL bAlready = FALSE;
        for ( int j = 0; j < iTableDiskDriveUsedCount; j++ )
        {
            if ( _wcsicmp ( TableDiskDriveUsed [ j ].name, TableDiskDriveAll [ i ].name ) == 0 )
            {
                bAlready = TRUE;
            }
        }

        //
        if ( ! bAlready )
        {
            if ( _wcsicmp ( TableDiskDriveAll [ i ].name, pName ) == 0 || PathMatchSpec ( TableDiskDriveAll [ i ].name, pName ) )
            {
                TableDiskDriveUsed [ iTableDiskDriveUsedCount ].name        = TableDiskDriveAll [ i ].name;
                TableDiskDriveUsed [ iTableDiskDriveUsedCount ].width       = TableDiskDriveAll [ i ].width;
                TableDiskDriveUsed [ iTableDiskDriveUsedCount ].numeric     = TableDiskDriveAll [ i ].numeric;
                iTableDiskDriveUsedCount++;
            }
        }
    }
}

//
//====================================================================================
//  Print Help
//====================================================================================
void PrintHelp ( int iArgCount, WCHAR* pArgValues[], bool bLong )
{
    int iWidth = 30;

    PrintHelpLine ( iWidth, PROGRAM_NAME_P, PROGRAM_DATE_F, PROGRAM_VERSION );

    PrintHelpLine ( );
    PrintHelpLine ( iWidth, L"Usage", PROGRAM_NAME, L"[options] [deviceNumber or name...]" );
    PrintHelpLine ( );

    PrintHelpLine ( iWidth, L"-h, -?, -help", L"Print Help" );
    PrintHelpLine ( iWidth, L"-hl, -helplong", L"Print Long Help" );
    PrintHelpLine ( iWidth, L"-version, -v", L"Print Version" );
    PrintHelpLine ( );

    //
    PrintHelpLine ( iWidth, L"-flag", L"Show Flag Info" );

    //
    PrintHelpLine ( );
    PrintHelpLine ( iWidth, L"-low, -lo", L"List Low and Above" );
    PrintHelpLine ( iWidth, L"-high, -hi", L"List High and Above" );
    PrintHelpLine ( iWidth, L"-critical, -c", L"List Critical only" );
    PrintHelpLine ( iWidth, L"-name {name}", L"Device Name" );
    PrintHelpLine ( iWidth, L"-device {n}", L"Device Number or Name" );
    PrintHelpLine ( iWidth, L"-smart", L"Show Other Smart Information" );

    PrintHelpLine ( iWidth, L"-fulldetail, -fu", L"Display Full Detail" );
    PrintHelpLine ( iWidth, L"-physical", L"Physical Disk" );
    PrintHelpLine ( iWidth, L"-scsi", L"Scsi Disk" );
    PrintHelpLine ( iWidth, L"-atamode, -ata", L"Ata Mode" );

    PrintHelpLine ( iWidth, L"-noheader, -nh", L"Display No Header" );
    PrintHelpLine ( iWidth, L"-nodetail, -nd", L"Display No Detail" );
    PrintHelpLine ( iWidth, L"-nonzero, -nz", L"Non Zero values only" );
    PrintHelpLine ( iWidth, L"-noerror, -ne", L"Dont Display Errors" );
    PrintHelpLine ( iWidth, L"-showerror, -se", L"Show Errors" );

    PrintHelpLine ( iWidth, L"-showall", L"Show All Information" );

    PrintHelpLine ( iWidth, L"-showdevice, -showdev", L"Show Device Information" );
    PrintHelpLine ( iWidth, L"-showmedia, -showmed", L"Show Media Information" );
    PrintHelpLine ( iWidth, L"-showdrive, -showdrv", L"Show Drive Information" );
    PrintHelpLine ( iWidth, L"-showstorage, -showsto", L"Show Storage Information" );
    PrintHelpLine ( iWidth, L"-showgeometry, -showgeo", L"Show Geometry Information" );
    PrintHelpLine ( iWidth, L"-showscsi", L"Show Scsi Information" );
    PrintHelpLine ( iWidth, L"-showpredict", L"Show Predict Information" );

    PrintHelpLine ( iWidth, L"-noshowdevice, -noshowdev", L"No Show Device Information" );
    PrintHelpLine ( iWidth, L"-noshowmedia, -noshowmed", L"No Show Media Information" );
    PrintHelpLine ( iWidth, L"-noshowdrive, -noshowdrv", L"No Show Drive Information" );
    PrintHelpLine ( iWidth, L"-noshowstorage, -noshowsto", L"No Show Storage Information" );
    PrintHelpLine ( iWidth, L"-noshowgeometry, -noshowgeo", L"No Show Geometry Information" );
    PrintHelpLine ( iWidth, L"-noshowscsi", L"No Show Scsi Information" );
    PrintHelpLine ( iWidth, L"-noshowpredict", L"No Show Predict Information" );

    //
    PrintHelpLine ( );
    PrintHelpLine ( iWidth, L"-list, -l", L"List Devices using wmic" );
    PrintHelpLine ( iWidth, L"-listbrief, -lb", L"List Devices using wmic" );
    PrintHelpLine ( iWidth, L"-listfull, -lf", L"List Devices using wmic" );
    PrintHelpLine ( iWidth, L"-list32, -l32", L"List Devices using wmic c++" );
    PrintHelpLine ( iWidth, L"-field, -fld", L"Add Fields to -list32 (can be wildcarded)" );
    PrintHelpLine ( iWidth, L"-listfields, -lfld", L"List Available Fields for -field" );

    //
    PrintHelpLine ( );
    PrintHelpLine ( iWidth, L"-listalldevice, -lad", L"List All Devices" );

    PrintHelpLine ( iWidth, L"-listall, -la", L"List All Device Interfaces" );

    PrintHelpLine ( iWidth, L"-listcd, -lc", L"List CDRom Device Interfaces" );
    PrintHelpLine ( iWidth, L"-listdisk, -ld", L"List Disk Device Interfaces" );
    PrintHelpLine ( iWidth, L"-listhub, -lh", L"List Hub Device Interfaces" );
    PrintHelpLine ( iWidth, L"-listusb, -lu", L"List Usb Device Interfaces" );
    PrintHelpLine ( iWidth, L"-listvol, -lv", L"List Volume Device Interfaces" );
    PrintHelpLine ( iWidth, L"-listflop, -lf", L"List Floppy Device Interfaces" );
    PrintHelpLine ( iWidth, L"-listpart, -lpa", L"List Partition Device Interfaces" );
    PrintHelpLine ( iWidth, L"-listport, -lpo", L"List Port Device Interfaces" );

    PrintHelpLine ( iWidth, L"-properties, -prop", L"List Properties" );
    PrintHelpLine ( iWidth, L"-guid, -lg", L"List GUID" );

    PrintHelpLine ( );
    PrintHelpLine ( iWidth, L"-locale {locale}", L"Localize Output .1252, fr-fr, ..." );

    PrintHelpLine ( iWidth, L"-showstruct, -ss", L"Show Structure" );
    PrintHelpLine ( iWidth, L"-debug, -dbg", L"Show Debug Information" );
    PrintHelpLine ( iWidth, L"-dump, -d", L"Dump Attributes Buffer" );

    PrintHelpLine ( iWidth, L"-pause, -p", L"Wait for Enter" );

    PrintHelpLaunch ( iWidth, PROGRAM_NAME, iArgCount, pArgValues, bLong );

}

//
//====================================================================================
//  Treat Options
//====================================================================================
int TreatOptions(int iArgCount, WCHAR* pArgValues[])
{
    //
    for ( int iX = 1; iX < iArgCount; iX++ )
    {
        WCHAR *pCurrentArgument = pArgValues [ iX ];
        if ( *pCurrentArgument == L'/' || *pCurrentArgument == L'-' )
        {
            WCHAR *pCurrentOption = pCurrentArgument + 1;
            //
            //  Help
            if (    __wcsicmpL ( pCurrentOption, L"h", L"?", L"help", NULL ) ==  0 )
            {
                PrintHelp (iArgCount, pArgValues, false);
                exit ( 0 );
                return 0;
            }
            else if (   __wcsicmpL ( pCurrentOption, L"hl", L"helplong", NULL ) ==  0 )
            {
                PrintHelp (iArgCount, pArgValues, true);
                exit ( 0 );
                return 0;
            }
            //
            //  Help
            else if ( __wcsicmpL ( pCurrentOption, L"v", L"version", NULL ) ==  0 )
            {
                PrintVersion (iArgCount, pArgValues);
                exit ( 0 );
                return 0;
            }
            //
            //  Brief
            else if ( __wcsicmpL ( pCurrentOption, L"low", L"lo", NULL ) == 0 )
            {
                Low = true;
            }
            //
            //  Critical
            else if ( __wcsicmpL ( pCurrentOption, L"critical", L"c", NULL ) == 0 )
            {
                Critical = true;
            }
            //
            //  Debug
            else if ( __wcsicmpL ( pCurrentOption, L"debug", L"dbg", NULL ) == 0 )
            {
                DebugMode = true;
            }

            //
            //  Full Detail
            else if ( __wcsicmpL ( pCurrentOption, L"fulldetail", L"fu", NULL ) == 0 )
            {
                FullDetail = true;
            }

            //
            //  Ata Mode
            else if ( __wcsicmpL ( pCurrentOption, L"atamode", L"ata", NULL ) == 0 )
            {
                AtaMode = true;
            }

            //
            //  Device
            else if ( __wcsicmpL ( pCurrentOption, L"device", NULL ) == 0 )
            {
                //
                if ( DevicesCount < DEVICE_MAX )
                {
                    //
                    iX++;
                    if ( iX < iArgCount )
                    {
                        //
                        if ( IsNumericSigned ( pArgValues [ iX ] ) )
                        {
                            DevicesNumber [ DevicesCount ]  = _wtoi ( pArgValues [ iX ] );
                        }
                        else
                        {
                            size_t  iLen                    =  wcslen ( pArgValues [ iX ] ) + 1;
                            size_t  iSizeInBytes            = ( iLen + 1 ) * sizeof(WCHAR);
                            DevicesName [ DevicesCount ]    = ( WCHAR* ) malloc ( iSizeInBytes  );
                            wcscpy_s ( DevicesName [ DevicesCount ], iLen, pArgValues [ iX ] );
                        }
                        DevicesCount++;
                    }
                    else
                    {
                        PrintStderrW ( L"Error : Invalid Device Number\n" );
                        exit ( 1 );
                        return 1;
                    }
                }
            }

            //
            //  Name
            else if ( __wcsicmpL ( pCurrentOption, L"name", NULL ) == 0 )
            {
                //
                if ( DevicesCount < DEVICE_MAX )
                {
                    //
                    iX++;
                    if ( iX < iArgCount )
                    {
                        if ( DevicesCount < DEVICE_MAX )
                        {
                            size_t  iLen            =  wcslen ( pArgValues [ iX ] ) + 1;
                            size_t  iSizeInBytes    = ( iLen + 1 ) * sizeof(WCHAR);
                            DevicesName [ DevicesCount ]    = ( WCHAR* ) malloc ( iSizeInBytes  );
                            wcscpy_s ( DevicesName [ DevicesCount ], iLen, pArgValues [ iX ] );
                            DevicesCount++;
                        }
                    }
                    else
                    {
                        PrintStderrW ( L"Error : Invalid Name\n" );
                        exit ( 1 );
                        return 1;
                    }
                }
            }

            //
            //  Dump
            else if ( __wcsicmpL ( pCurrentOption, L"dump", L"d", NULL ) == 0 )
            {
                Dump = true;
            }

            //
            //  Flags
            else if ( __wcsicmpL ( pCurrentOption, L"flag", NULL ) == 0 )
            {
                Flag = true;
            }

            //
            //  High
            else if ( __wcsicmpL ( pCurrentOption, L"high", L"hi", NULL ) == 0 )
            {
                High = true;
            }

            //
            //  List
            else if ( __wcsicmpL ( pCurrentOption, L"list", L"l", NULL ) == 0 )
            {
                ListDevice = true;
            }

            //
            //  List
            else if ( __wcsicmpL ( pCurrentOption, L"listbrief", L"lb", NULL ) == 0 )
            {
                ListDeviceBrief = true;
            }

            //
            //  List
            else if ( __wcsicmpL ( pCurrentOption, L"listfull", L"lf", NULL ) == 0 )
            {
                ListDeviceFull = true;
            }

            //
            //  List
            else if ( __wcsicmpL ( pCurrentOption, L"list32", L"l32", NULL ) == 0 )
            {
                ListDevice32 = true;
            }

            //
            //  List Available Fields
            else if ( __wcsicmpL ( pCurrentOption, L"listfields", L"lfld", NULL ) == 0 )
            {
                ListFields = true;
            }

            //
            //  List All Device
            else if ( __wcsicmpL ( pCurrentOption, L"listalldevice", L"lad", NULL ) == 0 )
            {
                ListAllDevice = true;
            }

            //
            //  List All Interface
            else if ( __wcsicmpL ( pCurrentOption, L"listall", L"la", NULL ) == 0 )
            {
                ListAllInt = true;
            }

            //
            //  List CD Interface
            else if ( __wcsicmpL ( pCurrentOption, L"listcd", L"lc", NULL ) == 0 )
            {
                ListCdRomInt    = true;
                ListSomeInt     = true;
            }

            //
            //  List Disk Interface
            else if ( __wcsicmpL ( pCurrentOption, L"listdisk", L"ld", NULL ) == 0 )
            {
                ListDiskInt     = true;
                ListSomeInt     = true;
            }

            //
            //  List Hub Interface
            else if ( __wcsicmpL ( pCurrentOption, L"listhub", L"lh", NULL ) == 0 )
            {
                ListHubInt      = true;
                ListSomeInt     = true;
            }

            //
            //  List Usb Interface
            else if ( __wcsicmpL ( pCurrentOption, L"listusb", L"lu", NULL ) == 0 )
            {
                ListUsbInt      = true;
                ListSomeInt     = true;
            }

            //
            //  List Volume Interface
            else if ( __wcsicmpL ( pCurrentOption, L"listvol", L"lv", NULL ) == 0 )
            {
                ListVolumeInt   = true;
                ListSomeInt     = true;
            }

            //
            //  List Floppy Interface
            else if ( __wcsicmpL ( pCurrentOption, L"listflop", L"lf", NULL ) == 0 )
            {
                ListFloppyInt   = true;
                ListSomeInt     = true;
            }

            //
            //  List Partition Interface
            else if ( __wcsicmpL ( pCurrentOption, L"listpart", L"lpa", NULL ) == 0 )
            {
                ListPartInt     = true;
                ListSomeInt     = true;
            }

            //
            //  List Port Interface
            else if ( __wcsicmpL ( pCurrentOption, L"listport", L"lpo", NULL ) == 0 )
            {
                ListPortInt     = true;
                ListSomeInt     = true;
            }

            //
            //  List Properties
            else if ( __wcsicmpL ( pCurrentOption, L"properties", L"prop", NULL ) == 0 )
            {
                ListProperties = true;
            }

            //
            //  List GUID
            else if ( __wcsicmpL ( pCurrentOption, L"guid", L"lg", NULL ) == 0 )
            {
                ListGUID = true;
            }

            //
            //  Locale
            else if ( __wcsicmpL ( pCurrentOption, L"locale", NULL ) == 0 )
            {
                //
                iX++;
                if ( iX < iArgCount )
                {
                    wcscpy_s ( LocaleString, _wsizeof(LocaleString), pArgValues [ iX ] );

                    CreateLocalePointer ( LocaleString );

                    Locale = true;
                }
                else
                {
                    PrintStderrW ( L"Error : Invalid Locale\n" );
                    exit ( 1 );
                    return 1;
                }

            }

            //
            //  Field
            else if ( __wcsicmpL ( pCurrentOption, L"field", L"fld", NULL ) == 0 )
            {
                //
                AddField = true;

                //
                iX++;
                if ( iX < iArgCount )
                {
                    AddToTable ( pArgValues [ iX ] );
                }
                else
                {
                    PrintStderrW ( L"Error : Invalid field\n" );
                    exit ( 1 );
                    return 1;
                }

            }

            //
            //  No Detail
            else if ( __wcsicmpL ( pCurrentOption, L"nodetail", L"nd", NULL ) == 0 )
            {
                NoDetail = true;
            }

            //
            //  No Header
            else if ( __wcsicmpL ( pCurrentOption, L"noheader", L"nh", NULL ) == 0 )
            {
                NoHeader = true;
            }

            //
            //  Non Zero
            else if ( __wcsicmpL ( pCurrentOption, L"nonzero", L"nz", NULL ) == 0 )
            {
                NonZero = true;
            }

            //
            //  No Error
            else if ( __wcsicmpL ( pCurrentOption, L"noerror", L"ne", NULL ) == 0 )
            {
                ShowError = false;
            }

            //
            //  Show Error
            else if ( __wcsicmpL ( pCurrentOption, L"showerror", L"se", NULL ) == 0 )
            {
                ShowError = true;
            }

            //
            //  Show Structure
            else if ( __wcsicmpL ( pCurrentOption, L"showstruct", L"ss", NULL ) == 0 )
            {
                ShowStruct = true;
            }

            //
            //  Show All
            else if ( __wcsicmpL ( pCurrentOption, L"showall", NULL ) == 0 )
            {
                ShowDevice      = true;
                ShowMedia       = true;
                ShowDrive       = true;
                ShowGeometry    = true;
                ShowStorage     = true;
                ShowScsi        = true;
                ShowPredict     = true;
            }

            //  Show Some
            else if ( __wcsicmpL ( pCurrentOption, L"showdevice", L"showdev", NULL ) == 0 )
            {
                ShowDevice      = true;
            }
            else if ( __wcsicmpL ( pCurrentOption, L"showmedia", L"showmed", NULL ) == 0 )
            {
                ShowMedia       = true;
            }
            else if ( __wcsicmpL ( pCurrentOption, L"showdrive", L"showdrv", NULL ) == 0 )
            {
                ShowDrive       = true;
            }
            else if ( __wcsicmpL ( pCurrentOption, L"showgeometry", L"showgeo", NULL ) == 0 )
            {
                ShowGeometry    = true;
            }
            else if ( __wcsicmpL ( pCurrentOption, L"showstorage", L"showsto", NULL ) == 0 )
            {
                ShowStorage = true;
            }
            else if ( __wcsicmpL ( pCurrentOption, L"showscsi", NULL ) == 0 )
            {
                ShowScsi    = true;
            }
            else if ( __wcsicmpL ( pCurrentOption, L"showpredict", NULL ) == 0 )
            {
                ShowPredict = true;
            }

            //  No Show Some
            else if ( __wcsicmpL ( pCurrentOption, L"noshowdevice", L"noshowdev", NULL ) == 0 )
            {
                ShowDevice      = false;
            }
            else if ( __wcsicmpL ( pCurrentOption, L"noshowmedia", L"noshowmed", NULL ) == 0 )
            {
                ShowMedia       = false;
            }
            else if ( __wcsicmpL ( pCurrentOption, L"noshowdrive", L"noshowdrv", NULL ) == 0 )
            {
                ShowDrive       = false;
            }
            else if ( __wcsicmpL ( pCurrentOption, L"noshowgeometry", L"noshowgeo", NULL ) == 0 )
            {
                ShowGeometry    = false;
            }
            else if ( __wcsicmpL ( pCurrentOption, L"noshowstorage", L"noshowsto", NULL ) == 0 )
            {
                ShowStorage     = false;
            }
            else if ( __wcsicmpL ( pCurrentOption, L"noshowscsi", NULL ) == 0 )
            {
                ShowScsi        = false;
            }
            else if ( __wcsicmpL ( pCurrentOption, L"noshowpredict", NULL ) == 0 )
            {
                ShowPredict     = false;
            }

            //
            //  Physical
            else if ( __wcsicmpL ( pCurrentOption, L"physical", NULL ) == 0 )
            {
                PhysicalDisk    = true;
                ScsiDisk        = false;
            }

            //
            //  Scsi
            else if ( __wcsicmpL ( pCurrentOption, L"scsi", NULL ) == 0 )
            {
                ScsiDisk        = true;
                PhysicalDisk    = false;
            }

            //
            //  Smart
            else if ( __wcsicmpL ( pCurrentOption, L"smart", NULL ) == 0 )
            {
                Smart = true;
            }

            //
            //  Pause
            else if ( __wcsicmpL ( pCurrentOption, L"pause", L"p", NULL ) == 0 )
            {
                Pause = true;
            }
            else if ( TreatLaunchOptions ( iArgCount, pArgValues, pCurrentOption, iX ) )
            {
                //
            }
            else
            {
                PrintStderrW ( L"Error : Invalid Parameter '%s'\n", pCurrentArgument );
                exit ( 1 );
                return 1;
            }
        }
        else
        {
            if ( AddField )
            {
                AddToTable ( pCurrentArgument );
            }
            else
            {
                if ( DevicesCount < DEVICE_MAX )
                {
                    if ( IsNumericSigned ( pCurrentArgument ) )
                    {
                        DevicesNumber [ DevicesCount ]  = _wtoi ( pCurrentArgument );
                    }
                    else
                    {
                        size_t  iLen                    =  wcslen ( pCurrentArgument ) + 1;
                        size_t  iSizeInBytes            = ( iLen + 1 ) * sizeof(WCHAR);
                        DevicesName [ DevicesCount ]    = ( WCHAR* ) malloc ( iSizeInBytes  );
                        wcscpy_s ( DevicesName [ DevicesCount ], iLen, pCurrentArgument );
                    }
                    DevicesCount++;
                }
            }
        }
    }
        
    return 0;
}

//
///////////////////////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////////////////////
void ShowDeviceList ()
{
    // system ( "wmic diskdrive list brief" );
    //system ( "wmic diskdrive get deviceid, model, SerialNumber, size, InterfaceType" );
    if ( ListDeviceBrief )
    {
        system ( "wmic diskdrive list brief" );
    }
    else if ( ListDeviceFull )
    {
        system ( "wmic diskdrive list" );
    }
    else
    {
        system ( "wmic diskdrive get deviceid, model, InterfaceType, pnpdeviceid, SCSITargetId" );
    }
}

//
///////////////////////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////////////////////
void ShowDeviceProperties ( HDEVINFO deviceInfoList, PSP_DEVINFO_DATA pDevInfoData )
{
    BOOL bRet           = TRUE;

    if ( ! ListProperties )
    {
        return;
    }

    //
    bRet =
        SetupDiGetDeviceRegistryProperty (
            deviceInfoList, pDevInfoData, SPDRP_FRIENDLYNAME, NULL, (PBYTE) TextInfo, _wsizeof(TextInfo), NULL);
    if ( bRet )
    {
        PrintNormalW ( L"\t\t%s=%s\n", L"Friendly Name", TextInfo );
    }
    else if ( ShowError )
    {
        PrintStderrW ( L"Error - SetupDiGetDeviceRegistryProperty SPDRP_FRIENDLYNAME - Cause : 0x%lx - %s\n", GetLastError(), GetLastErrorText() );
    }

    //
    bRet =
        SetupDiGetDeviceRegistryProperty (
            deviceInfoList, pDevInfoData, SPDRP_DEVICEDESC, NULL, (PBYTE) TextInfo, _wsizeof(TextInfo), NULL);
    if ( bRet )
    {
        PrintNormalW ( L"\t\t%s=%s\n", L"Description", TextInfo );
    }
    else if ( ShowError )
    {
        PrintStderrW ( L"Error - SetupDiGetDeviceRegistryProperty SPDRP_DEVICEDESC - Cause : 0x%lx - %s\n", GetLastError(), GetLastErrorText() );
    }

    //
    DWORD dwAddress = 0;
    bRet =
        SetupDiGetDeviceRegistryProperty (
            deviceInfoList, pDevInfoData, SPDRP_ADDRESS, NULL, (PBYTE) &dwAddress, sizeof(dwAddress), NULL);
    if ( bRet )
    {
        PrintNormalW ( L"\t\t%s=%lu\n", L"Address", dwAddress );
    }
    else if ( ShowError )
    {
        PrintStderrW ( L"Error - SetupDiGetDeviceRegistryProperty SPDRP_ADDRESS - Cause : 0x%lx - %s\n", GetLastError(), GetLastErrorText() );
    }

    //
    DWORD dwBusNumber = 0;
    bRet =
        SetupDiGetDeviceRegistryProperty (
            deviceInfoList, pDevInfoData, SPDRP_BUSNUMBER, NULL, (PBYTE) &dwBusNumber, sizeof(dwBusNumber), NULL);
    if ( bRet )
    {
        PrintNormalW ( L"\t\t%s=%lu\n", L"Bus Number", dwBusNumber );
    }
    else if ( ShowError )
    {
        PrintStderrW ( L"Error - SetupDiGetDeviceRegistryProperty SPDRP_BUSNUMBER - Cause : 0x%lx - %s\n", GetLastError(), GetLastErrorText() );
    }

    //
    GUID guid;
    bRet =
        SetupDiGetDeviceRegistryProperty (
            deviceInfoList, pDevInfoData, SPDRP_BUSTYPEGUID, NULL, (PBYTE) &guid, sizeof(guid), NULL);
    if ( bRet )
    {
        PrintNormalW ( L"\t\t%s=%s\n", L"Bus Type Guid", GuidToString(&guid) );
    }
    else if ( ShowError )
    {
        PrintStderrW ( L"Error - SetupDiGetDeviceRegistryProperty SPDRP_BUSTYPEGUID - Cause : 0x%lx - %s\n", GetLastError(), GetLastErrorText() );
    }
    
    //
    bRet =
        SetupDiGetDeviceRegistryProperty (
            deviceInfoList, pDevInfoData, SPDRP_CLASSGUID, NULL, (PBYTE) TextInfo, _wsizeof(TextInfo), NULL);
    if ( bRet )
    {
        PrintNormalW ( L"\t\t%s=%s\n", L"Class GUID", TextInfo );
    }
    else if ( ShowError )
    {
        PrintStderrW ( L"Error - SetupDiGetDeviceRegistryProperty SPDRP_CLASSGUID - Cause : 0x%lx - %s\n", GetLastError(), GetLastErrorText() );
    }
    
    //
    bRet =
        SetupDiGetDeviceRegistryProperty (
            deviceInfoList, pDevInfoData, SPDRP_COMPATIBLEIDS, NULL, (PBYTE) TextInfo, _wsizeof(TextInfo), NULL);
    if ( bRet )
    {
        PrintNormalW ( L"\t\t%s=%s\n", L"Compatible IDS", TextInfo );
    }
    else if ( ShowError )
    {
        PrintStderrW ( L"Error - SetupDiGetDeviceRegistryProperty SPDRP_COMPATIBLEIDS - Cause : 0x%lx - %s\n", GetLastError(), GetLastErrorText() );
    }
    
    //
    bRet =
        SetupDiGetDeviceRegistryProperty (
            deviceInfoList, pDevInfoData, SPDRP_ENUMERATOR_NAME, NULL, (PBYTE) TextInfo, _wsizeof(TextInfo), NULL);
    if ( bRet )
    {
        PrintNormalW ( L"\t\t%s=%s\n", L"Enumerator Name", TextInfo );
    }
    else if ( ShowError )
    {
        PrintStderrW ( L"Error - SetupDiGetDeviceRegistryProperty SPDRP_ENUMERATOR_NAME - Cause : 0x%lx - %s\n", GetLastError(), GetLastErrorText() );
    }
    
    //
    bRet =
        SetupDiGetDeviceRegistryProperty (
            deviceInfoList, pDevInfoData, SPDRP_HARDWAREID, NULL, (PBYTE) TextInfo, _wsizeof(TextInfo), NULL);
    if ( bRet )
    {
        PrintNormalW ( L"\t\t%s=%s\n", L"Hardware ID", TextInfo );
    }
    else if ( ShowError )
    {
        PrintStderrW ( L"Error - SetupDiGetDeviceRegistryProperty SPDRP_HARDWAREID - Cause : 0x%lx - %s\n", GetLastError(), GetLastErrorText() );
    }

    //
    bRet =
        SetupDiGetDeviceRegistryProperty (
            deviceInfoList, pDevInfoData, SPDRP_LOCATION_INFORMATION, NULL, (PBYTE) TextInfo, _wsizeof(TextInfo), NULL);
    if ( bRet )
    {
        PrintNormalW ( L"\t\t%s=%s\n", L"Location Information", TextInfo );
    }
    else if ( ShowError )
    {
        PrintStderrW ( L"Error - SetupDiGetDeviceRegistryProperty SPDRP_LOCATION_INFORMATION - Cause : 0x%lx - %s\n", GetLastError(), GetLastErrorText() );
    }

    //
    DWORD dwDeviceType;
    bRet =
        SetupDiGetDeviceRegistryProperty (
            deviceInfoList, pDevInfoData, SPDRP_DEVTYPE, NULL, (PBYTE) &dwDeviceType, sizeof(dwDeviceType), NULL);
    if ( bRet )
    {
        PrintNormalW ( L"\t\t%s=%s\n", L"Device Type", DeviceTypeToString ( dwDeviceType ) );
    }
    else if ( ShowError )
    {
        PrintStderrW ( L"Error - SetupDiGetDeviceRegistryProperty - Cause : 0x%lx - %s\n", GetLastError(), GetLastErrorText() );
    }

    //
    bRet =
        SetupDiGetDeviceRegistryProperty (
            deviceInfoList, pDevInfoData, SPDRP_LOCATION_PATHS, NULL, (PBYTE) TextInfo, _wsizeof(TextInfo), NULL);
    if ( bRet )
    {
        PrintNormalW ( L"\t\t%s=%s\n", L"Location Path", TextInfo );
    }
    else if ( ShowError )
    {
        PrintStderrW ( L"Error - SetupDiGetDeviceRegistryProperty SPDRP_LOCATION_PATHS - Cause : 0x%lx - %s\n", GetLastError(), GetLastErrorText() );
    }

    //
    bRet =
        SetupDiGetDeviceRegistryProperty (
            deviceInfoList, pDevInfoData, SPDRP_MFG, NULL, (PBYTE) TextInfo, _wsizeof(TextInfo), NULL);
    if ( bRet )
    {
        PrintNormalW ( L"\t\t%s=%s\n", L"Manufacturer", TextInfo );
    }
    else if ( ShowError )
    {
        PrintStderrW ( L"Error - SetupDiGetDeviceRegistryProperty SPDRP_MFG - Cause : 0x%lx - %s\n", GetLastError(), GetLastErrorText() );
    }

    //
    bRet =
        SetupDiGetDeviceRegistryProperty (
            deviceInfoList, pDevInfoData, SPDRP_PHYSICAL_DEVICE_OBJECT_NAME, NULL, (PBYTE) TextInfo, _wsizeof(TextInfo), NULL);
    if ( bRet )
    {
        PrintNormalW ( L"\t\t%s=%s\n", L"Physical Device Object Name", TextInfo );
    }
    else if ( ShowError )
    {
        PrintStderrW ( L"Error - SetupDiGetDeviceRegistryProperty SPDRP_PHYSICAL_DEVICE_OBJECT_NAME - Cause : 0x%lx - %s\n", GetLastError(), GetLastErrorText() );
    }

    //
    bRet =
        SetupDiGetDeviceRegistryProperty (
            deviceInfoList, pDevInfoData, SPDRP_SERVICE, NULL, (PBYTE) TextInfo, _wsizeof(TextInfo), NULL);
    if ( bRet )
    {
        PrintNormalW ( L"\t\t%s=%s\n", L"Service", TextInfo );
    }
    else if ( ShowError )
    {
        PrintStderrW ( L"Error - SetupDiGetDeviceRegistryProperty SPDRP_SERVICE - Cause : 0x%lx - %s\n", GetLastError(), GetLastErrorText() );
    }

    //
    bRet =
        SetupDiGetDeviceRegistryProperty (
            deviceInfoList, pDevInfoData, SPDRP_UI_NUMBER_DESC_FORMAT, NULL, (PBYTE) TextInfo, _wsizeof(TextInfo), NULL);
    if ( bRet )
    {
        PrintNormalW ( L"\t\t%s=%s\n", L"UI Number Desc Format", TextInfo );
    }
    else if ( ShowError )
    {
        PrintStderrW ( L"Error - SetupDiGetDeviceRegistryProperty SPDRP_UI_NUMBER_DESC_FORMAT - Cause : 0x%lx - %s\n", GetLastError(), GetLastErrorText() );
    }
}

//
///////////////////////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////////////////////
void ShowAllDeviceInterfaces ( PSP_DEVINFO_DATA pDevInfoData, GUID *pGuid, BOOL bShowHeader, WCHAR *pLabel )
{
    BOOL bRet           = TRUE;
    DWORD requiredSize  = 0;

    //
    HDEVINFO deviceInfoList;
    deviceInfoList = SetupDiGetClassDevs(NULL, NULL, NULL, DIGCF_DEVICEINTERFACE | DIGCF_ALLCLASSES | DIGCF_PRESENT);

    if ( deviceInfoList != INVALID_HANDLE_VALUE )
    {
        //
        SP_DEVICE_INTERFACE_DATA deviceInterfaceData;
        ZeroMemory ( &deviceInterfaceData, sizeof(deviceInterfaceData));
        deviceInterfaceData.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);

        if ( bShowHeader )
        {
            PrintNormalW ( L"Show All %s Interfaces [%s]\n", pLabel, GuidToString ( L"GUID", pGuid ) );
        }

        //
        BOOL bContinue = TRUE;
        for ( DWORD i = 0; bContinue; i++ )
        {
            bContinue = SetupDiEnumDeviceInterfaces ( deviceInfoList, pDevInfoData, pGuid, i, &deviceInterfaceData );
            if ( bContinue )
            {
                WCHAR deviceInterfaceDetailData [ MAX_PATH ];
                DWORD iSize = sizeof(deviceInterfaceDetailData);
                PSP_DEVICE_INTERFACE_DETAIL_DATA pDeviceInterfaceDetailData = (PSP_DEVICE_INTERFACE_DETAIL_DATA) deviceInterfaceDetailData;
                ZeroMemory ( deviceInterfaceDetailData, sizeof(deviceInterfaceDetailData) );
                pDeviceInterfaceDetailData->cbSize  = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);

                SP_DEVINFO_DATA devInfoData;
                ZeroMemory ( &devInfoData, sizeof(devInfoData));
                devInfoData.cbSize = sizeof(devInfoData);

                //
                bRet =
                    SetupDiGetDeviceInterfaceDetail (
                        deviceInfoList, &deviceInterfaceData, pDeviceInterfaceDetailData, iSize, &requiredSize, &devInfoData );
                if ( bRet )
                {
                    PrintNormalW ( L"# %4d : %s\n", i, pDeviceInterfaceDetailData->DevicePath );
                    if ( ListGUID )
                    {
                        PrintNormalW ( L"\t\t[%s]\n", GuidToString ( L"InterfaceClassGuid", & deviceInterfaceData.InterfaceClassGuid ) );
                        PrintNormalW ( L"\t\t[%s]\n", GuidToString ( L"ClassGuid", & devInfoData.ClassGuid ) );
                    }
                }
                else if ( ShowError )
                {
                    PrintStderrW ( L"Error - SetupDiGetDeviceInterfaceDetail - Cause : 0x%lx - %s\n", GetLastError(), GetLastErrorText() );
                }

                //
                ShowDeviceProperties ( deviceInfoList, &devInfoData );

            }
            else if ( ShowError )
            {
                DWORD dwError = GetLastError();
                if ( dwError != ERROR_NO_MORE_ITEMS )
                {
                    PrintStderrW ( L"Error - SetupDiEnumDeviceInterfaces - Cause : 0x%lx - %s\n", GetLastError(), GetLastErrorText() );
                }
            }
        }

        //
        SetupDiDestroyDeviceInfoList  ( deviceInfoList );
    }
    else if ( ShowError )
    {
        PrintStderrW ( L"Error - SetupDiGetClassDevs - Cause : 0x%lx - %s\n", GetLastError(), GetLastErrorText() );
    }
}

//
///////////////////////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////////////////////
void ShowAllDevices ()
{
    BOOL bRet           = TRUE;
    DWORD requiredSize  = 0;

    BOOL somethingPrinted = FALSE;

    //
    if ( ListAllDevice )
    {
        HDEVINFO deviceInfoList;
        deviceInfoList = SetupDiGetClassDevs(NULL, NULL, NULL, DIGCF_DEVICEINTERFACE | DIGCF_ALLCLASSES | DIGCF_PRESENT );

        PrintNormalW ( L"Show All Devices\n" );
        if ( deviceInfoList != INVALID_HANDLE_VALUE )
        {
            SP_DEVINFO_DATA deviceInfoData;
            ZeroMemory ( &deviceInfoData, sizeof(deviceInfoData) );
            deviceInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
            BOOL bContinue = TRUE;
            for ( DWORD i = 0; bContinue; i++ )
            {
                bContinue = SetupDiEnumDeviceInfo(deviceInfoList, i, &deviceInfoData);
                if ( bContinue )
                {
                    //
                    WCHAR buffer [ LEN_STRING ];
                    DWORD buffersize    = _wsizeof(buffer);
                    bRet =
                        SetupDiGetDeviceInstanceId (
                            deviceInfoList, &deviceInfoData, buffer, buffersize, &requiredSize );
                    if ( bRet )
                    {
                        PrintNormalW ( L"# %4d : %s\n", i, buffer );
                        if ( ListGUID )
                        {
                            PrintNormalW ( L"\t\t[%s]\n", GuidToString ( L"ClassGuid", & deviceInfoData.ClassGuid ) );
                        }
                    }
                    else if ( ShowError )
                    {
                        PrintStderrW ( L"Error - SetupDiGetDeviceInstanceId - Cause : 0x%lx - %s\n", GetLastError(), GetLastErrorText() );
                        continue;
                    }

                    //
                    ShowDeviceProperties ( deviceInfoList, &deviceInfoData );

                }
                else if ( ShowError )
                {
                    DWORD dwError = GetLastError();
                    if ( dwError != ERROR_NO_MORE_ITEMS )
                    {
                        PrintStderrW ( L"Error - SetupDiEnumDeviceInfo - Cause : 0x%lx - %s\n", GetLastError(), GetLastErrorText() );
                    }
                }
            }

            SetupDiDestroyDeviceInfoList  ( deviceInfoList );
        }
        else if ( ShowError )
        {
            PrintStderrW ( L"Error - SetupDiGetClassDevs - Cause : 0x%lx - %s\n", GetLastError(), GetLastErrorText() );
        }

        somethingPrinted = TRUE;
    }

    //
    if ( ListHubInt || ListAllInt )
    {
        //
        GUID guid = GUID_DEVINTERFACE_USB_HUB;
        if ( somethingPrinted )
        {
            PrintNormalW ( L"\n" );
        }
        ShowAllDeviceInterfaces ( NULL, &guid, TRUE, L"USB HUB" );
        somethingPrinted = TRUE;

        //
        guid = GUID_DEVINTERFACE_USB_HOST_CONTROLLER;
        if ( somethingPrinted )
        {
            PrintNormalW ( L"\n" );
        }
        ShowAllDeviceInterfaces ( NULL, &guid, TRUE, L"USB HUB Controller" );
        somethingPrinted = TRUE;
    }

    //
    if ( ListUsbInt || ListAllInt )
    {
        GUID guid = GUID_DEVINTERFACE_USB_DEVICE;
        if ( somethingPrinted )
        {
            PrintNormalW ( L"\n" );
        }
        ShowAllDeviceInterfaces ( NULL, &guid, TRUE, L"USB" );
        somethingPrinted = TRUE;
    }

    //
    if ( ListDiskInt || ListAllInt )
    {
        GUID guid = GUID_DEVINTERFACE_DISK;
        if ( somethingPrinted )
        {
            PrintNormalW ( L"\n" );
        }
        ShowAllDeviceInterfaces ( NULL, &guid, TRUE, L"Disk" );
        somethingPrinted = TRUE;
    }

    //
    if ( ListVolumeInt || ListAllInt )
    {
        GUID guid = GUID_DEVINTERFACE_VOLUME;
        if ( somethingPrinted )
        {
            PrintNormalW ( L"\n" );
        }
        ShowAllDeviceInterfaces ( NULL, &guid, TRUE, L"Volume" );
        somethingPrinted = TRUE;
    }

    //
    if ( ListPortInt || ListAllInt )
    {
        GUID guid = GUID_DEVINTERFACE_STORAGEPORT;
        if ( somethingPrinted )
        {
            PrintNormalW ( L"\n" );
        }
        ShowAllDeviceInterfaces ( NULL, &guid, TRUE, L"Port" );
        somethingPrinted = TRUE;
    }

    //
    if ( ListPartInt || ListAllInt )
    {
        GUID guid = GUID_DEVINTERFACE_PARTITION;
        if ( somethingPrinted )
        {
            PrintNormalW ( L"\n" );
        }
        ShowAllDeviceInterfaces ( NULL, &guid, TRUE, L"Partition" );
        somethingPrinted = TRUE;
    }

    //
    if ( ListFloppyInt || ListAllInt )
    {
        GUID guid = GUID_DEVINTERFACE_FLOPPY;
        if ( somethingPrinted )
        {
            PrintNormalW ( L"\n" );
        }
        ShowAllDeviceInterfaces ( NULL, &guid, TRUE, L"Floppy" );
        somethingPrinted = TRUE;
    }

    //
    if ( ListCdRomInt || ListAllInt )
    {
        GUID guid = GUID_DEVINTERFACE_CDROM;
        if ( somethingPrinted )
        {
            PrintNormalW ( L"\n" );
        }
        ShowAllDeviceInterfaces ( NULL, &guid, TRUE, L"CD ROM" );
        somethingPrinted = TRUE;
    }
    
}

//
///////////////////////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////////////////////
WCHAR *PrintOneItem ( VARTYPE varType, VARIANT_UNION value, int i, int depth )
{
    //
    int lenResult = 1024;
    int lenResultBytes = ( lenResult + 1 ) * sizeof(WCHAR);
    WCHAR *pResult = ( WCHAR * ) malloc ( lenResultBytes );
    ZeroMemory ( pResult, lenResultBytes );

    //
    if ( DebugMode )
    {
        swprintf_s ( pResult + wcslen(pResult), lenResult - wcslen(pResult), L"#%d:", depth );
    }

    switch ( varType )
    {
        case VT_EMPTY:
        {
            if ( DebugMode )
            {
                swprintf_s ( pResult + wcslen(pResult), lenResult - wcslen(pResult), L"e:" );
            }
            swprintf_s ( pResult + wcslen(pResult), lenResult - wcslen(pResult), L"%s", L"<EMPTY>" );
            break;
        }
        case VT_NULL:
        {
            if ( DebugMode )
            {
                swprintf_s ( pResult + wcslen(pResult), lenResult - wcslen(pResult), L"n:" );
            }
            // swprintf_s ( pResult + wcslen(pResult), lenResult - wcslen(pResult), L"%s", L"<NULL>" );
            swprintf_s ( pResult + wcslen(pResult), lenResult - wcslen(pResult), L"%s", L"-" );
            break;
        }
        case VT_BOOL:
        {
            if ( DebugMode )
            {
                swprintf_s ( pResult + wcslen(pResult), lenResult - wcslen(pResult), L"b:" );
            }
            swprintf_s ( pResult + wcslen(pResult), lenResult - wcslen(pResult), L"%d", value.cVal );
            break;
        }
        case VT_INT:
        {
            if ( DebugMode )
            {
                swprintf_s ( pResult + wcslen(pResult), lenResult - wcslen(pResult), L"d:" );
            }
            swprintf_s ( pResult + wcslen(pResult), lenResult - wcslen(pResult), L"%d", value.intVal );
            break;
        }
        case VT_I1:
        {
            if ( DebugMode )
            {
                swprintf_s ( pResult + wcslen(pResult), lenResult - wcslen(pResult), L"d:" );
            }
            swprintf_s ( pResult + wcslen(pResult), lenResult - wcslen(pResult), L"%d", value.cVal );
            break;
        }
        case VT_I2:
        {
            if ( DebugMode )
            {
                swprintf_s ( pResult + wcslen(pResult), lenResult - wcslen(pResult), L"d:" );
            }
            swprintf_s ( pResult + wcslen(pResult), lenResult - wcslen(pResult), L"%d", value.iVal );
            break;
        }
        case VT_I4:
        {
            if ( DebugMode )
            {
                swprintf_s ( pResult + wcslen(pResult), lenResult - wcslen(pResult), L"l:" );
            }
            swprintf_s ( pResult + wcslen(pResult), lenResult - wcslen(pResult), L"%ld", value.lVal );
            break;
        }
        case VT_I8:
        {
            if ( DebugMode )
            {
                swprintf_s ( pResult + wcslen(pResult), lenResult - wcslen(pResult), L"ll:" );
            }
            swprintf_s ( pResult + wcslen(pResult), lenResult - wcslen(pResult), L"%lld", value.llVal );
            break;
        }
        case VT_UINT:
        {
            if ( DebugMode )
            {
                swprintf_s ( pResult + wcslen(pResult), lenResult - wcslen(pResult), L"u:" );
            }
            swprintf_s ( pResult + wcslen(pResult), lenResult - wcslen(pResult), L"%u", value.uintVal );
            break;
        }
        case VT_UI1:
        {
            if ( DebugMode )
            {
                swprintf_s ( pResult + wcslen(pResult), lenResult - wcslen(pResult), L"u:" );
            }
            swprintf_s ( pResult + wcslen(pResult), lenResult - wcslen(pResult), L"%u", value.bVal );
            break;
        }
        case VT_UI2:
        {
            if ( DebugMode )
            {
                swprintf_s ( pResult + wcslen(pResult), lenResult - wcslen(pResult), L"u:" );
            }
            swprintf_s ( pResult + wcslen(pResult), lenResult - wcslen(pResult), L"%u", value.uiVal );
            break;
        }
        case VT_UI4:
        {
            swprintf_s ( pResult + wcslen(pResult), lenResult - wcslen(pResult), L"%lu", value.ulVal );
            break;
        }
        case VT_UI8:
        {
            if ( DebugMode )
            {
                swprintf_s ( pResult + wcslen(pResult), lenResult - wcslen(pResult), L"llu:" );
            }
            swprintf_s ( pResult + wcslen(pResult), lenResult - wcslen(pResult), L"%llu", value.ullVal );
            break;
        }
        case VT_R4:
        {
            if ( DebugMode )
            {
                swprintf_s ( pResult + wcslen(pResult), lenResult - wcslen(pResult), L"f:" );
            }
            swprintf_s ( pResult + wcslen(pResult), lenResult - wcslen(pResult), L"%f", value.fltVal );
            break;
        }
        case VT_R8:
        {
            if ( DebugMode )
            {
                swprintf_s ( pResult + wcslen(pResult), lenResult - wcslen(pResult), L"f:" );
            }
            swprintf_s ( pResult + wcslen(pResult), lenResult - wcslen(pResult), L"%f", value.dblVal );
            break;
        }
        case VT_BSTR:
        {
            if ( DebugMode )
            {
                swprintf_s ( pResult + wcslen(pResult), lenResult - wcslen(pResult), L"s:" );
            }
            swprintf_s ( pResult + wcslen(pResult), lenResult - wcslen(pResult), L"%s", value.bstrVal );
            break;
        }
        default :
        {
            WCHAR szSmall [ LEN_SHORT_STRING ];
            ZeroMemory ( szSmall, sizeof(szSmall) );
            if ( varType & VT_VECTOR )
            {
                swprintf ( szSmall, _wsizeof(szSmall), L"<VECTOR_0x%04x>", varType );
                swprintf_s ( pResult + wcslen(pResult), lenResult - wcslen(pResult), L"%s", szSmall );
            }
            else if ( varType & VT_ARRAY )
            {
#if 0
                swprintf ( szSmall, _wsizeof(szSmall), L"<ARRAY_0x%04x>", varType );
                swprintf_s ( pResult + wcslen(pResult), lenResult - wcslen(pResult), L"%s", szSmall );
#endif
                HRESULT hResult;

                SAFEARRAY* pSafeArray = (SAFEARRAY*) value.parray;

                LONG lStart, lEnd;
                hResult = SafeArrayGetLBound( pSafeArray , 1, &lStart );
                hResult = SafeArrayGetUBound( pSafeArray, 1, &lEnd );
#if 0
                swprintf_s ( pResult + wcslen(pResult), lenResult - wcslen(pResult), L"(%d,%d)", lStart, lEnd );
#endif
                swprintf_s ( pResult + wcslen(pResult), lenResult - wcslen(pResult), L"[" );
                for ( LONG iS = lStart; iS <= lEnd; iS++ )
                {
                    LONG iX = iS;

                    VARIANT_UNION varColumn;

                    hResult = SafeArrayGetElement ( pSafeArray, &iX, &varColumn );
                    if (SUCCEEDED(hResult))
                    {
                        WCHAR *pTemp = PrintOneItem ( varType & VT_TYPEMASK, varColumn, i, depth + 1 );
                        swprintf_s ( pResult + wcslen(pResult), lenResult - wcslen(pResult), L"%s", pTemp );
                        free ( pTemp );
                    }
                    else
                    {
                        swprintf_s ( pResult + wcslen(pResult), lenResult - wcslen(pResult), L"(fails)" );
                    }

                    if ( iS != lEnd )
                    {
                        swprintf_s ( pResult + wcslen(pResult), lenResult - wcslen(pResult), L"," );
                    }
                }
                swprintf_s ( pResult + wcslen(pResult), lenResult - wcslen(pResult), L"]" );
            }
            else if ( varType & VT_BYREF )
            {
                swprintf ( szSmall, _wsizeof(szSmall), L"<BYREF_0x%04x>", varType );
                swprintf_s ( pResult + wcslen(pResult), lenResult - wcslen(pResult), L"%s", szSmall );
            }
            else
            {
                swprintf ( szSmall, _wsizeof(szSmall), L"<UNKNOWN_0x%04x>", varType );
                swprintf_s ( pResult + wcslen(pResult), lenResult - wcslen(pResult), L"%s", szSmall );
            }
            break;
        }
    }

    return pResult;
}

//
///////////////////////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////////////////////
int ListDiskDriveWin32 ( )
{
    HRESULT hResult;

    // Step 1: --------------------------------------------------
    // Initialize COM. ------------------------------------------

    hResult =  CoInitializeEx(0, COINIT_MULTITHREADED);
    if (FAILED(hResult))
    {
        PrintStderrW ( L"Failed to initialize COM library. Error code = 0x%04x\n", hResult );
        return 1;                  // Program has failed.
    }

    // Step 2: --------------------------------------------------
    // Set general COM security levels --------------------------

    hResult =  CoInitializeSecurity(
        NULL,
        -1,                          // COM authentication
        NULL,                        // Authentication services
        NULL,                        // Reserved
        RPC_C_AUTHN_LEVEL_DEFAULT,   // Default authentication
        RPC_C_IMP_LEVEL_IMPERSONATE, // Default Impersonation
        NULL,                        // Authentication info
        EOAC_NONE,                   // Additional capabilities
        NULL                         // Reserved
        );


    if (FAILED(hResult))
    {
        PrintStderrW ( L"CoInitializeSecurity Error 0x%04x\n", hResult );
        CoUninitialize();
        return 1;                    // Program has failed.
    }

    // Step 3: ---------------------------------------------------
    // Obtain the initial locator to WMI -------------------------

    IWbemLocator *pLoc = NULL;

    hResult = CoCreateInstance(
        CLSID_WbemLocator,
        0,
        CLSCTX_INPROC_SERVER,
        IID_IWbemLocator, (LPVOID *) &pLoc);

    if (FAILED(hResult))
    {
        PrintStderrW ( L"CoCreateInstance Error 0x%04x\n", hResult );
        CoUninitialize();
        return 1;                 // Program has failed.
    }

    // Step 4: -----------------------------------------------------
    // Connect to WMI through the IWbemLocator::ConnectServer method

    IWbemServices *pSvc = NULL;

    // Connect to the root\cimv2 namespace with
    // the current user and obtain pointer pSvc
    // to make IWbemServices calls.
    hResult = pLoc->ConnectServer(
         _bstr_t(L"ROOT\\CIMV2"), // Object path of WMI namespace
         NULL,                    // User name. NULL = current user
         NULL,                    // User password. NULL = current
         0,                       // Locale. NULL indicates current
         NULL,                    // Security flags.
         0,                       // Authority (for example, Kerberos)
         0,                       // Context object
         &pSvc                    // pointer to IWbemServices proxy
         );

    if (FAILED(hResult))
    {
        PrintStderrW ( L"ConnectServer Error 0x%04x\n", hResult );
        pLoc->Release();
        CoUninitialize();
        return 1;                // Program has failed.
    }

    // Step 5: --------------------------------------------------
    // Set security levels on the proxy -------------------------

    hResult = CoSetProxyBlanket(
       pSvc,                        // Indicates the proxy to set
       RPC_C_AUTHN_WINNT,           // RPC_C_AUTHN_xxx
       RPC_C_AUTHZ_NONE,            // RPC_C_AUTHZ_xxx
       NULL,                        // Server principal name
       RPC_C_AUTHN_LEVEL_CALL,      // RPC_C_AUTHN_LEVEL_xxx
       RPC_C_IMP_LEVEL_IMPERSONATE, // RPC_C_IMP_LEVEL_xxx
       NULL,                        // client identity
       EOAC_NONE                    // proxy capabilities
    );

    if (FAILED(hResult))
    {
        PrintStderrW ( L"CoSetProxyBlanket Error 0x%04x\n", hResult );
        pSvc->Release();
        pLoc->Release();
        CoUninitialize();
        return 1;               // Program has failed.
    }

    //
    //  Compute Length
    for ( int i = 0; i <= iTableDiskDriveUsedCount; i++ )
    {
        if ( TableDiskDriveUsed [ i ].name != NULL )
        {
            TableDiskDriveUsed [ i ].width = wcslen(TableDiskDriveUsed [ i ].name);
        }
    }

    //
    //  Two steps: First to compute length. Second to Print
    for ( int step = 1; step <= 2; step ++ )
    {
        //  Search All DiskDrive
        IEnumWbemClassObject* pEnumerator = NULL;

        //
        hResult = pSvc->ExecQuery (
            bstr_t("WQL"),
            bstr_t("SELECT * FROM Win32_DiskDrive"),
            WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
            NULL,
            &pEnumerator );

        if (FAILED(hResult))
        {
            PrintStderrW ( L"ExecQuery Error 0x%04x\n", hResult );
            pSvc->Release();
            pLoc->Release();
            CoUninitialize();
            return 1;               // Program has failed.
        }

        //
        //  Correct sign
        if ( step == 2 )
        {
            for ( int i = 0; i <= iTableDiskDriveUsedCount; i++ )
            {
                if ( TableDiskDriveUsed [ i ].name != NULL )
                {
                    if ( ! TableDiskDriveUsed [ i ].numeric )
                    {
                        TableDiskDriveUsed [ i ].width = -1 * TableDiskDriveUsed [ i ].width;
                    }
                }
            }
        }

        //
        //  Print Header
        if ( step == 2 && ! NoHeader )
        {
            for ( int i = 0; i <= iTableDiskDriveUsedCount; i++ )
            {
                if ( TableDiskDriveUsed [ i ].name != NULL )
                {
                    PrintNormalW ( L" %*s", TableDiskDriveUsed [ i ].width, TableDiskDriveUsed [ i ].name );
                }
                else
                {
                    PrintNormalW ( L"\n" );
                }
            }
        }

        //
        while ( pEnumerator )
        {
            //
            IWbemClassObject *pclsObj = NULL;
            ULONG uReturn = 0;

            //
            HRESULT hr = pEnumerator->Next(WBEM_INFINITE, 1, &pclsObj, &uReturn) ;
            if( 0 == uReturn )
            {
                break;
            }

            for ( int i = 0; i <= iTableDiskDriveUsedCount; i++ )
            {
                if ( TableDiskDriveUsed [ i ].name != NULL )
                {
                    VARIANT vtProp;

                    // Get the value of the property
                    hr = pclsObj->Get(TableDiskDriveUsed [ i ].name, 0, &vtProp, 0, 0);
                    if ( hr == WBEM_S_NO_ERROR )
                    {
                        WCHAR *pTemp = PrintOneItem ( vtProp.vt, *(VARIANT_UNION *) &vtProp.pbVal, i, 0 );
                        if ( step == 1 )
                        {
                            if ( ! IsNumericSigned ( pTemp ) )
                            {
                                TableDiskDriveUsed [ i ].numeric = false;
                            }

                            if ( TableDiskDriveUsed [ i ].width < (int) wcslen(pTemp) )
                            {
                                TableDiskDriveUsed [ i ].width = wcslen(pTemp);
                            }
                        }
                        else
                        {
                            PrintNormalW ( L" %*s", TableDiskDriveUsed [ i ].width, pTemp );
                        }
                        free ( pTemp );
                    }
                    else
                    {
                        if ( step == 2 )
                        {
                            PrintNormalW ( L" %*s", TableDiskDriveUsed [ i ].width, L"N/A" );
                        }
                    }
                    VariantClear(&vtProp);
                }
                else
                {
                    if ( step == 2 )
                    {
                        PrintNormalW ( L"\n" );
                    }
                }
            }

            pclsObj->Release();
        }

        pEnumerator->Release();

    }

    // Cleanup
    // ========

    pSvc->Release();
    pLoc->Release();
    CoUninitialize();

    return 0;   // Program successfully completed.

}

//
///////////////////////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////////////////////
void ListAvailableFields ()
{
    for ( int i = 0; i < sizeof(TableDiskDriveAll) / sizeof ( TableDiskDriveStruct ); i++ )
    {
        if ( TableDiskDriveAll [ i ].name != NULL )
        {
            PrintNormalW ( L"%s\n", TableDiskDriveAll [ i ].name );
        }
    }
}

//
///////////////////////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////////////////////
int _tmain(int iArgCount, WCHAR* pArgValues[], WCHAR* envp[])
{
    //
    for ( int i = 0; i < iTableDiskDriveUsedMax; i++ )
    {
        TableDiskDriveUsed [ i ].name       = NULL;
        TableDiskDriveUsed [ i ].width      = 0;
        TableDiskDriveUsed [ i ].numeric    = true;
    }
    iTableDiskDriveUsedCount = 0;

    //
    int nRetCode = 0;

    //
    for ( int i = 0; i < DEVICE_MAX; i++ )
    {
        DevicesNumber [ i ]     = -1;
        DevicesName [ i ]       = NULL;
    }

    //
    MaxNameLen = FindAttributeNameMaxLen ( );

    //
    TreatOptions ( iArgCount, pArgValues );

    //
    BOOL IsAdmin = IsThisUserAdmin();
    if ( ! IsAdmin && RunAsAdminMode )
    {
        RunAsAdmin ( iArgCount, pArgValues, WaitProcessMode );

#ifndef _DEBUG
        exit ( 0 );
#endif
    }


    //
    if ( ShowStruct )
    {
        PrintNormalW ( L"%*s : %lu\n", MaxNameLen + 6, L"SENDCMDINPARAMS", sizeof(SENDCMDINPARAMS) );
        PrintNormalW ( L"%*s : %lu\n", MaxNameLen + 6, L"SENDCMDOUTPARAMS", sizeof(SENDCMDOUTPARAMS) );
        PrintNormalW ( L"%*s : %lu\n", MaxNameLen + 6, L"SCSI_PASS_THROUGH", sizeof(SCSI_PASS_THROUGH) );
        PrintNormalW ( L"%*s : %lu\n", MaxNameLen + 6, L"SCSI_PASS_THROUGH_DIRECT", sizeof(SCSI_PASS_THROUGH_DIRECT) );
        PrintNormalW ( L"%*s : %lu\n", MaxNameLen + 6, L"SCSI_ADAPTER_BUS_INFO", sizeof(SCSI_ADAPTER_BUS_INFO) );
        PrintNormalW ( L"%*s : %lu\n", MaxNameLen + 6, L"DEVICE_MEDIA_INFO", sizeof(DEVICE_MEDIA_INFO) );
        PrintNormalW ( L"%*s : %lu\n", MaxNameLen + 6, L"ATA_PASS_THROUGH_EX", sizeof(ATA_PASS_THROUGH_EX) );
        
        PrintNormalW ( L"%*s : 0x%llx\n", MaxNameLen + 6, L"ScsiPassThroughWithBuffer", &ScsiPassThroughWithBuffer );
        
        exit (0);
    }

    //
    if ( ListDevice || ListDeviceBrief || ListDeviceFull )
    {
        ShowDeviceList();
    }
    else if ( ListDevice32 )
    {
        if ( iTableDiskDriveUsedCount == 0 )
        {
            AddToTable ( L"DeviceID" );
            AddToTable ( L"Caption" );
            AddToTable ( L"Size" );
            AddToTable ( L"Partitions" );
            AddToTable ( L"InterfaceType" );
        }
        ListDiskDriveWin32 ();
    }
    else if ( ListFields )
    {
        ListAvailableFields ();
    }
    else if (   ListAllDevice || ListAllInt || ListSomeInt )
    {
        ShowAllDevices();
        exit (0);
    }
    else if ( Flag )
    {
        ShowFlagInfo ( iArgCount, pArgValues );
    }
    else
    {
        if ( DevicesCount > 0 )
        {
            for ( int i = 0; i < DevicesCount; i++ )
            {
                if ( i > 0 )
                {
                    PrintNormalW ( L"\n" );
                }

                if ( DebugMode )
                {
                    if ( DevicesName [ i ] != NULL )
                    {
                        PrintNormalW ( L"Treating %d - %u - %s\n", i, DevicesNumber [ i ], DevicesName [ i ] );
                    }
                    else
                    {
                        PrintNormalW ( L"Treating %d - %u\n", i, DevicesNumber [ i ] );
                    }
                }

                ReadSMARTInfo ( DevicesName [ i ], DevicesNumber [ i ] );
            }
        }
        else
        {
            ReadSMARTInfo ( NULL, 0 );
        }
    }

    //
    if ( Pause )
    {
        PrintNormalW ( L"Hit Enter to Exit " );
        WCHAR szEnter [ LEN_STRING ];
        _getws_s  ( szEnter, _wsizeof(szEnter) );
    }

    return 0;
}
