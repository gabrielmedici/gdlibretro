#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdbool.h>
#include <errno.h>
#include <dlfcn.h>

#include "libretro.h"

int width = 0;
int height = 0;

static struct {
	void * handle;
	bool initialized;

	void( * retro_init)(void);
	void( * retro_deinit)(void);
	unsigned( * retro_api_version)(void);
	void( * retro_get_system_info)(struct retro_system_info * info);
	void( * retro_get_system_av_info)(struct retro_system_av_info * info);
	void( * retro_set_controller_port_device)(unsigned port, unsigned device);
	void( * retro_reset)(void);
	void( * retro_run)(void);
	//	size_t retro_serialize_size(void);
	//	bool retro_serialize(void *data, size_t size);
	//	bool retro_unserialize(const void *data, size_t size);
	//	void retro_cheat_reset(void);
	//	void retro_cheat_set(unsigned index, bool enabled, const char *code);
	bool( * retro_load_game)(const struct retro_game_info * game);
	//	bool retro_load_game_special(unsigned game_type, const struct retro_game_info *info, size_t num_info);
	void( * retro_unload_game)(void);
	//	unsigned retro_get_region(void);
	//	void *retro_get_memory_data(unsigned id);
	//	size_t retro_get_memory_size(unsigned id);
}
g_retro;

#define load_sym(V, S) do {\
	if (!((*(void**)&V) = dlsym(g_retro.handle, #S))) \
		die("Failed to load symbol '" #S "'': %s", dlerror()); \
	} while (0)
#define load_retro_sym(S) load_sym(g_retro.S, S)

static void die(const char * fmt, ...) {
	char buffer[4096];

	va_list va;
	va_start(va, fmt);
	vsnprintf(buffer, sizeof(buffer), fmt, va);
	va_end(va);

	fputs(buffer, stderr);
	fputc('\n', stderr);
	fflush(stderr);

	exit(EXIT_FAILURE);
}

static void video_configure(const struct retro_game_geometry * geom) {
	width = geom->max_width;
	height = geom->max_height;
}

static void video_deinit() {}

static void audio_init(int frequency) {
	(void)frequency;
}

static void audio_deinit() {}

static void core_log(enum retro_log_level level,
	const char * fmt, ...) {
	char buffer[4096] = {
		0
	};
	static
	const char * levelstr[] = {
		"dbg",
		"inf",
		"wrn",
		"err"
	};
	va_list va;

	va_start(va, fmt);
	vsnprintf(buffer, sizeof(buffer), fmt, va);
	va_end(va);

	if (level == 0)
		return;

	fprintf(stderr, "[%s] %s", levelstr[level], buffer);
	fflush(stderr);

	if (level == RETRO_LOG_ERROR)
		exit(EXIT_FAILURE);
}

static bool core_environment(unsigned cmd, void * data) {
	bool * bval;

	switch (cmd) {
	case RETRO_ENVIRONMENT_GET_LOG_INTERFACE:
		{
			struct retro_log_callback * cb = (struct retro_log_callback * ) data;
			cb->log = core_log;
			break;
		}
	case RETRO_ENVIRONMENT_GET_CAN_DUPE:
		bval = (bool * ) data; * bval = true;
		break;
	case RETRO_ENVIRONMENT_SET_PIXEL_FORMAT:
		{
			return true;
		}
	case RETRO_ENVIRONMENT_GET_SYSTEM_DIRECTORY:
	case RETRO_ENVIRONMENT_GET_SAVE_DIRECTORY:
		* (const char * * ) data = ".";
		return true;

	default:
		core_log(RETRO_LOG_DEBUG, "Unhandled env #%u", cmd);
		return false;
	}

	return true;
}

static void core_video_refresh(const void * data, unsigned width, unsigned height, size_t pitch) {
	(void)data;
	(void)width;
	(void)height;
	(void)pitch;
}

static void core_input_poll(void) {
	// Nothing
}

static int16_t core_input_state(unsigned port, unsigned device, unsigned index, unsigned id) {
	(void)port;
	(void)device;
	(void)index;
	(void)id;
	return 0;
}

static void core_audio_sample(int16_t left, int16_t right) {
	(void)left;
	(void)right;
}

static size_t core_audio_sample_batch(const int16_t * data, size_t frames) {
	(void)data;
	(void)frames;
	return 0;
}

static void core_load(const char * sofile) {
	void( * set_environment)(retro_environment_t) = NULL;
	void( * set_video_refresh)(retro_video_refresh_t) = NULL;
	void( * set_input_poll)(retro_input_poll_t) = NULL;
	void( * set_input_state)(retro_input_state_t) = NULL;
	void( * set_audio_sample)(retro_audio_sample_t) = NULL;
	void( * set_audio_sample_batch)(retro_audio_sample_batch_t) = NULL;

	memset( & g_retro, 0, sizeof(g_retro));
	g_retro.handle = dlopen(sofile, RTLD_LAZY);

	if (!g_retro.handle)
		die("Failed to load core: %s", dlerror());

	dlerror();

	load_retro_sym(retro_init);
	load_retro_sym(retro_deinit);
	load_retro_sym(retro_api_version);
	load_retro_sym(retro_get_system_info);
	load_retro_sym(retro_get_system_av_info);
	load_retro_sym(retro_set_controller_port_device);
	load_retro_sym(retro_reset);
	load_retro_sym(retro_run);
	load_retro_sym(retro_load_game);
	load_retro_sym(retro_unload_game);

	load_sym(set_environment, retro_set_environment);
	load_sym(set_video_refresh, retro_set_video_refresh);
	load_sym(set_input_poll, retro_set_input_poll);
	load_sym(set_input_state, retro_set_input_state);
	load_sym(set_audio_sample, retro_set_audio_sample);
	load_sym(set_audio_sample_batch, retro_set_audio_sample_batch);

	set_environment(core_environment);
	set_video_refresh(core_video_refresh);
	set_input_poll(core_input_poll);
	set_input_state(core_input_state);
	set_audio_sample(core_audio_sample);
	set_audio_sample_batch(core_audio_sample_batch);

	g_retro.retro_init();
	g_retro.initialized = true;

	puts("Core loaded");
}

static void core_load_game(const char * filename) {
	struct retro_system_av_info av = {
		0
	};
	struct retro_system_info system = {
		0
	};
	struct retro_game_info info = {
		filename,
		0,
		0,
		NULL
	};
	FILE * file = fopen(filename, "rb");

	if (!file)
		goto libc_error;

	fseek(file, 0, SEEK_END);
	info.size = ftell(file);
	rewind(file);

	g_retro.retro_get_system_info( & system);

	if (!system.need_fullpath) {
		info.data = malloc(info.size);

		if (!info.data || !fread((void * ) info.data, info.size, 1, file))
			goto libc_error;
	}

	if (!g_retro.retro_load_game( & info))
		die("The core failed to load the content.");

	g_retro.retro_get_system_av_info( & av);

	video_configure( & av.geometry);
	audio_init(av.timing.sample_rate);

	return;

	libc_error:
		die("Failed to load content '%s'", filename);
}

static void core_unload() {
	if (g_retro.initialized)
		g_retro.retro_deinit();

	if (g_retro.handle)
		dlclose(g_retro.handle);
}

int main(int argc, char * argv[]) {
	if (argc < 3)
		die("usage: %s <core> <game>", argv[0]);

	core_load(argv[1]);
	core_load_game(argv[2]);

	g_retro.retro_run();

	core_unload();
	audio_deinit();
	video_deinit();

	return 0;
}
