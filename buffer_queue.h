#pragma once

// Circular queue of buffers used to produce and consume new blocks of audio data coming from and going to the I2S bus.
class BufferQueue
{
#define BUFFER_QUEUE_SIZE 3
public:
	int32_t dataLeft[AUDIO_BLOCK_SAMPLES * BUFFER_QUEUE_SIZE];
	int32_t dataRight[AUDIO_BLOCK_SAMPLES * BUFFER_QUEUE_SIZE];
	uint8_t readPos = 0;
	uint8_t writePos = 0;
	int available = 0;

	int32_t* readPtr[2];
	int32_t* writePtr[2];

	inline BufferQueue()
	{
		writePtr[0] = &dataLeft[writePos * AUDIO_BLOCK_SAMPLES];
		writePtr[1] = &dataRight[writePos * AUDIO_BLOCK_SAMPLES];
		readPtr[0] = &dataLeft[readPos * AUDIO_BLOCK_SAMPLES];
		readPtr[1] = &dataRight[readPos * AUDIO_BLOCK_SAMPLES];
        for (size_t i = 0; i < AUDIO_BLOCK_SAMPLES * BUFFER_QUEUE_SIZE; i++)
        {
            dataLeft[i] = 0;
            dataRight[i] = 0;
        }
        
        for (size_t i = 0; i < BUFFER_QUEUE_SIZE - 1; i++)
        {
            publish();
        }
	}

	// Increases writePos by one and updates the write pointers
	// First, write to the write pointers, then call this function to mark the data as available for reading.
	inline void publish()
	{
		writePos = (writePos + 1) % BUFFER_QUEUE_SIZE;
		writePtr[0] = &dataLeft[writePos * AUDIO_BLOCK_SAMPLES];
		writePtr[1] = &dataRight[writePos * AUDIO_BLOCK_SAMPLES];
		available++;

		// writing over the tail of the circular buffer. Should never happen as read and write should be synchronous!
		if (available > BUFFER_QUEUE_SIZE)
			consume(); // consume and discard one block
	}

	// increase ReadPos by one and updates the read pointers.
	// First read from the read pointers, then call this function to mark the data as dispensed.
	inline void consume()
	{
		if (available <= 0)
			return;

		readPos = (readPos + 1) % BUFFER_QUEUE_SIZE;
		readPtr[0] = &dataLeft[readPos * AUDIO_BLOCK_SAMPLES];
		readPtr[1] = &dataRight[readPos * AUDIO_BLOCK_SAMPLES];
		available--;
	}
};
