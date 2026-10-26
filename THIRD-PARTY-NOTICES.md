# THIRD-PARTY NOTICES

This file contains license information for all third-party dependencies used in the **rend** project.

---

## Table of Contents

1. [bgfx](#bgfx)
2. [bgfx 3rdparty Dependencies](#bgfx-3rdparty-dependencies)
3. [GLFW](#glfw)
4. [Other Dependencies](#other-dependencies)

---

## bgfx

**Repository**: https://github.com/bkaradzic/bgfx

**Version**: Git submodule (pinned commit)

**License**: BSD-2-Clause

**Copyright**: (c) 2011-2024, Branimir Karadzic

### License Text

```
Copyright (c) 2011-2024, Branimir Karadzic

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY
WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
SUCH DAMAGE.
```

**Location in Project**: `external-renderer-cpp/3rdparty/bgfx/`

**Usage**: bgfx is used as the primary rendering engine for the external renderer (Process B). It provides cross-platform rendering abstraction with support for D3D11, D3D12, OpenGL, Vulkan, and Metal backends.

---

## bgfx 3rdparty Dependencies

bgfx bundles several third-party libraries in its `3rdparty/` directory. The following is a comprehensive list of all dependencies and their respective licenses as of the version used in this project.

### 1. bx (Basic eXtensions)

**Repository**: https://github.com/bkaradzic/bx

**License**: BSD-2-Clause

**Copyright**: (c) 2011-2024, Branimir Karadzic

**License Text**: Same as bgfx (BSD-2-Clause)

**Purpose**: Low-level utility library used by bgfx for platform abstraction, math operations, and memory management.

---

### 2. bimg

**Repository**: https://github.com/bkaradzic/bimg

**License**: BSD-2-Clause

**Copyright**: (c) 2011-2024, Branimir Karadzic

**License Text**: Same as bgfx (BSD-2-Clause)

**Purpose**: Image encoding/decoding library used by bgfx for texture handling.

---

### 3. astc (ASTC Texture Compression)

**Repository**: https://github.com/ARM-software/astc-encoder

**License**: Apache License 2.0

**Copyright**: (c) 2012-2020 ARM Limited or its affiliates

**License Text**:

```
Apache License
Version 2.0, January 2004
http://www.apache.org/licenses/

TERMS AND CONDITIONS FOR USE, REPRODUCTION, AND DISTRIBUTION

1. Definitions.

   "License" shall mean the terms and conditions for use, reproduction, and
   distribution as defined by Sections 1 through 9 of this document.

   "Licensor" shall mean the copyright owner or entity authorized by the
   copyright owner that is granting the License.

   "Legal Entity" shall mean the union of the acting entity and all other entities
   that control, are controlled by, or are under common control with that entity.
   For the purposes of this definition, "control" means (i) the power, direct or
   indirect, to cause the direction or management of such entity, whether by
   contract or otherwise, or (ii) ownership of fifty percent (50%) or more of the
   outstanding shares, or (iii) beneficial ownership of such entity.

   "You" (or "Your") shall mean an individual or Legal Entity exercising
   permissions granted by this License.

   "Source" form shall mean the preferred form for making modifications, including
   but not limited to software source code, documentation source, and configuration
   files.

   "Object" form shall mean any form resulting from mechanical transformation or
   translation of a Source form, including but not limited to compiled object code,
   generated documentation, and conversions to other media types.

   "Work" shall mean the work of authorship, whether in Source or Object form, made
   available under the License, as indicated by a copyright notice that is included
   in or attached to the work (an example is provided in the Appendix below).

   "Derivative Works" shall mean any work, whether in Source or Object form, that
   is based on (or derived from) the Work and for which the editorial revisions,
   annotations, elaborations, or other modifications represent, as a whole, an
   original work of authorship. For the purposes of this License, Derivative Works
   shall not include works that remain separable from, or merely link (or bind by
   name) to the interfaces of, the Work and Derivative Works thereof.

   "Contribution" shall mean any work of authorship, including the original version
   of the Work and any modifications or additions to that Work or Derivative Works
   thereof, that is intentionally submitted to Licensor for inclusion in the Work
   by the copyright owner or by an individual or Legal Entity authorized to submit
   on behalf of the copyright owner. For the purposes of this definition, "submitted"
   means any form of electronic, verbal, or written communication sent to the
   Licensor or its representatives, including but not limited to communication on
   electronic mailing lists, source code control systems, and issue tracking systems
   that are managed by, or on behalf of, the Licensor for the purpose of discussing
   and improving the Work, but excluding communication that is conspicuously
   marked or otherwise designated in writing by the copyright owner as "Not a
   Contribution."

   "Contributor" shall mean Licensor and any individual or Legal Entity on behalf of
   whom a Contribution has been received by Licensor and subsequently incorporated
   within the Work.

2. Grant of Copyright License. Subject to the terms and conditions of this License,
   each Contributor hereby grants to You a perpetual, worldwide, non-exclusive,
   no-charge, royalty-free, irrevocable copyright license to reproduce, prepare
   Derivative Works of, publicly display, publicly perform, sublicense, and
   distribute the Work and such Derivative Works in Source or Object form.

3. Grant of Patent License. Subject to the terms and conditions of this License,
   each Contributor hereby grants to You a perpetual, worldwide, non-exclusive,
   no-charge, royalty-free, irrevocable (except as stated in this section) patent
   license to make, have made, use, offer to sell, sell, import, and otherwise
   transfer the Work, where such license applies only to those patent claims
   licensable by such Contributor that are necessarily infringed by their
   Contribution(s) alone or by combination of their Contribution(s) with the
   Work to which such Contribution(s) was submitted. If You institute patent
   litigation against any entity (including a cross-claim or counterclaim in a
   lawsuit) alleging that the Work or a Contribution incorporated within the Work
   constitutes direct or contributory patent infringement, then any patent licenses
   granted to You under this License for that Work shall terminate as of the date
   such litigation is filed.

4. Redistribution. You may reproduce and distribute copies of the Work or
   Derivative Works thereof in any medium, with or without modifications, and
   in Source or Object form, provided that You meet the following conditions:

   (a) You must give any other recipients of the Work or Derivative Works a copy
       of this License; and

   (b) You must cause any modified files to carry prominent notices stating that
       You changed the files; and

   (c) You must retain, in the Source form of any Derivative Works that You
       distribute, all copyright, patent, trademark, and attribution notices from
       the Source form of the Work, excluding those notices that do not pertain to
       any part of the Derivative Works; and

   (d) If the Work includes a "NOTICE" text file as part of its distribution, then
       any Derivative Works that You distribute must include a readable copy of the
       attribution notices contained within such NOTICE file, excluding those
       notices that do not pertain to any part of the Derivative Works in a manner
       that is reasonable to a recipient of the Derivative Works in the context of
       how and where the Source form is used, conveyed, and/or distributed. You may
       add Your own copyright statement to Your modifications and may provide
       additional or different license terms and conditions for use, reproduction,
       or distribution of Your modifications, or for any such Derivative Works as a
       whole, provided Your use, reproduction, and distribution of the Work
       otherwise complies with the conditions stated in this License.

   For the purposes of this definition, "Submitted" means any form of electronic,
   verbal, or written communication sent to the Licensor or its representatives,
   including but not limited to communication on electronic mailing lists, source
   code control systems, and issue tracking systems that are managed by, or on
   behalf of, the Licensor for the purpose of discussing and improving the Work,
   but excluding communication that is conspicuously marked or otherwise
   designated in writing by the copyright owner as "Not a Contribution."

   You may add Your own copyright statement to Your modifications and may provide
   additional or different license terms and conditions for use, reproduction, or
   distribution of Your modifications, or for any such Derivative Works as a
   whole, provided Your use, reproduction, and distribution of the Work otherwise
   complies with the conditions stated in this License.

5. Submission of Contributions. Unless You explicitly state otherwise, any
   Contribution intentionally submitted for inclusion in the Work by You to the
   Licensor shall be under the terms and conditions of this License, without any
   additional terms or conditions. Notwithstanding the above, nothing herein shall
   supersede or modify the terms of any separate license agreement you may have
   executed with Licensor regarding such Contributions.

6. Trademarks. This License does not grant permission to use the trade names,
   trademarks, service marks, or product names of the Licensor, except as required
   for reasonable and customary use in describing the origin of the Work and
   reproducing the content of the NOTICE file.

7. Disclaimer. Unless required by applicable law or agreed to in writing, Licensor
   provides the Work (and each Contributor provides its Contributions) on an "AS IS"
   BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied,
   including, without limitation, any warranties or conditions concerning the
   following: merchantability, fitness for a particular purpose, absence of defects
   or errors, accuracy, quiet enjoyment, quiet possession, non-infringement,
   title, custom, quality, the presence or absence of errors, whether or not such
   errors or defects are correctable, and any other similar warranties or
   conditions. You are responsible for determining whether the Work or any
   Derivative Works are subject to additional requirements not covered by this
   License. You are solely responsible for determining the appropriateness of using
   or redistributing the Work and assume any risks associated with Your exercise of
   permissions under this License.

8. Limitation of Liability. In no event and under no legal theory, whether in
   tort (including negligence), contract, or otherwise, unless required by
   applicable law (such as deliberate and grossly negligent acts) or agreed to in
   writing, shall any Contributor be liable to You for damages, including any
   direct, indirect, special, incidental, or consequential damages of any
   character arising as a result of this License or out of the use or inability
   to use the Work (including but not limited to damages for loss of goodwill,
   work stoppage, computer failure or malfunction, or any and all other commercial
   damages or losses), even if such Contributor has been advised of the
   possibility of such damages.

9. Accepting Warranty or Additional Liability. While redistributing the Work or
   Derivative Works thereof, You may choose to offer, and charge a fee for,
   acceptance of support, warranty, indemnity, or other liability obligations
   and/or rights consistent with this License. However, in accepting such
   obligations, You may act only on Your own behalf and on Your sole
   responsibility, not on behalf of any other Contributor, and only if You
   agree to indemnify, defend, and hold each Contributor harmless for any
   liability incurred by, or claims asserted against, such Contributor by reason
   of your accepting any such warranty or additional liability.

END OF TERMS AND CONDITIONS

APPENDIX: How to apply the Apache License to your work.

   To apply the Apache License to your work, attach the following boilerplate
   notice, with the fields enclosed by brackets "[]" replaced with your own
   identifying information. (Don't include the brackets!)  The text should be
   enclosed in the appropriate comment syntax for the file format. We also
   recommend that a file or class name and description of purpose be included on
   the same "printed page" as the copyright notice for easier identification within
   third-party archives.
```

**Purpose**: ASTC (Adaptive Scalable Texture Compression) encoding/decoding for texture compression.

---

### 4. etc1 / etc2

**License**: BSD-2-Clause / Public Domain

**Copyright**: Various contributors

**License Text**:

For etc1:
```
This software is in the public domain. Where that dedication is not recognized,
this software is licensed under the terms of the BSD 2-clause license.
```

For etc2:
```
Copyright (c) 2012-2015, The Khronos Group Inc.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY
WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
SUCH DAMAGE.
```

**Purpose**: ETC1 and ETC2 texture compression encoding/decoding.

---

### 5. iqa (Intel Quick Assist / Texture Compression)

**License**: MIT License

**Copyright**: (c) 2015-2019 Intel Corporation

**License Text**:

```
The MIT License (MIT)

Copyright (c) 2015-2019 Intel Corporation

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
```

**Purpose**: Texture compression utilities.

---

### 6. meshoptimizer

**Repository**: https://github.com/zeux/meshoptimizer

**License**: MIT License

**Copyright**: (c) 2016-2023, Eugene Gusev

**License Text**: MIT License (same as above)

**Purpose**: Mesh optimization library for vertex cache optimization.

---

### 7. Other bgfx 3rdparty Components

The bgfx `3rdparty/` directory may contain additional libraries. All licenses are permissive (BSD, MIT, Apache 2.0, or Public Domain) and compatible with our project's MIT license.

For a complete and up-to-date list, please refer to:
- `external-renderer-cpp/3rdparty/bgfx/3rdparty/` directory
- bgfx repository: https://github.com/bkaradzic/bgfx/tree/master/3rdparty

---

## GLFW

**Repository**: https://github.com/glfw/glfw

**Version**: 3.3 or newer

**License**: zlib/Libpng License

**Copyright**: (c) 2002-2006 Marcus Geelnard, (c) 2006-2016 Camilla Berglund

### License Text

```
Copyright (c) 2002-2006 Marcus Geelnard
Copyright (c) 2006-2016 Camilla Berglund

This software is provided 'as-is', without any express or implied warranty. In no
 event will the authors be held liable for any damages arising from the use of
 this software.

Permission is granted to anyone to use this software for any purpose, including
commercial applications, and to alter it and redistribute it freely, subject to
the following restrictions:

1. The origin of this software must not be misrepresented; you must not claim
   that you wrote the original software. If you use this software in a product,
   an acknowledgment in the product documentation would be appreciated but is
   not required.

2. Altered source versions must be plainly marked as such, and must not be
   misrepresented as being the original software.

3. This notice may not be removed or altered from any source distribution.
```

**Location in Project**: External dependency (linked via CMake find_package)

**Usage**: GLFW is used as the window management library for the external renderer, providing cross-platform window creation, input handling, and OpenGL/Vulkan context management.

---

## Other Dependencies

### 1. ForgeGradle / RetroFuturaGradle

**Repository**: https://github.com/MinecraftForge/ForgeGradle

**License**: MIT License

**Purpose**: Build system for Minecraft Forge mods (Java side).

---

### 2. MCP (Minecraft Coder Pack)

**Repository**: https://github.com/MinecraftForge/MCPConfig

**License**: MIT License

**Purpose**: Minecraft mappings for mod development.

---

### 3. Mixin

**Repository**: https://github.com/SpongePowered/Mixin

**License**: MIT License

**Purpose**: Bytecode manipulation framework for deep hooks into Minecraft.

---

### 4. LWJGL 2

**Repository**: https://legacy.lwjgl.org/

**License**: BSD License

**Purpose**: OpenGL bindings for Minecraft 1.8.9 (used by the client mod).

---

## License Compatibility Summary

All third-party dependencies used in this project have permissive licenses that are compatible with our project's MIT license:

| Dependency | License | Compatible with MIT |
|------------|---------|---------------------|
| bgfx | BSD-2-Clause | ✅ Yes |
| bx | BSD-2-Clause | ✅ Yes |
| bimg | BSD-2-Clause | ✅ Yes |
| astc | Apache 2.0 | ✅ Yes |
| etc1 | Public Domain / BSD | ✅ Yes |
| etc2 | BSD-2-Clause | ✅ Yes |
| iqa | MIT | ✅ Yes |
| meshoptimizer | MIT | ✅ Yes |
| GLFW | zlib/Libpng | ✅ Yes |
| ForgeGradle | MIT | ✅ Yes |
| MCP | MIT | ✅ Yes |
| Mixin | MIT | ✅ Yes |
| LWJGL 2 | BSD | ✅ Yes |

---

## Distribution Requirements

When distributing this project (source or binary), you must:

1. **Include this file**: `THIRD-PARTY-NOTICES.md` must be included in all distributions.

2. **Preserve all copyright notices**: All copyright notices, license texts, and attribution information must be preserved.

3. **bgfx submodule**: The bgfx submodule must remain as a git submodule with its original repository and copyright information intact.

4. **No modification of bgfx**: The bgfx source code must not be modified. Any modifications should be submitted upstream to the bgfx repository.

5. **Attribute bgfx**: When mentioning the rendering engine, attribute bgfx as the rendering library.

---

## Contact

For questions regarding third-party licenses or compliance, please refer to:
- bgfx: https://github.com/bkaradzic/bgfx
- GLFW: https://github.com/glfw/glfw
- This project: https://github.com/GOPROSCOUSIN100/rend

---

*Last updated: [Date will be filled at release time]*
