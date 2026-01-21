# Compute Array Norm

## Group (Subgroup)

SimplnxReview (Statistics)

## Description

This **Filter** computes the p<sup>th</sup> norm of an **Attribute Array**. Specifically, for each tuple of the array, the following is computed:

$$\left\| \mathbf{x} \right\| _p := \bigg( \sum_{i=1}^n \left| x_i \right| ^p \bigg) ^{1/p}$$

where $n$ is the number of components for the **Attribute Array**.

- When $p = 2$, this results in the *Euclidean norm*
- When $p = 1$, this results in the *Manhattan norm* (also called the *taxicab norm*)

The p-space value may be any real number greater than or equal to zero. When $0 \leq p < 1$, the result may not strictly be a *norm* in the exact mathematical sense. Additionally, when $p = 0$, the result is simply the number of components for the **Attribute Array**.

**Note:** If the input array is a scalar array (1 component), the output array will contain the same values as the input array, but in 32-bit floating point precision.

% Auto generated parameter table will be inserted here

## Example Pipelines

## License & Copyright

Please see the description file distributed with this plugin.

## DREAM3D-NX Help

If you need help, need to file a bug report or want to request a new feature, please head over to the [DREAM3DNX-Issues](https://github.com/BlueQuartzSoftware/DREAM3DNX-Issues/discussions) GitHub site where the community of DREAM3D-NX users can help answer your questions.
