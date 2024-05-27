# Smartinfo
Provides Smart Info On Disks

````
       SmartInfo (x86 Unicode) : Mon May 27 15:02 2024 1.0.04.015

                         Usage : SmartInfo [options] [deviceNumber or name...]

                 -h, -?, -help : Print Help
                -hl, -helplong : Print Long Help
                  -version, -v : Print Version

                         -flag : Show Flag Info

                     -low, -lo : List Low and Above
                    -high, -hi : List High and Above
                 -critical, -c : List Critical only
                  -name {name} : Device Name
                   -device {n} : Device Number or Name
                        -smart : Show Other Smart Information
              -fulldetail, -fu : Display Full Detail
                     -physical : Physical Disk
                         -scsi : Scsi Disk
                -atamode, -ata : Ata Mode
                -noheader, -nh : Display No Header
                -nodetail, -nd : Display No Detail
                 -nonzero, -nz : Non Zero values only
                 -noerror, -ne : Dont Display Errors
               -showerror, -se : Show Errors
                      -showall : Show All Information
         -showdevice, -showdev : Show Device Information
          -showmedia, -showmed : Show Media Information
          -showdrive, -showdrv : Show Drive Information
        -showstorage, -showsto : Show Storage Information
       -showgeometry, -showgeo : Show Geometry Information
                     -showscsi : Show Scsi Information
                  -showpredict : Show Predict Information
     -noshowdevice, -noshowdev : No Show Device Information
      -noshowmedia, -noshowmed : No Show Media Information
      -noshowdrive, -noshowdrv : No Show Drive Information
    -noshowstorage, -noshowsto : No Show Storage Information
   -noshowgeometry, -noshowgeo : No Show Geometry Information
                   -noshowscsi : No Show Scsi Information
                -noshowpredict : No Show Predict Information

                     -list, -l : List Devices using wmic
               -listbrief, -lb : List Devices using wmic
                -listfull, -lf : List Devices using wmic
                 -list32, -l32 : List Devices using wmic c++
                  -field, -fld : Add Fields to -list32 (can be wildcarded)
            -listfields, -lfld : List Available Fields for -field

          -listalldevice, -lad : List All Devices
                 -listall, -la : List All Device Interfaces
                  -listcd, -lc : List CDRom Device Interfaces
                -listdisk, -ld : List Disk Device Interfaces
                 -listhub, -lh : List Hub Device Interfaces
                 -listusb, -lu : List Usb Device Interfaces
                 -listvol, -lv : List Volume Device Interfaces
                -listflop, -lf : List Floppy Device Interfaces
               -listpart, -lpa : List Partition Device Interfaces
               -listport, -lpo : List Port Device Interfaces
            -properties, -prop : List Properties
                    -guid, -lg : List GUID

              -locale {locale} : Localize Output .1252, fr-fr, ...
              -showstruct, -ss : Show Structure
                  -debug, -dbg : Show Debug Information
                     -dump, -d : Dump Attributes Buffer
                    -pause, -p : Wait for Enter

                  -waitprocess : Wait Process
                -nowaitprocess : No Wait Process
                   -runasadmin : Run As Admin
                    -runasuser : Run As User

````
