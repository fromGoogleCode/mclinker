### About ###
MCLinker is a full-fledged system linker for mobile devices. Hence it is fast, small with low memory footprints. MCLinker leverages the **LLVM** to link object files, and generate shared objects or executable files. Code in MCLinker project is licensed under the "UIUC" BSD-Style license which is compatible with commercial products.

### Reliability ###
Following systems can be linked successfully
  * ARM Android (4.2+) kernel and system image
  * FreeBSD buildworld and buildkernel for i386 targets
  * ARM Linux kernel 3.x

### Supported Environment ###
  * Target Platforms: AArch64, ARM, X86, X86\_64, x32, MIPS32, MIPS64 and Hexagon machines
  * Host OS: X86/X86\_64 Linux, Android, Mac OS X and Windows
  * For [Android NDK r9](http://developer.android.com/tools/sdk/ndk/index.html) and above, developers can simply use _-fuse-ld=mcld_ to link JNI programs by MCLinker


### Features ###
  * A system linker
  * GNU ld options compatibility
  * Support cross linking
  * Can be [used as a library](https://code.google.com/p/mclinker/wiki/IRBuilder) or a stand-alone tool
  * Support multiple targets
  * Fast, small with low memory footprint


### Current Status ###
Date: 2014/11/24<br>
LLVM: 3.6svn (trunk)<br>
MCLinker: 2.10.0-dev, master branch<br>
<br>
Date: 2014/12/09<br>
LLVM: 3.6svn (trunk@222644)<br>
MCLinker: 2.9.0-Dana, release_29 branch<br>
<br>
<h3>Next Milestone</h3>
<ul><li>ARM64 relaxation support<br>
</li><li>ARM exception handling format enhancement<br>
</li><li>TLS<br>
</li><li>Plugin support (LTO)<br>
</li><li>Version script support<br>
</li><li>To be one of system linkers of BSD platform</li></ul>

<h3>Code License</h3>
University of Illinois/NCSA<br>
Open Source License