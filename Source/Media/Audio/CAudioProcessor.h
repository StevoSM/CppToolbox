//----------------------------------------------------------------------------------------------------------------------
//	CAudioProcessor.h			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "SAudioFormats.h"
#include "SAudioReadStatus.h"
#include "SMediaPosition.h"
#include "TInstance.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CAudioProcessor

class CAudioProcessorInternals;
class CAudioProcessor {
	// Methods
	public:
												// Lifecycle methods
												CAudioProcessor();
		virtual									~CAudioProcessor();

												// Instance methods
		virtual	OI<SError>						connectInput(const I<CAudioProcessor>& audioProcessor,
														const SAudioProcessingFormat& audioProcessingFormat);

		virtual	SAudioReadStatus				perform(const SMediaPosition& mediaPosition, CAudioData& audioData);
		virtual	OI<SError>						reset();

												// Subclass methods
		virtual	TArray<SAudioProcessingSetup>	getInputSetups() const = 0;

		virtual	TArray<SAudioProcessingSetup>	getOutputSetups() const = 0;
		virtual	void							setOutputFormat(const SAudioProcessingFormat& audioProcessingFormat)
														= 0;

	// Properties
	private:
		CAudioProcessorInternals*	mInternals;
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - CAudioSource

class CAudioSource : public CAudioProcessor {
	// Methods
	public:
										// Lifecycle methods
										CAudioSource() : CAudioProcessor() {}

										// CAudioProcessor methods
		TArray<SAudioProcessingSetup>	getInputSetups() const
											{ AssertFailUnimplemented(); return TNArray<SAudioProcessingSetup>(); }
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - CAudioDestination

class CAudioDestination : public CAudioProcessor {
	// Methods
	public:
												// Lifecycle methods
												CAudioDestination() : CAudioProcessor() {}

												// CAudioProcessor methods
				TArray<SAudioProcessingSetup>	getOutputSetups() const
													{ AssertFailUnimplemented();
															return TNArray<SAudioProcessingSetup>(); }
				void							setOutputFormat(const SAudioProcessingFormat& audioProcessingFormat)
													{ AssertFailUnimplemented(); }

												// Instance methods
		virtual	void							setupComplete() = 0;
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - CAudioChannelMapper

class CAudioChannelMapperInternals;
class CAudioChannelMapper : public CAudioProcessor {
	public:
												// Lifecycle methods
												CAudioChannelMapper();
												~CAudioChannelMapper();

												// Instance methods
				OI<SError>						connectInput(const I<CAudioProcessor>& audioProcessor,
														const SAudioProcessingFormat& audioProcessingFormat);

				SAudioReadStatus				perform(const SMediaPosition& mediaPosition, CAudioData& audioData);

												// Subclass methods
				TArray<SAudioProcessingSetup>	getInputSetups() const;

				TArray<SAudioProcessingSetup>	getOutputSetups() const;
				void							setOutputFormat(const SAudioProcessingFormat& audioProcessingFormat);

												// Class methods
		static	bool							canPerform(EAudioChannelMap fromAudioChannelMap,
														EAudioChannelMap toAudioChannelMap);

	// Properties
	private:
		CAudioChannelMapperInternals*	mInternals;
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - CAudioConverter

class CAudioConverterInternals;
class CAudioConverter : public CAudioProcessor {
	public:
										// Lifecycle methods
										CAudioConverter();
										~CAudioConverter();

										// Instance methods
		OI<SError>						connectInput(const I<CAudioProcessor>& audioProcessor,
												const SAudioProcessingFormat& audioProcessingFormat);

		SAudioReadStatus				perform(const SMediaPosition& mediaPosition, CAudioData& audioData);
		OI<SError>						reset();

										// Subclass methods
		TArray<SAudioProcessingSetup>	getInputSetups() const;

		TArray<SAudioProcessingSetup>	getOutputSetups() const;
		void							setOutputFormat(const SAudioProcessingFormat& audioProcessingFormat);

	// Properties
	private:
		CAudioConverterInternals*	mInternals;
};
