//----------------------------------------------------------------------------------------------------------------------
//	CQueue.cpp			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CQueue.h"

#include "CArray.h"

/*
	Inspired by
		https://www.codeproject.com/Articles/3479/The-Bip-Buffer-The-Circular-Buffer-with-a-Twist
		https://ferrous-systems.com/blog/lock-free-ring-buffer/

	Notes...
		Basically we have read (R), write (W), and write watermark (WW) pointers.  The writer informs the minimum amount
			to write, and the queue ensures that at least the requested amount is available contiguously or fails.  We
			set up the following rules:
				WW is always >= W
				R is always <= WW
				Always update WW, then update W
		As the system moves through the various situations, with reads and writes happening concurrently, we end up with
			the following states and meanings:
		(1) R...W,WW	Can read R -> W and can read R -> WW
		(2) R,W,WW		Empty
		(3) W,WW...R	Illegal (R > WW)
		(4) R...W...WW	Can read R -> W
		(5) R,W...WW	Empty
		(6) W...R...WW	Can read R -> WW
		(7) W...R,WW	Empty
		(8) W...WW...R	Illegal (R > WW)

		In summary...
			R <= W => Can read R -> W (possibly empty)
			R > W => Can read R -> WW (possibly empty)
*/

//----------------------------------------------------------------------------------------------------------------------
// MARK: CSRSWBIPQueueInternals

class CSRSWBIPQueueInternals : public TCopyOnWriteReferenceCountable<CSRSWBIPQueueInternals> {
	public:
		CSRSWBIPQueueInternals(UInt32 size) :
			mBuffer((UInt8*) ::malloc(size)), mSize(size), mReadPtr(mBuffer), mWritePtr(mBuffer),
					mWriteWatermarkPtr(mBuffer)
			{}
		~CSRSWBIPQueueInternals()
			{ ::free(mBuffer); }

		// General
		UInt8*	mBuffer;
		UInt32	mSize;

		// Reader
		UInt8*	mReadPtr;

		// Writer
		UInt8*	mWritePtr;
		UInt8*	mWriteWatermarkPtr;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CSRSWBIPQueue

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CSRSWBIPQueue::CSRSWBIPQueue(UInt32 size)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = new CSRSWBIPQueueInternals(size);
}

//----------------------------------------------------------------------------------------------------------------------
CSRSWBIPQueue::CSRSWBIPQueue(const CSRSWBIPQueue& other)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = other.mInternals->addReference();
}

//----------------------------------------------------------------------------------------------------------------------
CSRSWBIPQueue::~CSRSWBIPQueue()
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals->removeReference();
}

// MARK: Instance methods

//----------------------------------------------------------------------------------------------------------------------
CSRSWBIPQueue::ReadBufferInfo CSRSWBIPQueue::requestRead() const
//----------------------------------------------------------------------------------------------------------------------
{
	// Capture info locally
	UInt8*	writePtr = mInternals->mWritePtr;
	UInt8*	writeWatermarkPtr = mInternals->mWriteWatermarkPtr;

	// Check if time to go back to the beginning, (7) => (5)
	if ((writePtr != writeWatermarkPtr) && (mInternals->mReadPtr == writeWatermarkPtr))
		// Reached the end, wrap around
		mInternals->mReadPtr = mInternals->mBuffer;

	// Check situation
	if (mInternals->mReadPtr < writePtr)
		// Can read to write pointer, (1), (4)
		return ReadBufferInfo(mInternals->mReadPtr, (UInt32) (writePtr - mInternals->mReadPtr));
	else if (mInternals->mReadPtr > writePtr)
		// Can read to write watermark pointer, (6)
		return ReadBufferInfo(mInternals->mReadPtr, (UInt32) (writeWatermarkPtr - mInternals->mReadPtr));
	else
		// Queue is empty, (2), (5)
		return ReadBufferInfo();
}

//----------------------------------------------------------------------------------------------------------------------
void CSRSWBIPQueue::commitRead(UInt32 size)
//----------------------------------------------------------------------------------------------------------------------
{
	// Update stuffs
	mInternals->mReadPtr += size;
}

//----------------------------------------------------------------------------------------------------------------------
CSRSWBIPQueue::WriteBufferInfo CSRSWBIPQueue::requestWrite(UInt32 requiredSize) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Capture info locally
	UInt8*	readPtr = mInternals->mReadPtr;

	// Check situation
	if (readPtr <= mInternals->mWritePtr) {
		// Read is before write, (1), (2), (4), (5)
		mInternals->mWriteWatermarkPtr = mInternals->mWritePtr;	// (4) or (5) => (1) or (2)
		if ((UInt32) (mInternals->mBuffer + mInternals->mSize - mInternals->mWritePtr) >= requiredSize)
			// Can write more from current position
			return WriteBufferInfo(mInternals->mWritePtr,
					(UInt32) (mInternals->mBuffer + mInternals->mSize - mInternals->mWritePtr));
		else if ((UInt32) (readPtr - mInternals->mBuffer) > requiredSize) {
			// Can write at the beginning of the buffer, (1) or (2) => (6) or (7)
			//	(but not allowed to completely fill or would appear empty)
			mInternals->mWritePtr = mInternals->mBuffer;

			return WriteBufferInfo(mInternals->mWritePtr, (UInt32) (readPtr - mInternals->mWritePtr - 1));
		} else
			// Not enough space
			return WriteBufferInfo();
	} else {
		// Read is after write, (6), (7)
		if ((UInt32) (readPtr - mInternals->mWritePtr) > requiredSize)
			// Can write more from current position (but not allowed to completely fill or would appear empty)
			return WriteBufferInfo(mInternals->mWritePtr, (UInt32) (readPtr - mInternals->mWritePtr - 1));
		else
			// Not enough space
			return WriteBufferInfo();
	}
}

//----------------------------------------------------------------------------------------------------------------------
void CSRSWBIPQueue::commitWrite(UInt32 size)
//----------------------------------------------------------------------------------------------------------------------
{
	// Capture info locally
	UInt8*	readPtr = mInternals->mReadPtr;

	// Update
	if (readPtr <= mInternals->mWritePtr) {
		// Read is before write, (1), (2)
		UInt8*	ptr = mInternals->mWritePtr + size;
		mInternals->mWriteWatermarkPtr = ptr;	// (1) or (2) => (4) or (5)
		mInternals->mWritePtr = ptr;			// (4) or (5) => (1) or (2)
	} else {
		// Read is after write, (6), (7); size needs to be less than the space available or will appear empty)
		mInternals->mWritePtr += size;
	}
}

//----------------------------------------------------------------------------------------------------------------------
void CSRSWBIPQueue::reset()
//----------------------------------------------------------------------------------------------------------------------
{
	// Reset
	mInternals->mReadPtr = mInternals->mBuffer;

	mInternals->mWritePtr = mInternals->mBuffer;
	mInternals->mWriteWatermarkPtr = mInternals->mBuffer + mInternals->mSize;
}

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CSRSWBIPSegmentedQueueInternals

class CSRSWBIPSegmentedQueueInternals {
	public:
		CSRSWBIPSegmentedQueueInternals(UInt32 segmentSize, UInt32 segmentCount) :
			mBuffer((UInt8*) ::malloc(segmentSize * segmentCount)), mSegmentSize(segmentSize),
					mSegmentCount(segmentCount), mReadPtr(mBuffer), mWritePtr(mBuffer),
					mWriteWatermarkPtr(mBuffer)
			{}
		~CSRSWBIPSegmentedQueueInternals()
			{ ::free(mBuffer); }

		// General
		UInt8*	mBuffer;
		UInt32	mSegmentSize;
		UInt32	mSegmentCount;

		// Reader
		UInt8*	mReadPtr;

		// Writer
		UInt8*	mWritePtr;
		UInt8*	mWriteWatermarkPtr;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CSRSWBIPSegmentedQueue

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CSRSWBIPSegmentedQueue::CSRSWBIPSegmentedQueue(UInt32 segmentSize, UInt32 segmentCount)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = new CSRSWBIPSegmentedQueueInternals(segmentSize, segmentCount);
}

//----------------------------------------------------------------------------------------------------------------------
CSRSWBIPSegmentedQueue::~CSRSWBIPSegmentedQueue()
//----------------------------------------------------------------------------------------------------------------------
{
	Delete(mInternals);
}

// MARK: Instance methods

//----------------------------------------------------------------------------------------------------------------------
UInt32 CSRSWBIPSegmentedQueue::getSegmentCount() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mSegmentCount;
}

//----------------------------------------------------------------------------------------------------------------------
CSRSWBIPSegmentedQueue::ReadBufferInfo CSRSWBIPSegmentedQueue::requestRead() const
//----------------------------------------------------------------------------------------------------------------------
{
	// Capture info locally
	UInt8*	writePtr = mInternals->mWritePtr;
	UInt8*	writeWatermarkPtr = mInternals->mWriteWatermarkPtr;

	// Check if time to go back to the beginning, (7) => (5)
	if ((writePtr != writeWatermarkPtr) && (mInternals->mReadPtr == writeWatermarkPtr))
		// Reached the end, wrap around
		mInternals->mReadPtr = mInternals->mBuffer;

	// Check situation
	if (mInternals->mReadPtr < writePtr)
		// Can read to write pointer, (1), (4)
		return ReadBufferInfo(mInternals->mReadPtr, mInternals->mSegmentSize,
				(UInt32) (writePtr - mInternals->mReadPtr));
	else if (mInternals->mReadPtr > writePtr)
		// Can read to write watermark pointer, (6)
		return ReadBufferInfo(mInternals->mReadPtr, mInternals->mSegmentSize,
				(UInt32) (writeWatermarkPtr - mInternals->mReadPtr));
	else
		// Queue is empty, (2), (5)
		return ReadBufferInfo();
}

//----------------------------------------------------------------------------------------------------------------------
void CSRSWBIPSegmentedQueue::commitRead(UInt32 size)
//----------------------------------------------------------------------------------------------------------------------
{
	// Update stuffs
	mInternals->mReadPtr += size;
}

//----------------------------------------------------------------------------------------------------------------------
CSRSWBIPSegmentedQueue::WriteBufferInfo CSRSWBIPSegmentedQueue::requestWrite(UInt32 requiredSize) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Capture info locally
	UInt8*	readPtr = mInternals->mReadPtr;

	// Check situation
	if (readPtr <= mInternals->mWritePtr) {
		// Read is before write, (1), (2), (4), (5)
		mInternals->mWriteWatermarkPtr = mInternals->mWritePtr;	// (4) or (5) => (1) or (2)
		if ((UInt32) (mInternals->mBuffer + mInternals->mSegmentSize - mInternals->mWritePtr) >= requiredSize)
			// Can write more from current position
			return WriteBufferInfo(mInternals->mWritePtr, mInternals->mSegmentSize,
					(UInt32) (mInternals->mBuffer + mInternals->mSegmentSize - mInternals->mWritePtr));
		else if ((UInt32) (readPtr - mInternals->mBuffer) > requiredSize) {
			// Can write at the beginning of the buffer, (1) or (2) => (6) or (7)
			//	(but not allowed to completely fill or would appear empty)
			mInternals->mWritePtr = mInternals->mBuffer;

			return WriteBufferInfo(mInternals->mWritePtr, mInternals->mSegmentSize,
					(UInt32) (readPtr - mInternals->mWritePtr - 1));
		} else
			// Not enough space
			return WriteBufferInfo();
	} else {
		// Read is after write, (6), (7)
		if ((UInt32) (readPtr - mInternals->mWritePtr) > requiredSize)
			// Can write more from current position (but not allowed to completely fill or would appear empty)
			return WriteBufferInfo(mInternals->mWritePtr, mInternals->mSegmentSize,
					(UInt32) (readPtr - mInternals->mWritePtr - 1));
		else
			// Not enough space
			return WriteBufferInfo();
	}
}

//----------------------------------------------------------------------------------------------------------------------
void CSRSWBIPSegmentedQueue::commitWrite(UInt32 size)
//----------------------------------------------------------------------------------------------------------------------
{
	// Capture info locally
	UInt8*	readPtr = mInternals->mReadPtr;

	// Update
	if (readPtr <= mInternals->mWritePtr) {
		// Read is before write, (1), (2)
		UInt8*	ptr = mInternals->mWritePtr + size;
		mInternals->mWriteWatermarkPtr = ptr;	// (1) or (2) => (4) or (5)
		mInternals->mWritePtr = ptr;			// (4) or (5) => (1) or (2)
	} else {
		// Read is after write, (6), (7); size needs to be less than the space available or will appear empty)
		mInternals->mWritePtr += size;
	}
}

//----------------------------------------------------------------------------------------------------------------------
void CSRSWBIPSegmentedQueue::reset()
//----------------------------------------------------------------------------------------------------------------------
{
	// Reset
	mInternals->mReadPtr = mInternals->mBuffer;

	mInternals->mWritePtr = mInternals->mBuffer;
	mInternals->mWriteWatermarkPtr = mInternals->mBuffer + mInternals->mSegmentSize;
}

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CSRSWMessageQueue

// MARK: Properties

OSType	CSRSWMessageQueue::ProcMessage::mType = MAKE_OSTYPE('P', 'r', 'o', 'c');

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CSRSWMessageQueue::CSRSWMessageQueue(UInt32 size) : CSRSWBIPQueue(size)
//----------------------------------------------------------------------------------------------------------------------
{
}

//----------------------------------------------------------------------------------------------------------------------
CSRSWMessageQueue::CSRSWMessageQueue(const CSRSWMessageQueue& other) : CSRSWBIPQueue(other)
//----------------------------------------------------------------------------------------------------------------------
{
}

// MARK: Instance methods

//----------------------------------------------------------------------------------------------------------------------
void CSRSWMessageQueue::handleAll()
//----------------------------------------------------------------------------------------------------------------------
{
	// Process
	CSRSWBIPQueue::ReadBufferInfo	readBufferInfo = requestRead();
	while (readBufferInfo.hasBuffer()) {
		// Process message
		Message&	message = *((Message*) readBufferInfo.mBuffer);
		handle(message);

		// Processed
		commitRead(message.mSize);

		// Get next
		readBufferInfo = requestRead();
	}
}

//----------------------------------------------------------------------------------------------------------------------
void CSRSWMessageQueue::submit(const Message& message)
//----------------------------------------------------------------------------------------------------------------------
{
	// Query situation
	CSRSWBIPQueue::WriteBufferInfo	writeBufferInfo = requestWrite(message.mSize);

	// Validate
	AssertFailIf(writeBufferInfo.mSize < message.mSize);

	// Copy and commit
	::memcpy(writeBufferInfo.mBuffer, &message, message.mSize);
	commitWrite(message.mSize);
}

// MARK: Subclass methods

//----------------------------------------------------------------------------------------------------------------------
void CSRSWMessageQueue::handle(const Message& message)
//----------------------------------------------------------------------------------------------------------------------
{
	// Check type
	if (message.mType == ProcMessage::mType)
		// Perform
		((ProcMessage&) message).perform();
}

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CSRSWMessageQueuesInternals

class CSRSWMessageQueuesInternals {
	public:
		CSRSWMessageQueuesInternals() {}

		TNArray<CSRSWMessageQueue>	mMessageQueues;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CSRSWMessageQueues

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CSRSWMessageQueues::CSRSWMessageQueues()
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = new CSRSWMessageQueuesInternals();
}

//----------------------------------------------------------------------------------------------------------------------
CSRSWMessageQueues::~CSRSWMessageQueues()
//----------------------------------------------------------------------------------------------------------------------
{
	Delete(mInternals);
}

// MARK: Instance methods

//----------------------------------------------------------------------------------------------------------------------
void CSRSWMessageQueues::add(CSRSWMessageQueue& messageQueue)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals->mMessageQueues.add(messageQueue);
}

//----------------------------------------------------------------------------------------------------------------------
void CSRSWMessageQueues::remove(CSRSWMessageQueue& messageQueue)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals->mMessageQueues.remove(messageQueue);
}

//----------------------------------------------------------------------------------------------------------------------
void CSRSWMessageQueues::handleAll()
//----------------------------------------------------------------------------------------------------------------------
{
	// Iterate all
	for (TIteratorD<CSRSWMessageQueue> iterator = mInternals->mMessageQueues.getIterator(); iterator.hasValue();
			iterator.advance()) {
		// Handle all
		CSRSWMessageQueue&	messageQueue = iterator.getValue();
		messageQueue.handleAll();
	}
}
