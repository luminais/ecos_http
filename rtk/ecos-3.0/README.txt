eCos - the Embedded Configurable Operating System - release 3.0 README
======================================================================

March 2009


Welcome to the eCos 3.0 public release. This README contains a list of known
problems with the eCos 3.0 release. Please check for further issues by
searching the Bugzilla database for product "eCos" version "3.0":

  http://bugs.ecos.sourceware.org/query.cgi

If you discover new bugs with this release please report them using Bugzilla:

  http://bugs.ecos.sourceware.org/enter_bug.cgi


------------------------------------------------------------------------------
System Requirements
-------------------

This release has been tested against Microsoft Windows 2000, Windows XP,
Windows Vista and multiple x86 Linux distributions. Microsoft Windows 95,
Windows 98 and Windows ME are not supported.

The Linux-hosted eCos development tools require libstdc++ v3
(/usr/lib/libstdc++.so.5). Users of Linux distributions which provide a more
recent libstdc++ may need to install a libstdc++ v3 compatibility package.
Installation of the compatibility package may be achieved as follows:

  Fedora:       yum install compat-libstdc++-33
  openSUSE:     zypper install compat-libstdc++
  Ubuntu:       apt-get install libstdc++5

The Linux-hosted eCos Configuration Tool also requires the GTK+ toolkit
version 2.0 or later.

The Microsoft Windows-hosted eCos development tools require a recent
installation of the Cygwin compatibility layer and associated command-line
tools. The Cygwin setup program (installer) may be downloaded at:

  http://www.cygwin.com

An installation of these tools dating from September 2008 or later is
recommended. In addition to the packages provided within the "Base" category
of the Cygwin installer, the following packages must be installed for correct
operation of the eCos installer, eCos host tools, GNU compilation tools and
eCos build system:

  gcc libmpfr1 libpng12 make patch tcltk sharutils wget


------------------------------------------------------------------------------
eCos 3.0 Errata
---------------

* Compilation of eCos has been tested for those targets which use the
following GNU toolchains:

  arm-eabi arm-elf i386-elf mipsisa32-elf m68k-elf powerpc-eabi sh-elf

Compilation of eCos for targets using other toolchains is untested and may
not work correctly.

* The pre-built arm-eabi toolchain does not support ARM7DI and StrongARM
processors. When building eCos for such targets, developers are advised to
use the older arm-elf toolchain based on GCC 3.2.1.

* Occasional internal compiler errors have been observed when building eCos
and tests with the prebuilt m68k-elf toolchain. Such errors are highly
sensitive to any changes in the source code. In many cases, the compiler
optimisation level may be reduced as a workaround. However, note that
building the kernel file clock.cxx without optimisation when configured with
CYGDBG_USE_ASSERTS will also trigger an internal compiler error. In such
cases, removal of the the compiler flag '-fomit-frame-pointer' may serve as a
workaround.

* The cxxsupp test fails for target 'linux' (the synthetic target) on certain
recent Linux distributions (eg Fedora 10). This failure arises when libgcc
assumes that glibc has initialised the GS register to reference per-thread
data.

* Compilation of the eCos C library for target 'linux' (the synthetic target)
is known to fail when using certain older versions of GCC (eg GCC 3.2) due to
a duplicate definition of getc().

* Compilation of eCos for targets 'atlas_mips64_5kc' and 'malta_mips64_5kc'
fails due to various errors involving a loss of precision while casting.

* Compilation of eCos for targets 'cma28x' and 'fads' fails due to a
dependency of CYGPKG_HAL_POWERPC_MPC8xx on CYGPKG_HAL_QUICC.

* Compilation of eCos for target 'iq80310' fails due to multiple coding
issues within the platform support files.

* The eCos tests 'pselect' and 'cpuload' may fail erroneously on some eCos
targets (false negative).

* eCos lwIP tests may fail to build when CYGDBG_LWIP_DEBUG is enabled due to
a dependency on snprintf().

* The eCos tests 'except1' and 'kexcept1' may fail on certain processors
where an exception is not raised in response to bad alignment (for example).

* The eCos math library test 'pow' is known to fail on SH4 hardware when
using the prebuilt sh-elf toolchain. The cause is unknown but the problem
does not occur with older toolchains.

* The MPC8xx test 'intr0' fails on many MPC8xx targets since the test is not
generalised for correct operation will all MPC8xx CPUs.

* The PSIM PowerPC simulator treats all data cache instructions as noops.
The behaviour is benign with the exception of the dcbz instruction. It causes
the eCos 'kcache2' test to fail on target 'psim'.

* There are a number of minor issues with the eCos Configuration Tool:

  84946 Configtool build progress bar inoperative
  89778 Configtool platforms list is not sorted
  1000619 Configuration tree does not respond to scroll wheel
  1000716 Error launching GDB to run eCos tests
