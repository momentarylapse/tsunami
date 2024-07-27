#if HAS_LIB_COREAUDIO

#include "AudioInputStreamCoreAudio.h"
#include "../DeviceManager.h"
#include "../Device.h"
#include "../../Session.h"
#include "../../lib/hui/hui.h"
#include <CoreAudio/CoreAudio.h>

// https://developer.apple.com/library/archive/samplecode/RecordAudioToFile/Listings/DCAudioFileRecorder_cpp.html#//apple_ref/doc/uid/DTS10003986-DCAudioFileRecorder_cpp-DontLinkElementID_4


namespace tsunami {

static OSStatus AudioInputProc(void* inRefCon, AudioUnitRenderActionFlags* ioActionFlags, const AudioTimeStamp* inTimeStamp, UInt32 inBusNumber, UInt32 inNumberFrames, AudioBufferList* ioData) {
    auto stream = reinterpret_cast<AudioInputStreamCoreAudio*>(inRefCon);

    auto err = AudioUnitRender(stream->unit, ioActionFlags, inTimeStamp, inBusNumber, inNumberFrames, stream->audio_buffer_list);
    if (err)
        fprintf(stderr, "AudioUnitRender() failed with error %i\n", err);
    auto data = reinterpret_cast<float*>(stream->audio_buffer_list->mBuffers[0].mData);
    stream->handle_input(data, (int)inNumberFrames);
    return 0;
}

void destroy_audio_buffer_list(AudioBufferList* list) {
    for (unsigned int i = 0; i < list->mNumberBuffers; i++) {
        if(list->mBuffers[i].mData)
            free(list->mBuffers[i].mData);
    }
    free(list);
}

AudioBufferList* allocate_audio_buffer_list(unsigned int num_channels, unsigned int size) {
    auto list = (AudioBufferList*)calloc(1, sizeof(AudioBufferList) + num_channels * sizeof(AudioBuffer));
    if (!list)
        return nullptr;

    list->mNumberBuffers = num_channels;
    for (unsigned int i=0; i<num_channels; i++) {
        list->mBuffers[i].mNumberChannels = 1;
        list->mBuffers[i].mDataByteSize = size;
        list->mBuffers[i].mData = malloc(size);
        if (!list->mBuffers[i].mData) {
            destroy_audio_buffer_list(list);
            return nullptr;
        }
    }
    return list;
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"

AudioInputStreamCoreAudio::AudioInputStreamCoreAudio(Session *session, Device *device, AudioInputStream::SharedData &shared_data) : AudioInputStream(session, shared_data) {

	int _sample_rate = session->sample_rate();
	int num_channels = shared_data.num_channels;
    unit = nullptr;
    device_id = 0;


    UInt32  fAudioChannels, fAudioSamples;
    AudioStreamBasicDescription fOutputFormat, fDeviceFormat;

	if (device->is_default()) {
		session->i(format("open def stream %d  %d", _sample_rate, num_channels));
	} else {
		session->i(format("open stream %d  %d", _sample_rate, num_channels));
	}

    AudioComponentDescription acd = {
	    .componentType = ::kAudioUnitType_Output,
        .componentSubType = ::kAudioUnitSubType_HALOutput,
        .componentManufacturer = ::kAudioUnitManufacturer_Apple,
    };

    output = AudioComponentFindNext(nullptr, &acd);
    if (!output) {
        session->e("CoreAudio: Can't find default output");
        return;
    }

    OSStatus err = AudioComponentInstanceNew(output, &unit);
    if (err) {
        session->e(format("CoreAudio: Error creating unit: %d", (int)err));
        return;
    }



    AURenderCallbackStruct  callback;


    // Configure the AudioOutputUnit
    // You must enable the Audio Unit (AUHAL) for input and output for the same  device.
    // When using AudioUnitSetProperty the 4th parameter in the method
    // refer to an AudioUnitElement.  When using an AudioOutputUnit
    // for input the element will be '1' and the output element will be '0'.

    // Enable input on the AUHAL
    UInt32 param = 1;
    err = AudioUnitSetProperty(unit, kAudioOutputUnitProperty_EnableIO, kAudioUnitScope_Input, 1, &param, sizeof(UInt32));
    if(err == noErr)
    {
        // Disable Output on the AUHAL
        param = 0;
        err = AudioUnitSetProperty(unit, kAudioOutputUnitProperty_EnableIO, kAudioUnitScope_Output, 0, &param, sizeof(UInt32));
    }

    // Select the default input device
    if (device->is_default()) {
        param = sizeof(AudioDeviceID);
        err = AudioHardwareGetProperty(kAudioHardwarePropertyDefaultInputDevice, &param, &device_id);
        if(err != noErr)
        {
            session->e("failed to get default input device");
            return;
        }
    } else {
        device_id = device->index_in_lib;
    }

    // Set the current device to the default input unit.
    err = AudioUnitSetProperty(unit, kAudioOutputUnitProperty_CurrentDevice, kAudioUnitScope_Global, 0, &device_id, sizeof(AudioDeviceID));
    if(err != noErr)
    {
        session->e("failed to set AU input device");
        return;
    }

    // Setup render callback
    // This will be called when the AUHAL has input data
    callback.inputProc = AudioInputProc; // defined as static in the header file
    callback.inputProcRefCon = this;
    err = AudioUnitSetProperty(unit, kAudioOutputUnitProperty_SetInputCallback, kAudioUnitScope_Global, 0, &callback, sizeof(AURenderCallbackStruct));

    // get hardware device format
    param = sizeof(AudioStreamBasicDescription);
    err = AudioUnitGetProperty(unit, kAudioUnitProperty_StreamFormat, kAudioUnitScope_Input, 1, &fDeviceFormat, &param);
    if(err != noErr)
    {
        session->e("failed to get input device ASBD");
        return;
    }

    // Twiddle the format to our liking
    fAudioChannels = max(fDeviceFormat.mChannelsPerFrame, 2u);
    fOutputFormat.mChannelsPerFrame = fAudioChannels;
    fOutputFormat.mSampleRate = fDeviceFormat.mSampleRate;
    //fOutputFormat.mSampleRate = _sample_rate; TODO
    fOutputFormat.mFormatID = kAudioFormatLinearPCM;
    fOutputFormat.mFormatFlags = kAudioFormatFlagIsFloat | kAudioFormatFlagIsPacked;// | kAudioFormatFlagIsNonInterleaved;
    fOutputFormat.mBitsPerChannel = sizeof(Float32) * 8;
    fOutputFormat.mBytesPerFrame = fOutputFormat.mBitsPerChannel / 8 * fOutputFormat.mChannelsPerFrame;
    fOutputFormat.mFramesPerPacket = 1;
    fOutputFormat.mBytesPerPacket = fOutputFormat.mBytesPerFrame;
    //msg_write(i2s((int)fDeviceFormat.mSampleRate));

    // Set the AudioOutputUnit output data format
    err = AudioUnitSetProperty(unit, kAudioUnitProperty_StreamFormat, kAudioUnitScope_Output, 1, &fOutputFormat, sizeof(AudioStreamBasicDescription));
    if (err != noErr)
    {
        session->e("failed to set input device ASBD");
        return;
    }

    // Get the number of frames in the IO buffer(s)
    param = sizeof(UInt32);
    err = AudioUnitGetProperty(unit, kAudioDevicePropertyBufferFrameSize, kAudioUnitScope_Global, 0, &fAudioSamples, &param);
    if (err != noErr)
    {
        session->e("failed to get audio sample size");
        return;
    }
    //msg_write(i2s((int)fAudioSamples));

    // Initialize the AU
    err = AudioUnitInitialize(unit);
    if (err != noErr)
    {
        session->e("failed to initialize AU");
        return;
    }


    audio_buffer_list = allocate_audio_buffer_list(1, fAudioSamples * fOutputFormat.mBytesPerFrame);
    if (!audio_buffer_list) {
        session->e("failed to allocate buffers");
        return;
    }
}

#pragma GCC diagnostic pop

AudioInputStreamCoreAudio::~AudioInputStreamCoreAudio() {
    if (audio_buffer_list)
        destroy_audio_buffer_list(audio_buffer_list);
}

void AudioInputStreamCoreAudio::pause() {
    OSStatus err = AudioOutputUnitStop(unit);
    if(err != noErr)
        session->e("failed to stop AU");
}

void AudioInputStreamCoreAudio::unpause() {
    OSStatus err = AudioOutputUnitStart(unit);
    if(err != noErr)
        session->e("failed to start AU");
}

base::optional<int64> AudioInputStreamCoreAudio::estimate_samples_captured() {
	return base::None;
}

}

#endif
