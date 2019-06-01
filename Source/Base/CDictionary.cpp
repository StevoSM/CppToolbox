//----------------------------------------------------------------------------------------------------------------------
//	CDictionary.cpp			©2007 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CDictionary.h"

#include "CppToolboxAssert.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: SDictionaryItemInfo

struct SDictionaryItemInfo {

			// Lifecycle methods
			SDictionaryItemInfo(UInt32 keyHashValue, const SDictionaryItem& item) :
				mKeyHashValue(keyHashValue), mItem(item), mNextItemInfo(nil)
				{}
			SDictionaryItemInfo(const SDictionaryItemInfo& itemInfo) :
				mKeyHashValue(itemInfo.mKeyHashValue), mItem(itemInfo.mItem), mNextItemInfo(nil)
				{
					// Check value type
					if (mItem.mValue.mValueType == kDictionaryValueTypeArrayOfDictionaries)
						// Array of dictionaries
						mItem.mValue.mValue.mArrayOfDictionaries =
								new TArrayT<CDictionary>(*mItem.mValue.mValue.mArrayOfDictionaries);
					else if (mItem.mValue.mValueType == kDictionaryValueTypeArrayOfStrings)
						// Array of strings
						mItem.mValue.mValue.mArrayOfStrings =
								new TArrayT<CString>(*mItem.mValue.mValue.mArrayOfStrings);
					else if (mItem.mValue.mValueType == kDictionaryValueTypeData)
						// Data
						mItem.mValue.mValue.mData = new CData(*mItem.mValue.mValue.mData);
					else if (mItem.mValue.mValueType == kDictionaryValueTypeDictionary)
						// Dictionary
						mItem.mValue.mValue.mDictionary = new CDictionary(*mItem.mValue.mValue.mDictionary);
					else if (mItem.mValue.mValueType == kDictionaryValueTypeString)
						// String
						mItem.mValue.mValue.mString = new CString(*mItem.mValue.mValue.mString);
				}

			// Instance methods
	bool	doesMatch(UInt32 hashValue, const CString& key)
				{ return (hashValue == mKeyHashValue) && (key == mItem.mKey); }
	void	disposeValue()
				{
					// Check value type
					if (mItem.mValue.mValueType == kDictionaryValueTypeArrayOfDictionaries) {
						// Array of dictionaries
						DisposeOf(mItem.mValue.mValue.mArrayOfDictionaries);
					} else if (mItem.mValue.mValueType == kDictionaryValueTypeArrayOfStrings) {
						// Array of strings
						DisposeOf(mItem.mValue.mValue.mArrayOfStrings);
					} else if (mItem.mValue.mValueType == kDictionaryValueTypeData) {
						// Data
						DisposeOf(mItem.mValue.mValue.mData);
					} else if (mItem.mValue.mValueType == kDictionaryValueTypeDictionary) {
						// Dictionary
						DisposeOf(mItem.mValue.mValue.mDictionary);
					} else if (mItem.mValue.mValueType == kDictionaryValueTypeString) {
						// String
						DisposeOf(mItem.mValue.mValue.mString);
					}
				}

	// Properties
	UInt32					mKeyHashValue;
	SDictionaryItem			mItem;
	SDictionaryItemInfo*	mNextItemInfo;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CDictionaryIteratorInfo

struct CDictionaryIteratorInfo : public CIteratorInfo {
	// Methods
	public:
					// Lifecycle methods
					CDictionaryIteratorInfo(const CDictionaryInternals& internals, UInt32 initialReference) :
						CIteratorInfo(), mInternals(internals), mInitialReference(initialReference), mCurrentIndex(0),
								mCurrentItemInfo(nil)
						{}

					// CIteratorInfo methods
	CIteratorInfo*	copy()
						{
							// Make copy
							CDictionaryIteratorInfo*	iteratorInfo =
																new CDictionaryIteratorInfo(mInternals,
																		mInitialReference);
							iteratorInfo->mCurrentIndex = mCurrentIndex;
							iteratorInfo->mCurrentItemInfo = mCurrentItemInfo;

							return iteratorInfo;
						}

	// Properties
	const	CDictionaryInternals&	mInternals;
			UInt32					mInitialReference;
			UInt32					mCurrentIndex;
			SDictionaryItemInfo*	mCurrentItemInfo;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CDictionaryInternals

class CDictionaryInternals {
	public:
											CDictionaryInternals() :
												mCount(0), mReferenceCount(1), mReference(0), mItemInfosCount(16)
												{
													mItemInfos =
															(SDictionaryItemInfo**)
																	::calloc(mItemInfosCount,
																			sizeof(SDictionaryItemInfo*));
												}
											~CDictionaryInternals()
												{
													::free(mItemInfos);
												}

				CDictionaryInternals*		addReference() { mReferenceCount++; return this; }
				void						removeReference()
												{
													// Decrement reference count and check if we are the last one
													if (--mReferenceCount == 0) {
														// We going away
														CDictionaryInternals*	THIS = this;
														DisposeOf(THIS);
													}
												}

				CDictionaryInternals*		prepareForWrite()
												{
													// Check reference count.  If there is more than 1 reference, we
													//	implement a "copy on write".  So we will clone ourselves so we
													//	have a personal buffer that can be changed while leaving the
													//	exiting buffer as-is for the other references.
													if (mReferenceCount > 1) {
														// Multiple references, copy
														CDictionaryInternals*	dictionaryInternals =
																						new CDictionaryInternals();
														dictionaryInternals->mCount = mCount;
														dictionaryInternals->mReference = mReference;

														for (UInt32 i = 0; i < mItemInfosCount; i++) {
															// Setup for this linked list
															SDictionaryItemInfo*	itemInfo = mItemInfos[i];
															SDictionaryItemInfo*	dictionaryInternalsItemInfo = nil;

															while (itemInfo != nil) {
																// Check for first in the linked list
																if (dictionaryInternalsItemInfo == nil) {
																	// First in this linked list
																	dictionaryInternals->mItemInfos[i] =
																			new SDictionaryItemInfo(*itemInfo);
																	dictionaryInternalsItemInfo =
																			dictionaryInternals->mItemInfos[i];
																} else {
																	// Next one in this linked list
																	dictionaryInternalsItemInfo->mNextItemInfo =
																			new SDictionaryItemInfo(*itemInfo);
																	dictionaryInternalsItemInfo =
																			dictionaryInternalsItemInfo->mNextItemInfo;
																}

																// Next
																itemInfo = itemInfo->mNextItemInfo;
															}
														}

														// One less reference here
														mReferenceCount--;

														return dictionaryInternals;
													} else
														// Only a single reference
														return this;
												}
				SDictionaryValue*			getValue(const CString& key)
												{
													// Setup
													UInt32	hashValue = CHasher::getValueForHashable(key);
													UInt32	index = hashValue & (mItemInfosCount - 1);

													// Find item info that matches
													SDictionaryItemInfo*	itemInfo = mItemInfos[index];
													while ((itemInfo != nil) && !itemInfo->doesMatch(hashValue, key))
														// Advance to next item info
														itemInfo = itemInfo->mNextItemInfo;

													return (itemInfo != nil) ? &itemInfo->mItem.mValue : nil;
												}
				CDictionaryInternals*		set(const CString& key, const SDictionaryValue& value)
												{
													// Prepare for write
													CDictionaryInternals*	dictionaryInternals = prepareForWrite();

													// Setup
													UInt32	hashValue = CHasher::getValueForHashable(key);
													UInt32	index =
																	hashValue &
																			(dictionaryInternals->mItemInfosCount - 1);

													// Find
													SDictionaryItemInfo*	previousItemInfo = nil;
													SDictionaryItemInfo*	currentItemInfo =
																					dictionaryInternals->
																							mItemInfos[index];
													while ((currentItemInfo != nil) &&
															!currentItemInfo->doesMatch(hashValue, key)) {
														// Next in linked list
														previousItemInfo = currentItemInfo;
														currentItemInfo = currentItemInfo->mNextItemInfo;
													}

													// Check results
													if (currentItemInfo == nil) {
														// Did not find
														if (previousItemInfo == nil)
															// First one
															dictionaryInternals->mItemInfos[index] =
																	new SDictionaryItemInfo(hashValue,
																			SDictionaryItem(key, value));
														else
															// Add to the end
															previousItemInfo->mNextItemInfo =
																	new SDictionaryItemInfo(hashValue,
																			SDictionaryItem(key, value));

														// Update info
														dictionaryInternals->mCount++;
														dictionaryInternals->mReference++;
													} else {
														// Did find a match
														currentItemInfo->disposeValue();
														currentItemInfo->mItem.mValue = value;
													}

													return dictionaryInternals;
												}
				CDictionaryInternals*		remove(const CString& key)
												{
													// Prepare for write
													CDictionaryInternals*	dictionaryInternals = prepareForWrite();

													// Setup
													UInt32	hashValue = CHasher::getValueForHashable(key);
													UInt32	index =
																	hashValue &
																			(dictionaryInternals->mItemInfosCount - 1);

													// Find
													SDictionaryItemInfo*	previousItemInfo = nil;
													SDictionaryItemInfo*	currentItemInfo =
																					dictionaryInternals->
																							mItemInfos[index];
													while ((currentItemInfo != nil) &&
															!currentItemInfo->doesMatch(hashValue, key)) {
														// Next in linked list
														previousItemInfo = currentItemInfo;
														currentItemInfo = currentItemInfo->mNextItemInfo;
													}

													// Check results
													if (currentItemInfo != nil) {
														// Did find a match
														if (previousItemInfo == nil)
															// First item info
															dictionaryInternals->mItemInfos[index] =
																	currentItemInfo->mNextItemInfo;
														else
															// Not the first item info
															previousItemInfo->mNextItemInfo =
																	currentItemInfo->mNextItemInfo;

														// Cleanup
														remove(currentItemInfo, false);

														// Update info
														dictionaryInternals->mCount--;
														dictionaryInternals->mReference++;
													}

													return dictionaryInternals;
												}
				CDictionaryInternals*		removeAll()
												{
													// Prepare for write
													CDictionaryInternals*	dictionaryInternals = prepareForWrite();

													// Iterate all item infos
													for (UInt32 i = 0; i < dictionaryInternals->mItemInfosCount; i++) {
														// Check if have an item info
														if (dictionaryInternals->mItemInfos[i] != nil) {
															// Remove this chain
															remove(dictionaryInternals->mItemInfos[i], true);

															// Clear
															dictionaryInternals->mItemInfos[i] = nil;
														}
													}

													// Update info
													dictionaryInternals->mCount = 0;
													dictionaryInternals->mReference++;

													return dictionaryInternals;
												}
				void						remove(SDictionaryItemInfo* itemInfo, bool removeAll)
												{
													// Check for next item info
													if (removeAll && (itemInfo->mNextItemInfo != nil))
														// Remove the next item info
														remove(itemInfo->mNextItemInfo, true);

													// Dispose
													itemInfo->disposeValue();

													DisposeOf(itemInfo);
												}

				TIterator<SDictionaryItem>	getIterator() const
												{
													// Setup
													CDictionaryIteratorInfo*	iteratorInfo =
																						new CDictionaryIteratorInfo(
																								*this, mReference);

													// Find first item info
													while ((mItemInfos[iteratorInfo->mCurrentIndex] == nil) &&
															(++iteratorInfo->mCurrentIndex < mItemInfosCount)) ;

													SDictionaryItem*	firstItem = nil;
													if (iteratorInfo->mCurrentIndex < mItemInfosCount) {
														// Have first item info
														iteratorInfo->mCurrentItemInfo =
																mItemInfos[iteratorInfo->mCurrentIndex];
														firstItem = &mItemInfos[iteratorInfo->mCurrentIndex]->mItem;
													}

													return TIterator<SDictionaryItem>(firstItem, iteratorAdvance,
															*iteratorInfo);
												}

		static	void*						iteratorAdvance(CIteratorInfo& iteratorInfo)
												{
													// Setup
													CDictionaryIteratorInfo&	dictionaryIteratorInfo =
																						(CDictionaryIteratorInfo&)
																								iteratorInfo;

													// Internals check
													AssertFailIf(dictionaryIteratorInfo.mInitialReference !=
															dictionaryIteratorInfo.mInternals.mReference);

													// Check for additional item info in linked list
													if (dictionaryIteratorInfo.mCurrentItemInfo->mNextItemInfo != nil) {
														// Have next item info
														dictionaryIteratorInfo.mCurrentItemInfo =
																dictionaryIteratorInfo.mCurrentItemInfo->mNextItemInfo;
													} else {
														// End of item info linked list
														while ((++dictionaryIteratorInfo.mCurrentIndex <
																		dictionaryIteratorInfo.mInternals.
																				mItemInfosCount)
																&& (dictionaryIteratorInfo.mInternals.mItemInfos
																		[dictionaryIteratorInfo.mCurrentIndex] ==
																		nil)) ;

														// Check if found another item info
														if (dictionaryIteratorInfo.mCurrentIndex <
																dictionaryIteratorInfo.mInternals.mItemInfosCount)
															// Found another item info
															dictionaryIteratorInfo.mCurrentItemInfo =
																	dictionaryIteratorInfo.mInternals
																			.mItemInfos[
																					dictionaryIteratorInfo.
																							mCurrentIndex];
														else
															// No more item infos
															dictionaryIteratorInfo.mCurrentItemInfo = nil;
													}

													return (dictionaryIteratorInfo.mCurrentItemInfo != nil) ?
															(void*) &dictionaryIteratorInfo.mCurrentItemInfo->mItem :
															nil;
												}

		CDictionaryKeyCount		mCount;
		UInt32					mReferenceCount;
		UInt32					mReference;

		SDictionaryItemInfo**	mItemInfos;
		UInt32					mItemInfosCount;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CDictionary

CDictionary	CDictionary::mEmpty;

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CDictionary::CDictionary()
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	mInternals = new CDictionaryInternals();
}

//----------------------------------------------------------------------------------------------------------------------
CDictionary::CDictionary(const CDictionary& other)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	mInternals = other.mInternals->addReference();
}

//----------------------------------------------------------------------------------------------------------------------
CDictionary::~CDictionary()
//----------------------------------------------------------------------------------------------------------------------
{
	// Remove reference
	mInternals->removeReference();
}

// MARK: Instance methods

//----------------------------------------------------------------------------------------------------------------------
CDictionaryKeyCount CDictionary::getKeyCount() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mCount;
}

//----------------------------------------------------------------------------------------------------------------------
TSet<CString> CDictionary::getKeys() const
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	TSet<CString>	keys;

	// Iterate all items
	for (TIterator<SDictionaryItem> iterator = mInternals->getIterator(); iterator.hasValue(); iterator.advance())
		// Add key
		keys.add(iterator.getValue().mKey);

	return keys;
}

//----------------------------------------------------------------------------------------------------------------------
bool CDictionary::contains(const CString& key) const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->getValue(key) != nil;
}

//----------------------------------------------------------------------------------------------------------------------
bool CDictionary::getBool(const CString& key, bool notFoundValue) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Get value
	SDictionaryValue*	value = mInternals->getValue(key);

	// Check if have value
	if (value == nil)
		// Return not found value
		return notFoundValue;

	// Verify value type
	AssertFailIf(value->mValueType != kDictionaryValueTypeBool);
	if (value->mValueType != kDictionaryValueTypeBool)
		// Return not found value
		return notFoundValue;

	return value->mValue.mBool;
}

//----------------------------------------------------------------------------------------------------------------------
TArrayT<CDictionary> CDictionary::getArrayOfDictionaries(const CString& key, const TArrayT<CDictionary>& notFoundValue)
		const
//----------------------------------------------------------------------------------------------------------------------
{
	// Get value
	SDictionaryValue*	value = mInternals->getValue(key);

	// Check if have value
	if (value == nil)
		// Return not found value
		return notFoundValue;

	// Verify value type
	AssertFailIf(value->mValueType != kDictionaryValueTypeArrayOfDictionaries);
	if (value->mValueType != kDictionaryValueTypeArrayOfDictionaries)
		// Return not found value
		return notFoundValue;

	return *value->mValue.mArrayOfDictionaries;
}

//----------------------------------------------------------------------------------------------------------------------
TArrayT<CString> CDictionary::getArrayOfStrings(const CString& key, const TArrayT<CString>& notFoundValue) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Get value
	SDictionaryValue*	value = mInternals->getValue(key);

	// Check if have value
	if (value == nil)
		// Return not found value
		return notFoundValue;

	// Verify value type
	AssertFailIf(value->mValueType != kDictionaryValueTypeArrayOfStrings);
	if (value->mValueType != kDictionaryValueTypeArrayOfStrings)
		// Return not found value
		return notFoundValue;

	return *value->mValue.mArrayOfStrings;
}

//----------------------------------------------------------------------------------------------------------------------
CData CDictionary::getData(const CString& key, const CData& notFoundValue) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Get value
	SDictionaryValue*	value = mInternals->getValue(key);

	// Check if have value
	if (value == nil)
		// Return not found value
		return notFoundValue;

	// Verify value type
	AssertFailIf(value->mValueType != kDictionaryValueTypeData);
	if (value->mValueType != kDictionaryValueTypeData)
		// Return not found value
		return notFoundValue;

	return *value->mValue.mData;
}

//----------------------------------------------------------------------------------------------------------------------
CDictionary CDictionary::getDictionary(const CString& key, const CDictionary& notFoundValue) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Get value
	SDictionaryValue*	value = mInternals->getValue(key);

	// Check if have value
	if (value == nil)
		// Return not found value
		return notFoundValue;

	// Verify value type
	AssertFailIf(value->mValueType != kDictionaryValueTypeDictionary);
	if (value->mValueType != kDictionaryValueTypeDictionary)
		// Return not found value
		return notFoundValue;

	return *value->mValue.mDictionary;
}

//----------------------------------------------------------------------------------------------------------------------
CString CDictionary::getString(const CString& key, const CString& notFoundValue) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Get value
	SDictionaryValue*	value = mInternals->getValue(key);

	// Check if have value
	if (value == nil)
		// Return not found value
		return notFoundValue;

	// Verify value type
	AssertFailIf(value->mValueType != kDictionaryValueTypeString);
	if (value->mValueType != kDictionaryValueTypeString)
		// Return not found value
		return notFoundValue;

	return *value->mValue.mString;
}

//----------------------------------------------------------------------------------------------------------------------
Float32 CDictionary::getFloat32(const CString& key, Float32 notFoundValue) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Get value
	SDictionaryValue*	value = mInternals->getValue(key);

	// Check if have value
	if (value == nil)
		// Return not found value
		return notFoundValue;

	// Verify value type
	AssertFailIf(value->mValueType != kDictionaryValueTypeFloat32);
	if (value->mValueType != kDictionaryValueTypeFloat32)
		// Return not found value
		return notFoundValue;

	return value->mValue.mFloat32;
}

//----------------------------------------------------------------------------------------------------------------------
Float64 CDictionary::getFloat64(const CString& key, Float64 notFoundValue) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Get value
	SDictionaryValue*	value = mInternals->getValue(key);

	// Check if have value
	if (value == nil)
		// Return not found value
		return notFoundValue;

	// Verify value type
	AssertFailIf(value->mValueType != kDictionaryValueTypeFloat64);
	if (value->mValueType != kDictionaryValueTypeFloat64)
		// Return not found value
		return notFoundValue;

	return value->mValue.mFloat64;
}

//----------------------------------------------------------------------------------------------------------------------
SInt8 CDictionary::getSInt8(const CString& key, SInt8 notFoundValue) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Get value
	SDictionaryValue*	value = mInternals->getValue(key);

	// Check if have value
	if (value == nil)
		// Return not found value
		return notFoundValue;

	// Check value type
	switch (value->mValueType) {
		case kDictionaryValueTypeSInt8:		return value->mValue.mSInt8;
		case kDictionaryValueTypeSInt16:	return value->mValue.mSInt16;
		case kDictionaryValueTypeSInt32:	return value->mValue.mSInt32;
		case kDictionaryValueTypeSInt64:	return value->mValue.mSInt64;
		case kDictionaryValueTypeUInt8:		return value->mValue.mUInt8;
		case kDictionaryValueTypeUInt16:	return value->mValue.mUInt16;
		case kDictionaryValueTypeUInt32:	return value->mValue.mUInt32;
		case kDictionaryValueTypeUInt64:	return value->mValue.mUInt64;
		default:
			// Cannot coerce value
			AssertFailWith(kAssertFailedError);

			return notFoundValue;
	}
}

//----------------------------------------------------------------------------------------------------------------------
SInt16 CDictionary::getSInt16(const CString& key, SInt16 notFoundValue) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Get value
	SDictionaryValue*	value = mInternals->getValue(key);

	// Check if have value
	if (value == nil)
		// Return not found value
		return notFoundValue;

	// Check value type
	switch (value->mValueType) {
		case kDictionaryValueTypeSInt8:		return value->mValue.mSInt8;
		case kDictionaryValueTypeSInt16:	return value->mValue.mSInt16;
		case kDictionaryValueTypeSInt32:	return value->mValue.mSInt32;
		case kDictionaryValueTypeSInt64:	return value->mValue.mSInt64;
		case kDictionaryValueTypeUInt8:		return value->mValue.mUInt8;
		case kDictionaryValueTypeUInt16:	return value->mValue.mUInt16;
		case kDictionaryValueTypeUInt32:	return value->mValue.mUInt32;
		case kDictionaryValueTypeUInt64:	return value->mValue.mUInt64;
		default:
			// Cannot coerce value
			AssertFailWith(kAssertFailedError);

			return notFoundValue;
	}
}

//----------------------------------------------------------------------------------------------------------------------
SInt32 CDictionary::getSInt32(const CString& key, SInt32 notFoundValue) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Get value
	SDictionaryValue*	value = mInternals->getValue(key);

	// Check if have value
	if (value == nil)
		// Return not found value
		return notFoundValue;

	// Check value type
	switch (value->mValueType) {
		case kDictionaryValueTypeSInt8:		return value->mValue.mSInt8;
		case kDictionaryValueTypeSInt16:	return value->mValue.mSInt16;
		case kDictionaryValueTypeSInt32:	return value->mValue.mSInt32;
		case kDictionaryValueTypeSInt64:	return (SInt32) value->mValue.mSInt64;
		case kDictionaryValueTypeUInt8:		return value->mValue.mUInt8;
		case kDictionaryValueTypeUInt16:	return value->mValue.mUInt16;
		case kDictionaryValueTypeUInt32:	return value->mValue.mUInt32;
		case kDictionaryValueTypeUInt64:	return (SInt32) value->mValue.mUInt64;
		default:
			// Cannot coerce value
			AssertFailWith(kAssertFailedError);

			return notFoundValue;
	}
}

//----------------------------------------------------------------------------------------------------------------------
SInt64 CDictionary::getSInt64(const CString& key, SInt64 notFoundValue) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Get value
	SDictionaryValue*	value = mInternals->getValue(key);

	// Check if have value
	if (value == nil)
		// Return not found value
		return notFoundValue;

	// Check value type
	switch (value->mValueType) {
		case kDictionaryValueTypeSInt8:		return value->mValue.mSInt8;
		case kDictionaryValueTypeSInt16:	return value->mValue.mSInt16;
		case kDictionaryValueTypeSInt32:	return value->mValue.mSInt32;
		case kDictionaryValueTypeSInt64:	return value->mValue.mSInt64;
		case kDictionaryValueTypeUInt8:		return value->mValue.mUInt8;
		case kDictionaryValueTypeUInt16:	return value->mValue.mUInt16;
		case kDictionaryValueTypeUInt32:	return value->mValue.mUInt32;
		case kDictionaryValueTypeUInt64:	return value->mValue.mUInt64;
		default:
			// Cannot coerce value
			AssertFailWith(kAssertFailedError);

			return notFoundValue;
	}
}

//----------------------------------------------------------------------------------------------------------------------
UInt8 CDictionary::getUInt8(const CString& key, UInt8 notFoundValue) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Get value
	SDictionaryValue*	value = mInternals->getValue(key);

	// Check if have value
	if (value == nil)
		// Return not found value
		return notFoundValue;

	// Check value type
	switch (value->mValueType) {
		case kDictionaryValueTypeSInt8:		return value->mValue.mSInt8;
		case kDictionaryValueTypeSInt16:	return value->mValue.mSInt16;
		case kDictionaryValueTypeSInt32:	return value->mValue.mSInt32;
		case kDictionaryValueTypeSInt64:	return value->mValue.mSInt64;
		case kDictionaryValueTypeUInt8:		return value->mValue.mUInt8;
		case kDictionaryValueTypeUInt16:	return value->mValue.mUInt16;
		case kDictionaryValueTypeUInt32:	return value->mValue.mUInt32;
		case kDictionaryValueTypeUInt64:	return value->mValue.mUInt64;
		default:
			// Cannot coerce value
			AssertFailWith(kAssertFailedError);

			return notFoundValue;
	}
}

//----------------------------------------------------------------------------------------------------------------------
UInt16 CDictionary::getUInt16(const CString& key, UInt16 notFoundValue) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Get value
	SDictionaryValue*	value = mInternals->getValue(key);

	// Check if have value
	if (value == nil)
		// Return not found value
		return notFoundValue;

	// Check value type
	switch (value->mValueType) {
		case kDictionaryValueTypeSInt8:		return value->mValue.mSInt8;
		case kDictionaryValueTypeSInt16:	return value->mValue.mSInt16;
		case kDictionaryValueTypeSInt32:	return value->mValue.mSInt32;
		case kDictionaryValueTypeSInt64:	return value->mValue.mSInt64;
		case kDictionaryValueTypeUInt8:		return value->mValue.mUInt8;
		case kDictionaryValueTypeUInt16:	return value->mValue.mUInt16;
		case kDictionaryValueTypeUInt32:	return value->mValue.mUInt32;
		case kDictionaryValueTypeUInt64:	return value->mValue.mUInt64;
		default:
			// Cannot coerce value
			AssertFailWith(kAssertFailedError);

			return notFoundValue;
	}
}

//----------------------------------------------------------------------------------------------------------------------
UInt32 CDictionary::getUInt32(const CString& key, UInt32 notFoundValue) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Get value
	SDictionaryValue*	value = mInternals->getValue(key);

	// Check if have value
	if (value == nil)
		// Return not found value
		return notFoundValue;

	// Check value type
	switch (value->mValueType) {
		case kDictionaryValueTypeSInt8:		return value->mValue.mSInt8;
		case kDictionaryValueTypeSInt16:	return value->mValue.mSInt16;
		case kDictionaryValueTypeSInt32:	return value->mValue.mSInt32;
		case kDictionaryValueTypeSInt64:	return (UInt32) value->mValue.mSInt64;
		case kDictionaryValueTypeUInt8:		return value->mValue.mUInt8;
		case kDictionaryValueTypeUInt16:	return value->mValue.mUInt16;
		case kDictionaryValueTypeUInt32:	return value->mValue.mUInt32;
		case kDictionaryValueTypeUInt64:	return (UInt32) value->mValue.mUInt64;
		default:
			// Cannot coerce value
			AssertFailWith(kAssertFailedError);

			return notFoundValue;
	}
}

//----------------------------------------------------------------------------------------------------------------------
UInt64 CDictionary::getUInt64(const CString& key, UInt64 notFoundValue) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Get value
	SDictionaryValue*	value = mInternals->getValue(key);

	// Check if have value
	if (value == nil)
		// Return not found value
		return notFoundValue;

	// Check value type
	switch (value->mValueType) {
		case kDictionaryValueTypeSInt8:		return value->mValue.mSInt8;
		case kDictionaryValueTypeSInt16:	return value->mValue.mSInt16;
		case kDictionaryValueTypeSInt32:	return value->mValue.mSInt32;
		case kDictionaryValueTypeSInt64:	return value->mValue.mSInt64;
		case kDictionaryValueTypeUInt8:		return value->mValue.mUInt8;
		case kDictionaryValueTypeUInt16:	return value->mValue.mUInt16;
		case kDictionaryValueTypeUInt32:	return value->mValue.mUInt32;
		case kDictionaryValueTypeUInt64:	return value->mValue.mUInt64;
		default:
			// Cannot coerce value
			AssertFailWith(kAssertFailedError);

			return notFoundValue;
	}
}

//----------------------------------------------------------------------------------------------------------------------
CDictionaryItemRef CDictionary::getItemRef(const CString& key) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Get value
	SDictionaryValue*	value = mInternals->getValue(key);

	// Check if have value
	if (value == nil)
		// Return not found value
		return nil;

	// Verify value type
	AssertFailIf(value->mValueType != kDictionaryValueItemRef);
	if (value->mValueType != kDictionaryValueItemRef)
		// Return not found value
		return nil;

	return value->mValue.mItemRef;
}

//----------------------------------------------------------------------------------------------------------------------
void CDictionary::set(const CString& key, bool value)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	SDictionaryValue	dictionaryValue;
	dictionaryValue.mValueType = kDictionaryValueTypeBool;
	dictionaryValue.mValue.mBool = value;

	// Store
	mInternals = mInternals->set(key, dictionaryValue);
}

//----------------------------------------------------------------------------------------------------------------------
void CDictionary::set(const CString& key, const TArrayT<CDictionary>& value)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	SDictionaryValue	dictionaryValue;
	dictionaryValue.mValueType = kDictionaryValueTypeArrayOfDictionaries;
	dictionaryValue.mValue.mArrayOfDictionaries = new TArrayT<CDictionary>(value);

	// Store
	mInternals = mInternals->set(key, dictionaryValue);
}

//----------------------------------------------------------------------------------------------------------------------
void CDictionary::set(const CString& key, const TArrayT<CString>& value)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	SDictionaryValue	dictionaryValue;
	dictionaryValue.mValueType = kDictionaryValueTypeArrayOfStrings;
	dictionaryValue.mValue.mArrayOfStrings = new TArrayT<CString>(value);

	// Store
	mInternals = mInternals->set(key, dictionaryValue);
}

//----------------------------------------------------------------------------------------------------------------------
void CDictionary::set(const CString& key, const CData& value)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	SDictionaryValue	dictionaryValue;
	dictionaryValue.mValueType = kDictionaryValueTypeData;
	dictionaryValue.mValue.mData = new CData(value);

	// Store
	mInternals = mInternals->set(key, dictionaryValue);
}

//----------------------------------------------------------------------------------------------------------------------
void CDictionary::set(const CString& key, const CDictionary& value)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	SDictionaryValue	dictionaryValue;
	dictionaryValue.mValueType = kDictionaryValueTypeDictionary;
	dictionaryValue.mValue.mDictionary = new CDictionary(value);

	// Store
	mInternals = mInternals->set(key, dictionaryValue);
}

//----------------------------------------------------------------------------------------------------------------------
void CDictionary::set(const CString& key, const CString& value)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	SDictionaryValue	dictionaryValue;
	dictionaryValue.mValueType = kDictionaryValueTypeString;
	dictionaryValue.mValue.mString = new CString(value);

	// Store
	mInternals = mInternals->set(key, dictionaryValue);
}

//----------------------------------------------------------------------------------------------------------------------
void CDictionary::set(const CString& key, Float32 value)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	SDictionaryValue	dictionaryValue;
	dictionaryValue.mValueType = kDictionaryValueTypeFloat32;
	dictionaryValue.mValue.mFloat32 = value;

	// Store
	mInternals = mInternals->set(key, dictionaryValue);
}

//----------------------------------------------------------------------------------------------------------------------
void CDictionary::set(const CString& key, Float64 value)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	SDictionaryValue	dictionaryValue;
	dictionaryValue.mValueType = kDictionaryValueTypeFloat64;
	dictionaryValue.mValue.mFloat64 = value;

	// Store
	mInternals = mInternals->set(key, dictionaryValue);
}

//----------------------------------------------------------------------------------------------------------------------
void CDictionary::set(const CString& key, SInt8 value)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	SDictionaryValue	dictionaryValue;
	dictionaryValue.mValueType = kDictionaryValueTypeSInt8;
	dictionaryValue.mValue.mSInt8 = value;

	// Store
	mInternals = mInternals->set(key, dictionaryValue);
}

//----------------------------------------------------------------------------------------------------------------------
void CDictionary::set(const CString& key, SInt16 value)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	SDictionaryValue	dictionaryValue;
	dictionaryValue.mValueType = kDictionaryValueTypeSInt16;
	dictionaryValue.mValue.mSInt16 = value;

	// Store
	mInternals = mInternals->set(key, dictionaryValue);
}

//----------------------------------------------------------------------------------------------------------------------
void CDictionary::set(const CString& key, SInt32 value)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	SDictionaryValue	dictionaryValue;
	dictionaryValue.mValueType = kDictionaryValueTypeSInt32;
	dictionaryValue.mValue.mSInt32 = value;

	// Store
	mInternals = mInternals->set(key, dictionaryValue);
}

//----------------------------------------------------------------------------------------------------------------------
void CDictionary::set(const CString& key, SInt64 value)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	SDictionaryValue	dictionaryValue;
	dictionaryValue.mValueType = kDictionaryValueTypeSInt64;
	dictionaryValue.mValue.mSInt64 = value;

	// Store
	mInternals = mInternals->set(key, dictionaryValue);
}

//----------------------------------------------------------------------------------------------------------------------
void CDictionary::set(const CString& key, UInt8 value)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	SDictionaryValue	dictionaryValue;
	dictionaryValue.mValueType = kDictionaryValueTypeUInt8;
	dictionaryValue.mValue.mUInt8 = value;

	// Store
	mInternals = mInternals->set(key, dictionaryValue);
}

//----------------------------------------------------------------------------------------------------------------------
void CDictionary::set(const CString& key, UInt16 value)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	SDictionaryValue	dictionaryValue;
	dictionaryValue.mValueType = kDictionaryValueTypeUInt16;
	dictionaryValue.mValue.mUInt16 = value;

	// Store
	mInternals = mInternals->set(key, dictionaryValue);
}

//----------------------------------------------------------------------------------------------------------------------
void CDictionary::set(const CString& key, UInt32 value)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	SDictionaryValue	dictionaryValue;
	dictionaryValue.mValueType = kDictionaryValueTypeUInt32;
	dictionaryValue.mValue.mUInt32 = value;

	// Store
	mInternals = mInternals->set(key, dictionaryValue);
}

//----------------------------------------------------------------------------------------------------------------------
void CDictionary::set(const CString& key, UInt64 value)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	SDictionaryValue	dictionaryValue;
	dictionaryValue.mValueType = kDictionaryValueTypeUInt64;
	dictionaryValue.mValue.mUInt64 = value;

	// Store
	mInternals = mInternals->set(key, dictionaryValue);
}

//----------------------------------------------------------------------------------------------------------------------
void CDictionary::set(const CString& key, CDictionaryItemRef value)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	SDictionaryValue	dictionaryValue;
	dictionaryValue.mValueType = kDictionaryValueItemRef;
	dictionaryValue.mValue.mItemRef = value;

	// Store
	mInternals = mInternals->set(key, dictionaryValue);
}

//----------------------------------------------------------------------------------------------------------------------
void CDictionary::remove(const CString& key)
//----------------------------------------------------------------------------------------------------------------------
{
	// Remove
	mInternals = mInternals->remove(key);
}

//----------------------------------------------------------------------------------------------------------------------
void CDictionary::removeAll()
//----------------------------------------------------------------------------------------------------------------------
{
	// Remove all
	mInternals = mInternals->removeAll();
}

//----------------------------------------------------------------------------------------------------------------------
TIterator<SDictionaryItem> CDictionary::getIterator() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->getIterator();
}

//----------------------------------------------------------------------------------------------------------------------
CDictionary& CDictionary::operator=(const CDictionary& other)
//----------------------------------------------------------------------------------------------------------------------
{
	// Remove reference to ourselves
	mInternals->removeReference();

	// Add reference to other
	mInternals = other.mInternals->addReference();

	return *this;
}

////----------------------------------------------------------------------------------------------------------------------
//bool CDictionary::operator==(const CDictionary& other) const
////----------------------------------------------------------------------------------------------------------------------
//{
//}
