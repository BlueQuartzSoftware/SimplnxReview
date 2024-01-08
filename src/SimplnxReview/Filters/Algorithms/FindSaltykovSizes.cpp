#include "FindSaltykovSizes.hpp"

#include "simplnx/DataStructure/DataArray.hpp"
#include "simplnx/DataStructure/DataGroup.hpp"

#include <random>

using namespace nx::core;

namespace
{
// the below is formal Saltykov method of spheres with the formal variable names and 12 Saltykov coefficients.  It was decided
// that any further contributions from the coefficients is negligible
constexpr std::array<float64, 12> saltyCoefficients = {1.6461, -0.4561, -0.1162, -0.0415, -0.0173, -0.0079, -0.0038, -0.0018, -0.0010, -0.0003, -0.0002, -0.0002};

int32 DoSaltykov(std::vector<int32> nA, float32 DMax, int32 k)
{
  int32 i = 0;
  float64 temp1 = 0.0;
  while((i < 12) && ((k - i) > 0))
  {
    temp1 += saltyCoefficients[i + 1 - 1] * float64(nA[k - i - 1]);
    i++;
  }

  temp1 /= (DMax / k);
  return static_cast<int32>(std::round(temp1));
}

int32 ForwardDifference(int32 fx, int32 f1, int32 f0, int32 x1, int32 x0)
{
  // this is the forward finite difference method
  float32 temp1 = float32(fx - f0) / float32(f1 - f0);
  float32 temp2 = float32(x0) + float32(x1 - x0) * temp1;
  return static_cast<int32>(std::round(temp2));
}
} // namespace

// -----------------------------------------------------------------------------
FindSaltykovSizes::FindSaltykovSizes(DataStructure& dataStructure, const IFilter::MessageHandler& mesgHandler, const std::atomic_bool& shouldCancel, FindSaltykovSizesInputValues* inputValues)
: m_DataStructure(dataStructure)
, m_InputValues(inputValues)
, m_ShouldCancel(shouldCancel)
, m_MessageHandler(mesgHandler)
{
}

// -----------------------------------------------------------------------------
FindSaltykovSizes::~FindSaltykovSizes() noexcept = default;

// -----------------------------------------------------------------------------
const std::atomic_bool& FindSaltykovSizes::getCancel()
{
  return m_ShouldCancel;
}

// -----------------------------------------------------------------------------
Result<> FindSaltykovSizes::operator()()
{
  std::mt19937_64 gen(m_InputValues->Seed);
  std::uniform_int_distribution<int32> dist(std::numeric_limits<int32>::min(), std::numeric_limits<int32>::max());

  const auto& equivalentDiameters = m_DataStructure.getDataRefAs<Float32Array>(m_InputValues->EquivalentDiametersPath);
  auto& m_SaltykovEquivalentDiameters = m_DataStructure.getDataRefAs<Float32Array>(m_InputValues->SaltykovEquivalentDiametersPath);

  const int32 MaxAttempts = 10;
  const uint64 RandMaxFloat = 0xffffffff;
  // an initial guess of 10 bins is used
  int32 numberOfBins = 10, numberOfBins1 = 0, numberOfBins2 = 20;
  // the second guess is 20 bins, after this, a forward difference will ensue

  float32 binLength = 0.0f;
  int32 iter = 0;
  int32 saltykovLength = 0, saltykovLength1 = 0, saltykovLength2 = 0;
  int32 saltykovIndex = 1;
  int32 attempts = 0;
  int32 binToAddTo = 0;
  uint64 randomLong = 0.0;
  float64 randomFraction = 0.0;
  int32 difference = 0;

  usize numFeatures = equivalentDiameters.getNumberOfTuples();

  std::vector<int32> binLengths(numberOfBins, 0);

  std::vector<int32> saltykovBinLengths(numberOfBins, 0);

  std::vector<float32> sortedEquivDiams(numFeatures - 1, 0.0f);

  std::vector<float32> saltykovEquivalentDiameters(numFeatures - 1, 0.0f);

  std::vector<float32> temp;

  // transfer equivalent diameters to a std::vector
  std::copy(equivalentDiameters.begin() + 1, equivalentDiameters.end(), sortedEquivDiams.begin());

  std::sort(sortedEquivDiams.begin(), sortedEquivDiams.end(), std::less<>());

  // find the min and max
  float32 minEqDia = *std::min_element(sortedEquivDiams.begin(), sortedEquivDiams.end());
  float32 maxEqDia = *std::max_element(sortedEquivDiams.begin(), sortedEquivDiams.end());

  //  for(usize i = 0; i < numFeatures - 1; i++)
  //  {
  //    // find the min size
  //    if(sortedEquivDiams[i] < minEqDia)
  //    {
  //      minEqDia = sortedEquivDiams[i];
  //    }
  //    // find the max size
  //    if(sortedEquivDiams[i] > maxEqDia)
  //    {
  //      maxEqDia = sortedEquivDiams[i];
  //    }
  //  }

  float32 currentMinimum = minEqDia;
  float32 nextMinimum = maxEqDia;

  // continue performing Saltkov with changing the number of bins
  // until the number of feature eq dia's to be sampled = the
  // number of features.  We do this so the SaltykovArray length jives
  // with the feature-level level array, and then we get to be able to
  // compare each Saltkov eq dia one-for-one against the feature eq dia's
  // so, with that, each Saltykov eq dia is matched in ascending order to
  // the feature eq dia.  But, it is important to note that the Saltykov eq dia
  // is not a direct transformation of the particular eq dia that it is matched
  // up with
  while(saltykovLength != numFeatures - 1)
  {
    // find the bin length
    binLength = maxEqDia / static_cast<float32>(numberOfBins - 1);

    // initialize bin lengths to 0
    for(int32 i = 0; i < numberOfBins; i++)
    {
      binLengths[i] = 0;
    }

    iter = 0;
    for(int32 i = 0; i < numberOfBins; i++)
    {
      temp.resize(0);
      std::copy(std::lower_bound(sortedEquivDiams.begin(), sortedEquivDiams.end(), binLength * i), std::upper_bound(sortedEquivDiams.begin(), sortedEquivDiams.end(), binLength * (i + 1)),
                std::back_inserter(temp));
      binLengths[i] = temp.size();
      iter++;
    }

    std::reverse(binLengths.begin(), binLengths.end());

    // For formal Saltykov we start at the largest bin then unfold our
    // way down to the smallest bin which is why the loop is set up
    // this way
    // saltykovLength is the total number of features to be sampled from
    // the saltykov bins
    saltykovLength = 0;
    for(int32 i = 1; i <= numberOfBins; i++)
    {
      saltykovBinLengths[i - 1] = DoSaltykov(binLengths, maxEqDia, i);

      if(saltykovBinLengths[i - 1] < 0)
      {
        saltykovBinLengths[i - 1] = 0;
      }

      saltykovLength += saltykovBinLengths[i - 1];
    }

    std::reverse(saltykovBinLengths.begin(), saltykovBinLengths.end());

    // if the number of features to be sampled from the Saltykov
    // bins (saltykovLength) is not = the number of features
    // then the difference between the two is calculated.  then, either
    // the number of attempts is incremented, the number of bins is increased/decreased
    // based on the "difference" and the bin arrays are resized accordingly.  This
    // will approach us to the correct number of features because the increasing
    // number of bins, increases the number of features eq dia's to be sampled and decreasing
    // the number of bins does the opposite. However, if the number of maximum attempts
    // is reached, hardcoded at 10, then a random Saltykov bin is selected to add one sample to.
    // This is done because it is assumed that after 10 attempts, the solution is oscillating
    // between one less than and one more than feature so, for convenience, only the one less
    // than option triggers this fudge addition to reach our desired number of features
    if(saltykovLength != numFeatures - 1)
    {
      // better minimum formula
      difference = saltykovLength - (numFeatures - 1);
      if(attempts >= MaxAttempts && difference < 0)
      {
        for(int32 i = 0; i > difference; i--)
        {
          binToAddTo = dist(gen) % numberOfBins + 1;

          if(binToAddTo == numberOfBins)
          {
            binToAddTo--;
          };
          // only add to bins that already have values in them
          if(saltykovBinLengths[binToAddTo] < 1)
          {
            saltykovBinLengths[binToAddTo]++;
            saltykovLength = numFeatures - 1;
          }
          else
          {
            i++;
          }
        }
      }
      else
      {
        attempts++;

        if(numberOfBins2 == 20 && numberOfBins1 == 0)
        {
          numberOfBins1 = numberOfBins;
          numberOfBins = numberOfBins2;
          saltykovLength1 = saltykovLength;
        }
        else
        {
          numberOfBins2 = numberOfBins1;
          numberOfBins1 = numberOfBins;
          saltykovLength2 = saltykovLength1;
          saltykovLength1 = saltykovLength;
          numberOfBins = ForwardDifference(numFeatures - 1, saltykovLength2, saltykovLength1, numberOfBins2, numberOfBins1);
        }

        // in case the number of bins is less than 1
        if(numberOfBins < 1)
        {
          numberOfBins = 1;
        }

        binLengths.resize(numberOfBins);
        saltykovBinLengths.resize(numberOfBins);
      }
    }
  }

  // Only proceed with sampling the Saltykov bins if the Salytkov samples
  // = the number of features, else let the loop cycle again with the
  // new number of bins
  saltykovIndex = 0;
  if(saltykovLength == numFeatures - 1)
  {
    for(int32 i = 0; i < numberOfBins; i++)
    {
      for(int32 j = 0; j < saltykovBinLengths[i]; j++)
      {
        // generate a random float between the current bin extents
        randomLong = dist(gen);
        randomFraction = (float64(randomLong) / float64(RandMaxFloat));
        saltykovEquivalentDiameters[saltykovIndex] = float32(randomFraction) * binLength + float32(i) * binLength;
        saltykovIndex++;
      }
    }

    // sort the Saltykov eq dia's into ascending order, so they can be matched up with their feature eq dia pair
    std::sort(saltykovEquivalentDiameters.begin(), saltykovEquivalentDiameters.end(), std::less<>());

    // this nested loop matches the Saltykov eq dia's with the feature eq dia's in ascending order
    for(usize i = 1; i < numFeatures; i++)
    {
      for(usize j = 1; j < numFeatures; j++)
      {
        if(equivalentDiameters[j] == currentMinimum)
        {
          m_SaltykovEquivalentDiameters[j] = saltykovEquivalentDiameters[i - 1];
        }
        if(equivalentDiameters[j] > currentMinimum && equivalentDiameters[j] < nextMinimum)
        {
          nextMinimum = equivalentDiameters[j];
        }
      }
      currentMinimum = nextMinimum;
      nextMinimum = maxEqDia;
    }
  }

  return {};
}
