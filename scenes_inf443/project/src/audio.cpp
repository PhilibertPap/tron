#include "audio.hpp"

// miniaudio inclut windows.h sous Windows : on évite qu'il définisse les macros min et max
#ifndef NOMINMAX
#define NOMINMAX
#endif
#define MINIAUDIO_IMPLEMENTATION
#include "third_party/miniaudio.h"

using namespace cgp;

namespace
{
	constexpr int max_engines = 2;
	constexpr int effect_count = 5;
	char const* const effect_files[effect_count] = { "explosion.wav", "boost.wav", "fall.wav", "countdown.wav", "go.wav" };

	constexpr float engine_volume = 0.5f;
}

struct audio_system::implementation
{
	ma_engine engine;
	bool ready = false;
	int engine_count = 0;
	ma_sound engines[max_engines];
	ma_sound effects[effect_count];

	~implementation()
	{
		if (!ready)
			return;
		for (int k = 0; k < engine_count; ++k)
			ma_sound_uninit(&engines[k]);
		for (int k = 0; k < effect_count; ++k)
			ma_sound_uninit(&effects[k]);
		ma_engine_uninit(&engine);
	}
};

audio_system::audio_system() : impl(new implementation) {}
audio_system::~audio_system() = default;

void audio_system::initialize(std::string const& dir, int engine_count)
{
	if (ma_engine_init(nullptr, &impl->engine) != MA_SUCCESS) {
		std::cout << "[audio] Pas de sortie audio disponible, le jeu tourne sans son" << std::endl;
		return;
	}
	ma_engine_listener_set_world_up(&impl->engine, 0, 0, 0, 1);

	impl->engine_count = std::min(engine_count, max_engines);
	for (int k = 0; k < impl->engine_count; ++k) {
		ma_sound& s = impl->engines[k];
		ma_sound_init_from_file(&impl->engine, (dir + "engine.wav").c_str(), MA_SOUND_FLAG_DECODE, nullptr, nullptr, &s);
		ma_sound_set_looping(&s, MA_TRUE);
		ma_sound_set_min_distance(&s, 6.0f);
		ma_sound_set_max_distance(&s, 200.0f);
		ma_sound_set_doppler_factor(&s, 0.5f);
		ma_sound_set_volume(&s, 0.0f);
		ma_sound_start(&s);
	}

	for (int k = 0; k < effect_count; ++k) {
		ma_sound& s = impl->effects[k];
		ma_sound_init_from_file(&impl->engine, (dir + effect_files[k]).c_str(), MA_SOUND_FLAG_DECODE, nullptr, nullptr, &s);
		ma_sound_set_min_distance(&s, 10.0f);
		ma_sound_set_max_distance(&s, 250.0f);
	}

	impl->ready = true;
}

void audio_system::set_muted(bool muted_arg)
{
	muted = muted_arg;
	if (impl->ready)
		ma_engine_set_volume(&impl->engine, muted ? 0.0f : 1.0f);
}

void audio_system::set_listener(vec3 const& position, vec3 const& forward, vec3 const& velocity)
{
	if (!impl->ready)
		return;
	ma_engine_listener_set_position(&impl->engine, 0, position.x, position.y, position.z);
	ma_engine_listener_set_direction(&impl->engine, 0, forward.x, forward.y, forward.z);
	ma_engine_listener_set_velocity(&impl->engine, 0, velocity.x, velocity.y, velocity.z);
}

void audio_system::update_engine(int index, vec3 const& position, vec3 const& velocity, bool running)
{
	if (!impl->ready || index >= impl->engine_count)
		return;

	// Le régime suit la vitesse horizontale : grave au ralenti, aigu en plein boost
	float const speed = std::sqrt(velocity.x * velocity.x + velocity.y * velocity.y);
	ma_sound& s = impl->engines[index];
	ma_sound_set_volume(&s, running ? engine_volume : 0.0f);
	ma_sound_set_pitch(&s, 0.55f + 0.035f * speed);
	ma_sound_set_position(&s, position.x, position.y, position.z);
	ma_sound_set_velocity(&s, velocity.x, velocity.y, velocity.z);
}

void audio_system::play(sound_effect effect, vec3 const& position)
{
	if (!impl->ready)
		return;
	ma_sound& s = impl->effects[int(effect)];
	ma_sound_set_spatialization_enabled(&s, MA_TRUE);
	ma_sound_set_position(&s, position.x, position.y, position.z);
	ma_sound_seek_to_pcm_frame(&s, 0);
	ma_sound_start(&s);
}

void audio_system::play(sound_effect effect)
{
	if (!impl->ready)
		return;
	ma_sound& s = impl->effects[int(effect)];
	ma_sound_set_spatialization_enabled(&s, MA_FALSE);
	ma_sound_seek_to_pcm_frame(&s, 0);
	ma_sound_start(&s);
}
