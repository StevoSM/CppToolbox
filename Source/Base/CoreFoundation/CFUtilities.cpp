//----------------------------------------------------------------------------------------------------------------------
//	CFUtilities.cpp			©2019 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CFUtilities.h"

#include "CCoreServices.h"
#include "CData.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: Array utilities

//----------------------------------------------------------------------------------------------------------------------
TArray<CData> eArrayOfDatasFrom(CFArrayRef arrayRef)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	TArray<CData>	array;

	// Get values
	CFIndex		count = ::CFArrayGetCount(arrayRef);
	CFDataRef	dataRefs[count];
	::CFArrayGetValues(arrayRef, CFRangeMake(0, count), (const void**) &dataRefs);
	for (CFIndex i = 0; i < count; i++)
		// Add data
		array += eDataFrom(dataRefs[i]);

	return array;
}

//----------------------------------------------------------------------------------------------------------------------
TArray<CDictionary> eArrayOfDictionariesFrom(CFArrayRef arrayRef)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	TArray<CDictionary>	array;

	// Get values
	CFIndex			count = ::CFArrayGetCount(arrayRef);
	CFDictionaryRef	dictionaryRefs[count];
	::CFArrayGetValues(arrayRef, CFRangeMake(0, count), (const void**) &dictionaryRefs);
	for (CFIndex i = 0; i < count; i++)
		// Add dictionary
		array += CDictionary(eDictionaryFrom(dictionaryRefs[i]));

	return array;
}

//----------------------------------------------------------------------------------------------------------------------
CFArrayRef eArrayCopyCFArrayRef(const TArray<CDictionary>& array)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	CFMutableArrayRef	arrayRef =
								::CFArrayCreateMutable(kCFAllocatorDefault, array.getCount(), &kCFTypeArrayCallBacks);
	for (CArrayItemIndex i = 0; i < array.getCount(); i++) {
		// Add dictionary
		CFDictionaryRef	dictionaryRef = eDictionaryCopyCFDictionaryRef(array[i]);
		::CFArrayAppendValue(arrayRef, dictionaryRef);
		::CFRelease(dictionaryRef);
	}

	return arrayRef;
}

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - Data utilities

//----------------------------------------------------------------------------------------------------------------------
CData eDataFrom(CFDataRef dataRef)
//----------------------------------------------------------------------------------------------------------------------
{
	return CData(::CFDataGetBytePtr(dataRef), (CDataSize) ::CFDataGetLength(dataRef));
}

//----------------------------------------------------------------------------------------------------------------------
CFDataRef eDataCopyCFDataRef(const CData& data)
//----------------------------------------------------------------------------------------------------------------------
{
	return ::CFDataCreate(kCFAllocatorDefault, (const UInt8*) data.getBytePtr(), data.getSize());
}

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - Dictionary utilities

//----------------------------------------------------------------------------------------------------------------------
CDictionary eDictionaryFrom(CFDictionaryRef dictionaryRef)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	CDictionary	dictionary;

	// Get keys and values
	CFIndex		count = ::CFDictionaryGetCount(dictionaryRef);
	CFStringRef	keyStringRefs[count];
	CFTypeRef	valueTypeRefs[count];
	::CFDictionaryGetKeysAndValues(dictionaryRef, (const void**) &keyStringRefs, (const void**) &valueTypeRefs);

	// Add all items
	for (CFIndex i = 0; i < count; i++) {
		// What type
		CFTypeRef	valueTypeRef = valueTypeRefs[i];
		if (::CFGetTypeID(valueTypeRef) == ::CFBooleanGetTypeID())
			// Boolean
			dictionary.set(CString(keyStringRefs[i]), valueTypeRef == ::kCFBooleanTrue);
		else if (::CFGetTypeID(valueTypeRef) == ::CFArrayGetTypeID()) {
			// Array
			CFArrayRef	arrayRef = (CFArrayRef) valueTypeRef;
			if (::CFArrayGetCount(arrayRef) > 0) {
				// What type
				CFTypeRef	arrayValueTypeRef = ::CFArrayGetValueAtIndex(arrayRef, 0);
				if (::CFGetTypeID(arrayValueTypeRef) == ::CFDictionaryGetTypeID())
					// Array of dictionaries
					dictionary.set(CString(keyStringRefs[i]), eArrayOfDictionariesFrom(arrayRef));
				else
					// Uh oh
					CCoreServices::stopInDebugger();
			}
		} else if (::CFGetTypeID(valueTypeRef) == ::CFDataGetTypeID())
			// Data
			dictionary.set(CString(keyStringRefs[i]), eDataFrom((CFDataRef) valueTypeRef));
		else if (::CFGetTypeID(valueTypeRef) == ::CFDictionaryGetTypeID())
			// Dictionary
			dictionary.set(CString(keyStringRefs[i]), eDictionaryFrom((CFDictionaryRef) valueTypeRef));
		else if (::CFGetTypeID(valueTypeRef) == ::CFNumberGetTypeID()) {
			// Number
			switch (::CFNumberGetType((CFNumberRef) valueTypeRef)) {
				case kCFNumberFloat32Type:
				case kCFNumberFloatType: {
					// Float 32
					Float32	float32;
					::CFNumberGetValue((CFNumberRef) valueTypeRef, kCFNumberFloat32Type, &float32);
					dictionary.set(CString(keyStringRefs[i]), float32);
				} break;

				case kCFNumberFloat64Type:
				case kCFNumberDoubleType: {
					// Float 64
					Float64	float64;
					::CFNumberGetValue((CFNumberRef) valueTypeRef, kCFNumberFloat64Type, &float64);
					dictionary.set(CString(keyStringRefs[i]), float64);
				} break;

				case kCFNumberSInt8Type:
				case kCFNumberSInt16Type:
				case kCFNumberSInt32Type:
				case kCFNumberSInt64Type:
				case kCFNumberCharType:
				case kCFNumberShortType:
				case kCFNumberIntType:
				case kCFNumberLongType:
				case kCFNumberLongLongType: {
					// Integer
					switch (::CFNumberGetByteSize((CFNumberRef) valueTypeRef)) {
						case 1: {
							// SInt8
							SInt8	sInt8;
							::CFNumberGetValue((CFNumberRef) valueTypeRef, kCFNumberSInt8Type, &sInt8);
							dictionary.set(CString(keyStringRefs[i]), sInt8);
						} break;

						case 2: {
							// SInt16
							SInt16	sInt16;
							::CFNumberGetValue((CFNumberRef) valueTypeRef, kCFNumberSInt16Type, &sInt16);
							dictionary.set(CString(keyStringRefs[i]), sInt16);
						} break;

						case 4: {
							// SInt32
							SInt32	sInt32;
							::CFNumberGetValue((CFNumberRef) valueTypeRef, kCFNumberSInt32Type, &sInt32);
							dictionary.set(CString(keyStringRefs[i]), sInt32);
						} break;

						case 8: {
							// SInt64
							SInt64	sInt64;
							::CFNumberGetValue((CFNumberRef) valueTypeRef, kCFNumberSInt64Type, &sInt64);
							dictionary.set(CString(keyStringRefs[i]), sInt64);
						} break;

						default:
							// Uh oh
							CCoreServices::stopInDebugger();
					}
				} break;

				default:
					// The rest are unsupported (unseen actually)
					break;
			}
		} else if (::CFGetTypeID(valueTypeRef) == ::CFStringGetTypeID())
			// String
			dictionary.set(CString(keyStringRefs[i]), CString((CFStringRef) valueTypeRef));
		else {
			// Uh oh
			CCoreServices::stopInDebugger();
		}
	}

	return dictionary;
}

//----------------------------------------------------------------------------------------------------------------------
CFDictionaryRef eDictionaryCopyCFDictionaryRef(const CDictionary& dictionary)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	CFMutableDictionaryRef	dictionaryRef =
									::CFDictionaryCreateMutable(kCFAllocatorDefault, 0, &kCFTypeDictionaryKeyCallBacks,
											&kCFTypeDictionaryValueCallBacks);

	// Copy all items
	for (TIteratorS<SDictionaryItem> iterator = dictionary.getIterator(); iterator.hasValue(); iterator.advance()) {
		// Get info
		const	CString&			key = iterator.getValue().mKey;
				CFStringRef			keyStringRef = eStringCopyCFStringRef(key);

		// Store value in dictionary
		const	SDictionaryValue&	value = iterator.getValue().mValue;
		switch (value.mValueType) {
			case kDictionaryValueTypeBool:
				// Bool
				::CFDictionarySetValue(dictionaryRef, keyStringRef,
						value.mValue.mBool ? kCFBooleanTrue : kCFBooleanFalse);
				break;

			case kDictionaryValueTypeArrayOfDictionaries: {
				// Array of dictionaries
				CFArrayRef	arrayRef = eArrayCopyCFArrayRef(*value.mValue.mArrayOfDictionaries);
				::CFDictionarySetValue(dictionaryRef, keyStringRef, arrayRef);
				::CFRelease(arrayRef);
				} break;

			case kDictionaryValueTypeData: {
				// Data
				CFDataRef	dataRef = eDataCopyCFDataRef(*value.mValue.mData);
				::CFDictionarySetValue(dictionaryRef, keyStringRef, dataRef);
				::CFRelease(dataRef);
				} break;

			case kDictionaryValueTypeDictionary: {
				// Dictionary
				CFDictionaryRef	valueDictionaryRef = eDictionaryCopyCFDictionaryRef(*value.mValue.mDictionary);
				::CFDictionarySetValue(dictionaryRef, keyStringRef, valueDictionaryRef);
				::CFRelease(valueDictionaryRef);
				} break;

			case kDictionaryValueTypeString: {
				// String
				CFStringRef	stringRef = eStringCopyCFStringRef(*value.mValue.mString);
				::CFDictionarySetValue(dictionaryRef, keyStringRef, stringRef);
				::CFRelease(stringRef);
				} break;

			case kDictionaryValueTypeFloat32: {
				// Float32
				CFNumberRef	numberRef =
									::CFNumberCreate(kCFAllocatorDefault, kCFNumberFloat32Type, &value.mValue.mFloat32);
				::CFDictionarySetValue(dictionaryRef, keyStringRef, numberRef);
				::CFRelease(numberRef);
				} break;

			case kDictionaryValueTypeFloat64: {
				// Float64
				CFNumberRef	numberRef =
									::CFNumberCreate(kCFAllocatorDefault, kCFNumberFloat64Type, &value.mValue.mFloat64);
				::CFDictionarySetValue(dictionaryRef, keyStringRef, numberRef);
				::CFRelease(numberRef);
				} break;

			case kDictionaryValueTypeSInt8: {
				// SInt8
				CFNumberRef	numberRef = ::CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt8Type, &value.mValue.mSInt8);
				::CFDictionarySetValue(dictionaryRef, keyStringRef, numberRef);
				::CFRelease(numberRef);
				} break;

			case kDictionaryValueTypeSInt16: {
				// SInt16
				CFNumberRef	numberRef =
									::CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt16Type, &value.mValue.mSInt16);
				::CFDictionarySetValue(dictionaryRef, keyStringRef, numberRef);
				::CFRelease(numberRef);
				} break;

			case kDictionaryValueTypeSInt32: {
				// SInt32
				CFNumberRef	numberRef =
									::CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt32Type, &value.mValue.mSInt32);
				::CFDictionarySetValue(dictionaryRef, keyStringRef, numberRef);
				::CFRelease(numberRef);
				} break;

			case kDictionaryValueTypeSInt64: {
				// SInt64
				CFNumberRef	numberRef =
									::CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt64Type, &value.mValue.mSInt64);
				::CFDictionarySetValue(dictionaryRef, keyStringRef, numberRef);
				::CFRelease(numberRef);
				} break;

			case kDictionaryValueTypeUInt8: {
				// UInt8
				SInt64		sInt64 = value.mValue.mUInt8;
				CFNumberRef	numberRef = ::CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt64Type, &sInt64);
				::CFDictionarySetValue(dictionaryRef, keyStringRef, numberRef);
				::CFRelease(numberRef);
				} break;

			case kDictionaryValueTypeUInt16: {
				// UInt16
				SInt64		sInt64 = value.mValue.mUInt16;
				CFNumberRef	numberRef = ::CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt64Type, &sInt64);
				::CFDictionarySetValue(dictionaryRef, keyStringRef, numberRef);
				::CFRelease(numberRef);
				} break;

			case kDictionaryValueTypeUInt32: {
				// UInt32
				SInt64		sInt64 = value.mValue.mUInt32;
				CFNumberRef	numberRef = ::CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt64Type, &sInt64);
				::CFDictionarySetValue(dictionaryRef, keyStringRef, numberRef);
				::CFRelease(numberRef);
				} break;

			case kDictionaryValueTypeUInt64: {
				// UInt64
				SInt64		sInt64 = value.mValue.mUInt64;
				CFNumberRef	numberRef = ::CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt64Type, &sInt64);
				::CFDictionarySetValue(dictionaryRef, keyStringRef, numberRef);
				::CFRelease(numberRef);
				} break;

			case kDictionaryValueItemRef:
				// Something else that cannot be represented by Core Foundation
				break;
		}

		// Cleanup
		::CFRelease(keyStringRef);
	}

	return dictionaryRef;
}

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - String utilities

//----------------------------------------------------------------------------------------------------------------------
CFStringRef eStringCopyCFStringRef(const CString& string)
//----------------------------------------------------------------------------------------------------------------------
{
	return ::CFStringCreateWithCString(kCFAllocatorDefault, string.getCString(kStringEncodingUTF8),
			kCFStringEncodingUTF8);
}
