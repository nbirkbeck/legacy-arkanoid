#include <SDL2/SDL_audio.h>
#include <iostream>

namespace {
SDL_AudioDeviceID audio_dev = 0;
}

void LazyInit();

struct Sound {
  Uint8* audio_buf = 0;
  Uint32 audio_len = 0;

  void Play() {
    if (!audio_buf) {
      LazyInit();
    }
    SDL_PauseAudioDevice(audio_dev, 0);

    SDL_ClearQueuedAudio(audio_dev);
    SDL_QueueAudio(audio_dev, audio_buf, audio_len);
  }
};

namespace {
Sound break_sound;
};

int playBreakSound() {
  break_sound.Play();
  return 0;
}

void LazyInit() {
  SDL_AudioSpec desired_audio_spec = {0};
  desired_audio_spec.freq = 11025;
  desired_audio_spec.channels = 1;
  desired_audio_spec.samples = 2048;
  desired_audio_spec.format = AUDIO_U8;
  desired_audio_spec.callback = 0;

  SDL_AudioSpec audio_spec;
  audio_dev =
      SDL_OpenAudioDevice(nullptr, 0, &desired_audio_spec, &audio_spec, 0);

  SDL_LoadWAV("sounds/break1.wav", &audio_spec, &break_sound.audio_buf,
              &break_sound.audio_len);
}
