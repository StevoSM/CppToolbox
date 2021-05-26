//----------------------------------------------------------------------------------------------------------------------
//	CAudioCodec.h			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CAudioFrames.h"
#include "CCodec.h"
#include "CDataSource.h"
#include "SAudioFormats.h"
#include "SAudioReadStatus.h"
#include "SMediaPosition.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CAudioCodec

class CAudioCodec : public CCodec {
	// Structs
	public:
		struct Info {
			// Procs
			typedef	TArray<SAudioProcessingSetup>	(*GetAudioProcessingSetupsProc)(OSType id,
															const SAudioStorageFormat& audioStorageFormat);
			typedef	I<CAudioCodec>					(*InstantiateProc)(OSType id);

													// Lifecycle methods
													Info(OSType id, const CString& name,
															GetAudioProcessingSetupsProc getAudioProcessingSetupsProc,
															InstantiateProc instantiateProc) :
														mID(id), mDecodeName(name), mEncodeName(name),
																mGetAudioProcessingSetupsProc(
																		getAudioProcessingSetupsProc),
																mInstantiateProc(instantiateProc)
														{}
													Info(OSType id, const CString& name,
															const EncodeSettings& encodeSettings,
															GetAudioProcessingSetupsProc getAudioProcessingSetupsProc,
															InstantiateProc instantiateProc) :
														mID(id), mDecodeName(name), mEncodeName(name),
																mEncodeSettings(OI<EncodeSettings>(encodeSettings)),
																mGetAudioProcessingSetupsProc(
																		getAudioProcessingSetupsProc),
																mInstantiateProc(instantiateProc)
														{}
													Info(OSType id, const CString& decodeName,
															const CString& encodeName,
															const EncodeSettings& encodeSettings,
															GetAudioProcessingSetupsProc getAudioProcessingSetupsProc,
															InstantiateProc instantiateProc) :
														mID(id), mDecodeName(decodeName), mEncodeName(encodeName),
																mEncodeSettings(OI<EncodeSettings>(encodeSettings)),
																mGetAudioProcessingSetupsProc(
																		getAudioProcessingSetupsProc),
																mInstantiateProc(instantiateProc)
														{}
													Info(const Info& other) :
														mID(other.mID), mDecodeName(other.mDecodeName),
																mEncodeName(other.mEncodeName),
																mEncodeSettings(other.mEncodeSettings),
																mGetAudioProcessingSetupsProc(
																		other.mGetAudioProcessingSetupsProc),
																mInstantiateProc(other.mInstantiateProc)
														{}

													// Instance methods
			OSType									getID() const
														{ return mID; }
			const	CString&						getDecodeName() const
														{ return mDecodeName; }
			const	CString&						getEncodeName() const
														{ return mEncodeName; }
					I<CAudioCodec>					instantiate() const
														{ return mInstantiateProc(mID); }
					TArray<SAudioProcessingSetup>	getAudioProcessingSetups(
															const SAudioStorageFormat& audioStorageFormat) const
														{ return mGetAudioProcessingSetupsProc(mID,
																audioStorageFormat); }

			// Properties
			private:
				OSType							mID;
				CString							mDecodeName;
				CString							mEncodeName;
				OI<EncodeSettings>				mEncodeSettings;
				InstantiateProc					mInstantiateProc;
				GetAudioProcessingSetupsProc	mGetAudioProcessingSetupsProc;
		};

	// Methods
	public:
												// Lifecycle methods
												CAudioCodec() : CCodec() {}
												~CAudioCodec() {}

												// Instance methods
		virtual	void							setupForDecode(const SAudioProcessingFormat& audioProcessingFormat,
														const I<CDataSource>& dataSource,
														const I<CCodec::DecodeInfo>& decodeInfo) = 0;
		virtual	SAudioReadStatus				decode(const SMediaPosition& mediaPosition, CAudioFrames& audioFrames) =
														0;

		virtual	TArray<SAudioProcessingSetup>	getEncodeAudioProcessingSetups() const = 0;
		virtual	void							setupForEncode(const SAudioProcessingFormat& audioProcessingFormat) = 0;

	protected:
												// Class methods
		static	UInt32							getPacketIndex(const SMediaPosition& mediaPosition,
														const SAudioProcessingFormat& audioProcessingFormat,
														const TArray<CCodec::PacketAndLocation>& packetAndLocations);
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - CDecodeOnlyAudioCodec

class CDecodeOnlyAudioCodec : public CAudioCodec {
	// Methods
	public:
										// Lifecycle methods
										CDecodeOnlyAudioCodec() : CAudioCodec() {}

										// CAudioCodec methods
		TArray<SAudioProcessingSetup>	getEncodeAudioProcessingSetups() const
											{ AssertFailUnimplemented(); return TNArray<SAudioProcessingSetup>(); }
		void							setupForEncode(const SAudioProcessingFormat& audioProcessingFormat)
											{ AssertFailUnimplemented(); }
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - CEncodeOnlyAudioCodec

class CEncodeOnlyAudioCodec : public CAudioCodec {
	// Methods
	public:
							// Lifecycle methods
							CEncodeOnlyAudioCodec() : CAudioCodec() {}

							// CAudioCodec methods
		void				setupForDecode(const SAudioProcessingFormat& audioProcessingFormat,
									const I<CDataSource>& dataSource, const I<CCodec::DecodeInfo>& decodeInfo)
								{ AssertFailUnimplemented(); }
		SAudioReadStatus	decode(const SMediaPosition& mediaPosition, CAudioFrames& audioFrames)
								{
									AssertFailUnimplemented();

									return SAudioReadStatus(SError::mUnimplemented);
								}
};
