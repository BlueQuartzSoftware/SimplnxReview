# Group MicroTexture Regions (C-Axis Misorientation)

**THIS FILTER IS UNTESTED, UNVERIFIED AND UNVALIDATED. IT IS AN EXPERIMENTAL FILTER THAT IS UNDERGOING LONG TERM DEVELOPMENT
AND TESTING. USE AT YOUR OWN RISK**

## Group (Subgroup)

Reconstruction Filters (Grouping)

## Description

This Filter groups neighboring **Features** that have c-axes aligned within a user defined tolerance.  The algorithm for grouping the **Features** is analogous to the algorithm for segmenting the **Features** - only the average orientation of the **Features** are used instead of the orientations of the individual **Cells** and the criterion for grouping only considers the alignment of the c-axes.  The user can specify a tolerance for how closely aligned the c-axes must be for neighbor **Features** to be grouped.

NOTE: This filter is intended for use with *Hexagonal* materials.  While the c-axis is actually just referring to the <001> direction and thus will operate on any symmetry, the utility of grouping by <001> alignment is likely only important/useful in materials with anisotropy in that direction (like materials with *Hexagonal* symmetry).

Developer Note: It is **very important** for end users to **seed** the data if they *ever* intend to replicate the calculations. The randomness involed in this filter aims to help avoid bias developing in the output, however with out of core this will severely slow down the calculations at runtime. In order to remedy this we are in the process of developing a floodfill method, but the bias formed by sequential parsing is something one should consider when utilizing this method.

% Auto generated parameter table will be inserted here

## References

## Example Pipelines

## License & Copyright

Please see the description file distributed with this **Plugin**

## DREAM3D-NX Help

If you need help, need to file a bug report or want to request a new feature, please head over to the [DREAM3DNX-Issues](https://github.com/BlueQuartzSoftware/DREAM3DNX-Issues/discussions) GItHub site where the community of DREAM3D-NX users can help answer your questions.
