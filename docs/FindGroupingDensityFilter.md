# Find Grouping Densities

## Group (Subgroup)

Statistics (Reconstruction)

## Description

This **Filter** calculates the grouping densities for specific **Parent Features**.  This filter is intended to be used for hierarchical reconstructions (i.e., reconstructions involving more than one segmentation; thus, the **Feature**-**Parent Feature** relationship). The **Filter** iterates through all **Features** that belong to each **Parent Feature,** querying each of the **Feature** **Neighbors** to determine if it was checked during grouping. A list of **Checked Features** is kept for each **Parent Feature**.  Then, each **Parent Volume** is divided by the corresponding total volume of **Checked Features** to give the **Grouping Densities**.

If non-contiguous neighbors were used in addition to standard neighbors for grouping, then the *Use Non-Contiguous Neighbors* Parameter may be used.

Since many **Checked Features** are checked by more than one **Feature** during grouping, a premium is placed on the **Parent Feature** querying the **Checked Feature** having the largest **Parent Volume.**  For **Checked Features** to be written, the *Find Checked Features* Parameter may be used.

## Parameters

| Name | Type | Description |
|------|------| ----------- |
| Use Non-Contiguous Neighbors | bool | Whether to also use **Neighborhoods** in addition to **Neighbors** in the **Grouping Densities** calculation. |
| Find Checked Features | bool | Whether to write **Checked Features**. |

## Required Geometry

Cell

## Required Objects

| Kind | Default Name | Type | Component Dimensions | Description |
|------|--------------|------|----------------------|-------------|
| **DataArray** | N/A | float32 | (1) | Volume of the **Feature** |
| **DataArray** | N/A | float32 | (1) | *Parent Id* of the **Feature** |
| **NeighborList**  | N/A | int32 | (1) | List of the contiguous neighboring **Features** for a given **Feature** |
| **NeighborLists** | N/A | int32 | (1) | List of the **Features** whose centroids are within the user specified multiple of equivalent sphere diameter from each **Feature** |
| **DataArray** | N/A | float32 | (1) | Volume of the **Parent Feature** |

## Created Objects

| Kind | Default Name | Type | Component Dimensions | Description |
|------|--------------|------|----------------------|-------------|
| **DataArray** | CheckedFeatures |  int32 | (1) | The **Features** that were checked during grouping for a given **Parent Feature**.  This is written as the **ParentId** with the largest *Parent Volume* of the **Parent Features** that checked it. |
| **DataArray** | GroupingDensities | float | (1) | **Parent Volume** divided by the sum of the **Checked Features** for the **Parent Feature** |

## License & Copyright

Please see the description file distributed with this **Plugin**

## DREAM3DNX Help

Check out our GitHub community page at [DREAM3DNX-Issues](https://github.com/BlueQuartzSoftware/DREAM3DNX-Issues) to report bugs, ask the community for help, discuss features, or get help from the developers.
