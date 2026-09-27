# P0-T007 - RVT / BimRv Read Capability Matrix

**Task:** P0-T007 - RVT / BimRv Evaluation
**Classification:** Phase-0 read-only evaluation
**Product integration:** NOT AUTHORIZED
**Writer:** PROHIBITED
**Architecture Change Record:** NONE

## Evidence classes

| Class | Meaning |
|---|---|
| RUNTIME PROVEN | Directly exercised against the controlled public Revit 2017 fixture |
| STATICALLY ESTABLISHED | Vendor SDK implementation surface established, but not runtime-proven for the controlled fixture |
| OFFICIALLY DOCUMENTED | Established from vendor documentation, not from the package runtime test |
| NOT ESTABLISHED | Available evidence is insufficient |
| OUT OF SCOPE | Explicitly excluded by P0-T007 / AG-012 |

## Controlled fixture

| Item | Value |
|---|---|
| File | `rac_basic_sample_project.rvt` |
| Revit version | Autodesk Revit 2017 |
| Revit build | `20160130_1515(x64)` |
| Size | `17125376` bytes |
| SHA256 | `A1D3D0775AE0937E3F4C8FB97015B6E0884ACEFAE52435D9CACD27D008F5FEFB` |

## Capability matrix

| Capability | Classification | Evidence boundary |
|---|---|---|
| RVT file recognition | RUNTIME PROVEN | Controlled Revit 2017 public sample |
| RVT database open | RUNTIME PROVEN | `OdaBimApp` controlled open |
| Nested database load | RUNTIME PROVEN | `OdBmGetGeomEx` runtime |
| Element load | RUNTIME PROVEN | `OdBmGetGeomEx` runtime |
| Target element resolution | RUNTIME PROVEN | One selected `SWall`, handle `0x30826` |
| Vectorization code path | RUNTIME PROVEN | `OdBmVectorizeEx`, exit code 0 |
| Geometry extraction | RUNTIME PROVEN | Selected `SWall` only |
| Shell geometry | RUNTIME PROVEN | Geometry dump |
| Vertices | RUNTIME PROVEN | XYZ coordinate output |
| Faces | RUNTIME PROVEN | Indexed face output |
| Edge attributes / visibility | RUNTIME PROVEN | Geometry dump |
| Texture coordinates | RUNTIME PROVEN | Geometry dump |
| Polyline geometry | RUNTIME PROVEN | Geometry dump |
| Visual geometry rendering | NOT ESTABLISHED | Explicit visual-render confirmation was not retained as authoritative evidence |
| Basic/document information | STATICALLY ESTABLISHED | Vendor SDK/source surface |
| Element enumeration | STATICALLY ESTABLISHED | Vendor SDK/source surface |
| Parameters / properties | STATICALLY ESTABLISHED | Vendor SDK/source surface |
| Views / drawings | STATICALLY ESTABLISHED | Vendor SDK/source surface |
| Categories | STATICALLY ESTABLISHED | Vendor SDK/source surface |
| Materials | STATICALLY ESTABLISHED | Vendor SDK/source surface |
| External / linked references | STATICALLY ESTABLISHED | Vendor SDK/source surface |
| RVT read format support beyond test fixture | OFFICIALLY DOCUMENTED | Vendor documentation; not package-specific runtime proof |
| RFA read | OFFICIALLY DOCUMENTED / NOT RUNTIME TESTED | No controlled RFA runtime fixture executed |
| RTE read | OFFICIALLY DOCUMENTED / NOT RUNTIME TESTED | No controlled RTE runtime fixture executed |
| BimRv 27.7 maximum Revit version | NOT ESTABLISHED | Must not be inferred from current-product documentation |
| RVT write | OUT OF SCOPE | AG-012 / writer prohibited |
| RVT Save / Save As | OUT OF SCOPE | Not authorized |
| RVT export/conversion | OUT OF SCOPE | Not authorized |
| Product adapter | OUT OF SCOPE | Product integration not authorized |
| Repository/CMake BimRv integration | OUT OF SCOPE | External evaluation only |

## Runtime integrity

Successful controlled runtime checks established:

- working RVT unchanged after read/open;
- canonical RVT unchanged;
- no extra file created in the isolated runtime directories used for final successful validation;
- main repository untouched;
- task repository source/build surfaces untouched.

## Geometry proof boundary

Successful geometry extraction used:

- element class: `SWall`;
- OdaBimApp displayed decimal handle: `198694`;
- `OdDbHandle` hexadecimal command-line representation: `30826`;
- successful geometry output: 1057 lines;
- no entity-resolution error in the successful run.

This proves the selected-element path only. It is not a statistical or
completeness claim for arbitrary RVT content.

## Final disposition

~~~text
Read-only RVT feasibility        = ESTABLISHED FOR CONTROLLED EVALUATION
Product integration              = NOT AUTHORIZED
Writer                           = PROHIBITED
RFA runtime proof                = NOT ESTABLISHED
RTE runtime proof                = NOT ESTABLISHED
Package-specific max version     = NOT ESTABLISHED
Architecture Change Record       = NONE
~~~
