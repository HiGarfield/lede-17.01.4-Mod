Source-Makefile: feeds/telephony/net/asterisk-13.x/Makefile
Package: asterisk13
Menu: 1
Submenu: Telephony
Version: 13.9.1-1
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread +jansson +libncurses +libopenssl +libpopt +libsqlite3 +libstdcpp +libuuid +libxml2 +libxslt +zlib
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: libxml2/host
Section: net
Category: Network
Repository: base
Title: Complete open source PBX, v13.9.1
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-13.9.1.tar.gz
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

Package: asterisk13-sounds
Submenu: Telephony
Version: 13.9.1-1
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk13
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: libxml2/host
Section: net
Category: Network
Repository: base
Title: Sounds support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-13.9.1.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: This package provides the sound-files for Asterisk-13.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk13-app-alarmreceiver
Submenu: Telephony
Version: 13.9.1-1
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk13
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: libxml2/host
Section: net
Category: Network
Repository: base
Title: Alarm receiver support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-13.9.1.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: This package provides support for 'Central Station Alarm receiver for Ademco Contact ID' in Asterisk.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk13-app-authenticate
Submenu: Telephony
Version: 13.9.1-1
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk13
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: libxml2/host
Section: net
Category: Network
Repository: base
Title: Authenticate commands support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-13.9.1.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: This package provides support for 'Execute arbitrary authenticate commands' in Asterisk.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk13-app-chanisavail
Submenu: Telephony
Version: 13.9.1-1
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk13
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: libxml2/host
Section: net
Category: Network
Repository: base
Title: Channel availability check support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-13.9.1.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: This package provides support for 'support for checking if a channel is available' in Asterisk.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk13-app-chanspy
Submenu: Telephony
Version: 13.9.1-1
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk13
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: libxml2/host
Section: net
Category: Network
Repository: base
Title: Channel listen in support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-13.9.1.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: This package provides support for 'support for listening in on any channel' in Asterisk.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk13-app-confbridge
Submenu: Telephony
Version: 13.9.1-1
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk13 +PACKAGE_asterisk13-app-confbridge:asterisk13-bridge-builtin-features +PACKAGE_asterisk13-app-confbridge:asterisk13-bridge-simple +PACKAGE_asterisk13-app-confbridge:asterisk13-bridge-softmix
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: libxml2/host
Section: net
Category: Network
Repository: base
Title: ConfBridge support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-13.9.1.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: This package provides support for 'Software bridge for multi-party audio conferencing' in Asterisk.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk13-app-dahdiras
Submenu: Telephony
Version: 13.9.1-1
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk13 +PACKAGE_asterisk13-app-dahdiras:asterisk13-chan-dahdi
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: libxml2/host
Section: net
Category: Network
Repository: base
Title: Execute an ISDN RAS support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-13.9.1.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: This package provides support for 'support for executing an ISDN RAS using DAHDI' in Asterisk.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk13-app-directed_pickup
Submenu: Telephony
Version: 13.9.1-1
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk13
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: libxml2/host
Section: net
Category: Network
Repository: base
Title: Directed call pickup support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-13.9.1.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: This package provides support for 'support for directed call pickup' in Asterisk.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk13-app-disa
Submenu: Telephony
Version: 13.9.1-1
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk13
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: libxml2/host
Section: net
Category: Network
Repository: base
Title: Direct Inward System Access support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-13.9.1.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: This package provides support for 'Direct Inward System Access' in Asterisk.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk13-app-exec
Submenu: Telephony
Version: 13.9.1-1
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk13
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: libxml2/host
Section: net
Category: Network
Repository: base
Title: Exec application support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-13.9.1.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: This package provides support for 'support for application execution' in Asterisk.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk13-app-minivm
Submenu: Telephony
Version: 13.9.1-1
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk13
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: libxml2/host
Section: net
Category: Network
Repository: base
Title: Minimal voicemail system support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-13.9.1.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: This package provides support for 'a voicemail system in small building blocks working together based on the Comedian Mail voicemail' in Asterisk.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk13-app-mixmonitor
Submenu: Telephony
Version: 13.9.1-1
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk13
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: libxml2/host
Section: net
Category: Network
Repository: base
Title: Record a call and mix the audio support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-13.9.1.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: This package provides support for 'record a call and mix the audio during the recording' in Asterisk.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk13-app-originate
Submenu: Telephony
Version: 13.9.1-1
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk13
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: libxml2/host
Section: net
Category: Network
Repository: base
Title: Originate a call support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-13.9.1.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: This package provides support for 'originating an outbound call and connecting it to a specified extension or application' in Asterisk.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk13-app-playtones
Submenu: Telephony
Version: 13.9.1-1
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk13
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: libxml2/host
Section: net
Category: Network
Repository: base
Title: Playtones application support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-13.9.1.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: This package provides support for 'play a tone list' in Asterisk.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk13-app-queue
Submenu: Telephony
Version: 13.9.1-1
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk13
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: libxml2/host
Section: net
Category: Network
Repository: base
Title: True Call Queueing support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-13.9.1.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: This package provides support for 'support for ACD' in Asterisk.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk13-app-read
Submenu: Telephony
Version: 13.9.1-1
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk13
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: libxml2/host
Section: net
Category: Network
Repository: base
Title: Variable read support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-13.9.1.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: This package provides support for 'a trivial application to read a variable' in Asterisk.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk13-app-readexten
Submenu: Telephony
Version: 13.9.1-1
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk13
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: libxml2/host
Section: net
Category: Network
Repository: base
Title: Extension to variable support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-13.9.1.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: This package provides support for 'a trivial application to read an extension into a variable' in Asterisk.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk13-app-record
Submenu: Telephony
Version: 13.9.1-1
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk13
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: libxml2/host
Section: net
Category: Network
Repository: base
Title: Record sound file support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-13.9.1.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: This package provides support for 'to record a sound file' in Asterisk.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk13-app-sayunixtime
Submenu: Telephony
Version: 13.9.1-1
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk13
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: libxml2/host
Section: net
Category: Network
Repository: base
Title: Say Unix time support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-13.9.1.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: This package provides support for 'an application to say Unix time' in Asterisk.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk13-app-senddtmf
Submenu: Telephony
Version: 13.9.1-1
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk13
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: libxml2/host
Section: net
Category: Network
Repository: base
Title: Send DTMF digits support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-13.9.1.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: This package provides support for 'Sends arbitrary DTMF digits' in Asterisk.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk13-app-sms
Submenu: Telephony
Version: 13.9.1-1
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk13 +PACKAGE_asterisk13-app-sms:libpopt +PACKAGE_asterisk13-app-sms:libstdcpp
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: libxml2/host
Section: net
Category: Network
Repository: base
Title: SMS support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-13.9.1.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: This package provides support for 'SMS support (ETSI ES 201 912 protocol 1)' in Asterisk.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk13-app-stack
Submenu: Telephony
Version: 13.9.1-1
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk13 +PACKAGE_asterisk13-app-stack:asterisk13-res-agi
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: libxml2/host
Section: net
Category: Network
Repository: base
Title: Stack applications support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-13.9.1.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: This package provides support for 'Stack applications Gosub Return etc.' in Asterisk.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk13-app-system
Submenu: Telephony
Version: 13.9.1-1
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk13
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: libxml2/host
Section: net
Category: Network
Repository: base
Title: System exec support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-13.9.1.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: This package provides support for 'support for executing system commands' in Asterisk.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk13-app-talkdetect
Submenu: Telephony
Version: 13.9.1-1
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk13
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: libxml2/host
Section: net
Category: Network
Repository: base
Title: File playback with audio detect support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-13.9.1.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: This package provides support for 'for file playback with audio detect' in Asterisk.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk13-app-verbose
Submenu: Telephony
Version: 13.9.1-1
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk13
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: libxml2/host
Section: net
Category: Network
Repository: base
Title: Verbose logging support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-13.9.1.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: This package provides support for 'Verbose logging application' in Asterisk.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk13-app-waituntil
Submenu: Telephony
Version: 13.9.1-1
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk13
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: libxml2/host
Section: net
Category: Network
Repository: base
Title: Sleep support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-13.9.1.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: This package provides support for 'support sleeping until the given epoch' in Asterisk.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk13-app-while
Submenu: Telephony
Version: 13.9.1-1
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk13
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: libxml2/host
Section: net
Category: Network
Repository: base
Title: While loop support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-13.9.1.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: This package provides support for 'a while loop implementation' in Asterisk.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk13-bridge-builtin-features
Submenu: Telephony
Version: 13.9.1-1
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk13
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: libxml2/host
Section: net
Category: Network
Repository: base
Title: Bridging features support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-13.9.1.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: This package provides support for 'built in bridging features' in Asterisk.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk13-bridge-builtin-interval-features
Submenu: Telephony
Version: 13.9.1-1
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk13
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: libxml2/host
Section: net
Category: Network
Repository: base
Title: Built in bridging interval features support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-13.9.1.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: This package provides support for 'built in bridging interval features' in Asterisk.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk13-bridge-holding
Submenu: Telephony
Version: 13.9.1-1
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk13
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: libxml2/host
Section: net
Category: Network
Repository: base
Title: Bridging for storing channels in a bridge support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-13.9.1.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: This package provides support for 'bridging technology for storing channels in a bridge' in Asterisk.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk13-bridge-native-rtp
Submenu: Telephony
Version: 13.9.1-1
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk13
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: libxml2/host
Section: net
Category: Network
Repository: base
Title: Native RTP bridging technology module support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-13.9.1.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: This package provides support for 'native RTP bridging technology module' in Asterisk.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk13-bridge-simple
Submenu: Telephony
Version: 13.9.1-1
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk13
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: libxml2/host
Section: net
Category: Network
Repository: base
Title: Simple two channel bridging module support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-13.9.1.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: This package provides support for 'simple two channel bridging module' in Asterisk.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk13-bridge-softmix
Submenu: Telephony
Version: 13.9.1-1
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk13
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: libxml2/host
Section: net
Category: Network
Repository: base
Title: Multi-party software based channel mixing support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-13.9.1.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: This package provides support for 'multi-party software based channel mixing' in Asterisk.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk13-cdr
Submenu: Telephony
Version: 13.9.1-1
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk13
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: libxml2/host
Section: net
Category: Network
Repository: base
Title: Provides CDR support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-13.9.1.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: This package provides support for 'Call Detail Record' in Asterisk.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk13-cdr-csv
Submenu: Telephony
Version: 13.9.1-1
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk13
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: libxml2/host
Section: net
Category: Network
Repository: base
Title: Provides CDR CSV support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-13.9.1.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: This package provides support for 'Call Detail Record with CSV support' in Asterisk.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk13-cdr-sqlite3
Submenu: Telephony
Version: 13.9.1-1
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk13 libsqlite3
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: libxml2/host
Section: net
Category: Network
Repository: base
Title: Provides CDR SQLITE3 support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-13.9.1.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: This package provides support for 'Call Detail Record with SQLITE3 support' in Asterisk.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk13-chan-alsa
Submenu: Telephony
Version: 13.9.1-1
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk13 +PACKAGE_asterisk13-chan-alsa:alsa-lib
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: libxml2/host
Section: net
Category: Network
Repository: base
Title: ALSA channel support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-13.9.1.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: This package provides support for 'the channel chan_alsa' in Asterisk.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk13-chan-dahdi
Submenu: Telephony
Version: 13.9.1-1
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk13 +PACKAGE_asterisk13-chan-dahdi:dahdi-tools-libtonezone +PACKAGE_asterisk13-chan-dahdi:kmod-dahdi +PACKAGE_asterisk13-chan-dahdi:libpri
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: libxml2/host
Section: net
Category: Network
Repository: base
Title: DAHDI channel support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-13.9.1.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: This package provides support for 'DAHDI channel support' in Asterisk.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk13-chan-iax2
Submenu: Telephony
Version: 13.9.1-1
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk13 +PACKAGE_asterisk13-chan-iax2:asterisk13-res-timing-timerfd
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: libxml2/host
Section: net
Category: Network
Repository: base
Title: IAX2 channel support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-13.9.1.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: This package provides support for 'IAX support' in Asterisk.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk13-chan-oss
Submenu: Telephony
Version: 13.9.1-1
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk13
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: libxml2/host
Section: net
Category: Network
Repository: base
Title: OSS channel support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-13.9.1.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: This package provides support for 'the channel chan_oss' in Asterisk.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk13-chan-sip
Submenu: Telephony
Version: 13.9.1-1
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk13 +PACKAGE_asterisk13-chan-sip:asterisk13-app-confbridge
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: libxml2/host
Section: net
Category: Network
Repository: base
Title: SIP channel support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-13.9.1.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: This package provides support for 'the channel chan_sip' in Asterisk.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk13-chan-skinny
Submenu: Telephony
Version: 13.9.1-1
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk13
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: libxml2/host
Section: net
Category: Network
Repository: base
Title: Skinny channel support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-13.9.1.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: This package provides support for 'the channel chan_skinny' in Asterisk.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk13-chan-unistim
Submenu: Telephony
Version: 13.9.1-1
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk13
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: libxml2/host
Section: net
Category: Network
Repository: base
Title: Unistim channel support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-13.9.1.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: This package provides support for 'channel driver for the UNISTIM (Unified Networks IP Stimulus) protocol' in Asterisk.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk13-codec-a-mu
Submenu: Telephony
Version: 13.9.1-1
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk13
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: libxml2/host
Section: net
Category: Network
Repository: base
Title: Alaw to ulaw translation support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-13.9.1.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: This package provides support for 'translation between alaw and ulaw codecs' in Asterisk.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk13-codec-adpcm
Submenu: Telephony
Version: 13.9.1-1
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk13
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: libxml2/host
Section: net
Category: Network
Repository: base
Title: ADPCM text support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-13.9.1.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: This package provides support for 'ADPCM text ' in Asterisk.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk13-codec-alaw
Submenu: Telephony
Version: 13.9.1-1
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk13
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: libxml2/host
Section: net
Category: Network
Repository: base
Title: Signed linear to alaw translation support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-13.9.1.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: This package provides support for 'translation between signed linear and alaw codecs' in Asterisk.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk13-codec-dahdi
Submenu: Telephony
Version: 13.9.1-1
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk13 +PACKAGE_asterisk13-codec-dahdi:asterisk13-chan-dahdi
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: libxml2/host
Section: net
Category: Network
Repository: base
Title: DAHDI codec support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-13.9.1.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: This package provides support for 'DAHDI native transcoding support' in Asterisk.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk13-codec-g722
Submenu: Telephony
Version: 13.9.1-1
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk13
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: libxml2/host
Section: net
Category: Network
Repository: base
Title: G.722 support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-13.9.1.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: This package provides support for 'a high bit rate 48/56/64Kbps ITU standard codec' in Asterisk.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk13-codec-g726
Submenu: Telephony
Version: 13.9.1-1
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk13
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: libxml2/host
Section: net
Category: Network
Repository: base
Title: Signed linear to G.726 translation support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-13.9.1.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: This package provides support for 'translation between signed linear and ITU G.726-32kbps codecs' in Asterisk.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk13-codec-gsm
Submenu: Telephony
Version: 13.9.1-1
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk13
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: libxml2/host
Section: net
Category: Network
Repository: base
Title: linear to GSM translation support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-13.9.1.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: This package provides support for 'translate between signed linear and GSM' in Asterisk.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk13-codec-ilbc
Submenu: Telephony
Version: 13.9.1-1
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk13
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: libxml2/host
Section: net
Category: Network
Repository: base
Title: linear to ILBC translation support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-13.9.1.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: This package provides support for 'translate between signed linear and ILBC' in Asterisk.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk13-codec-lpc10
Submenu: Telephony
Version: 13.9.1-1
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk13
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: libxml2/host
Section: net
Category: Network
Repository: base
Title: Linear to LPC10 translation support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-13.9.1.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: This package provides support for 'translate between signed linear and LPC10' in Asterisk.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk13-codec-resample
Submenu: Telephony
Version: 13.9.1-1
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk13
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: libxml2/host
Section: net
Category: Network
Repository: base
Title: resample sLinear audio support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-13.9.1.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: This package provides support for 'resample sLinear audio' in Asterisk.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk13-codec-ulaw
Submenu: Telephony
Version: 13.9.1-1
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk13
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: libxml2/host
Section: net
Category: Network
Repository: base
Title: Signed linear to ulaw translation support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-13.9.1.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: This package provides support for 'translation between signed linear and ulaw codecs' in Asterisk.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk13-curl
Submenu: Telephony
Version: 13.9.1-1
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk13 +PACKAGE_asterisk13-curl:libcurl
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: libxml2/host
Section: net
Category: Network
Repository: base
Title: CURL support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-13.9.1.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: This package provides support for 'CURL support' in Asterisk.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk13-format-g726
Submenu: Telephony
Version: 13.9.1-1
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk13
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: libxml2/host
Section: net
Category: Network
Repository: base
Title: G.726 support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-13.9.1.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: This package provides support for 'support for headerless G.726 16/24/32/40kbps data format' in Asterisk.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk13-format-g729
Submenu: Telephony
Version: 13.9.1-1
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk13
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: libxml2/host
Section: net
Category: Network
Repository: base
Title: G.729 support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-13.9.1.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: This package provides support for 'support for raw headerless G729 data' in Asterisk.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk13-format-gsm
Submenu: Telephony
Version: 13.9.1-1
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk13
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: libxml2/host
Section: net
Category: Network
Repository: base
Title: GSM format support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-13.9.1.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: This package provides support for 'support for GSM format' in Asterisk.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk13-format-h263
Submenu: Telephony
Version: 13.9.1-1
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk13
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: libxml2/host
Section: net
Category: Network
Repository: base
Title: H263 format support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-13.9.1.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: This package provides support for 'support for H264 format' in Asterisk.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk13-format-h264
Submenu: Telephony
Version: 13.9.1-1
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk13
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: libxml2/host
Section: net
Category: Network
Repository: base
Title: H264 format support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-13.9.1.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: This package provides support for 'support for H264 format' in Asterisk.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk13-format-ilbc
Submenu: Telephony
Version: 13.9.1-1
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk13
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: libxml2/host
Section: net
Category: Network
Repository: base
Title: ILBC format support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-13.9.1.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: This package provides support for 'support for ILBC format' in Asterisk.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk13-format-pcm
Submenu: Telephony
Version: 13.9.1-1
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk13
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: libxml2/host
Section: net
Category: Network
Repository: base
Title: PCM format support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-13.9.1.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: This package provides support for 'support for PCM format' in Asterisk.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk13-format-sln
Submenu: Telephony
Version: 13.9.1-1
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk13
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: libxml2/host
Section: net
Category: Network
Repository: base
Title: Raw slinear format support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-13.9.1.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: This package provides support for 'support for raw slinear format' in Asterisk.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk13-format-vox
Submenu: Telephony
Version: 13.9.1-1
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk13
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: libxml2/host
Section: net
Category: Network
Repository: base
Title: VOX format support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-13.9.1.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: This package provides support for 'support for ADPCM vox format' in Asterisk.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk13-format-wav
Submenu: Telephony
Version: 13.9.1-1
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk13
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: libxml2/host
Section: net
Category: Network
Repository: base
Title: WAV format (8000hz Signed Linear) support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-13.9.1.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: This package provides support for 'support for proprietary Microsoft WAV format (8000hz Signed Linear)' in Asterisk.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk13-format-wav-gsm
Submenu: Telephony
Version: 13.9.1-1
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk13
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: libxml2/host
Section: net
Category: Network
Repository: base
Title: WAV format (Proprietary GSM) support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-13.9.1.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: This package provides support for 'support for proprietary Microsoft WAV format (Proprietary GSM)' in Asterisk.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk13-func-base64
Submenu: Telephony
Version: 13.9.1-1
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk13
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: libxml2/host
Section: net
Category: Network
Repository: base
Title: base64 support support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-13.9.1.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: This package provides support for 'support of base64 function' in Asterisk.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk13-func-blacklist
Submenu: Telephony
Version: 13.9.1-1
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk13
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: libxml2/host
Section: net
Category: Network
Repository: base
Title: Blacklist on callerid support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-13.9.1.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: This package provides support for 'looking up the callerid number and see if it is blacklisted' in Asterisk.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk13-func-channel
Submenu: Telephony
Version: 13.9.1-1
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk13
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: libxml2/host
Section: net
Category: Network
Repository: base
Title: Channel info support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-13.9.1.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: This package provides support for 'Channel info dialplan function' in Asterisk.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk13-func-cut
Submenu: Telephony
Version: 13.9.1-1
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk13
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: libxml2/host
Section: net
Category: Network
Repository: base
Title: CUT function support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-13.9.1.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: This package provides support for 'CUT function' in Asterisk.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk13-func-db
Submenu: Telephony
Version: 13.9.1-1
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk13
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: libxml2/host
Section: net
Category: Network
Repository: base
Title: Database interaction support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-13.9.1.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: This package provides support for 'functions for interaction with the database' in Asterisk.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk13-func-devstate
Submenu: Telephony
Version: 13.9.1-1
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk13
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: libxml2/host
Section: net
Category: Network
Repository: base
Title: Blinky lights control support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-13.9.1.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: This package provides support for 'functions for manually controlled blinky lights' in Asterisk.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk13-func-enum
Submenu: Telephony
Version: 13.9.1-1
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk13
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: libxml2/host
Section: net
Category: Network
Repository: base
Title: ENUM support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-13.9.1.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: This package provides support for 'ENUM' in Asterisk.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk13-func-env
Submenu: Telephony
Version: 13.9.1-1
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk13
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: libxml2/host
Section: net
Category: Network
Repository: base
Title: Environment functions support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-13.9.1.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: This package provides support for 'Environment dialplan functions' in Asterisk.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk13-func-extstate
Submenu: Telephony
Version: 13.9.1-1
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk13
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: libxml2/host
Section: net
Category: Network
Repository: base
Title: Hinted extension state support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-13.9.1.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: This package provides support for 'retrieving the state of a hinted extension for dialplan control' in Asterisk.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk13-func-global
Submenu: Telephony
Version: 13.9.1-1
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk13
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: libxml2/host
Section: net
Category: Network
Repository: base
Title: Global variable support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-13.9.1.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: This package provides support for 'global variable dialplan functions' in Asterisk.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk13-func-groupcount
Submenu: Telephony
Version: 13.9.1-1
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk13
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: libxml2/host
Section: net
Category: Network
Repository: base
Title: Group count support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-13.9.1.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: This package provides support for 'for counting number of channels in the specified group' in Asterisk.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk13-func-math
Submenu: Telephony
Version: 13.9.1-1
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk13
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: libxml2/host
Section: net
Category: Network
Repository: base
Title: Math functions support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-13.9.1.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: This package provides support for 'Math functions' in Asterisk.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk13-func-module
Submenu: Telephony
Version: 13.9.1-1
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk13
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: libxml2/host
Section: net
Category: Network
Repository: base
Title: Simple module check function support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-13.9.1.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: This package provides support for 'Simple module check function' in Asterisk.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk13-func-presencestate
Submenu: Telephony
Version: 13.9.1-1
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk13
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: libxml2/host
Section: net
Category: Network
Repository: base
Title: Hinted presence state support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-13.9.1.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: This package provides support for 'Gets or sets a presence state in the dialplan' in Asterisk.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk13-func-realtime
Submenu: Telephony
Version: 13.9.1-1
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk13
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: libxml2/host
Section: net
Category: Network
Repository: base
Title: realtime support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-13.9.1.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: This package provides support for 'the realtime dialplan function' in Asterisk.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk13-func-shell
Submenu: Telephony
Version: 13.9.1-1
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk13
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: libxml2/host
Section: net
Category: Network
Repository: base
Title: Shell support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-13.9.1.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: This package provides support for 'support for shell execution' in Asterisk.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk13-func-uri
Submenu: Telephony
Version: 13.9.1-1
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk13
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: libxml2/host
Section: net
Category: Network
Repository: base
Title: URI encoding and decoding support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-13.9.1.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: This package provides support for 'Encodes and decodes URI-safe strings' in Asterisk.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk13-func-vmcount
Submenu: Telephony
Version: 13.9.1-1
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk13
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: libxml2/host
Section: net
Category: Network
Repository: base
Title: vmcount dialplan support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-13.9.1.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: This package provides support for 'a vmcount dialplan function' in Asterisk.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk13-odbc
Submenu: Telephony
Version: 13.9.1-1
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk13 +PACKAGE_asterisk13-odbc:libpthread +PACKAGE_asterisk13-odbc:libc +PACKAGE_asterisk13-odbc:unixodbc
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: libxml2/host
Section: net
Category: Network
Repository: base
Title: ODBC support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-13.9.1.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: This package provides support for 'ODBC support' in Asterisk.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk13-pbx-ael
Submenu: Telephony
Version: 13.9.1-1
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk13
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: libxml2/host
Section: net
Category: Network
Repository: base
Title: Asterisk Extension Logic support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-13.9.1.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: This package provides support for 'support for symbolic Asterisk Extension Logic' in Asterisk.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk13-pbx-dundi
Submenu: Telephony
Version: 13.9.1-1
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk13
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: libxml2/host
Section: net
Category: Network
Repository: base
Title: Dundi support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-13.9.1.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: This package provides support for 'provides Dundi Lookup service for Asterisk' in Asterisk.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk13-pbx-realtime
Submenu: Telephony
Version: 13.9.1-1
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk13
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: libxml2/host
Section: net
Category: Network
Repository: base
Title: Realtime Switch support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-13.9.1.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: This package provides support for 'realtime switch support' in Asterisk.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk13-pbx-spool
Submenu: Telephony
Version: 13.9.1-1
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk13
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: libxml2/host
Section: net
Category: Network
Repository: base
Title: Call Spool support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-13.9.1.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: This package provides support for 'outgoing call spool support' in Asterisk.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk13-pgsql
Submenu: Telephony
Version: 13.9.1-1
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk13 +PACKAGE_asterisk13-pgsql:libpq
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: libxml2/host
Section: net
Category: Network
Repository: base
Title: PostgreSQL support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-13.9.1.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: This package provides support for 'PostgreSQL support' in Asterisk.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk13-pjsip
Submenu: Telephony
Version: 13.9.1-1
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk13 +PACKAGE_asterisk13-pjsip:asterisk13-res-sorcery +PACKAGE_asterisk13-pjsip:libpjsip +PACKAGE_asterisk13-pjsip:libpjmedia +PACKAGE_asterisk13-pjsip:libpjnath +PACKAGE_asterisk13-pjsip:libpjsip-simple +PACKAGE_asterisk13-pjsip:libpjsip-ua +PACKAGE_asterisk13-pjsip:libpjsua +PACKAGE_asterisk13-pjsip:libpjsua2
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: libxml2/host
Section: net
Category: Network
Repository: base
Title: pjsip channel support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-13.9.1.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: This package provides support for 'the channel pjsip' in Asterisk.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk13-res-adsi
Submenu: Telephony
Version: 13.9.1-1
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk13
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: libxml2/host
Section: net
Category: Network
Repository: base
Title: Provide ADSI support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-13.9.1.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: This package provides support for 'Analog Display Services Interface capability' in Asterisk.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk13-res-ael-share
Submenu: Telephony
Version: 13.9.1-1
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk13
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: libxml2/host
Section: net
Category: Network
Repository: base
Title: Shareable AEL code support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-13.9.1.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: This package provides support for 'support for shareable AEL code mainly between internal and external modules' in Asterisk.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk13-res-agi
Submenu: Telephony
Version: 13.9.1-1
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk13
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: libxml2/host
Section: net
Category: Network
Repository: base
Title: Asterisk Gateway Interface support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-13.9.1.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: This package provides support for 'Support for the Asterisk Gateway Interface extension' in Asterisk.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk13-res-calendar
Submenu: Telephony
Version: 13.9.1-1
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk13
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: libxml2/host
Section: net
Category: Network
Repository: base
Title: Calendaring API support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-13.9.1.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: This package provides support for 'Calendaring support (ICal and Google Calendar)' in Asterisk.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk13-res-clioriginate
Submenu: Telephony
Version: 13.9.1-1
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk13
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: libxml2/host
Section: net
Category: Network
Repository: base
Title: Calls via CLI support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-13.9.1.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: This package provides support for 'Originate calls via the CLI' in Asterisk.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk13-res-hep
Submenu: Telephony
Version: 13.9.1-1
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk13
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: libxml2/host
Section: net
Category: Network
Repository: base
Title: HEPv3 API support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-13.9.1.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: This package provides support for '' in Asterisk.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk13-res-hep-pjsip
Submenu: Telephony
Version: 13.9.1-1
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk13 +PACKAGE_asterisk13-res-hep-pjsip:asterisk13-res-hep +PACKAGE_asterisk13-res-hep-pjsip:asterisk13-pjsip
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: libxml2/host
Section: net
Category: Network
Repository: base
Title: PJSIP HEPv3 Logger support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-13.9.1.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: This package provides support for '' in Asterisk.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk13-res-hep-rtcp
Submenu: Telephony
Version: 13.9.1-1
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk13 +PACKAGE_asterisk13-res-hep-rtcp:asterisk13-res-hep
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: libxml2/host
Section: net
Category: Network
Repository: base
Title: RTCP HEPv3 Logger support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-13.9.1.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: This package provides support for '' in Asterisk.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk13-res-http-websocket
Submenu: Telephony
Version: 13.9.1-1
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk13
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: libxml2/host
Section: net
Category: Network
Repository: base
Title: HTTP websocket support support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-13.9.1.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: This package provides support for '' in Asterisk.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk13-res-monitor
Submenu: Telephony
Version: 13.9.1-1
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk13
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: libxml2/host
Section: net
Category: Network
Repository: base
Title: Provide Monitor support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-13.9.1.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: This package provides support for 'Cryptographic Signature capability' in Asterisk.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk13-res-musiconhold
Submenu: Telephony
Version: 13.9.1-1
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk13
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: libxml2/host
Section: net
Category: Network
Repository: base
Title: MOH support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-13.9.1.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: This package provides support for 'Music On Hold support' in Asterisk.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk13-res-parking
Submenu: Telephony
Version: 13.9.1-1
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk13
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: libxml2/host
Section: net
Category: Network
Repository: base
Title: Phone Parking support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-13.9.1.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: This package provides support for 'Phone Parking application' in Asterisk.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk13-res-phoneprov
Submenu: Telephony
Version: 13.9.1-1
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk13
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: libxml2/host
Section: net
Category: Network
Repository: base
Title: Phone Provisioning support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-13.9.1.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: This package provides support for 'Phone provisioning application for the asterisk internal http server' in Asterisk.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk13-res-realtime
Submenu: Telephony
Version: 13.9.1-1
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk13
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: libxml2/host
Section: net
Category: Network
Repository: base
Title: Realtime support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-13.9.1.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: This package provides support for 'Realtime Interface' in Asterisk.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk13-res-rtp-asterisk
Submenu: Telephony
Version: 13.9.1-1
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk13 +PACKAGE_asterisk13-res-rtp-asterisk:libpjsip +PACKAGE_asterisk13-res-rtp-asterisk:libpjmedia +PACKAGE_asterisk13-res-rtp-asterisk:libpjnath +PACKAGE_asterisk13-res-rtp-asterisk:libpjsip-simple +PACKAGE_asterisk13-res-rtp-asterisk:libpjsip-ua +PACKAGE_asterisk13-res-rtp-asterisk:libpjsua +PACKAGE_asterisk13-res-rtp-asterisk:libpjsua2
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: libxml2/host
Section: net
Category: Network
Repository: base
Title: RTP stack support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-13.9.1.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: This package provides support for '' in Asterisk.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk13-res-rtp-multicast
Submenu: Telephony
Version: 13.9.1-1
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk13
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: libxml2/host
Section: net
Category: Network
Repository: base
Title: RTP multicast engine support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-13.9.1.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: This package provides support for '' in Asterisk.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk13-res-smdi
Submenu: Telephony
Version: 13.9.1-1
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk13
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: libxml2/host
Section: net
Category: Network
Repository: base
Title: Provide SMDI support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-13.9.1.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: This package provides support for 'Simple Message Desk Interface capability' in Asterisk.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk13-res-sorcery
Submenu: Telephony
Version: 13.9.1-1
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk13
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: libxml2/host
Section: net
Category: Network
Repository: base
Title: Sorcery data layer support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-13.9.1.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: This package provides support for '' in Asterisk.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk13-res-srtp
Submenu: Telephony
Version: 13.9.1-1
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk13 +PACKAGE_asterisk13-res-srtp:libsrtp
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: libxml2/host
Section: net
Category: Network
Repository: base
Title: SRTP Support support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-13.9.1.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: This package provides support for 'Secure RTP connection' in Asterisk.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk13-res-timing-dahdi
Submenu: Telephony
Version: 13.9.1-1
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk13 +PACKAGE_asterisk13-res-timing-dahdi:asterisk13-chan-dahdi
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: libxml2/host
Section: net
Category: Network
Repository: base
Title: DAHDI Timing Interface support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-13.9.1.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: This package provides support for '' in Asterisk.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk13-res-timing-pthread
Submenu: Telephony
Version: 13.9.1-1
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk13
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: libxml2/host
Section: net
Category: Network
Repository: base
Title: pthread Timing Interface support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-13.9.1.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: This package provides support for '' in Asterisk.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk13-res-timing-timerfd
Submenu: Telephony
Version: 13.9.1-1
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk13
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: libxml2/host
Section: net
Category: Network
Repository: base
Title: Timerfd Timing Interface support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-13.9.1.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: This package provides support for '' in Asterisk.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: asterisk13-voicemail
Submenu: Telephony
Version: 13.9.1-1
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread asterisk13 +PACKAGE_asterisk13-voicemail:asterisk13-res-adsi +PACKAGE_asterisk13-voicemail:asterisk13-res-smdi
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: libxml2/host
Section: net
Category: Network
Repository: base
Title: Voicemail support
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: asterisk-13.9.1.tar.gz
License: GPL-2.0
LicenseFiles: COPYING LICENSE
Type: ipkg
Description: This package provides support for 'voicemail related modules' in Asterisk.
http://www.asterisk.org/
Jiri Slachta <jiri@slachta.eu>
@@


