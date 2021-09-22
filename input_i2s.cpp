/* Audio Library for Teensy 3.X
 * Copyright (c) 2014, Paul Stoffregen, paul@pjrc.com
 *
 * Development of this audio library was funded by PJRC.COM, LLC by sales of
 * Teensy and Audio Adaptor boards.  Please support PJRC's efforts to develop
 * open source software by purchasing Teensy or other PJRC products.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice, development funding notice, and this permission
 * notice shall be included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include "AudioConfig.h"
#include "input_i2s.h"

DMAMEM __attribute__((aligned(32))) static uint64_t i2s_rx_buffer[AUDIO_BLOCK_SAMPLES];
BufferQueue AudioInputI2S::buffers;
DMAChannel AudioInputI2S::dma(false);
int32_t* outBuffers[2]; // temporary holder for the values returned by getData

void AudioInputI2S::begin()
{
	dma.begin(true); // Allocate the DMA channel first

	CORE_PIN8_CONFIG  = 3;  //1:RX_DATA0
	IOMUXC_SAI1_RX_DATA0_SELECT_INPUT = 2;

	dma.TCD->SADDR = (void *)((uint32_t)&I2S1_RDR0 + 2);
	dma.TCD->SOFF = 0;
	dma.TCD->ATTR = DMA_TCD_ATTR_SSIZE(1) | DMA_TCD_ATTR_DSIZE(1);
	dma.TCD->NBYTES_MLNO = 2;
	dma.TCD->SLAST = 0;
	dma.TCD->DADDR = i2s_rx_buffer;
	dma.TCD->DOFF = 2;
	dma.TCD->CITER_ELINKNO = sizeof(i2s_rx_buffer) / 2;
	dma.TCD->DLASTSGA = -sizeof(i2s_rx_buffer);
	dma.TCD->BITER_ELINKNO = sizeof(i2s_rx_buffer) / 2;
	dma.TCD->CSR = DMA_TCD_CSR_INTHALF | DMA_TCD_CSR_INTMAJOR;
	dma.triggerAtHardwareEvent(DMAMUX_SOURCE_SAI1_RX);

	I2S1_RCSR = I2S_RCSR_RE | I2S_RCSR_BCE | I2S_RCSR_FRDE | I2S_RCSR_FR;

	dma.enable();
	dma.attachInterrupt(isr);
}

int32_t** AudioInputI2S::getData()
{
	outBuffers[0] = buffers.readPtr[0];
	outBuffers[1] = buffers.readPtr[1];
	buffers.consume();
	return outBuffers;
}

void AudioInputI2S::isr(void)
{
	uint32_t daddr, offset;
	const int32_t *src;
	int32_t *dest_left, *dest_right;
	bool incrementQueue;

	daddr = (uint32_t)(dma.TCD->DADDR);
	dma.clearInterrupt();

	if (daddr < (uint32_t)i2s_rx_buffer + sizeof(i2s_rx_buffer) / 2) 
	{
		// DMA is receiving to the first half of the buffer
		// need to remove data from the second half
		src = (int32_t *)&i2s_rx_buffer[AUDIO_BLOCK_SAMPLES/2];
		offset = AUDIO_BLOCK_SAMPLES/2;
		incrementQueue = true;
	} 
	else 
	{
		// DMA is receiving to the second half of the buffer
		// need to remove data from the first half
		src = (int32_t *)&i2s_rx_buffer[0];
		offset = 0;
		incrementQueue = false;
	}
	
	int32_t* left = buffers.writePtr[0];
	int32_t* right = buffers.writePtr[1];
	dest_left = &(left[offset]);
	dest_right = &(right[offset]);

	for (size_t i = 0; i < AUDIO_BLOCK_SAMPLES/2; i++)
	{
		dest_left[i] = src[2*i];
		dest_right[i] = src[2*i+1];
	}

	if (incrementQueue)
	{
		buffers.publish();
	}
	
	arm_dcache_delete((void*)src, sizeof(i2s_rx_buffer) / 2);
}
