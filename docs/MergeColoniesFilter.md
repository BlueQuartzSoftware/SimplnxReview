# Merge Colonies

**THIS FILTER IS UNTESTED, UNVERIFIED AND UNVALIDATED. IT IS AN EXPERIMENTAL FILTER THAT IS UNDERGOING LONG TERM DEVELOPMENT
AND TESTING. USE AT YOUR OWN RISK**

## Group (Subgroup) ##

Reconstruction (Grouping)

## Description ##

This **Filter** groups neighboring **Features** that have a *special* misorientation that is associated with *alpha* variants that transformed from the same *beta* grain in titanium.  The algorithm for grouping the **Features** is analogous to the algorithm for segmenting the **Features**, except the average orientation of the **Features** are used instead of the orientations of the individual **Elements** and the criterion for grouping is specific to the *alpha-beta transformation*.  The user can specify a tolerance on both the *axis* and the *angle* that defines the misorientation relationship (i.e., a tolerance of 1 degree for both tolerances would allow the neighboring **Features** to be grouped if their misorientation was between 59-61 degrees about an axis within 1 degree of a2, as given by the 3rd *special* misorientation below).

The list of *special* misorientations can be found in the paper by Germain et al.<sup>1</sup> and are listed here:

| Angle | Axis |
|------|------|
| 0 | Identity |
| 10.529 | c = <0001> |
| 60 | a2 = <-12-10> |
| 60.832 | d1 at 80.97 degrees from c in the plane of (d3,c) |
| 63.262 | d2 at 72.73 degrees from c in the plane of (a2,c) |
| 90 | d3 at 5.26 degrees from a2 in the basal plane |

% Auto generated parameter table will be inserted here

## References

## Example Pipelines

## License & Copyright

Please see the description file distributed with this **Plugin**

## DREAM3D-NX Help

If you need help, need to file a bug report or want to request a new feature, please head over to the [DREAM3DNX-Issues](https://github.com/BlueQuartzSoftware/DREAM3DNX-Issues/discussions) GItHub site where the community of DREAM3D-NX users can help answer your questions.
