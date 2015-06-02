SDL_ROOT = C:/SDL
SDL_x86 = i686-w64-mingw32
SDL_x64 = x86_64-w64-mingw32


SDL_SDL_V = SDL2-2.0.3
SDL_IMAGE_V = SDL2_image-2.0.0
SDL_MIXER_V = SDL2_mixer-2.0.0
SDL_NET_V = SDL2_net-2.0.0
SDL_TTF_V = SDL2_ttf-2.0.12

SDL_LIBS = -L"$(SDL_ROOT)/$(SDL_SDL_V)/$(SDL_x86)/lib" -L"$(SDL_ROOT)/$(SDL_IMAGE_V)/$(SDL_x86)/lib" -L"$(SDL_ROOT)/$(SDL_MIXER_V)/$(SDL_x86)/lib" -L"$(SDL_ROOT)/$(SDL_NET_V)/$(SDL_x86)/lib" -L"$(SDL_ROOT)/$(SDL_TTF_V)/$(SDL_x86)/lib" -L"$(SDL_ROOT)/$(SDL_SDL_V)/$(SDL_x64)/lib" -L"$(SDL_ROOT)/$(SDL_IMAGE_V)/$(SDL_x64)/lib" -L"$(SDL_ROOT)/$(SDL_MIXER_V)/$(SDL_x64)/lib" -L"$(SDL_ROOT)/$(SDL_NET_V)/$(SDL_x64)/lib" -L"$(SDL_ROOT)/$(SDL_TTF_V)/$(SDL_x64)/lib" -L"./libs"

SDL_INCS = -I"$(SDL_ROOT)/$(SDL_SDL_V)/$(SDL_x86)/include/SDL2" -I"$(SDL_ROOT)/$(SDL_IMAGE_V)/$(SDL_x86)/include/SDL2" -I"$(SDL_ROOT)/$(SDL_MIXER_V)/$(SDL_x86)/include/SDL2" -I"$(SDL_ROOT)/$(SDL_NET_V)/$(SDL_x86)/include/SDL2" -I"$(SDL_ROOT)/$(SDL_TTF_V)/$(SDL_x86)/include/SDL2" -I"$(SDL_ROOT)/$(SDL_SDL_V)/$(SDL_x86)/include" -I"$(SDL_ROOT)/$(SDL_IMAGE_V)/$(SDL_x86)/include" -I"$(SDL_ROOT)/$(SDL_MIXER_V)/$(SDL_x86)/include" -I"$(SDL_ROOT)/$(SDL_NET_V)/$(SDL_x86)/include" -I"$(SDL_ROOT)/$(SDL_TTF_V)/$(SDL_x86)/include"

LIBS := $(SDL_LIBS) $(LIBS)

CXXFLAGS := $(SDL_INCS) $(CXXFLAGS)
CFLAGS := $(SDL_INCS) $(CFLAGS)