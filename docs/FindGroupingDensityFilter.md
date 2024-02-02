# Find Grouping Densities

**THIS FILTER IS UNTESTED, UNVERIFIED AND UNVALIDATED. IT IS AN EXPERIMENTAL FILTER THAT IS UNDERGOING LONG TERM DEVELOPMENT
AND TESTING. USE AT YOUR OWN RISK**

## Group (Subgroup)

Statistics (Reconstruction)

## Description

This **Filter** calculates the grouping densities for specific **Parent Features**.  This filter is intended to be used for hierarchical reconstructions (i.e., reconstructions involving more than one segmentation; thus, the **Feature**-**Parent Feature** relationship). The **Filter** iterates through all **Features** that belong to each **Parent Feature,** querying each of the **Feature** **Neighbors** to determine if it was checked during grouping. A list of **Checked Features** is kept for each **Parent Feature**.  Then, each **Parent Volume** is divided by the corresponding total volume of **Checked Features** to give the **Grouping Densities**.

If non-contiguous neighbors were used in addition to standard neighbors for grouping, then the *Use Non-Contiguous Neighbors* Parameter may be used.

Since many **Checked Features** are checked by more than one **Feature** during grouping, a premium is placed on the **Parent Feature** querying the **Checked Feature** having the largest **Parent Volume.**  For **Checked Features** to be written, the *Find Checked Features* Parameter may be used.

% Auto generated parameter table will be inserted here

## References

## Example Pipelines

## License & Copyright

Please see the description file distributed with this **Plugin**

## DREAM3D-NX Help

If you need help, need to file a bug report or want to request a new feature, please head over to the [DREAM3DNX-Issues](https://github.com/BlueQuartzSoftware/DREAM3DNX-Issues/discussions) GItHub site where the community of DREAM3D-NX users can help answer your questions.
