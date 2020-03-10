Source-Makefile: feeds/telephony/net/kamailio-4.x/Makefile
Package: kamailio4
Menu: 1
Submenu: Telephony
Version: 4.4.0-1
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread +BUILD_NLS:libiconv-full +libncurses +libpthread +libreadline +libxml2
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: !BUILD_NLS:libiconv !BUILD_NLS:libintl
Section: net
Category: Network
Repository: base
Title: Mature and flexible open source SIP server, v4.4.0
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-4.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: Mature and flexible open source SIP server, v4.4.0
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio4-mod-acc
Submenu: Telephony
Version: 4.4.0-1
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio4 +kamailio4-mod-tm
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: !BUILD_NLS:libiconv !BUILD_NLS:libintl
Section: net
Category: Network
Repository: base
Title: kamailio4 Accounting module
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-4.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: kamailio4 Accounting module
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio4-mod-alias-db
Submenu: Telephony
Version: 4.4.0-1
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio4 +kamailio4-mod-db-sqlite
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: !BUILD_NLS:libiconv !BUILD_NLS:libintl
Section: net
Category: Network
Repository: base
Title: kamailio4 Database-backend aliases module
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-4.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: kamailio4 Database-backend aliases module
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio4-mod-auth
Submenu: Telephony
Version: 4.4.0-1
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio4
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: !BUILD_NLS:libiconv !BUILD_NLS:libintl
Section: net
Category: Network
Repository: base
Title: kamailio4 Authentication Framework module
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-4.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: kamailio4 Authentication Framework module
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio4-mod-auth-db
Submenu: Telephony
Version: 4.4.0-1
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio4 +kamailio4-mod-auth +kamailio4-mod-db-sqlite
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: !BUILD_NLS:libiconv !BUILD_NLS:libintl
Section: net
Category: Network
Repository: base
Title: kamailio4 Database-backend authentication module
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-4.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: kamailio4 Database-backend authentication module
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio4-mod-auth-diameter
Submenu: Telephony
Version: 4.4.0-1
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio4 +kamailio4-mod-sl
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: !BUILD_NLS:libiconv !BUILD_NLS:libintl
Section: net
Category: Network
Repository: base
Title: kamailio4 Diameter-backend authentication module
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-4.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: kamailio4 Diameter-backend authentication module
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio4-mod-auth-xkeys
Submenu: Telephony
Version: 4.4.0-1
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio4 +kamailio4-mod-auth
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: !BUILD_NLS:libiconv !BUILD_NLS:libintl
Section: net
Category: Network
Repository: base
Title: kamailio4 Shared-key authentication module
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-4.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: kamailio4 Shared-key authentication module
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio4-mod-avpops
Submenu: Telephony
Version: 4.4.0-1
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio4
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: !BUILD_NLS:libiconv !BUILD_NLS:libintl
Section: net
Category: Network
Repository: base
Title: kamailio4 AVP operation module
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-4.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: kamailio4 AVP operation module
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio4-mod-benchmark
Submenu: Telephony
Version: 4.4.0-1
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio4
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: !BUILD_NLS:libiconv !BUILD_NLS:libintl
Section: net
Category: Network
Repository: base
Title: kamailio4 Config benchmark module
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-4.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: kamailio4 Config benchmark module
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio4-mod-cfgutils
Submenu: Telephony
Version: 4.4.0-1
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio4
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: !BUILD_NLS:libiconv !BUILD_NLS:libintl
Section: net
Category: Network
Repository: base
Title: kamailio4 Config utilities module
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-4.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: kamailio4 Config utilities module
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio4-mod-cfg-db
Submenu: Telephony
Version: 4.4.0-1
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio4 +kamailio4-mod-db-sqlite
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: !BUILD_NLS:libiconv !BUILD_NLS:libintl
Section: net
Category: Network
Repository: base
Title: kamailio4 Load core and module parameters from database module
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-4.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: kamailio4 Load core and module parameters from database module
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio4-mod-cfg-rpc
Submenu: Telephony
Version: 4.4.0-1
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio4
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: !BUILD_NLS:libiconv !BUILD_NLS:libintl
Section: net
Category: Network
Repository: base
Title: kamailio4 Update core and module parameters at runtime via RPC interface module
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-4.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: kamailio4 Update core and module parameters at runtime via RPC interface module
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio4-mod-cnxcc
Submenu: Telephony
Version: 4.4.0-1
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio4 +kamailio4-mod-dialog +libhiredis +libevent2
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: !BUILD_NLS:libiconv !BUILD_NLS:libintl
Section: net
Category: Network
Repository: base
Title: kamailio4 Limit call duration module
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-4.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: kamailio4 Limit call duration module
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio4-mod-corex
Submenu: Telephony
Version: 4.4.0-1
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio4
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: !BUILD_NLS:libiconv !BUILD_NLS:libintl
Section: net
Category: Network
Repository: base
Title: kamailio4 Legacy functions module
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-4.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: kamailio4 Legacy functions module
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio4-mod-ctl
Submenu: Telephony
Version: 4.4.0-1
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio4
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: !BUILD_NLS:libiconv !BUILD_NLS:libintl
Section: net
Category: Network
Repository: base
Title: kamailio4 BINRPC transport interface module
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-4.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: kamailio4 BINRPC transport interface module
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio4-mod-db-flatstore
Submenu: Telephony
Version: 4.4.0-1
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio4
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: !BUILD_NLS:libiconv !BUILD_NLS:libintl
Section: net
Category: Network
Repository: base
Title: kamailio4 Fast writing-only text database-backed module
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-4.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: kamailio4 Fast writing-only text database-backed module
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio4-mod-db-mysql
Submenu: Telephony
Version: 4.4.0-1
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio4 +libmysqlclient
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: !BUILD_NLS:libiconv !BUILD_NLS:libintl
Section: net
Category: Network
Repository: base
Title: kamailio4 MySQL database-backend module
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-4.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: kamailio4 MySQL database-backend module
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio4-mod-db-postgres
Submenu: Telephony
Version: 4.4.0-1
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio4 +libpq
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: !BUILD_NLS:libiconv !BUILD_NLS:libintl
Section: net
Category: Network
Repository: base
Title: kamailio4 PostgreSQL Database-backend module
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-4.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: kamailio4 PostgreSQL Database-backend module
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio4-mod-db-sqlite
Submenu: Telephony
Version: 4.4.0-1
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio4 +libsqlite3
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: !BUILD_NLS:libiconv !BUILD_NLS:libintl
Section: net
Category: Network
Repository: base
Title: kamailio4 Sqlite DB support module
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-4.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: kamailio4 Sqlite DB support module
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio4-mod-db-text
Submenu: Telephony
Version: 4.4.0-1
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio4
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: !BUILD_NLS:libiconv !BUILD_NLS:libintl
Section: net
Category: Network
Repository: base
Title: kamailio4 Text database-backend module
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-4.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: kamailio4 Text database-backend module
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio4-mod-db-unixodbc
Submenu: Telephony
Version: 4.4.0-1
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio4 +unixodbc
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: !BUILD_NLS:libiconv !BUILD_NLS:libintl
Section: net
Category: Network
Repository: base
Title: kamailio4 UnixODBC Database-backend module
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-4.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: kamailio4 UnixODBC Database-backend module
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio4-mod-debugger
Submenu: Telephony
Version: 4.4.0-1
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio4
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: !BUILD_NLS:libiconv !BUILD_NLS:libintl
Section: net
Category: Network
Repository: base
Title: kamailio4 Interactive config file debugger module
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-4.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: kamailio4 Interactive config file debugger module
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio4-mod-dialog
Submenu: Telephony
Version: 4.4.0-1
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio4 +kamailio4-mod-rr +kamailio4-mod-tm
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: !BUILD_NLS:libiconv !BUILD_NLS:libintl
Section: net
Category: Network
Repository: base
Title: kamailio4 Dialog support module
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-4.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: kamailio4 Dialog support module
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio4-mod-dialog-ng
Submenu: Telephony
Version: 4.4.0-1
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio4 +kamailio4-mod-rr +kamailio4-mod-tm
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: !BUILD_NLS:libiconv !BUILD_NLS:libintl
Section: net
Category: Network
Repository: base
Title: kamailio4 Dialog support module
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-4.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: kamailio4 Dialog support module
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio4-mod-dialplan
Submenu: Telephony
Version: 4.4.0-1
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio4
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: !BUILD_NLS:libiconv !BUILD_NLS:libintl
Section: net
Category: Network
Repository: base
Title: kamailio4 Dialplan management module
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-4.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: kamailio4 Dialplan management module
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio4-mod-dispatcher
Submenu: Telephony
Version: 4.4.0-1
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio4
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: !BUILD_NLS:libiconv !BUILD_NLS:libintl
Section: net
Category: Network
Repository: base
Title: kamailio4 Dispatcher module
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-4.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: kamailio4 Dispatcher module
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio4-mod-diversion
Submenu: Telephony
Version: 4.4.0-1
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio4
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: !BUILD_NLS:libiconv !BUILD_NLS:libintl
Section: net
Category: Network
Repository: base
Title: kamailio4 Diversion header insertion module
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-4.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: kamailio4 Diversion header insertion module
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio4-mod-domain
Submenu: Telephony
Version: 4.4.0-1
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio4
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: !BUILD_NLS:libiconv !BUILD_NLS:libintl
Section: net
Category: Network
Repository: base
Title: kamailio4 Multi-domain support module
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-4.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: kamailio4 Multi-domain support module
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio4-mod-domainpolicy
Submenu: Telephony
Version: 4.4.0-1
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio4
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: !BUILD_NLS:libiconv !BUILD_NLS:libintl
Section: net
Category: Network
Repository: base
Title: kamailio4 Domain policy module
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-4.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: kamailio4 Domain policy module
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio4-mod-drouting
Submenu: Telephony
Version: 4.4.0-1
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio4
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: !BUILD_NLS:libiconv !BUILD_NLS:libintl
Section: net
Category: Network
Repository: base
Title: kamailio4 Dynamic routing module module
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-4.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: kamailio4 Dynamic routing module module
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio4-mod-enum
Submenu: Telephony
Version: 4.4.0-1
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio4
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: !BUILD_NLS:libiconv !BUILD_NLS:libintl
Section: net
Category: Network
Repository: base
Title: kamailio4 ENUM lookup module
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-4.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: kamailio4 ENUM lookup module
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio4-mod-evapi
Submenu: Telephony
Version: 4.4.0-1
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio4 +libev
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: !BUILD_NLS:libiconv !BUILD_NLS:libintl
Section: net
Category: Network
Repository: base
Title: kamailio4 push event details via tcp module
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-4.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: kamailio4 push event details via tcp module
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio4-mod-exec
Submenu: Telephony
Version: 4.4.0-1
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio4
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: !BUILD_NLS:libiconv !BUILD_NLS:libintl
Section: net
Category: Network
Repository: base
Title: kamailio4 External exec module
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-4.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: kamailio4 External exec module
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio4-mod-group
Submenu: Telephony
Version: 4.4.0-1
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio4
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: !BUILD_NLS:libiconv !BUILD_NLS:libintl
Section: net
Category: Network
Repository: base
Title: kamailio4 Database-backend user-groups module
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-4.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: kamailio4 Database-backend user-groups module
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio4-mod-h350
Submenu: Telephony
Version: 4.4.0-1
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio4 +kamailio4-mod-ldap +libopenldap
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: !BUILD_NLS:libiconv !BUILD_NLS:libintl
Section: net
Category: Network
Repository: base
Title: kamailio4 H.350 module
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-4.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: kamailio4 H.350 module
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio4-mod-htable
Submenu: Telephony
Version: 4.4.0-1
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio4
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: !BUILD_NLS:libiconv !BUILD_NLS:libintl
Section: net
Category: Network
Repository: base
Title: kamailio4 Hash Table module
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-4.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: kamailio4 Hash Table module
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio4-mod-imc
Submenu: Telephony
Version: 4.4.0-1
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio4 +kamailio4-mod-db-mysql +kamailio4-mod-tm
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: !BUILD_NLS:libiconv !BUILD_NLS:libintl
Section: net
Category: Network
Repository: base
Title: kamailio4 IM conferencing module
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-4.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: kamailio4 IM conferencing module
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio4-mod-ipops
Submenu: Telephony
Version: 4.4.0-1
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio4
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: !BUILD_NLS:libiconv !BUILD_NLS:libintl
Section: net
Category: Network
Repository: base
Title: kamailio4 IP and IPv6 operations module
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-4.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: kamailio4 IP and IPv6 operations module
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio4-mod-jansson
Submenu: Telephony
Version: 4.4.0-1
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio4 +jansson
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: !BUILD_NLS:libiconv !BUILD_NLS:libintl
Section: net
Category: Network
Repository: base
Title: kamailio4 Alternative access to JSON document attributes module
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-4.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: kamailio4 Alternative access to JSON document attributes module
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio4-mod-janssonrpc-c
Submenu: Telephony
Version: 4.4.0-1
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio4 +kamailio4-mod-jansson +libevent2
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: !BUILD_NLS:libiconv !BUILD_NLS:libintl
Section: net
Category: Network
Repository: base
Title: kamailio4 Alternative JSONRPC server module
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-4.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: kamailio4 Alternative JSONRPC server module
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio4-mod-json
Submenu: Telephony
Version: 4.4.0-1
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio4 +libjson-c
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: !BUILD_NLS:libiconv !BUILD_NLS:libintl
Section: net
Category: Network
Repository: base
Title: kamailio4 Access to JSON document attributes module
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-4.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: kamailio4 Access to JSON document attributes module
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio4-mod-jsonrpc-s
Submenu: Telephony
Version: 4.4.0-1
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio4 +kamailio4-mod-json +libevent2
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: !BUILD_NLS:libiconv !BUILD_NLS:libintl
Section: net
Category: Network
Repository: base
Title: kamailio4 JSONRPC server over HTTP module
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-4.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: kamailio4 JSONRPC server over HTTP module
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio4-mod-kex
Submenu: Telephony
Version: 4.4.0-1
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio4
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: !BUILD_NLS:libiconv !BUILD_NLS:libintl
Section: net
Category: Network
Repository: base
Title: kamailio4 Core extensions module
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-4.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: kamailio4 Core extensions module
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio4-mod-lcr
Submenu: Telephony
Version: 4.4.0-1
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio4 +kamailio4-mod-tm +libpcre
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: !BUILD_NLS:libiconv !BUILD_NLS:libintl
Section: net
Category: Network
Repository: base
Title: kamailio4 Least Cost Routing module
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-4.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: kamailio4 Least Cost Routing module
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio4-mod-ldap
Submenu: Telephony
Version: 4.4.0-1
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio4 +libopenldap
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: !BUILD_NLS:libiconv !BUILD_NLS:libintl
Section: net
Category: Network
Repository: base
Title: kamailio4 LDAP connector module
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-4.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: kamailio4 LDAP connector module
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio4-mod-maxfwd
Submenu: Telephony
Version: 4.4.0-1
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio4
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: !BUILD_NLS:libiconv !BUILD_NLS:libintl
Section: net
Category: Network
Repository: base
Title: kamailio4 Max-Forward processor module
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-4.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: kamailio4 Max-Forward processor module
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio4-mod-mediaproxy
Submenu: Telephony
Version: 4.4.0-1
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio4 +kamailio4-mod-dialog
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: !BUILD_NLS:libiconv !BUILD_NLS:libintl
Section: net
Category: Network
Repository: base
Title: kamailio4 Automatic NAT traversal module
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-4.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: kamailio4 Automatic NAT traversal module
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio4-mod-mi-datagram
Submenu: Telephony
Version: 4.4.0-1
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio4
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: !BUILD_NLS:libiconv !BUILD_NLS:libintl
Section: net
Category: Network
Repository: base
Title: kamailio4 Datagram support for Management Interface module
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-4.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: kamailio4 Datagram support for Management Interface module
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio4-mod-mi-fifo
Submenu: Telephony
Version: 4.4.0-1
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio4
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: !BUILD_NLS:libiconv !BUILD_NLS:libintl
Section: net
Category: Network
Repository: base
Title: kamailio4 FIFO support for Management Interface module
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-4.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: kamailio4 FIFO support for Management Interface module
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio4-mod-mi-rpc
Submenu: Telephony
Version: 4.4.0-1
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio4
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: !BUILD_NLS:libiconv !BUILD_NLS:libintl
Section: net
Category: Network
Repository: base
Title: kamailio4 RPC support for Management Interface module
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-4.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: kamailio4 RPC support for Management Interface module
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio4-mod-msilo
Submenu: Telephony
Version: 4.4.0-1
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio4 +kamailio4-mod-tm
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: !BUILD_NLS:libiconv !BUILD_NLS:libintl
Section: net
Category: Network
Repository: base
Title: kamailio4 SIP message silo module
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-4.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: kamailio4 SIP message silo module
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio4-mod-msrp
Submenu: Telephony
Version: 4.4.0-1
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio4 +kamailio4-mod-tls
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: !BUILD_NLS:libiconv !BUILD_NLS:libintl
Section: net
Category: Network
Repository: base
Title: kamailio4 MSRP routing engine module
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-4.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: kamailio4 MSRP routing engine module
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio4-mod-nathelper
Submenu: Telephony
Version: 4.4.0-1
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio4 +kamailio4-mod-usrloc
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: !BUILD_NLS:libiconv !BUILD_NLS:libintl
Section: net
Category: Network
Repository: base
Title: kamailio4 NAT helper module
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-4.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: kamailio4 NAT helper module
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio4-mod-nat-traversal
Submenu: Telephony
Version: 4.4.0-1
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio4 +kamailio4-mod-dialog +kamailio4-mod-sl +kamailio4-mod-tm
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: !BUILD_NLS:libiconv !BUILD_NLS:libintl
Section: net
Category: Network
Repository: base
Title: kamailio4 NAT traversal module
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-4.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: kamailio4 NAT traversal module
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio4-mod-nosip
Submenu: Telephony
Version: 4.4.0-1
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio4 +kamailio4-mod-rr
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: !BUILD_NLS:libiconv !BUILD_NLS:libintl
Section: net
Category: Network
Repository: base
Title: kamailio4 non-sip package handling module
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-4.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: kamailio4 non-sip package handling module
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio4-mod-path
Submenu: Telephony
Version: 4.4.0-1
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio4 +kamailio4-mod-rr
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: !BUILD_NLS:libiconv !BUILD_NLS:libintl
Section: net
Category: Network
Repository: base
Title: kamailio4 SIP path insertion module
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-4.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: kamailio4 SIP path insertion module
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio4-mod-pdt
Submenu: Telephony
Version: 4.4.0-1
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio4
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: !BUILD_NLS:libiconv !BUILD_NLS:libintl
Section: net
Category: Network
Repository: base
Title: kamailio4 Prefix-to-Domain translator module
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-4.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: kamailio4 Prefix-to-Domain translator module
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio4-mod-permissions
Submenu: Telephony
Version: 4.4.0-1
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio4
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: !BUILD_NLS:libiconv !BUILD_NLS:libintl
Section: net
Category: Network
Repository: base
Title: kamailio4 Permissions control module
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-4.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: kamailio4 Permissions control module
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio4-mod-pike
Submenu: Telephony
Version: 4.4.0-1
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio4
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: !BUILD_NLS:libiconv !BUILD_NLS:libintl
Section: net
Category: Network
Repository: base
Title: kamailio4 Flood detector module
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-4.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: kamailio4 Flood detector module
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio4-mod-presence
Submenu: Telephony
Version: 4.4.0-1
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio4 +kamailio4-mod-sl +kamailio4-mod-tm +libxml2
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: !BUILD_NLS:libiconv !BUILD_NLS:libintl
Section: net
Category: Network
Repository: base
Title: kamailio4 Presence server module
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-4.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: kamailio4 Presence server module
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio4-mod-presence-dialoginfo
Submenu: Telephony
Version: 4.4.0-1
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio4 +kamailio4-mod-presence
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: !BUILD_NLS:libiconv !BUILD_NLS:libintl
Section: net
Category: Network
Repository: base
Title: kamailio4 Dialog Event presence module
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-4.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: kamailio4 Dialog Event presence module
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio4-mod-presence-mwi
Submenu: Telephony
Version: 4.4.0-1
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio4 +kamailio4-mod-presence
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: !BUILD_NLS:libiconv !BUILD_NLS:libintl
Section: net
Category: Network
Repository: base
Title: kamailio4 Message Waiting Indication presence module
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-4.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: kamailio4 Message Waiting Indication presence module
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio4-mod-presence-xml
Submenu: Telephony
Version: 4.4.0-1
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio4 +kamailio4-mod-presence +kamailio4-mod-xcap-client
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: !BUILD_NLS:libiconv !BUILD_NLS:libintl
Section: net
Category: Network
Repository: base
Title: kamailio4 XCAP presence module
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-4.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: kamailio4 XCAP presence module
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio4-mod-pua
Submenu: Telephony
Version: 4.4.0-1
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio4 +kamailio4-mod-tm +libxml2
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: !BUILD_NLS:libiconv !BUILD_NLS:libintl
Section: net
Category: Network
Repository: base
Title: kamailio4 Presence User Agent module
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-4.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: kamailio4 Presence User Agent module
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio4-mod-pua-bla
Submenu: Telephony
Version: 4.4.0-1
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio4 +kamailio4-mod-presence +kamailio4-mod-pua +kamailio4-mod-usrloc
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: !BUILD_NLS:libiconv !BUILD_NLS:libintl
Section: net
Category: Network
Repository: base
Title: kamailio4 Bridged Line Appearence PUA module
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-4.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: kamailio4 Bridged Line Appearence PUA module
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio4-mod-pua-dialoginfo
Submenu: Telephony
Version: 4.4.0-1
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio4 +kamailio4-mod-dialog +kamailio4-mod-pua
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: !BUILD_NLS:libiconv !BUILD_NLS:libintl
Section: net
Category: Network
Repository: base
Title: kamailio4 Dialog Event PUA module
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-4.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: kamailio4 Dialog Event PUA module
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio4-mod-pua-mi
Submenu: Telephony
Version: 4.4.0-1
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio4 +kamailio4-mod-pua
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: !BUILD_NLS:libiconv !BUILD_NLS:libintl
Section: net
Category: Network
Repository: base
Title: kamailio4 PUA Management Interface module
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-4.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: kamailio4 PUA Management Interface module
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio4-mod-pua-usrloc
Submenu: Telephony
Version: 4.4.0-1
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio4 +kamailio4-mod-pua +kamailio4-mod-usrloc
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: !BUILD_NLS:libiconv !BUILD_NLS:libintl
Section: net
Category: Network
Repository: base
Title: kamailio4 PUA User Location module
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-4.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: kamailio4 PUA User Location module
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio4-mod-pua-xmpp
Submenu: Telephony
Version: 4.4.0-1
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio4 +kamailio4-mod-presence +kamailio4-mod-pua +kamailio4-mod-xmpp
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: !BUILD_NLS:libiconv !BUILD_NLS:libintl
Section: net
Category: Network
Repository: base
Title: kamailio4 PUA XMPP module
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-4.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: kamailio4 PUA XMPP module
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio4-mod-pv
Submenu: Telephony
Version: 4.4.0-1
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio4
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: !BUILD_NLS:libiconv !BUILD_NLS:libintl
Section: net
Category: Network
Repository: base
Title: kamailio4 Pseudo-Variables module
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-4.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: kamailio4 Pseudo-Variables module
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio4-mod-qos
Submenu: Telephony
Version: 4.4.0-1
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio4 +kamailio4-mod-dialog
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: !BUILD_NLS:libiconv !BUILD_NLS:libintl
Section: net
Category: Network
Repository: base
Title: kamailio4 QoS control module
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-4.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: kamailio4 QoS control module
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio4-mod-ratelimit
Submenu: Telephony
Version: 4.4.0-1
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio4
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: !BUILD_NLS:libiconv !BUILD_NLS:libintl
Section: net
Category: Network
Repository: base
Title: kamailio4 Traffic shapping module
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-4.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: kamailio4 Traffic shapping module
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio4-mod-regex
Submenu: Telephony
Version: 4.4.0-1
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio4 +libpcre
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: !BUILD_NLS:libiconv !BUILD_NLS:libintl
Section: net
Category: Network
Repository: base
Title: kamailio4 Regular Expression module
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-4.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: kamailio4 Regular Expression module
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio4-mod-registrar
Submenu: Telephony
Version: 4.4.0-1
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio4 +kamailio4-mod-usrloc
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: !BUILD_NLS:libiconv !BUILD_NLS:libintl
Section: net
Category: Network
Repository: base
Title: kamailio4 SIP Registrar module
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-4.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: kamailio4 SIP Registrar module
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio4-mod-rls
Submenu: Telephony
Version: 4.4.0-1
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio4 +kamailio4-mod-presence +kamailio4-mod-pua +kamailio4-mod-tm +libxml2
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: !BUILD_NLS:libiconv !BUILD_NLS:libintl
Section: net
Category: Network
Repository: base
Title: kamailio4 Resource List Server module
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-4.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: kamailio4 Resource List Server module
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio4-mod-rr
Submenu: Telephony
Version: 4.4.0-1
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio4
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: !BUILD_NLS:libiconv !BUILD_NLS:libintl
Section: net
Category: Network
Repository: base
Title: kamailio4 Record-Route and Route module
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-4.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: kamailio4 Record-Route and Route module
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio4-mod-rtimer
Submenu: Telephony
Version: 4.4.0-1
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio4
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: !BUILD_NLS:libiconv !BUILD_NLS:libintl
Section: net
Category: Network
Repository: base
Title: kamailio4 Routing Timer module
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-4.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: kamailio4 Routing Timer module
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio4-mod-rtpengine
Submenu: Telephony
Version: 4.4.0-1
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio4 +kamailio4-mod-tm
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: !BUILD_NLS:libiconv !BUILD_NLS:libintl
Section: net
Category: Network
Repository: base
Title: kamailio4 RTP engine module
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-4.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: kamailio4 RTP engine module
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio4-mod-rtpproxy
Submenu: Telephony
Version: 4.4.0-1
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio4 +kamailio4-mod-tm
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: !BUILD_NLS:libiconv !BUILD_NLS:libintl
Section: net
Category: Network
Repository: base
Title: kamailio4 RTP proxy module
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-4.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: kamailio4 RTP proxy module
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio4-mod-sanity
Submenu: Telephony
Version: 4.4.0-1
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio4 +kamailio4-mod-sl
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: !BUILD_NLS:libiconv !BUILD_NLS:libintl
Section: net
Category: Network
Repository: base
Title: kamailio4 SIP sanity checks module
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-4.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: kamailio4 SIP sanity checks module
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio4-mod-sctp
Submenu: Telephony
Version: 4.4.0-1
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio4 +libsctp
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: !BUILD_NLS:libiconv !BUILD_NLS:libintl
Section: net
Category: Network
Repository: base
Title: kamailio4 SCTP support module
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-4.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: kamailio4 SCTP support module
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio4-mod-sipcapture
Submenu: Telephony
Version: 4.4.0-1
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio4
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: !BUILD_NLS:libiconv !BUILD_NLS:libintl
Section: net
Category: Network
Repository: base
Title: kamailio4 SIP capture module
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-4.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: kamailio4 SIP capture module
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio4-mod-siptrace
Submenu: Telephony
Version: 4.4.0-1
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio4
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: !BUILD_NLS:libiconv !BUILD_NLS:libintl
Section: net
Category: Network
Repository: base
Title: kamailio4 SIP trace module
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-4.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: kamailio4 SIP trace module
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio4-mod-siputils
Submenu: Telephony
Version: 4.4.0-1
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio4 +kamailio4-mod-sl
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: !BUILD_NLS:libiconv !BUILD_NLS:libintl
Section: net
Category: Network
Repository: base
Title: kamailio4 SIP utilities module
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-4.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: kamailio4 SIP utilities module
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio4-mod-sl
Submenu: Telephony
Version: 4.4.0-1
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio4
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: !BUILD_NLS:libiconv !BUILD_NLS:libintl
Section: net
Category: Network
Repository: base
Title: kamailio4 Stateless replier module
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-4.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: kamailio4 Stateless replier module
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio4-mod-sms
Submenu: Telephony
Version: 4.4.0-1
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio4 +kamailio4-mod-tm
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: !BUILD_NLS:libiconv !BUILD_NLS:libintl
Section: net
Category: Network
Repository: base
Title: kamailio4 SIP-to-SMS IM gateway module
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-4.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: kamailio4 SIP-to-SMS IM gateway module
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio4-mod-speeddial
Submenu: Telephony
Version: 4.4.0-1
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio4
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: !BUILD_NLS:libiconv !BUILD_NLS:libintl
Section: net
Category: Network
Repository: base
Title: kamailio4 Per-user speed-dial controller module
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-4.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: kamailio4 Per-user speed-dial controller module
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio4-mod-sqlops
Submenu: Telephony
Version: 4.4.0-1
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio4
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: !BUILD_NLS:libiconv !BUILD_NLS:libintl
Section: net
Category: Network
Repository: base
Title: kamailio4 SQL operations module
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-4.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: kamailio4 SQL operations module
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio4-mod-statistics
Submenu: Telephony
Version: 4.4.0-1
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio4
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: !BUILD_NLS:libiconv !BUILD_NLS:libintl
Section: net
Category: Network
Repository: base
Title: kamailio4 Script statistics module
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-4.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: kamailio4 Script statistics module
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio4-mod-stun
Submenu: Telephony
Version: 4.4.0-1
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio4
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: !BUILD_NLS:libiconv !BUILD_NLS:libintl
Section: net
Category: Network
Repository: base
Title: kamailio4 STUN server support module
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-4.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: kamailio4 STUN server support module
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio4-mod-sst
Submenu: Telephony
Version: 4.4.0-1
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio4 +kamailio4-mod-dialog +kamailio4-mod-sl
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: !BUILD_NLS:libiconv !BUILD_NLS:libintl
Section: net
Category: Network
Repository: base
Title: kamailio4 SIP Session Timer module
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-4.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: kamailio4 SIP Session Timer module
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio4-mod-tcpops
Submenu: Telephony
Version: 4.4.0-1
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio4
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: !BUILD_NLS:libiconv !BUILD_NLS:libintl
Section: net
Category: Network
Repository: base
Title: kamailio4 TCP options tweaking operations module
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-4.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: kamailio4 TCP options tweaking operations module
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio4-mod-textops
Submenu: Telephony
Version: 4.4.0-1
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio4
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: !BUILD_NLS:libiconv !BUILD_NLS:libintl
Section: net
Category: Network
Repository: base
Title: kamailio4 Text operations module
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-4.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: kamailio4 Text operations module
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio4-mod-tls
Submenu: Telephony
Version: 4.4.0-1
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio4 +libopenssl
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: !BUILD_NLS:libiconv !BUILD_NLS:libintl
Section: net
Category: Network
Repository: base
Title: kamailio4 TLS operations module
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-4.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: kamailio4 TLS operations module
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio4-mod-topoh
Submenu: Telephony
Version: 4.4.0-1
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio4 +kamailio4-mod-rr
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: !BUILD_NLS:libiconv !BUILD_NLS:libintl
Section: net
Category: Network
Repository: base
Title: kamailio4 Topology hiding module
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-4.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: kamailio4 Topology hiding module
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio4-mod-tm
Submenu: Telephony
Version: 4.4.0-1
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio4
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: !BUILD_NLS:libiconv !BUILD_NLS:libintl
Section: net
Category: Network
Repository: base
Title: kamailio4 Transaction module
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-4.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: kamailio4 Transaction module
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio4-mod-tmx
Submenu: Telephony
Version: 4.4.0-1
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio4
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: !BUILD_NLS:libiconv !BUILD_NLS:libintl
Section: net
Category: Network
Repository: base
Title: kamailio4 Transaction module extensions module
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-4.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: kamailio4 Transaction module extensions module
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio4-mod-uac
Submenu: Telephony
Version: 4.4.0-1
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio4 +kamailio4-mod-tm
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: !BUILD_NLS:libiconv !BUILD_NLS:libintl
Section: net
Category: Network
Repository: base
Title: kamailio4 User Agent Client module
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-4.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: kamailio4 User Agent Client module
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio4-mod-uac-redirect
Submenu: Telephony
Version: 4.4.0-1
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio4 +kamailio4-mod-tm
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: !BUILD_NLS:libiconv !BUILD_NLS:libintl
Section: net
Category: Network
Repository: base
Title: kamailio4 User Agent Client redirection module
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-4.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: kamailio4 User Agent Client redirection module
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio4-mod-uri-db
Submenu: Telephony
Version: 4.4.0-1
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio4
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: !BUILD_NLS:libiconv !BUILD_NLS:libintl
Section: net
Category: Network
Repository: base
Title: kamailio4 Database-backend SIP URI checking module
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-4.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: kamailio4 Database-backend SIP URI checking module
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio4-mod-userblacklist
Submenu: Telephony
Version: 4.4.0-1
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio4
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: !BUILD_NLS:libiconv !BUILD_NLS:libintl
Section: net
Category: Network
Repository: base
Title: kamailio4 User blacklists module
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-4.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: kamailio4 User blacklists module
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio4-mod-usrloc
Submenu: Telephony
Version: 4.4.0-1
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio4
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: !BUILD_NLS:libiconv !BUILD_NLS:libintl
Section: net
Category: Network
Repository: base
Title: kamailio4 User location module
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-4.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: kamailio4 User location module
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio4-mod-utils
Submenu: Telephony
Version: 4.4.0-1
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio4 +libcurl +libxml2
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: !BUILD_NLS:libiconv !BUILD_NLS:libintl
Section: net
Category: Network
Repository: base
Title: kamailio4 Misc utilities module
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-4.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: kamailio4 Misc utilities module
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio4-mod-uuid
Submenu: Telephony
Version: 4.4.0-1
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio4 +libuuid
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: !BUILD_NLS:libiconv !BUILD_NLS:libintl
Section: net
Category: Network
Repository: base
Title: kamailio4 UUID utilities module
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-4.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: kamailio4 UUID utilities module
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio4-mod-xcap-client
Submenu: Telephony
Version: 4.4.0-1
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio4 +libcurl
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: !BUILD_NLS:libiconv !BUILD_NLS:libintl
Section: net
Category: Network
Repository: base
Title: kamailio4 XCAP Client module
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-4.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: kamailio4 XCAP Client module
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio4-mod-xlog
Submenu: Telephony
Version: 4.4.0-1
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio4
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: !BUILD_NLS:libiconv !BUILD_NLS:libintl
Section: net
Category: Network
Repository: base
Title: kamailio4 Advanced logger module
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-4.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: kamailio4 Advanced logger module
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio4-mod-xmlrpc
Submenu: Telephony
Version: 4.4.0-1
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio4 +libxml2
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: !BUILD_NLS:libiconv !BUILD_NLS:libintl
Section: net
Category: Network
Repository: base
Title: kamailio4 XML RPC module module
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-4.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: kamailio4 XML RPC module module
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@

Package: kamailio4-mod-xmpp
Submenu: Telephony
Version: 4.4.0-1
Depends: +libc +SSP_SUPPORT:libssp +USE_GLIBC:librt +USE_GLIBC:libpthread kamailio4 +kamailio4-mod-tm +libexpat
Conflicts: 
Menu-Depends: 
Provides: 
Build-Depends: !BUILD_NLS:libiconv !BUILD_NLS:libintl
Section: net
Category: Network
Repository: base
Title: kamailio4 SIP-to-XMPP Gateway module
Maintainer: Jiri Slachta <jiri@slachta.eu>
Source: kamailio-4.4.0_src.tar.gz
License: GPL-2.0+
LicenseFiles: COPYING
Type: ipkg
Description: kamailio4 SIP-to-XMPP Gateway module
http://www.kamailio.org/
Jiri Slachta <jiri@slachta.eu>
@@


