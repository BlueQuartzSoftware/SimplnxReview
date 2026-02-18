# ComputeGroupingDensity Filter - Code Review

**Date:** 2026-02-18
**Branch:** `topic/compute_grouping_density_review`
**Files Reviewed:**
- `src/SimplnxReview/Filters/Algorithms/ComputeGroupingDensity.hpp`
- `src/SimplnxReview/Filters/Algorithms/ComputeGroupingDensity.cpp`
- `src/SimplnxReview/Filters/ComputeGroupingDensityFilter.hpp`
- `src/SimplnxReview/Filters/ComputeGroupingDensityFilter.cpp`
- `test/ComputeGroupingDensityTest.cpp`
- `docs/ComputeGroupingDensityFilter.md`

---

## 1. Algorithm Overview

The `ComputeGroupingDensity` filter computes **grouping densities** for parent features in a hierarchical reconstruction. It operates on a two-level feature hierarchy: child **Features** (grains) that belong to **Parent Features** (reconstructed groups).

### Data Model

| Data | Level | Type | Description |
|---|---|---|---|
| Feature Volumes | Feature | `Float32Array` | Volume of each child feature |
| Parent IDs | Feature | `Int32Array` | Which parent each feature belongs to |
| Contiguous Neighbor List | Feature | `NeighborList<int32>` | Contiguous neighbors per feature |
| Non-Contiguous Neighbor List | Feature | `NeighborList<int32>` | (Optional) Non-contiguous neighbors per feature |
| Parent Volumes | Parent | `Float32Array` | Volume of each parent feature |
| **Grouping Densities** | **Parent (output)** | `Float32Array` | Computed density for each parent |
| **Checked Features** | **Feature (output)** | `Int32Array` | (Optional) Which parent checked each feature |

### Algorithm Flowchart

```
START
  Initialize:
    totalFeatureCheckVolume = 0
    totalFeatureCheckList = {} (empty set)
    checkedFeatureVolumes = [0...] (only if FindCheckedFeatures)

  FOR each parentId (1..numParents):
  |
  |   FOR each featureId (1..numFeatures):
  |   |
  |   |   Is parentIds[featureId] == currentParentId?
  |   |   |
  |   |   NO --> skip to next featureId
  |   |   |
  |   |   YES
  |   |   |
  |   |   Is featureId already in totalFeatureCheckList?
  |   |   |
  |   |   NO:
  |   |   |   totalFeatureCheckVolume += featureVolumes[featureId]
  |   |   |   Add featureId to totalFeatureCheckList
  |   |   |
  |   |   |   [If FindCheckedFeatures]:
  |   |   |     If parentVolumes[parentId] > checkedFeatureVolumes[featureId]:
  |   |   |       checkedFeatureVolumes[featureId] = parentVolumes[parentId]
  |   |   |       outCheckedFeatures[featureId] = parentId
  |   |   |
  |   |   YES: (skip volume add, go straight to neighbor processing)
  |   |   |
  |   |   processNeighborListData(contiguousNeighborList):
  |   |   |   For each neighbor of featureId:
  |   |   |     If neighbor NOT in totalFeatureCheckList:
  |   |   |       totalFeatureCheckVolume += featureVolumes[neighbor]
  |   |   |       Add neighbor to totalFeatureCheckList
  |   |   |       [If FindCheckedFeatures]:
  |   |   |         If parentVolumes[parentId] > checkedFeatureVolumes[neighbor]:
  |   |   |           checkedFeatureVolumes[neighbor] = parentVolumes[parentId]
  |   |   |           outCheckedFeatures[neighbor] = parentId
  |   |   |
  |   |   [If UseNonContiguousNeighbors]:
  |   |     processNeighborListData(nonContiguousNeighborList)
  |   |     (same logic as above)
  |   |
  |   END FOR (featureId)
  |
  |   Compute density for this parent:
  |     If totalFeatureCheckVolume == 0:
  |       groupingDensities[parentId] = -1.0
  |     Else:
  |       groupingDensities[parentId] = parentVolumes[parentId] / totalFeatureCheckVolume
  |
  |   Clear totalFeatureCheckList
  |   Reset totalFeatureCheckVolume = 0
  |
  END FOR (parentId)

DONE
```

### Algorithm Summary

For each parent, the algorithm:
1. Finds all features belonging to that parent
2. Collects all contiguous (and optionally non-contiguous) neighbors of those features
3. Sums the volumes of all collected ("checked") features
4. Computes density as `parentVolume / totalCheckedVolume`
5. A density of `-1.0` indicates no features were found for that parent

### Template Specializations

The algorithm uses compile-time template booleans for the two optional paths, resulting in 4 specializations:

| `UseNonContiguousNeighbors` | `FindCheckedFeatures` | Description |
|---|---|---|
| `false` | `false` | Contiguous neighbors only, no checked features output |
| `true` | `false` | Both neighbor types, no checked features output |
| `false` | `true` | Contiguous neighbors only, with checked features output |
| `true` | `true` | Both neighbor types, with checked features output |

---

## 2. Issues Found

### 2.1 Bugs / Correctness Issues

#### Bug 1: Broken `fmt::format` in preflight error message

**File:** `ComputeGroupingDensityFilter.cpp:149`
**Severity:** Medium

```cpp
return MakePreflightErrorResult(-15672,
    fmt::format("All Input Feature level data arrays and neighbor lists MUST have the same number of tuples.",
                pParentVolumesPath.toString()));
```

The format string contains **no `{}` placeholder** but `pParentVolumesPath.toString()` is passed as an argument. The path is silently ignored. The error message should include the mismatched paths and their tuple counts (similar to error -15671 above it).

**Suggested fix:**
```cpp
return MakePreflightErrorResult(-15672,
    fmt::format("All Input Feature level data arrays and neighbor lists MUST have the same number of tuples.\n{}: {}\n{}: {}",
                pParentIdsPath.toString(), parentIdsPtr->getNumberOfTuples(),
                pNonContiguousNLPath.toString(), pNonContiguousNLPtr->getNumberOfTuples()));
```

#### Bug 2: Original unit test was a non-functional stub

**File:** `test/ComputeGroupingDensityTest.cpp` (original)
**Severity:** High

The original test loaded an exemplar DataStructure but then created a completely separate empty `DataStructure ds` and ran the filter against it with default arguments. The exemplar was never used for comparison. The test would either produce meaningless results or silently pass without testing anything.

**Status:** Fixed - replaced with 7 comprehensive test cases (see Section 4).

### 2.2 Variable Naming Violations

Per the project coding standards, DataStore references should use the `Ref` suffix, and variables should have descriptive names.

| File:Line | Current Name | Suggested Name | Reason |
|---|---|---|---|
| `ComputeGroupingDensity.cpp:45` | `featureParentIds` | `featureParentIdsRef` | DataStore reference needs `Ref` suffix |
| `ComputeGroupingDensity.cpp:46` | `parentVolumes` | `parentVolumesRef` | DataStore reference needs `Ref` suffix |
| `ComputeGroupingDensity.cpp:47` | `featureVolumes` | `featureVolumesRef` | DataStore reference needs `Ref` suffix |
| `ComputeGroupingDensity.cpp:50` | `outCheckedFeatures` | `outCheckedFeaturesRef` | DataStore reference needs `Ref` suffix |
| `ComputeGroupingDensity.cpp:51` | `outGroupingDensities` | `outGroupingDensitiesRef` | DataStore reference needs `Ref` suffix |
| `ComputeGroupingDensity.cpp:128` | `numCurNeighborList` | `numNeighbors` | Name is not descriptive |
| `ComputeGroupingDensity.cpp:130` | `l` | `neighborIdx` | Single-letter loop variable is unclear |
| `ComputeGroupingDensity.cpp:133` | `neigh` | `neighborId` | Abbreviation is unclear |
| `ComputeGroupingDensity.cpp:127` | `featureNeighborList` | `featureNeighbors` | Redundant "List" - type already conveys this |
| `ComputeGroupingDensity.cpp:191` | `volumes` | `featureVolumes` | Not descriptive enough; ambiguous with parent volumes |

### 2.3 Dead / Commented-Out Code

**File:** `ComputeGroupingDensity.cpp:131-137`

```cpp
// bool ok = false;
auto neigh = featureNeighborList.at(l);
// if(!ok)
// {
//   continue;
// }
```

This commented-out block should be removed entirely. If the logic is needed in the future, it can be recovered from version control.

### 2.4 Type Inconsistencies

**File:** `ComputeGroupingDensity.cpp:124-125`

```cpp
void processNeighborListData(const NeighborList<int32>& neighbor_list,
    const int32 currentFeatureId, const int32 currentParentId, ...)
```

The method signature uses `int32` for `currentFeatureId` and `currentParentId`, but the caller passes `usize` loop variables (lines 100-103). This causes an implicit narrowing conversion. The parameters should either be `usize` or explicit casts should be added at the call sites.

Additionally, the `neighbor_list` parameter uses `snake_case` naming which is inconsistent with the project's `camelBack` convention.

### 2.5 Const-Correctness

**File:** `ComputeGroupingDensity.cpp:165`

```cpp
Int32NeighborList& m_NonContiguousNL;  // non-const
```

The `m_NonContiguousNL` member is declared as a non-const reference but is only ever read (never modified). It should be `const Int32NeighborList&` for correctness and clarity.

### 2.6 Performance Suggestions

#### Use `std::unordered_set` instead of `std::set`

**File:** `ComputeGroupingDensity.cpp:58`

```cpp
std::set<int32> totalFeatureCheckList = {};
```

`std::set<int32>` provides O(log n) lookups and insertions. Replacing it with `std::unordered_set<int32>` would give O(1) amortized operations, which can be significant for large feature counts.

#### Inconsistent lookup style

**File:** `ComputeGroupingDensity.cpp:86 vs 140`

Line 86 uses the older `.find() == .end()` pattern:
```cpp
if(totalFeatureCheckList.find(static_cast<int32>(currentFeatureId)) == totalFeatureCheckList.end())
```

Line 140 uses the C++20 `.contains()` method:
```cpp
if(!totalFeatureCheckList.contains(neigh))
```

Both should use `.contains()` for consistency, since the project targets C++20.

### 2.7 Preflight Redundant Null Checks

**File:** `ComputeGroupingDensityFilter.cpp:130-133`

```cpp
auto* parentIdsPtr = dataStructure.getDataAs<IDataArray>(pParentIdsPath);
auto* featureVolumesPtr = dataStructure.getDataAs<IDataArray>(pFeatureVolumesPath);
auto* pContiguousNLPtr = dataStructure.getDataAs<INeighborList>(pContiguousNLPath);
auto* pNonContiguousNLPtr = dataStructure.getDataAs<INeighborList>(pNonContiguousNLPath);
```

Per coding standards, `ArraySelectionParameter` and `NeighborListSelectionParameter` automatically validate that the selected object exists. The `getDataAs` + null checks for `parentIdsPtr`, `featureVolumesPtr`, and `pContiguousNLPtr` are therefore unnecessary.

**Note:** The null check for `pNonContiguousNLPtr` on line 145 *is* warranted because when `UseNonContiguousNeighbors` is false, the path may be empty/invalid.

### 2.8 Style / Cleanup

#### Experimental warning should be removed once tests are in place

**File:** `ComputeGroupingDensityFilter.cpp:203-204`

```cpp
preflightUpdatedValues.push_back({"WARNING: This filter is experimental..."});
resultOutputActions.warnings().push_back({-65432, "WARNING: This filter is experimental..."});
```

Now that comprehensive unit tests exist, this warning can be removed.

---

## 3. Suggestions Summary

| # | Category | Priority | Description |
|---|---|---|---|
| 1 | Bug | High | Fix broken `fmt::format` in error -15672 (missing `{}` placeholder) |
| 2 | Naming | Medium | Rename 10 variables to follow coding conventions (see table in 2.2) |
| 3 | Cleanup | Low | Remove commented-out dead code in `processNeighborListData` |
| 4 | Type Safety | Medium | Fix `int32` vs `usize` mismatch in `processNeighborListData` signature |
| 5 | Const | Low | Make `m_NonContiguousNL` member `const` |
| 6 | Performance | Low | Replace `std::set` with `std::unordered_set` |
| 7 | Style | Low | Use `.contains()` consistently instead of `.find() == .end()` |
| 8 | Cleanup | Low | Remove redundant null checks for parameter-validated arrays |
| 9 | Cleanup | Low | Remove experimental warning now that tests exist |
| 10 | Style | Low | Rename `neighbor_list` parameter to `neighborList` (camelBack) |

---

## 4. Unit Test Coverage

### Test Data Design

A manually constructed DataStructure with:
- **6 features** (index 0 = placeholder, features 1-5)
- **4 parents** (index 0 = placeholder, parents 1-3)
- Features 1, 2, 3 belong to Parent 1
- Features 4, 5 belong to Parent 2
- Parent 3 has no features (exercises the `density == -1.0` path)

**Feature Volumes:** `[0, 10, 20, 15, 25, 30]`
**Parent Volumes:** `[0, 200, 100, 50]`

**Contiguous Neighbors:**
| Feature | Neighbors |
|---|---|
| 0 | {} |
| 1 | {2} |
| 2 | {1, 3} |
| 3 | {2, 4} |
| 4 | {3, 5} |
| 5 | {4} |

**Non-Contiguous Neighbors (when enabled):**
| Feature | Neighbors |
|---|---|
| 0 | {} |
| 1 | {4} |
| 2 | {5} |
| 3 | {} |
| 4 | {1} |
| 5 | {2} |

### Test Cases and Expected Results

#### Test 1: Basic Density (UseNonContiguous=false, FindChecked=false)

| Parent | Features Checked | Total Check Volume | Density |
|---|---|---|---|
| 1 | {1, 2, 3, 4} (4 via neighbor of 3) | 10+20+15+25 = 70 | 200/70 = 2.857 |
| 2 | {3, 4, 5} (3 via neighbor of 4) | 25+15+30 = 70 | 100/70 = 1.429 |
| 3 | (none) | 0 | -1.0 |

#### Test 2: With Non-Contiguous Neighbors (UseNonContiguous=true, FindChecked=false)

| Parent | Features Checked | Total Check Volume | Density |
|---|---|---|---|
| 1 | {1, 2, 3, 4, 5} (all via extended neighbors) | 100 | 200/100 = 2.0 |
| 2 | {1, 2, 3, 4, 5} (all via extended neighbors) | 100 | 100/100 = 1.0 |
| 3 | (none) | 0 | -1.0 |

#### Test 3: With Checked Features (UseNonContiguous=false, FindChecked=true)

Same densities as Test 1. Checked features track which parent (with the largest volume) checked each feature:

| Feature | Checked By Parent | Reason |
|---|---|---|
| 0 | 0 | Placeholder, never checked |
| 1 | 1 | Checked by Parent 1 (vol=200) |
| 2 | 1 | Checked by Parent 1 (vol=200) |
| 3 | 1 | Parent 1 (200) > Parent 2 (100), stays with Parent 1 |
| 4 | 1 | Parent 1 (200) > Parent 2 (100), stays with Parent 1 |
| 5 | 2 | Only checked by Parent 2 (vol=100) |

**Expected:** `[0, 1, 1, 1, 1, 2]`

#### Test 4: Both Options Enabled (UseNonContiguous=true, FindChecked=true)

Same densities as Test 2. With non-contiguous neighbors, Parent 1 (vol=200) checks ALL features first. Parent 2 (vol=100) cannot override any since 100 < 200.

**Expected checked features:** `[0, 1, 1, 1, 1, 1]`

#### Test 5: Preflight Error - Feature tuple count mismatch (Error -15671)

ParentIds array has a different tuple count than Volumes and ContiguousNL. Expects preflight to return an error.

#### Test 6: Preflight Error - Volumes not in AttributeMatrix (Error -15673)

Feature Volumes placed directly under the ImageGeom instead of inside an AttributeMatrix. Expects preflight to return an error.

#### Test 7: Preflight Error - Parent Volumes not in AttributeMatrix (Error -15670)

Parent Volumes placed directly under the ImageGeom instead of inside an AttributeMatrix. Expects preflight to return an error.

### Test Results

All 7 tests pass:

```
100% tests passed, 0 tests failed out of 7

Total Test time (real) = 0.41 sec
```
