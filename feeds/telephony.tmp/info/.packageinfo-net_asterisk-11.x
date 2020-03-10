Source-Makefile: feeds/telephony/net/asterisk-11.x/Makefile
Package: asterisk11
Menu: 1
Submenu: Telephony
Version: 11.22.0-2
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread +libopenssl +libncurses +libpopt +libpthread +libsqlite3 +librt +libuuid +zlib @!TARGET_avr32
Conflicts: asterisk13
Menu-Depends: 
Provides: 
Build-Depends: libxml2/host
Section: net
Category: Network
Repository: base
Title: Complete open source PBX, v11.22.0
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-11.22.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description:  Asterisk is a complete PBX in software. It provides all of the features
 you would expect from a PBX and more. Asterisk does voice over IP in three
 protocols, and can interoperate with almost all standards-based telephony
 equipment using relatively inexpensive hardware.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk11-sounds
Submenu: Telephony
Version: 11.22.0-2
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk11
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: libxml2/host
Section: net
Category: Network
Repository: base
Title: Sounds support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-11.22.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: This package provides sounds for Asterisk11.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk11-app-alarmreceiver
Submenu: Telephony
Version: 11.22.0-2
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk11
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: libxml2/host
Section: net
Category: Network
Repository: base
Title: Alarm receiver support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-11.22.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: This package provides support for 'Central Station Alarm receiver for Ademco Contact ID' in Asterisk11.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk11-app-authenticate
Submenu: Telephony
Version: 11.22.0-2
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk11
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: libxml2/host
Section: net
Category: Network
Repository: base
Title: Authenticate commands support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-11.22.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: This package provides support for 'Execute arbitrary authenticate commands' in Asterisk11.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk11-app-confbridge
Submenu: Telephony
Version: 11.22.0-2
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk11
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: libxml2/host
Section: net
Category: Network
Repository: base
Title: ConfBridge support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-11.22.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: This package provides support for 'Software bridge for multi-party audio conferencing' in Asterisk11.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk11-app-dahdiras
Submenu: Telephony
Version: 11.22.0-2
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk11 +PACKAGE_asterisk11-app-dahdiras:asterisk11-chan-dahdi
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: libxml2/host
Section: net
Category: Network
Repository: base
Title: Execute an ISDN RAS support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-11.22.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: This package provides support for 'support for executing an ISDN RAS using DAHDI' in Asterisk11.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk11-app-directory
Submenu: Telephony
Version: 11.22.0-2
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk11
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: libxml2/host
Section: net
Category: Network
Repository: base
Title: Provide a directory of extensions support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-11.22.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: This package provides support for 'provides a directory of extensions' in Asterisk11.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk11-app-directed_pickup
Submenu: Telephony
Version: 11.22.0-2
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk11
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: libxml2/host
Section: net
Category: Network
Repository: base
Title: Directed call pickup support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-11.22.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: This package provides support for 'support for directed call pickup' in Asterisk11.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk11-app-disa
Submenu: Telephony
Version: 11.22.0-2
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk11
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: libxml2/host
Section: net
Category: Network
Repository: base
Title: Direct Inward System Access support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-11.22.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: This package provides support for 'Direct Inward System Access' in Asterisk11.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk11-app-exec
Submenu: Telephony
Version: 11.22.0-2
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk11
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: libxml2/host
Section: net
Category: Network
Repository: base
Title: Exec application support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-11.22.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: This package provides support for 'support for application execution' in Asterisk11.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk11-app-chanisavail
Submenu: Telephony
Version: 11.22.0-2
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk11
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: libxml2/host
Section: net
Category: Network
Repository: base
Title: Channel availability check support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-11.22.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: This package provides support for 'support for checking if a channel is available' in Asterisk11.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk11-app-chanspy
Submenu: Telephony
Version: 11.22.0-2
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk11
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: libxml2/host
Section: net
Category: Network
Repository: base
Title: Channel listen in support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-11.22.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: This package provides support for 'support for listening in on any channel' in Asterisk11.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk11-app-minivm
Submenu: Telephony
Version: 11.22.0-2
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk11
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: libxml2/host
Section: net
Category: Network
Repository: base
Title: Minimal voicemail system support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-11.22.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: This package provides support for 'a voicemail system in small building blocks working together based on the Comedian Mail voicemail' in Asterisk11.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk11-app-mixmonitor
Submenu: Telephony
Version: 11.22.0-2
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk11
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: libxml2/host
Section: net
Category: Network
Repository: base
Title: Record a call and mix the audio support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-11.22.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: This package provides support for 'record a call and mix the audio during the recording' in Asterisk11.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk11-app-originate
Submenu: Telephony
Version: 11.22.0-2
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk11
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: libxml2/host
Section: net
Category: Network
Repository: base
Title: Originate a call support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-11.22.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: This package provides support for 'originating an outbound call and connecting it to a specified extension or application' in Asterisk11.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk11-app-playtones
Submenu: Telephony
Version: 11.22.0-2
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk11
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: libxml2/host
Section: net
Category: Network
Repository: base
Title: Playtones application support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-11.22.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: This package provides support for 'play a tone list' in Asterisk11.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk11-app-queue
Submenu: Telephony
Version: 11.22.0-2
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk11
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: libxml2/host
Section: net
Category: Network
Repository: base
Title: True Call Queueing support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-11.22.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: This package provides support for 'support for ACD' in Asterisk11.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk11-app-read
Submenu: Telephony
Version: 11.22.0-2
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk11
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: libxml2/host
Section: net
Category: Network
Repository: base
Title: Variable read support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-11.22.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: This package provides support for 'a trivial application to read a variable' in Asterisk11.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk11-app-readexten
Submenu: Telephony
Version: 11.22.0-2
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk11
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: libxml2/host
Section: net
Category: Network
Repository: base
Title: Extension to variable support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-11.22.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: This package provides support for 'a trivial application to read an extension into a variable' in Asterisk11.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk11-app-record
Submenu: Telephony
Version: 11.22.0-2
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk11
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: libxml2/host
Section: net
Category: Network
Repository: base
Title: Record sound file support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-11.22.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: This package provides support for 'to record a sound file' in Asterisk11.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk11-app-sayunixtime
Submenu: Telephony
Version: 11.22.0-2
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk11
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: libxml2/host
Section: net
Category: Network
Repository: base
Title: Say Unix time support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-11.22.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: This package provides support for 'an application to say Unix time' in Asterisk11.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk11-app-senddtmf
Submenu: Telephony
Version: 11.22.0-2
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk11
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: libxml2/host
Section: net
Category: Network
Repository: base
Title: Send DTMF digits support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-11.22.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: This package provides support for 'Sends arbitrary DTMF digits' in Asterisk11.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk11-app-sms
Submenu: Telephony
Version: 11.22.0-2
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk11 +PACKAGE_asterisk11-app-sms:libpopt +PACKAGE_asterisk11-app-sms:libstdcpp
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: libxml2/host
Section: net
Category: Network
Repository: base
Title: SMS support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-11.22.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: This package provides support for 'SMS support (ETSI ES 201 912 protocol 1)' in Asterisk11.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk11-app-stack
Submenu: Telephony
Version: 11.22.0-2
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk11 +PACKAGE_asterisk11-app-stack:asterisk11-res-agi
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: libxml2/host
Section: net
Category: Network
Repository: base
Title: Stack applications support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-11.22.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: This package provides support for 'Stack applications Gosub Return etc.' in Asterisk11.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk11-app-system
Submenu: Telephony
Version: 11.22.0-2
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk11
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: libxml2/host
Section: net
Category: Network
Repository: base
Title: System exec support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-11.22.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: This package provides support for 'support for executing system commands' in Asterisk11.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk11-app-talkdetect
Submenu: Telephony
Version: 11.22.0-2
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk11
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: libxml2/host
Section: net
Category: Network
Repository: base
Title: File playback with audio detect support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-11.22.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: This package provides support for 'for file playback with audio detect' in Asterisk11.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk11-app-verbose
Submenu: Telephony
Version: 11.22.0-2
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk11
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: libxml2/host
Section: net
Category: Network
Repository: base
Title: Verbose logging support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-11.22.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: This package provides support for 'Verbose logging application' in Asterisk11.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk11-app-waituntil
Submenu: Telephony
Version: 11.22.0-2
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk11
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: libxml2/host
Section: net
Category: Network
Repository: base
Title: Sleep support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-11.22.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: This package provides support for 'support sleeping until the given epoch' in Asterisk11.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk11-app-while
Submenu: Telephony
Version: 11.22.0-2
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk11
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: libxml2/host
Section: net
Category: Network
Repository: base
Title: While loop support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-11.22.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: This package provides support for 'a while loop implementation' in Asterisk11.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk11-cdr-csv
Submenu: Telephony
Version: 11.22.0-2
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk11
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: libxml2/host
Section: net
Category: Network
Repository: base
Title: Provides CDR CSV support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-11.22.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: This package provides support for 'Call Detail Record with CSV support' in Asterisk11.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk11-cdr-sqlite3
Submenu: Telephony
Version: 11.22.0-2
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk11 libsqlite3
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: libxml2/host
Section: net
Category: Network
Repository: base
Title: Provides CDR SQLITE3 support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-11.22.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: This package provides support for 'Call Detail Record with SQLITE3 support' in Asterisk11.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk11-cdr
Submenu: Telephony
Version: 11.22.0-2
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk11
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: libxml2/host
Section: net
Category: Network
Repository: base
Title: Provides CDR support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-11.22.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: This package provides support for 'Call Detail Record' in Asterisk11.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk11-chan-alsa
Submenu: Telephony
Version: 11.22.0-2
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk11 +PACKAGE_asterisk11-chan-alsa:alsa-lib
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: libxml2/host
Section: net
Category: Network
Repository: base
Title: ALSA channel support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-11.22.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: This package provides support for 'the channel chan_alsa' in Asterisk11.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk11-chan-agent
Submenu: Telephony
Version: 11.22.0-2
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk11
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: libxml2/host
Section: net
Category: Network
Repository: base
Title: Agents proxy channel support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-11.22.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: This package provides support for 'an implementation of agents proxy channel' in Asterisk11.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk11-chan-dahdi
Submenu: Telephony
Version: 11.22.0-2
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk11 +PACKAGE_asterisk11-chan-dahdi:dahdi-tools-libtonezone +PACKAGE_asterisk11-chan-dahdi:kmod-dahdi +PACKAGE_asterisk11-chan-dahdi:libpri
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: libxml2/host
Section: net
Category: Network
Repository: base
Title: DAHDI channel support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-11.22.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: This package provides support for 'DAHDI channel support' in Asterisk11.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk11-chan-iax2
Submenu: Telephony
Version: 11.22.0-2
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk11 +PACKAGE_asterisk11-chan-iax2:asterisk11-res-timing-timerfd
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: libxml2/host
Section: net
Category: Network
Repository: base
Title: IAX2 channel support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-11.22.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: This package provides support for 'IAX support' in Asterisk11.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk11-chan-mgcp
Submenu: Telephony
Version: 11.22.0-2
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk11
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: libxml2/host
Section: net
Category: Network
Repository: base
Title: MGCP channel support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-11.22.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: This package provides support for 'the channel chan_mgcp' in Asterisk11.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk11-chan-motif
Submenu: Telephony
Version: 11.22.0-2
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk11 +PACKAGE_asterisk11-chan-motif:asterisk11-res-xmpp
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: libxml2/host
Section: net
Category: Network
Repository: base
Title: Jingle channel support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-11.22.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: This package provides support for 'Motif Jingle Channel Driver' in Asterisk11.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk11-chan-ooh323
Submenu: Telephony
Version: 11.22.0-2
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk11
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: libxml2/host
Section: net
Category: Network
Repository: base
Title: ooH323 channel support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-11.22.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: This package provides support for 'the channel chan_ooh323' in Asterisk11.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk11-chan-oss
Submenu: Telephony
Version: 11.22.0-2
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk11
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: libxml2/host
Section: net
Category: Network
Repository: base
Title: OSS channel support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-11.22.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: This package provides support for 'the channel chan_oss' in Asterisk11.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk11-chan-skinny
Submenu: Telephony
Version: 11.22.0-2
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk11
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: libxml2/host
Section: net
Category: Network
Repository: base
Title: Skinny channel support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-11.22.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: This package provides support for 'the channel chan_skinny' in Asterisk11.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk11-chan-unistim
Submenu: Telephony
Version: 11.22.0-2
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk11
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: libxml2/host
Section: net
Category: Network
Repository: base
Title: Unistim channel support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-11.22.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: This package provides support for 'channel driver for the UNISTIM (Unified Networks IP Stimulus) protocol' in Asterisk11.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk11-codec-a-mu
Submenu: Telephony
Version: 11.22.0-2
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk11
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: libxml2/host
Section: net
Category: Network
Repository: base
Title: Alaw to ulaw translation support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-11.22.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: This package provides support for 'translation between alaw and ulaw codecs' in Asterisk11.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk11-codec-adpcm
Submenu: Telephony
Version: 11.22.0-2
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk11
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: libxml2/host
Section: net
Category: Network
Repository: base
Title: ADPCM text support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-11.22.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: This package provides support for 'ADPCM text ' in Asterisk11.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk11-codec-alaw
Submenu: Telephony
Version: 11.22.0-2
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk11
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: libxml2/host
Section: net
Category: Network
Repository: base
Title: Signed linear to alaw translation support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-11.22.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: This package provides support for 'translation between signed linear and alaw codecs' in Asterisk11.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk11-codec-dahdi
Submenu: Telephony
Version: 11.22.0-2
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk11 +PACKAGE_asterisk11-codec-dahdi:asterisk11-chan-dahdi
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: libxml2/host
Section: net
Category: Network
Repository: base
Title: DAHDI codec support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-11.22.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: This package provides support for 'DAHDI native transcoding support' in Asterisk11.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk11-codec-g722
Submenu: Telephony
Version: 11.22.0-2
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk11
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: libxml2/host
Section: net
Category: Network
Repository: base
Title: G.722 support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-11.22.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: This package provides support for 'a high bit rate 48/56/64Kbps ITU standard codec' in Asterisk11.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk11-codec-g726
Submenu: Telephony
Version: 11.22.0-2
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk11
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: libxml2/host
Section: net
Category: Network
Repository: base
Title: Signed linear to G.726 translation support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-11.22.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: This package provides support for 'translation between signed linear and ITU G.726-32kbps codecs' in Asterisk11.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk11-codec-gsm
Submenu: Telephony
Version: 11.22.0-2
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk11
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: libxml2/host
Section: net
Category: Network
Repository: base
Title: linear to GSM translation support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-11.22.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: This package provides support for 'translate between signed linear and GSM' in Asterisk11.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk11-codec-ilbc
Submenu: Telephony
Version: 11.22.0-2
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk11
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: libxml2/host
Section: net
Category: Network
Repository: base
Title: linear to ILBC translation support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-11.22.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: This package provides support for 'translate between signed linear and ILBC' in Asterisk11.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk11-codec-lpc10
Submenu: Telephony
Version: 11.22.0-2
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk11
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: libxml2/host
Section: net
Category: Network
Repository: base
Title: Linear to LPC10 translation support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-11.22.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: This package provides support for 'translate between signed linear and LPC10' in Asterisk11.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk11-codec-resample
Submenu: Telephony
Version: 11.22.0-2
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk11
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: libxml2/host
Section: net
Category: Network
Repository: base
Title: resample sLinear audio support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-11.22.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: This package provides support for 'resample sLinear audio' in Asterisk11.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk11-curl
Submenu: Telephony
Version: 11.22.0-2
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk11 +PACKAGE_asterisk11-curl:libcurl
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: libxml2/host
Section: net
Category: Network
Repository: base
Title: CURL support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-11.22.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: This package provides support for 'CURL support' in Asterisk11.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk11-format-g726
Submenu: Telephony
Version: 11.22.0-2
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk11
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: libxml2/host
Section: net
Category: Network
Repository: base
Title: G.726 support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-11.22.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: This package provides support for 'support for headerless G.726 16/24/32/40kbps data format' in Asterisk11.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk11-format-g729
Submenu: Telephony
Version: 11.22.0-2
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk11
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: libxml2/host
Section: net
Category: Network
Repository: base
Title: G.729 support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-11.22.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: This package provides support for 'support for raw headerless G729 data' in Asterisk11.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk11-format-gsm
Submenu: Telephony
Version: 11.22.0-2
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk11
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: libxml2/host
Section: net
Category: Network
Repository: base
Title: GSM format support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-11.22.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: This package provides support for 'support for GSM format' in Asterisk11.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk11-format-h263
Submenu: Telephony
Version: 11.22.0-2
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk11
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: libxml2/host
Section: net
Category: Network
Repository: base
Title: H263 format support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-11.22.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: This package provides support for 'support for H264 format' in Asterisk11.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk11-format-h264
Submenu: Telephony
Version: 11.22.0-2
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk11
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: libxml2/host
Section: net
Category: Network
Repository: base
Title: H264 format support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-11.22.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: This package provides support for 'support for H264 format' in Asterisk11.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk11-format-ilbc
Submenu: Telephony
Version: 11.22.0-2
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk11
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: libxml2/host
Section: net
Category: Network
Repository: base
Title: ILBC format support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-11.22.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: This package provides support for 'support for ILBC format' in Asterisk11.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk11-format-sln
Submenu: Telephony
Version: 11.22.0-2
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk11
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: libxml2/host
Section: net
Category: Network
Repository: base
Title: Raw slinear format support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-11.22.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: This package provides support for 'support for raw slinear format' in Asterisk11.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk11-format-vox
Submenu: Telephony
Version: 11.22.0-2
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk11
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: libxml2/host
Section: net
Category: Network
Repository: base
Title: VOX format support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-11.22.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: This package provides support for 'support for ADPCM vox format' in Asterisk11.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk11-format-wav-gsm
Submenu: Telephony
Version: 11.22.0-2
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk11
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: libxml2/host
Section: net
Category: Network
Repository: base
Title: WAV format (Proprietary GSM) support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-11.22.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: This package provides support for 'support for proprietary Microsoft WAV format (Proprietary GSM)' in Asterisk11.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk11-format-wav
Submenu: Telephony
Version: 11.22.0-2
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk11
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: libxml2/host
Section: net
Category: Network
Repository: base
Title: WAV format (8000hz Signed Linear) support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-11.22.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: This package provides support for 'support for proprietary Microsoft WAV format (8000hz Signed Linear)' in Asterisk11.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk11-func-base64
Submenu: Telephony
Version: 11.22.0-2
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk11
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: libxml2/host
Section: net
Category: Network
Repository: base
Title: base64 support support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-11.22.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: This package provides support for 'support of base64 function' in Asterisk11.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk11-func-blacklist
Submenu: Telephony
Version: 11.22.0-2
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk11
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: libxml2/host
Section: net
Category: Network
Repository: base
Title: Blacklist on callerid support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-11.22.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: This package provides support for 'looking up the callerid number and see if it is blacklisted' in Asterisk11.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk11-func-cut
Submenu: Telephony
Version: 11.22.0-2
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk11
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: libxml2/host
Section: net
Category: Network
Repository: base
Title: CUT function support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-11.22.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: This package provides support for 'CUT function' in Asterisk11.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk11-func-db
Submenu: Telephony
Version: 11.22.0-2
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk11
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: libxml2/host
Section: net
Category: Network
Repository: base
Title: Database interaction support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-11.22.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: This package provides support for 'functions for interaction with the database' in Asterisk11.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk11-func-devstate
Submenu: Telephony
Version: 11.22.0-2
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk11
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: libxml2/host
Section: net
Category: Network
Repository: base
Title: Blinky lights control support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-11.22.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: This package provides support for 'functions for manually controlled blinky lights' in Asterisk11.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk11-func-enum
Submenu: Telephony
Version: 11.22.0-2
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk11
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: libxml2/host
Section: net
Category: Network
Repository: base
Title: ENUM support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-11.22.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: This package provides support for 'ENUM' in Asterisk11.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk11-func-env
Submenu: Telephony
Version: 11.22.0-2
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk11
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: libxml2/host
Section: net
Category: Network
Repository: base
Title: Environment functions support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-11.22.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: This package provides support for 'Environment dialplan functions' in Asterisk11.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk11-func-extstate
Submenu: Telephony
Version: 11.22.0-2
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk11
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: libxml2/host
Section: net
Category: Network
Repository: base
Title: Hinted extension state support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-11.22.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: This package provides support for 'retrieving the state of a hinted extension for dialplan control' in Asterisk11.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk11-func-global
Submenu: Telephony
Version: 11.22.0-2
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk11
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: libxml2/host
Section: net
Category: Network
Repository: base
Title: Global variable support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-11.22.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: This package provides support for 'global variable dialplan functions' in Asterisk11.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk11-func-groupcount
Submenu: Telephony
Version: 11.22.0-2
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk11
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: libxml2/host
Section: net
Category: Network
Repository: base
Title: Group count support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-11.22.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: This package provides support for 'for counting number of channels in the specified group' in Asterisk11.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk11-func-channel
Submenu: Telephony
Version: 11.22.0-2
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk11
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: libxml2/host
Section: net
Category: Network
Repository: base
Title: Channel info support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-11.22.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: This package provides support for 'Channel info dialplan function' in Asterisk11.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk11-func-math
Submenu: Telephony
Version: 11.22.0-2
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk11
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: libxml2/host
Section: net
Category: Network
Repository: base
Title: Math functions support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-11.22.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: This package provides support for 'Math functions' in Asterisk11.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk11-func-module
Submenu: Telephony
Version: 11.22.0-2
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk11
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: libxml2/host
Section: net
Category: Network
Repository: base
Title: Simple module check function support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-11.22.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: This package provides support for 'Simple module check function' in Asterisk11.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk11-func-presencestate
Submenu: Telephony
Version: 11.22.0-2
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk11
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: libxml2/host
Section: net
Category: Network
Repository: base
Title: Hinted presence state support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-11.22.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: This package provides support for 'Gets or sets a presence state in the dialplan' in Asterisk11.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk11-func-shell
Submenu: Telephony
Version: 11.22.0-2
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk11
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: libxml2/host
Section: net
Category: Network
Repository: base
Title: Shell support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-11.22.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: This package provides support for 'support for shell execution' in Asterisk11.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk11-func-uri
Submenu: Telephony
Version: 11.22.0-2
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk11
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: libxml2/host
Section: net
Category: Network
Repository: base
Title: URI encoding and decoding support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-11.22.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: This package provides support for 'Encodes and decodes URI-safe strings' in Asterisk11.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk11-func-vmcount
Submenu: Telephony
Version: 11.22.0-2
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk11
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: libxml2/host
Section: net
Category: Network
Repository: base
Title: vmcount dialplan support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-11.22.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: This package provides support for 'a vmcount dialplan function' in Asterisk11.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk11-func-realtime
Submenu: Telephony
Version: 11.22.0-2
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk11
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: libxml2/host
Section: net
Category: Network
Repository: base
Title: realtime dialplan support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-11.22.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: This package provides support for 'the realtime dialplan function' in Asterisk11.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk11-mysql
Submenu: Telephony
Version: 11.22.0-2
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk11 +PACKAGE_asterisk11-mysql:libmysqlclient
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: libxml2/host
Section: net
Category: Network
Repository: base
Title: MySQL support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-11.22.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: This package provides support for 'MySQL support' in Asterisk11.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk11-odbc
Submenu: Telephony
Version: 11.22.0-2
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk11 +PACKAGE_asterisk11-odbc:libpthread +PACKAGE_asterisk11-odbc:libc +PACKAGE_asterisk11-odbc:unixodbc
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: libxml2/host
Section: net
Category: Network
Repository: base
Title: ODBC support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-11.22.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: This package provides support for 'ODBC support' in Asterisk11.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk11-pbx-ael
Submenu: Telephony
Version: 11.22.0-2
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk11
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: libxml2/host
Section: net
Category: Network
Repository: base
Title: Asterisk Extension Logic support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-11.22.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: This package provides support for 'support for symbolic Asterisk Extension Logic' in Asterisk11.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk11-pbx-dundi
Submenu: Telephony
Version: 11.22.0-2
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk11
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: libxml2/host
Section: net
Category: Network
Repository: base
Title: Dundi support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-11.22.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: This package provides support for 'provides Dundi Lookup service for Asterisk' in Asterisk11.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk11-pbx-lua
Submenu: Telephony
Version: 11.22.0-2
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk11 +PACKAGE_asterisk11-pbx-lua:libpthread +PACKAGE_asterisk11-pbx-lua:libc +PACKAGE_asterisk11-pbx-lua:liblua
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: libxml2/host
Section: net
Category: Network
Repository: base
Title: Lua support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-11.22.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: This package provides support for 'provides Lua resources for Asterisk' in Asterisk11.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk11-pbx-spool
Submenu: Telephony
Version: 11.22.0-2
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk11
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: libxml2/host
Section: net
Category: Network
Repository: base
Title: Call Spool support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-11.22.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: This package provides support for 'outgoing call spool support' in Asterisk11.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk11-pbx-realtime
Submenu: Telephony
Version: 11.22.0-2
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk11
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: libxml2/host
Section: net
Category: Network
Repository: base
Title: Realtime Switch support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-11.22.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: This package provides support for 'realtime switch support' in Asterisk11.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk11-pgsql
Submenu: Telephony
Version: 11.22.0-2
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk11 +PACKAGE_asterisk11-pgsql:libpq
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: libxml2/host
Section: net
Category: Network
Repository: base
Title: PostgreSQL support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-11.22.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: This package provides support for 'PostgreSQL support' in Asterisk11.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk11-res-adsi
Submenu: Telephony
Version: 11.22.0-2
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk11
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: libxml2/host
Section: net
Category: Network
Repository: base
Title: Provide ADSI support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-11.22.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: This package provides support for 'Analog Display Services Interface capability' in Asterisk11.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk11-res-ael-share
Submenu: Telephony
Version: 11.22.0-2
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk11
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: libxml2/host
Section: net
Category: Network
Repository: base
Title: Shareable AEL code support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-11.22.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: This package provides support for 'support for shareable AEL code mainly between internal and external modules' in Asterisk11.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk11-res-agi
Submenu: Telephony
Version: 11.22.0-2
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk11
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: libxml2/host
Section: net
Category: Network
Repository: base
Title: Asterisk Gateway Interface support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-11.22.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: This package provides support for 'Support for the Asterisk Gateway Interface extension' in Asterisk11.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk11-res-calendar
Submenu: Telephony
Version: 11.22.0-2
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk11
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: libxml2/host
Section: net
Category: Network
Repository: base
Title: Calendaring API support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-11.22.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: This package provides support for 'Calendaring support (ICal and Google Calendar)' in Asterisk11.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk11-res-clioriginate
Submenu: Telephony
Version: 11.22.0-2
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk11
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: libxml2/host
Section: net
Category: Network
Repository: base
Title: Calls via CLI support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-11.22.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: This package provides support for 'Originate calls via the CLI' in Asterisk11.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk11-res-monitor
Submenu: Telephony
Version: 11.22.0-2
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk11
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: libxml2/host
Section: net
Category: Network
Repository: base
Title: Provide Monitor support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-11.22.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: This package provides support for 'Cryptographic Signature capability' in Asterisk11.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk11-res-musiconhold
Submenu: Telephony
Version: 11.22.0-2
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk11
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: libxml2/host
Section: net
Category: Network
Repository: base
Title: MOH support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-11.22.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: This package provides support for 'Music On Hold support' in Asterisk11.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk11-res-phoneprov
Submenu: Telephony
Version: 11.22.0-2
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk11
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: libxml2/host
Section: net
Category: Network
Repository: base
Title: Phone Provisioning support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-11.22.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: This package provides support for 'Phone provisioning application for the asterisk internal http server' in Asterisk11.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk11-res-pktccops
Submenu: Telephony
Version: 11.22.0-2
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk11
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: libxml2/host
Section: net
Category: Network
Repository: base
Title: Provide PacketCable COPS support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-11.22.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: This package provides support for 'simple client/server model for supporting policy control over QoS signaling protocols' in Asterisk11.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk11-res-smdi
Submenu: Telephony
Version: 11.22.0-2
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk11
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: libxml2/host
Section: net
Category: Network
Repository: base
Title: Provide SMDI support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-11.22.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: This package provides support for 'Simple Message Desk Interface capability' in Asterisk11.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk11-res-srtp
Submenu: Telephony
Version: 11.22.0-2
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk11 +PACKAGE_asterisk11-res-srtp:libsrtp
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: libxml2/host
Section: net
Category: Network
Repository: base
Title: SRTP support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-11.22.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: This package provides support for 'Secure RTP' in Asterisk11.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk11-res-timing-dahdi
Submenu: Telephony
Version: 11.22.0-2
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk11 +PACKAGE_asterisk11-res-timing-dahdi:asterisk11-chan-dahdi
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: libxml2/host
Section: net
Category: Network
Repository: base
Title: DAHDI Timing Interface support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-11.22.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: This package provides support for '' in Asterisk11.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk11-res-timing-pthread
Submenu: Telephony
Version: 11.22.0-2
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk11
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: libxml2/host
Section: net
Category: Network
Repository: base
Title: pthread Timing Interface support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-11.22.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: This package provides support for '' in Asterisk11.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk11-res-timing-timerfd
Submenu: Telephony
Version: 11.22.0-2
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk11
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: libxml2/host
Section: net
Category: Network
Repository: base
Title: Timerfd Timing Interface support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-11.22.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: This package provides support for '' in Asterisk11.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk11-res-xmpp
Submenu: Telephony
Version: 11.22.0-2
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk11 +PACKAGE_asterisk11-res-xmpp:libiksemel +PACKAGE_asterisk11-res-xmpp:libopenssl
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: libxml2/host
Section: net
Category: Network
Repository: base
Title: XMPP client and component module support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-11.22.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: This package provides support for 'reference module for interfacting Asterisk directly as a client or component with XMPP server' in Asterisk11.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk11-res-realtime
Submenu: Telephony
Version: 11.22.0-2
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk11
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: libxml2/host
Section: net
Category: Network
Repository: base
Title: Realtime Interface support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-11.22.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: This package provides support for '' in Asterisk11.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk11-voicemail
Submenu: Telephony
Version: 11.22.0-2
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk11 +PACKAGE_asterisk11-voicemail:asterisk11-res-adsi +PACKAGE_asterisk11-voicemail:asterisk11-res-smdi
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: libxml2/host
Section: net
Category: Network
Repository: base
Title: Voicemail support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-11.22.0.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: This package provides support for 'voicemail related modules' in Asterisk11.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@


