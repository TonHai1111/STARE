/*
 * TemporalIndex.h
 *
 *  Created on: Apr 10, 2017
 *
 *  Author: Michael Rilee, Rilee Systems Technologies LLC
 *
 *  Copyright (C) 2021 Rilee Systems Technologies LLC
 *
 */

#ifndef SRC_TEMPORALINDEX_H_
#define SRC_TEMPORALINDEX_H_

#include "General.h"

#include "SpatialException.h"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <string>
#include <map>
#include <memory>

#include <ctime>
#include <cmath>

#include <algorithm>

#include "BitField.h"
#include "TemporalWordFormat1.h"

#include "erfa.h"

// #define TAG(X) cout << dec << X << hex << endl << flush;
// #define TAG(X) {}

using namespace std;
// namespace std {

class TemporalWordFormat : virtual public TemporalWordFormat1 {
public:

	/**
	 * Define the fields making up the encoding.
	 */
	TemporalWordFormat() {
		// resolutionLevelConstraint =  7; // Counts the number of levels, including
		// Not needed. nonDataLevels             =  3;

		// maxCoResolutionLevelValue = 9; // Is actually more than we count in resLevel.
		int64_t offset_base = 64;
//		int64_t coResolutionLevel = maxCoResolutionLevelValue;
//		int64_t coFieldId = maxCoResolutionLevelValue;
		int64_t fieldId = 0;
		// int64_t resolutionLevel = -2;  // TODO Fix hackery
		// resolutionLevelConstraint = coResolutionLevel + resolutionLevel;
		/// coResolutionLevel + resolutionLevel = maxCoResolutionLevelValue - 2
		/// Hence, we start the resolutionLevel at -1 on "Ma". It's like
		/// having another north-south bit (past/future bit)
		///
		/// TODO Put more thought into the "calendar" below...

		// TODO DANGER This bit is not really set. It's used to set the sign of the index.
		bitFields.push_back(
				    // new BitField("BeforeAfterStartBit",
				    make_shared<BitField>("BeforeAfterStartBit",
						1, // maxValue
						1, // width
						offset_base, // address/offset // in: 64; set to: 63 out: 63
						-2,   // scale -- really need N/A here
						fieldId++
							 )
		);
		bitFieldMap.insert(pair<string,shared_ptr<BitField> >(bitFields.back()->getName(),bitFields.back()));

		pos_CoarsestResolutionLevel = fieldId;
		bitFields.push_back(
				    make_shared<BitField>("year",
// <<<<<<< HEAD
							  pow(2,13)-1, // maxValue
							  13, // width
							  offset_base,
							  (13ll*28+1)*24*3600*1000, // months times millisec in  a regularized (24-day) month, plus one day for leap stuff
							  fieldId++
							  )
				    );
// =======
//                                   pow(2,13)-1, // maxValue
//                                   13, // width
//                                   offset_base,
//                                   (13ll*(13-1)+1)*24*3600*1000, // millisec in  a regularized (24-day) month
//                                   fieldId++
//                                   )
//                         );
// >>>>>>> f80cd105998be058c49e1f2441bd4d00fe06b16f
		bitFieldMap.insert(pair<string,shared_ptr<BitField> >(bitFields.back()->getName(),bitFields.back()));

		bitFields.push_back(
				    make_shared<BitField>("month",
						13, //maxValue // TODO Repent: 13 months with 4 weeks each, and a 14th with one day... Or maybe you just use 12...
						4, // width
						offset_base,
						28ll*24*3600*1000,
						fieldId++
							 )
		);
		bitFieldMap.insert(pair<string,shared_ptr<BitField> >(bitFields.back()->getName(),bitFields.back()));

		bitFields.push_back(
				make_shared<BitField>("week",
						3, // maxValue
						2, // width
						offset_base,
						7ll*24*3600*1000,
						fieldId++
						     )
		);
		bitFieldMap.insert(pair<string,shared_ptr<BitField> >(bitFields.back()->getName(),bitFields.back()));

		bitFields.push_back(
				make_shared<BitField>("day",
						6, // maxValue
						3, // width
						offset_base,
						24*3600*1000,
						fieldId++
						     )
		);
		bitFieldMap.insert(pair<string,shared_ptr<BitField> >(bitFields.back()->getName(),bitFields.back()));

		bitFields.push_back(
				make_shared<BitField>("hour",
						23, // maxValue
						5, // width
						offset_base,
						3600*1000,
						fieldId++
						     )
		);
		bitFieldMap.insert(pair<string,shared_ptr<BitField> >(bitFields.back()->getName(),bitFields.back()));

		bitFields.push_back(
				make_shared<BitField>("minute",
						59, // maxValue
						6, // width
						offset_base,
						60*1000,
						fieldId++
						     )
		);
		bitFieldMap.insert(pair<string,shared_ptr<BitField> >(bitFields.back()->getName(),bitFields.back()));

		bitFields.push_back(
				make_shared<BitField>("second",
						59, // maxValue
						6, // width
						offset_base,
						1000,
						fieldId++
						     )
		);
		bitFieldMap.insert(pair<string,shared_ptr<BitField> >(bitFields.back()->getName(),bitFields.back()));

		pos_FinestResolutionLevel = fieldId;
		bitFields.push_back(
				make_shared<BitField>("millisecond",
						999, // maxValue
						10, // width
						offset_base,
						1, // time unit
						fieldId++ // the actual low, should equal zero
						     )
		);
		bitFieldMap.insert(pair<string,shared_ptr<BitField> >(bitFields.back()->getName(),bitFields.back()));

		bitFields.push_back(
				make_shared<BitField>("forward_resolution",
						// pow(2,6)-1, // maxValue 63 -- takes the max resolution of 63, but we have only 64 bits
						pow(2,6)-1, // maxValue
						6, // width
						offset_base,
						-1, // time unit, not applicable
						fieldId++ // coResolutionLevel-- // -1, i.e. N/A
						     )
		);
		bitFieldMap.insert(pair<string,shared_ptr<BitField> >(bitFields.back()->getName(),bitFields.back()));

		bitFields.push_back(
				make_shared<BitField>("reverse_resolution",
						// pow(2,6)-1, // maxValue 63 -- takes the max resolution of 63, but we have only 64 bits
						pow(2,6)-1, // maxValue
						6, // width
						offset_base,
						-1, // time unit, not applicable
						fieldId++ // coResolutionLevel-- // -1, i.e. N/A
						     )
		);
		bitFieldMap.insert(pair<string,shared_ptr<BitField> >(bitFields.back()->getName(),bitFields.back()));
		

		bitFields.push_back(
				make_shared<BitField>("type",
						3, // maxValue
						2, // width
						offset_base,
						-1, // time unit, not applicable
						fieldId++ // N/A
						     )
		);
		bitFieldMap.insert(pair<string,shared_ptr<BitField> >(bitFields.back()->getName(),bitFields.back()));


		// setValue("BeforeAfterStartBit",1); // Default to "positive" dates.
		setFieldMaxId(fieldId-1);
		setValue("type",1); //

		/*
		bitFields.push_back(
				BitField("coResolutionLevel",
						-1, // maxValue
						-1, // width
						offset_base,
						-1, // Not applicable
						-1 // coResolutionLevel-- // -1, i.e. N/A
				)
		);
		bitFieldMap.insert(pair<string,BitField>(bitFields.back().getName(),bitFields.back()));
		*/

	}
	virtual ~TemporalWordFormat();
};

static const char* TimeStandard = "TAI"; // Not "UTC"
static const double  YearNativeCanonicalInMS_d   = 365.0*86400.0*1000.0;
static const int64_t YearNativeCanonicalInMS_i64 = 31536000000; // 365  *86400  *1000;

// static const TemporalWordFormat temporalWordFormat;
// static const int64_t maxValue_coResolutionLevel =  7 ; // = 7

//int scidbTemporalIndexCommonLevel(int64_t i0, int64_t i1) {
//	int commonLevel = -3; // error
//	if(i0 > 0 && i1 < 0) {
//		commonLevel = -2; // No levels in common
//	} else {
//		int64_t i01 =
//				( i0 &  i1) &
//				(~i0 & ~i1)
//				;
//		int k = 0;
//		uint64_t one = 1, testBit = one << 63, test;
//		do {
//			test = testBit & i01;
//			testBit >> 1;
//		} while( test != 0 && testBit > 0);
//	}
//
//}

// TODO Question: Am I working myself towards an n-adic template?
/**
 * TemporalIndex defines a particular temporal index, in this case type=1.
 * The constructor and the set_* methods are low level methods and do not
 * check bounds. Other methods can be defined that provide a higher level
 * API that includes such checks. We need at least one set of low-level
 * methods.
 *
 * TemporalWordFormat1, the base class for TemporalWordFormat, provides some
 * checks, e.g. resolutionInBounds.
 *
 *
 */
class TemporalIndex {
public:
	TemporalWordFormat data;
	// TODO Make better use of temporalWordFormat...



	TemporalIndex() {
		// data = temporalWordFormat; // Copy the format
		TemporalWordFormat data;
		// data.setValue("coResolutionLevel",maxValue_coResolutionLevel); // default
		data.setValue("forward_resolution",63);
		data.setValue("reverse_resolution",63);
	};
	virtual ~TemporalIndex();
	/**
	 * Native calendar format puts all leap time at the end of the year.
	 */
	TemporalIndex(
			int64_t BeforeAfterStartBit,
			int64_t year,
			int64_t month,
			int64_t week,
			int64_t day,
			int64_t hour,
			int64_t minute,
			int64_t second,
			int64_t millisecond,
			int64_t forward_resolution,
			int64_t reverse_resolution,
			int64_t type
			)
//	:
//				BeforeAfterStartBit(BeforeAfterStartBit),
//				Ma(Ma),
//				ka(ka),
//				year(year),
//				month(month),
//				day(day),
//				hour(hour),
//				second(second),
//				millisecond(millisecond),
//				coResolutionLevel(coResolutionLevel)
	{

		/*
	  if( BeforeAfterStartBit != 1 )
	    { stringstream ss; ss << "TemporalIndex::NOT_IMPLEMENTED_ERROR in TemporalIndex(...) BeforeAfterStartBit = 0 (past)" << endl;
	      ss << "TODO: Correct index scheme for the past. E.g. years go negative, but not months, weeks, etc." << endl;
	      throw SpatialFailure(ss.str().c_str()); }
	      */

// TODO Figure out redundancy of +/- 0 redundancy.
//
//	 There is no year zero. Should be aliased to 1 BCE, but this is a low-level constructor, so let's throw an error.
//		if( BeforeAfterStartBit == 1 && year == 0) {
//
//		}
	  
#define SET_VALUE(field) data.setValue(#field,field);
		SET_VALUE( BeforeAfterStartBit );
		SET_VALUE( year );
		SET_VALUE( month );
		SET_VALUE( week );
		SET_VALUE( day );
		SET_VALUE( hour );
		SET_VALUE( minute );
		SET_VALUE( second );
		SET_VALUE( millisecond );
		SET_VALUE( forward_resolution );
		SET_VALUE( reverse_resolution );
		SET_VALUE( type );
		// SET_VALUE( coResolutionLevel );
#undef SET_VALUE
	};

// #define SHIFT_AND_MASK(field) field = mask_##field & (idx_ >> offset_##field ) ;
#define SHIFT_AND_MASK(field) data.setValue(#field,\
		data.get(#field)->getMask() & (idx_ >> data.get(#field)->getOffset()));

		// cout << endl << #field << " " << dec << (data.get(#field)->getMask() & (idx_ >> data.get(#field)->getOffset())) << endl << flush;

#define REVERT(field) data.setValue(#field, data.get(#field)->getMaxValue()-data.getValue(#field) );

#define SET_MAX(field) data.setValue(#field,min(data.getValue(#field),data.get(#field)->getMaxValue()));

//	int leapYearDay(int64_t _year) const {
//		// From wikipedia
//		int leapyear_day = 0;
//		if( (_year % 4) != 0 ) {
//			leapyear_day = 0;
//		} else if( (_year % 100) != 0 ) {
//			leapyear_day = 1;
//		} else if( (_year % 400) != 0 ) {
//			leapyear_day = 0;
//		} else { leapyear_day = 1; }
//		return leapyear_day;
//	}

//	int64_t milliseconds_per_year(int64_t _year) {
//		int64_t ms;
//		return ms;
//	}
#if 0
#define SHIFT_AND_MASK_RESOLUTION(field) \
		data.setValue(#field, \
				data.resolutionLevelConstraint \
				- (data.get(#field)->getMask() & (idx_ >> data.get(#field)->getOffset())) );
#endif

	/**
	 * Set the TemporalIndex according to a scidb temporal index.
	 *
	 * This is a low-level constructor, with no or minimal value checking.
	 *
	 */
	TemporalIndex(int64_t scidbTemporalIndex) {
		int64_t idx_ = scidbTemporalIndex;


		data.setValue("BeforeAfterStartBit",idx_ > 0 ? 1 : 0);
		// SHIFT_AND_MASK_RESOLUTION(coResolutionLevel)
		int64_t babit = 1;
		if(idx_<0) {
			babit = 0;
			idx_ = - idx_;
			// idx_ = idx_ | (1ll << 63);
		}
		SHIFT_AND_MASK(year)
		SHIFT_AND_MASK(month)
		SHIFT_AND_MASK(week)
		SHIFT_AND_MASK(day)
		SHIFT_AND_MASK(hour)
		SHIFT_AND_MASK(minute)
		SHIFT_AND_MASK(second)
		SHIFT_AND_MASK(millisecond)
		SHIFT_AND_MASK(forward_resolution)
		SHIFT_AND_MASK(reverse_resolution)
		SHIFT_AND_MASK(type)

		// if(false) {
		if(babit == 0) {
			// REVERT(year);
			REVERT(month);
			REVERT(week);
			REVERT(day);
			REVERT(hour);
			REVERT(minute);
			REVERT(second);
			REVERT(millisecond);
		}
		if( data.getValue("year") == 0 && data.getValue("BeforeAfterStartBit") == 1 ) {

		  cout
		    << endl << "TI input: " << "0x" << setw(16) << setfill('0') << hex << scidbTemporalIndex << dec << endl << flush;
#define FMTtii(a) a << " " << data.getValue(a) << endl << flush
		  cout
		    << FMTtii("year")
		    << FMTtii("month")
		    << FMTtii("week")
		    << FMTtii("day")
		    << FMTtii("hour")
		    << FMTtii("minute")
		    << FMTtii("second")
		    << FMTtii("millisecond")
		    << FMTtii("forward_resolution")
		    << FMTtii("reverse_resolution")
		    << FMTtii("type")
		    << FMTtii("BeforeAfterStartBit")
		    ;
#undef FMTtii
			throw SpatialFailure("TemporalIndex:TemporalIndex(SciDBIndex):InvalidIndexYearZeroCE-A");
		}
	}

	TemporalIndex& fromTemporalIndexValue(int64_t scidbTemporalIndex) {
		int64_t idx_ = scidbTemporalIndex;

		data.setValue("BeforeAfterStartBit",idx_ > 0 ? 1 : 0);
		// SHIFT_AND_MASK_RESOLUTION(coResolutionLevel)
		int64_t babit = 1;
		if(idx_<0) {
			babit = 0;
			idx_ = - idx_;
			// idx_ = idx_ | (1ll << 63);
		}
		SHIFT_AND_MASK(year)
		SHIFT_AND_MASK(month)
		SHIFT_AND_MASK(week)
		SHIFT_AND_MASK(day)
		SHIFT_AND_MASK(hour)
		SHIFT_AND_MASK(minute)
		SHIFT_AND_MASK(second)
		SHIFT_AND_MASK(millisecond)
		SHIFT_AND_MASK(forward_resolution)
		SHIFT_AND_MASK(reverse_resolution)
		SHIFT_AND_MASK(type)

		// if(false) {
		if(babit == 0) {
			// REVERT(year);
			REVERT(month);
			REVERT(week);
			REVERT(day);
			REVERT(hour);
			REVERT(minute);
			REVERT(second);
			REVERT(millisecond);
		}

		// Note: data.getValue("resolution") == 63 for a terminator.

		// TODO Come up with a better leap year handler
		// TODO Think through temporal design re: translation to & from dates.

		/*
		if( data.getValue("resolution") == 63 ) {

//			// Fix things up if this is a terminator
			if(data.getValue("year") > 0) {
				data.setValue("month",13);
				data.setValue("week",0);
				data.setValue("day",0);
				// data.setValue("day",leapYearDay(data.getValue("year")));
			}
			SET_MAX(year);
			if(data.getValue("month") > 12) {
				data.setValue("month",13);
				data.setValue("week",0);
				data.setValue("day",0);
				// data.setValue("day",leapYearDay(data.getValue("year")));
			}
			SET_MAX(month);
			SET_MAX(week);   // ok
			SET_MAX(day);    // ok
			SET_MAX(hour);   // ok
			SET_MAX(minute); // ok
			SET_MAX(second); // ok
			SET_MAX(millisecond); // ok
		}
		*/

		// TODO Construct an index validity checker...
		if( data.getValue("year") == 0 && data.getValue("BeforeAfterStartBit") == 1 ) {
			throw SpatialFailure("TemporalIndex:fromTemporalIndexValue(SciDBIndex):InvalidIndexYearZeroCE-B");
		}
		return *this;
	};
#undef REVERT
#undef SET_MAX
#undef SHIFT_AND_MASK

	TemporalIndex(const TemporalIndex& orig) {
#define SET(field) data.setValue(#field,orig.data.getValue(#field));
		SET(BeforeAfterStartBit)
		SET(year)
		SET(month)
		SET(week)
		SET(day)
		SET(hour)
		SET(minute)
		SET(second)
		SET(millisecond)
		SET(forward_resolution)
		SET(reverse_resolution)
		SET(type)
#undef SET
	}

// #undef SHIFT_AND_MASK_RESOLUTION

	/*
	int64_t getCoResolutionLevel(string levelName) {
		return data.getCoResolutionLevel(levelName);
	}
	int64_t getResolutionLevel(string levelName) {
		return data.resolutionLevelConstraint - data.getCoResolutionLevel(levelName);
	}
	*/

	int64_t getCoFieldId(string levelName) {
		return data.getCoFieldId(levelName);
	}
	int64_t getFieldId(string levelName) {
		return data.getFieldId(levelName);
	}

	int64_t getIdOfCoarsestNonZeroField() {
		int64_t iPos = data.pos_CoarsestResolutionLevel;
		int64_t iCoarsestNonZero = -1;
		do {
			if ( data.getValueAtId(iPos) > 0 ) {
				iCoarsestNonZero = iPos;
			}
			--iPos;
		} while ( (iPos > data.pos_FinestResolutionLevel) && (iCoarsestNonZero < 0) );
		return iCoarsestNonZero;
	}

	int64_t bitOffsetFinest() const;
	int64_t bitOffsetCoarsest() const;
	int64_t bitOffsetResolution(int64_t resolution)  const;
	int64_t bitfieldIdFromResolution(int64_t resolution) const;

	int64_t scidbTemporalIndex();
	int64_t scidbTerminator();
	int64_t scidbLowerBound();
	int64_t scidbLowerBoundJulianTAI();
	int64_t scidbTerminatorJulianTAI();
	bool    scidbTerminatorp();

	/**
	   Get a mask that is ones for bits to the right of the temporal bits.
	 */
	uint64_t scidbResolutionAndTypeMask() { return (1llu << bitOffsetFinest()) - 1; }

	TemporalIndex& set_zero();
	TemporalIndex& setZero();
	TemporalIndex& setEOY(int64_t CE, int64_t year);

	void           toJulianUTC   ( double& utc1, double& utc2)const ;
        TemporalIndex& fromJulianUTC ( double  utc1, double  utc2, int forward_resolution=48, int reverse_resolution=48, int type=1  );

	void           toJulianTAI   ( double& d1, double& d2) const;
	TemporalIndex& fromJulianTAI ( double  d1, double  d2, int forward_resolution=48, int reverse_resolution=48, int type=1  );


	void toFormattedJulianTAI(
			int& year, int& month, int& day, int& hour, int& minute, int& second, int& ms
			);
	/**
	 * Set using a TAI date.
	 */
	TemporalIndex& fromFormattedJulianTAI(
			//old int64_t _CE,            // 0 or 1: 0 = BCE, 1 = CE
			int64_t _year, 			// Match the TAI format. 1 BCE is year 0. //old > 0
			int64_t _month, 		// 1..12 not 0..11
			int64_t _day_of_month, 	// 1..31
			int64_t _hour, 			// 0..23
			int64_t _minute, 		// 0..59
			int64_t _second, 		// 0..59
			int64_t _millisecond 	// 0..999
	);
	void toUTC(
			int& _year,
			int& _month, 		// 1..12 not 0..11
			int& _day_of_month, // 1..31
			int& _hour, 		// 0..23
			int& _minute, 		// 0..59
			int& _second, 		// 0..59
			int& _millisecond 	// 0..999
			);
	TemporalIndex& fromUTC(
			int _year,
			int _month, 		// 1..12 not 0..11
			int _day_of_month, 	// 1..31
			int _hour, 			// 0..23
			int _minute, 		// 0..59
			int _second, 		// 0..59
			int _millisecond 	// 0..999
			);

	string toStringJulianTAI();
	string toStringJulianTAI_ISO();
	TemporalIndex& fromStringJulianTAI(string inputString);
	TemporalIndex& fromStringJulianTAI_ISO(string inputString);

	int64_t millisecondsAtResolution(int64_t resolution) const;
	double daysAtResolution(int64_t resolution) const;
	int64_t coarsestResolutionFinerOrEqualMilliseconds (int64_t milliseconds);
	double getResolutionTimescaleDays() const {
	  return daysAtResolution(max(get_forward_resolution(),get_reverse_resolution()));
	}	
	double getForwardResolutionTimescaleDays() const {
		return daysAtResolution(get_forward_resolution());
	}
	double getReverseResolutionTimescaleDays() const {
		return daysAtResolution(get_reverse_resolution());
	}
	/**
	 * Set resolution to the finest level coarser than the resolutionDays input.
	 */
	TemporalIndex& setResolutionFromTimescaleDays( double resolutionDays ) {
		int64_t resolutionLevel = max((int64_t)0,coarsestResolutionFinerOrEqualMilliseconds( resolutionDays*86400.0e3 )-1);
		set_forward_resolution(resolutionLevel);
		set_reverse_resolution(resolutionLevel);
		return *this;
	}



// #define SET(field) TemporalIndex &set_##field(int64_t x) { field = x; if( (x < 0) || (x > maxValue_##field)) throw SpatialFailure("TemporalIndex:DomainFailure in ",name_##field.c_str()); return *this;}
#define SET(field) TemporalIndex &set_##field(int64_t x) { data.setValue(#field, x ); \
	if( (x < 0) || (x > data.get(#field)->getMaxValue()))\
    {stringstream ss; ss << data.get(#field)->getName().c_str() << " = " << x << " upper: " << data.get(#field)->getMaxValue(); throw SpatialFailure("TemporalIndex:DomainFailure in ",ss.str().c_str());} return *this;}
	SET(BeforeAfterStartBit)
	SET(year)
	SET(month)
	SET(week)
	SET(day)
	SET(hour)
	SET(minute)
	SET(second)
	SET(millisecond)
	SET(forward_resolution)
	SET(reverse_resolution)
	SET(type)
	// SET(coResolutionLevel)
#undef  SET
	// below is obsolete and wrong
//	TemporalIndex &set_resolutionLevel(int64_t level) {
//		data.setValue("coResolutionLevel",data.resolutionLevelConstraint - level); // TODO refactor
//		return *this;
//	}

#define GET(field) int64_t get_##field() const { return data.getValue(#field); }
	GET(BeforeAfterStartBit)
	GET(year)
	GET(month)
	GET(week)
	GET(day)
	GET(hour)
	GET(minute)
	GET(second)
	GET(millisecond)
	GET(forward_resolution)
	GET(reverse_resolution)
	GET(type)
	// GET(coResolutionLevel)
#undef GET
	// int64_t get_resolutionLevel() { return data.resolutionLevelConstraint - get_coResolutionLevel(); }

	void checkBitFormat();

	/**
	 * Convert the fields to milliseconds to aid conversions and support intervals.
	 *
	 * No corrections for "leap years" with this low-level calculation.
	 *
	 * TODO Augment with a Julian millisecond calculation?
	 */
	int64_t toInt64MillisecondsFractionOfYearJ() const;
	int64_t toInt64MillisecondsFractionOfYear() const;
	int64_t toInt64Milliseconds() const;
	TemporalIndex& fromInt64Milliseconds(int64_t milliseconds);

	double         toNativeYear();
	TemporalIndex& fromNativeYear(double year);

	TemporalIndex& fromNativeCEYearAndMilliseconds(
			int64_t CE,             // 0 or 1: 0 = BCE, 1 = CE
			int64_t _year, 			// > 0
			int64_t _milliseconds
			);
	void toNativeCEYearAndMilliseconds(int64_t& _CE, int64_t& _year, int64_t& _milliseconds);

	string stringInNativeDate();
	void fromNativeString(string nativeString);

	int eraTest();
};

void fractionalDayToHMSM   (double  fd, int& hour, int& minute, int& second, int& ms);
void fractionalDayFromHMSM (double& fd, int  hour, int  minute, int  second, int  ms);

/**
   Numerical compare (if) of the bits associated with two TemporalIndex objects.

   Question: might this have problems in edge cases involving leap years or seconds?
 */
inline int cmp(const TemporalIndex& a, const TemporalIndex& b) {
	if( a.get_type() != b.get_type() ) {
		throw SpatialFailure("TemporalIndex:cmp(a,b):TypeMismatch");
	}
	bool done = false; int iField = 0;
	int ret = 0;
	int     CE_factor = 1;
	int64_t thisCE  = a.get_BeforeAfterStartBit();
	int64_t otherCE = b.get_BeforeAfterStartBit();
	if( thisCE == otherCE ) {
		// We need to reorder the inequalities if we're in BCE territory.
		if( thisCE < 1 ) {
			CE_factor = -1;
		}
	}

	/*
	  Start at the coarsest field and compare successively finer fields.
	 */
	do {
		int64_t lhs = a.data.getValueAtId(iField);
		int64_t rhs = b.data.getValueAtId(iField);
//		int64_t lhs = a.data.getBitFieldAtId(iField)->getValue();
//		int64_t rhs = b.data.getBitFieldAtId(iField)->getValue();
		if( lhs < rhs ) {
			ret = -1*CE_factor; done = true;
		} else if ( lhs > rhs ) {
			ret =  1*CE_factor; done = true;
		}
		++iField;
		 if( iField >= a.data.getFieldId("forward_resolution") ){
			 done = true;
		 }
		++iField;
		 if( iField >= a.data.getFieldId("reverse_resolution") ){
		   // TODO this code should be unnecessary.
			 done = true;
		 }
	} while (!done);
	return ret;
}

/**
   Add two TemporalIndex values by adding their Int64Milliseconds representations.
 */
// inline TemporalIndex& add(const TemporalIndex& a, const TemporalIndex& b) {
inline TemporalIndex add(const TemporalIndex& a, const TemporalIndex& b) {
	if( a.get_type() != b.get_type() ) {
		throw SpatialFailure("TemporalIndex:add(a,b):TypeMismatch");
	}
	// Note by convention, there is no babit==1, year==0.
	// Now, use TemporalIndex as a scratchpad and fix semantics at end.

	int64_t	ab = a.toInt64Milliseconds() + b.toInt64Milliseconds();

	// TemporalIndex* c = new TemporalIndex;
	TemporalIndex c;
	c.fromInt64Milliseconds(ab);
	c.set_forward_resolution(min(a.get_forward_resolution(),b.get_forward_resolution()));
	c.set_reverse_resolution(min(a.get_reverse_resolution(),b.get_reverse_resolution()));
	return c;
}
/**
   Compare TemporalIndex objects according to their Julian TAI values.
 */
inline int cmpJ(const TemporalIndex& a, const TemporalIndex& b) {
	if( a.get_type() != b.get_type() ) {
		throw SpatialFailure("TemporalIndex:cmp(a,b):TypeMismatch");
	}

	double ad1, ad2, bd1, bd2;
	double ad, bd;
	a.toJulianTAI(ad1, ad2);
	b.toJulianTAI(bd1, bd2);
	ad = ad1 + ad2; bd = bd1 + bd2;

	int ret = 0; // if equal
	if ( ad < bd ) {
		ret = -1;
	} else if ( ad > bd ) {
		ret = 1;
	}
	return ret;
}

/**
   Add two TemporalIndex values by adding their Julian TAI representations.
 */
// inline TemporalIndex& addJ(const TemporalIndex& a, const TemporalIndex& b) {
inline TemporalIndex addJ(const TemporalIndex& a, const TemporalIndex& b) {
	if( a.get_type() != b.get_type() ) {
		throw SpatialFailure("TemporalIndex:add(a,b):TypeMismatch");
	}
	// Note by convention, there is no babit==1, year==0.
	// Now, use TemporalIndex as a scratchpad and fix semantics at end.
	double ad1, ad2, bd1, bd2, cd1, cd2;
	a.toJulianTAI(ad1, ad2);
	b.toJulianTAI(bd1, bd2);
	cd1 = ad1+bd1; cd2 = ad2+bd2;
//#define FMT1(x,y) cout << #x << "," << #y << " " << x << "," << y << endl << flush;
//	FMT1(ad1,ad2);
//	FMT1(bd1,bd2);
//	FMT1(cd1,cd2);
//#undef FMT1
	// TemporalIndex* c = new TemporalIndex;
	TemporalIndex c;
	c.fromJulianTAI(cd1, cd2);
	c.set_forward_resolution(min(a.get_forward_resolution(),b.get_forward_resolution()));
	c.set_reverse_resolution(min(a.get_reverse_resolution(),b.get_reverse_resolution()));
	return c;
}

inline double diff_JulianTAIDays(const TemporalIndex& a, const TemporalIndex& b) {
	if( a.get_type() != b.get_type() ) {
		throw SpatialFailure("TemporalIndex:add(a,b):TypeMismatch");
	}
	// Note by convention, there is no babit==1, year==0.
	// Now, use TemporalIndex as a scratchpad and fix semantics at end.
	double ad1, ad2, bd1, bd2;
	a.toJulianTAI(ad1, ad2);
	b.toJulianTAI(bd1, bd2);
	return (ad1+ad2)-(bd1+bd2);
}

/**
 * Are two TemporalIndex values within a "resolution" time scale of each other?
 *
 */
inline bool cmp_JulianTAIDays(const TemporalIndex& a, const TemporalIndex& b) {
	if( a.get_type() != b.get_type() ) {
		throw SpatialFailure("TemporalIndex:add(a,b):TypeMismatch");
	}
	double delta  = diff_JulianTAIDays(a,b);
	double timescale = max(a.getResolutionTimescaleDays(),b.getResolutionTimescaleDays());
	bool neighborsp = abs(delta) < timescale;
	return neighborsp;
}

/**
 * Are two TemporalIndex values within a "resolution" time scale of each other?
 *
 */
inline bool cmp_JulianTAIDays3(const TemporalIndex& a, const TemporalIndex& b, double daysTolerance) {
	if( a.get_type() != b.get_type() ) {
		throw SpatialFailure("TemporalIndex:add(a,b):TypeMismatch");
	}
	double delta  = diff_JulianTAIDays(a,b);
	bool neighborsp = abs(delta) < daysTolerance;
	return neighborsp;
}
inline bool operator==(const TemporalIndex& lhs, const TemporalIndex& rhs) { return cmpJ(lhs,rhs) == 0; }
inline bool operator!=(const TemporalIndex& lhs, const TemporalIndex& rhs) { return cmpJ(lhs,rhs) != 0; }
inline bool operator< (const TemporalIndex& lhs, const TemporalIndex& rhs) { return cmpJ(lhs,rhs) <  0; }
inline bool operator> (const TemporalIndex& lhs, const TemporalIndex& rhs) { return cmpJ(lhs,rhs) >  0; }
inline bool operator<=(const TemporalIndex& lhs, const TemporalIndex& rhs) { return cmpJ(lhs,rhs) <= 0; }
inline bool operator>=(const TemporalIndex& lhs, const TemporalIndex& rhs) { return cmpJ(lhs,rhs) >= 0; }
inline TemporalIndex operator+ (const TemporalIndex& a, const TemporalIndex& b) { return addJ(a,b); }
inline TemporalIndex operator| (const TemporalIndex& a, const TemporalIndex& b) { return add(a,b); }

inline std::ostream& operator<<(std::ostream& os, TemporalIndex tIndex) {
	os << tIndex.toStringJulianTAI();
	return os;
}

/**
 * Returns the lowest temporal index valid in SciDB. SciDB has symmetrical coordinate (index) dimensions, i.e. +/- (2**62 -1).
 */
int64_t scidbMinimumTemporalIndex();
/**
 * Returns the greatest non-terminator temporal index valid in SciDB. For a poor-man's terminator here, set the 6 resolution bits to 1, i.e. 63.
 */
int64_t scidbMaximumTemporalIndex();

int64_t millisecondsInYear(int64_t CE, int64_t year);

/**
   Set bits at finer resolutions to zero.
 */
int64_t scidbClearBitsFinerThanResolution(int64_t ti_value, int resolution);

/**
   Set bits at finer resolutions to one.
 */
int64_t scidbSetBitsFinerThanResolution(int64_t ti_value, int resolution);

/**
   Set bits at finer resolutions to one, but limit values to calendrical maxes. I.e. 60 seconds per minute, not 63.
 */
int64_t scidbSetBitsFinerThanResolutionLimited(int64_t ti_value, int resolution);

/**
   Approximate the upper bound (terminator) of a temporal index value.
 */
int64_t scidbUpperBoundMS(int64_t ti_value);

/**
   Approximate the lower bound (terminator) of a temporal index value.
 */
int64_t scidbLowerBoundMS(int64_t ti_value);

/**
   A more accurate the upper bound (terminator) of a temporal index value, based on TAI.
 */
int64_t scidbUpperBoundTAI(int64_t ti_value);

/**
   A more accurate the lower bound of a temporal index value, based on TAI.
 */
int64_t scidbLowerBoundTAI(int64_t ti_value);

/*
overlap
segment
gt, lt, eq, contains
*/

/**
   True if temporal index values overlap using approximate millisecond calculations.
 */
bool scidbOverlap(int64_t ti_value_0, int64_t ti_value_1);

/**
   True if temporal index values overlap using more accurate but expensive TAI calculations.
 */
bool scidbOverlapTAI(int64_t ti_value_0, int64_t ti_value_1);

/**
   True if the instant in ti_value_query is in the segment of ti_value.
 */
bool scidbContainsInstant(int64_t ti_value, int64_t ti_value_query);

/**
 Return a mask for extracting the temporal location (i.e. the instant)
 */
int64_t temporal_mask(int64_t ti_value);

/**
   Return forward resolution "level" of the temporal index value.

   TODO: Check on temporal format, if needed.
 */
int64_t forward_resolution(int64_t ti_value);

/**
   Return reverse resolution "level" of the temporal index value.

   TODO: Check on temporal format, if needed.
 */
int64_t reverse_resolution(int64_t ti_value);

int64_t set_reverse_resolution(int64_t ti_value, int64_t resolution);
int64_t set_forward_resolution(int64_t ti_value, int64_t resolution);
int64_t coarsen(int64_t ti_value, int64_t reverse_increment, int64_t forward_increment);

/**
   True if the temporal index value is a lower bound.
 */
bool lowerBoundP(int64_t ti_value);

/**
   True if the temporal index value is an upper bound.
 */
bool upperBoundP(int64_t ti_value);

bool validBoundP(int64_t ti_value);

bool validResolutionP(int64_t resolution);

/**
   Make a new temporal index value from a triple of values (lower, t0, upper).

   A negative value for lower or upper will cause that resolution to be set to the finest resolution (i.e. maximum resolution).

   A negative value for the "center" or tiv will be set to the mean of the lower and upper.
 */
int64_t scidbNewTemporalValue(int64_t tiv_lower, int64_t tiv, int64_t tiv_upper, bool include_bounds=true);

int64_t fromStringJulianTAI_ISO(string inputString);

string toStringJulianTAI_ISO(int64_t tiv);

int64_t scidbTemporalValueUnionIfOverlap(int64_t ti_value_0, int64_t ti_value_1);
int64_t scidbTemporalValueIntersectionIfOverlap(int64_t ti_value_0, int64_t ti_value_1);

void set_temporal_resolutions_from_sorted_inplace(int64_t* ti_sorted, const int64_t len, bool include_bounds = true);

// } /* namespace std */

void TemporalIndex_test();

#endif /* SRC_TEMPORALINDEX_H_ */
