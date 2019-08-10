//----------------------------------------------------------------------------------------------------------------------
//	CDataSource.h			©2019 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CDataProvider.h"
#include "CString.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CDataSource

class CDataSourceInternals;
class CDataSource {
	// Methods
	public:
				// Lifecycle methods
				CDataSource(const CDataProvider* dataProvider);	// Will take ownership of CDataProvider
				CDataSource(const CDataSource& other);
				~CDataSource();

				// Instance methods
		UInt64	getSize() const;

		UError	readData(void* buffer, UInt64 byteCount) const;
		CData	readData(UInt64 byteCount, UError& outError) const;
		CData	readData(UError& outError) const;
		SInt8	readSInt8(UError& outError) const
					{
						// Read
						SInt8	value = 0;
						outError = readData(&value, sizeof(SInt8));

						return value;
					}
		SInt16	readSInt16(UError& outError) const
					{
						// Read
						SInt16	value = 0;
						outError = readData(&value, sizeof(SInt16));

						return value;
					}
		SInt32	readSInt32(UError& outError) const
					{
						// Read
						SInt32	value = 0;
						outError = readData(&value, sizeof(SInt32));

						return value;
					}
		SInt64	readSInt64(UError& outError) const
					{
						// Read
						SInt64	value = 0;
						outError = readData(&value, sizeof(SInt64));

						return value;
					}
		UInt8	readUInt8(UError& outError) const
					{
						// Read
						UInt8	value = 0;
						outError = readData(&value, sizeof(UInt8));

						return value;
					}
		UInt16	readUInt16(UError& outError) const
					{
						// Read
						UInt16	value = 0;
						outError = readData(&value, sizeof(UInt16));

						return value;
					}
		UInt32	readUInt32(UError& outError) const
					{
						// Read
						UInt32	value = 0;
						outError = readData(&value, sizeof(UInt32));

						return value;
					}
		UInt64	readUInt64(UError& outError) const
					{
						// Read
						UInt64	value = 0;
						outError = readData(&value, sizeof(UInt64));

						return value;
					}
		OSType	readOSType(UError& outError) const
					{
						// Read
						OSType	value = 0;
						outError = readData(&value, sizeof(OSType));

						return value;
					}

		SInt64	getPos() const;
		UError	setPos(EDataProviderPosition position, SInt64 newPos) const;

		void	reset() const;

	// Properties
	private:
		CDataSourceInternals*	mInternals;
};
