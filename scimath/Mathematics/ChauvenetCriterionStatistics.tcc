//# Copyright (C) 2000,2001
//# Associated Universities, Inc. Washington DC, USA.
//#
//# This library is free software; you can redistribute it and/or modify it
//# under the terms of the GNU Library General Public License as published by
//# the Free Software Foundation; either version 2 of the License, or (at your
//# option) any later version.
//#
//# This library is distributed in the hope that it will be useful, but WITHOUT
//# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
//# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
//# License for more details.
//#
//# You should have received a copy of the GNU Library General Public License
//# along with this library; if not, write to the Free Software Foundation,
//# Inc., 675 Massachusetts Ave, Cambridge, MA 02139, USA.
//#
//# Correspondence concerning AIPS++ should be addressed as follows:
//#        Internet email: aips2-request@nrao.edu.
//#        Postal address: AIPS++ Project Office
//#                        National Radio Astronomy Observatory
//#                        520 Edgemont Road
//#                        Charlottesville, VA 22903-2475 USA
//#
//# $Id: Array.h 21545 2015-01-22 19:36:35Z gervandiepen $

#ifndef SCIMATH_CHAUVENETCRITERIONSTATISTICS_TCC
#define SCIMATH_CHAUVENETCRITERIONSTATISTICS_TCC

#include <casacore/scimath/Mathematics/ChauvenetCriterionStatistics.h>

#include <casacore/scimath/Mathematics/StatisticsIncrementer.h>
#include <casacore/scimath/Mathematics/StatisticsUtilities.h>
#include <casacore/scimath/Mathematics/ZScoreCalculator.h>

namespace casacore {

template <class AccumType, class DataIterator, class MaskIterator, class WeightsIterator>
ChauvenetCriterionStatistics<AccumType, DataIterator, MaskIterator, WeightsIterator>::ChauvenetCriterionStatistics(
	Double zscore, Int maxIterations
)
  : ConstrainedRangeStatistics<AccumType, DataIterator, MaskIterator, WeightsIterator>(),
    _zscore(zscore), _maxIterations(maxIterations), _rangeIsSet(False), _niter(0) {}

template <class AccumType, class DataIterator, class MaskIterator, class WeightsIterator>
ChauvenetCriterionStatistics<AccumType, DataIterator, MaskIterator, WeightsIterator>::~ChauvenetCriterionStatistics() {}

template <class AccumType, class DataIterator, class MaskIterator, class WeightsIterator>
ChauvenetCriterionStatistics<AccumType, DataIterator, MaskIterator, WeightsIterator>&
ChauvenetCriterionStatistics<AccumType, DataIterator, MaskIterator, WeightsIterator>::operator=(
	const ChauvenetCriterionStatistics<AccumType, DataIterator, MaskIterator, WeightsIterator>& other
) {
    if (this == &other) {
        return *this;
    }
    ClassicalStatistics<AccumType, DataIterator, MaskIterator, WeightsIterator>::operator=(other);
    _zscore = other._zscore;
    _maxIterations = other._maxIterations;
    _niter = other._niter;
    return *this;
}

template <class AccumType, class DataIterator, class MaskIterator, class WeightsIterator>
void ChauvenetCriterionStatistics<AccumType, DataIterator, MaskIterator, WeightsIterator>::reset() {
	ConstrainedRangeStatistics<AccumType, DataIterator, MaskIterator, WeightsIterator>::reset();
	_rangeIsSet = False;
}

template <class AccumType, class DataIterator, class MaskIterator, class WeightsIterator>
void ChauvenetCriterionStatistics<AccumType, DataIterator, MaskIterator, WeightsIterator>::setCalculateAsAdded(
	Bool c
) {
	ThrowIf(
		c, "ChauvenetCriterionStatistics does not support calculating statistics "
		"incrementally as data sets are added"
	);
}

template <class AccumType, class DataIterator, class MaskIterator, class WeightsIterator>
void ChauvenetCriterionStatistics<AccumType, DataIterator, MaskIterator, WeightsIterator>::_setRange() {
	if (_rangeIsSet) {
		return;
	}
	uInt maxI = _maxIterations >= 0 ? _maxIterations : 1000;
	uInt prevNpts = 0;
	StatsData<AccumType> sd;
	while (_niter <= maxI) {
		if (_niter == 0) {
			ClassicalStatistics<AccumType, DataIterator, MaskIterator, WeightsIterator> cs(*this);
			sd = cs.getStatistics();
		}
		else {
			sd = this->getStatistics();
			if ((uInt64)sd.npts == prevNpts) {
				break;
			}
		}
		Double zScore = _zscore >= 0 ? _zscore : ZScoreCalculator::getMaxZScore((uInt64)sd.npts);
		CountedPtr<std::pair<AccumType, AccumType> > range = new std::pair<AccumType, AccumType>(
			sd.mean - zScore*sd.stddev, sd.mean + zScore*sd.stddev
		);
		ConstrainedRangeStatistics<AccumType, DataIterator, MaskIterator, WeightsIterator>::_setRange(range);
		// _rangeIsSet is set here to prevent infinite recursion on next loop iteration
		_rangeIsSet = True;
		prevNpts = (uInt64)sd.npts;
		++_niter;
	}
	--_niter;
}

}

#endif
