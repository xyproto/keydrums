#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>

#include <algorithm>
#include <filesystem>
#include <iostream>
#include <iterator>
#include <random>
#include <string>
#include <unistd.h>
#include <vector>

using namespace std::string_literals;

using SampleIndex = int;

const int maxChannels = 32;

// thanks https://stackoverflow.com/a/16421677/131264
template <typename Iter, typename RandomGenerator>
Iter select_randomly(Iter start, Iter end, RandomGenerator& g)
{
    std::uniform_int_distribution<> dis(0, std::distance(start, end) - 1);
    std::advance(start, dis(g));
    return start;
}

// thanks https://stackoverflow.com/a/16421677/131264
template <typename Iter>
Iter select_randomly(Iter start, Iter end)
{
    static std::random_device rd;
    static std::mt19937 gen(rd());
    return select_randomly(start, end, gen);
}

inline bool hasSuffix(std::string const& fullString, std::string const& ending)
{
    // thanks https://stackoverflow.com/a/874160/131264
    if (fullString.length() >= ending.length()) {
        return (0
            == fullString.compare(fullString.length() - ending.length(), ending.length(), ending));
    }
    return false;
}

inline bool contains(std::string const& strHaystack, std::string const& strNeedle)
{
    return (strHaystack.find(strNeedle) != std::string::npos);
}

// case insensitive contains
inline bool iContains(const std::string& strHaystack, const std::string& strNeedle)
{
    // thanks https://stackoverflow.com/a/19839371/131264
    auto it
        = std::search(strHaystack.begin(), strHaystack.end(), strNeedle.begin(), strNeedle.end(),
            [](char ch1, char ch2) { return std::toupper(ch1) == std::toupper(ch2); });
    return it != strHaystack.end();
}

inline const std::vector<std::string> findFiles(std::string const& path, std::string const& ext)
{
    std::vector<std::string> collected;
    for (auto& p : std::filesystem::recursive_directory_iterator(path)) {
        if (hasSuffix(p.path(), ext)) {

            collected.push_back(p.path());
        }
    }
    return collected;
}

static SampleIndex defaultKick = 0;
static SampleIndex defaultSnare = 0;
static SampleIndex defaultHiHat = 0;
static SampleIndex defaultCrash = 0;
static SampleIndex defaultTom = 0;
static SampleIndex defaultRide = 0;
static SampleIndex defaultOpHat = 0;

// Initializes the application data and return a vector of samples
std::vector<Mix_Chunk*> InitAndLoad(std::vector<SampleIndex>& kicks,
    std::vector<SampleIndex>& snares, std::vector<SampleIndex>& hihats,
    std::vector<SampleIndex>& crashes, std::vector<SampleIndex>& toms,
    std::vector<SampleIndex>& rides, std::vector<SampleIndex>& ophats)
{

    // Set up the audio stream
    int result = Mix_OpenAudio(44100, AUDIO_S16SYS, 2, 512);
    if (result < 0) {
        fprintf(stderr, "Unable to open audio: %s\n", SDL_GetError());
        exit(-1);
    }

    result = Mix_AllocateChannels(maxChannels);
    if (result < 0) {
        fprintf(stderr, "Unable to allocate mixing channels: %s\n", SDL_GetError());
        exit(-1);
    }

    // All the samples
    std::vector<Mix_Chunk*> samples;

    // Find and load all wav files
    std::cout << "Loading ";
    SampleIndex sampleIndex = 0;
    for (auto filename : findFiles(".", ".wav")) {
        if (contains(filename, "bpm"s) || contains(filename, "loop"s)) {
            // Skip samples that are loops or drum loops
            continue;
        }
        // std::cout << "Loading " << filename << std::endl;

        std::cout << ".";
        auto sample = Mix_LoadWAV(filename.c_str());
        if (sample == nullptr) {
            fprintf(stderr, "\nCould not load %s\n", filename.c_str());
            continue;
        }

        auto foundSpecific = false;
        if (iContains(filename, "cycdh_eleck01-kick02")) {
            // std::cout << "FOUND KICK " << filename << ", " << sampleIndex << std::endl;
            defaultKick = sampleIndex;
            foundSpecific = true;
        } else if (iContains(filename, "acoustic snare-02")) {
            // std::cout << "FOUND SNARE " << filename << ", " << sampleIndex << std::endl;
            defaultSnare = sampleIndex;
            foundSpecific = true;
        } else if (iContains(filename, "cycdh_sab_clhat-10")) {
            // std::cout << "FOUND HIHAT " << filename << ", " << sampleIndex << std::endl;
            defaultHiHat = sampleIndex;
            foundSpecific = true;
        } else if (iContains(filename, "cycdh_trashe-01")) {
            // std::cout << "FOUND CRASH " << filename << ", " << sampleIndex << std::endl;
            defaultCrash = sampleIndex;
            foundSpecific = true;
        } else if (iContains(filename, "cycdh_k3tom-01")) {
            // std::cout << "FOUND TOM " << filename << ", " << sampleIndex << std::endl;
            defaultTom = sampleIndex;
            foundSpecific = true;
        } else if (iContains(filename, "cycdh_eleck01-cymbal")) {
            // std::cout << "FOUND RIDE " << filename << ", " << sampleIndex << std::endl;
            defaultRide = sampleIndex;
            foundSpecific = true;
        } else if (iContains(filename, "cycdh_k3ophat-01")) {
            // std::cout << "FOUND OPHAT " << filename << ", " << sampleIndex << std::endl;
            defaultOpHat = sampleIndex;
            foundSpecific = true;
        }

        auto foundCategory = true;
        if (iContains(filename, "kick")) {
            // printf("Sample index %d filename %s is a kick!\n", sampleIndex, filename.c_str());
            kicks.push_back(sampleIndex);
        } else if (iContains(filename, "snare") || iContains(filename, "snr")) {
            // printf("Sample index %d filename %s is a snare!\n", sampleIndex, filename.c_str());
            snares.push_back(sampleIndex);
        } else if (iContains(filename, "clhat")) { // closed hi hat
            // printf("Sample index %d filename %s is a closed hi hat!\n", sampleIndex,
            // filename.c_str());
            hihats.push_back(sampleIndex);
        } else if (iContains(filename, "crash") && !iContains(filename, "noise")) {
            // printf("Sample index %d filename %s is a crash!\n", sampleIndex, filename.c_str());
            crashes.push_back(sampleIndex);
        } else if (iContains(filename, "tom")) {
            toms.push_back(sampleIndex);
        } else if (iContains(filename, "ride")) {
            rides.push_back(sampleIndex);
        } else if (iContains(filename, "ophat")) {
            ophats.push_back(sampleIndex);
        } else {
            foundCategory = false;
            // printf("WOOT? %s\n", filename.c_str());
        }

        if (foundCategory || foundSpecific) {
            // Only keep the samples that fit one of the above categories
            samples.push_back(sample);
            sampleIndex++;
        }
    }
    std::cout << std::endl;

    auto w = ""s;
    if (kicks.size() == 0) {
        w = "kick";
    } else if (snares.empty()) {
        w = "snare";
    } else if (hihats.empty()) {
        w = "hihat";
    } else if (crashes.empty()) {
        w = "crash";
    } else if (crashes.empty()) {
        w = "tom";
    } else if (rides.empty()) {
        w = "ride";
    } else if (ophats.empty()) {
        w = "ophat";
    }
    if (!w.empty()) {
        std::cerr << "Found no " << w << "s!" << std::endl;
    }

    return samples;
}

int main(int argc, char** argv)
{
    // Initialize the SDL library with the Video subsystem
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
    atexit(SDL_Quit);

    SDL_Window* win = SDL_CreateWindow("Keydrums 1.0.0", SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED, 1027, 768, SDL_WINDOW_SHOWN);
    if (win == nullptr) {
        std::cerr << "SDL_CreateWindow Error: " << SDL_GetError() << std::endl;
        return EXIT_FAILURE;
    }

    SDL_Renderer* ren = SDL_CreateRenderer(win, -1, 0);
    if (ren == nullptr) {
        if (win != nullptr) {
            SDL_DestroyWindow(win);
        }
        SDL_Quit();
        return EXIT_FAILURE;
    }

    SDL_Surface* bmp = SDL_LoadBMP("img/keybindings.bmp");
    if (bmp == nullptr) {
        std::cerr << "SDL_LoadBMP Error: " << SDL_GetError() << std::endl;
        if (ren != nullptr) {
            SDL_DestroyRenderer(ren);
        }
        if (win != nullptr) {
            SDL_DestroyWindow(win);
        }
        SDL_Quit();
        return EXIT_FAILURE;
    }

    SDL_Texture* tex = SDL_CreateTextureFromSurface(ren, bmp);
    if (tex == nullptr) {
        std::cerr << "SDL_CreateTextureFromSurface Error: " << SDL_GetError() << std::endl;
        if (bmp != nullptr) {
            SDL_FreeSurface(bmp);
        }
        if (ren != nullptr) {
            SDL_DestroyRenderer(ren);
        }
        if (win != nullptr) {
            SDL_DestroyWindow(win);
        }
        SDL_Quit();
        return EXIT_FAILURE;
    }
    SDL_FreeSurface(bmp);

    std::vector<SampleIndex> kicks, snares, hihats, crashes, toms, rides, ophats;

    // Application specific Initialize of data structures
    auto samples = InitAndLoad(kicks, snares, hihats, crashes, toms, rides, ophats);

    SampleIndex currentKick = defaultKick; // kicks[0]
    SampleIndex currentSnare = defaultSnare; // snares[0];
    SampleIndex currentHiHat = defaultHiHat; // hihats[0]
    SampleIndex currentCrash = defaultCrash; // crashes[0]
    SampleIndex currentTom = defaultTom; // toms[0]
    SampleIndex currentRide = defaultRide; // rides[0]
    SampleIndex currentOpHat = defaultOpHat; // ophats[0]

    // Event descriptor
    SDL_Event Event;

    bool done = false;
    while (!done) {

        SDL_RenderClear(ren);
        SDL_RenderCopy(ren, tex, nullptr, nullptr);
        SDL_RenderPresent(ren);

        bool gotEvent = SDL_PollEvent(&Event);

        const auto delay = 100000;
        int volume = 128;

        std::vector<int> usedChannels;
        int i, freeChannel = -1;

        while (!done && gotEvent) {
            switch (Event.type) {
            case SDL_KEYDOWN:
                switch (Event.key.keysym.sym) {
                case 'a': // kick
                    i = Mix_GroupAvailable(-1);
                    Mix_Volume(i, 128);
                    Mix_PlayChannel(i, samples[currentKick], 0);
                    break;
                case 'p': // special snare
                    // TODO: Don't play the sample repeatedly,
                    //       rather prepare the sample in advance.
                    volume = 128;
                    for (i = (maxChannels - 4); i < maxChannels; ++i) {
                        freeChannel = Mix_GroupAvailable(-1);
                        Mix_Volume(freeChannel, volume);
                        Mix_PlayChannel(freeChannel, samples[currentSnare], 0);
                        usleep(delay);
                        volume /= 2;
                        usedChannels.push_back(freeChannel);
                    }
                    // for (auto i : usedChannels) {
                    //    Mix_FadeOutChannel(i, 200);
                    //}
                    usedChannels.clear();
                    // usedChannels = nullptr;
                    break;
                case 'w': // snare
                case 'r': // snare
                    i = Mix_GroupAvailable(-1);
                    Mix_Volume(i, 128);
                    Mix_PlayChannel(i, samples[currentSnare], 0);
                    break;
                case 'd': // crash
                    i = Mix_GroupAvailable(-1);
                    Mix_Volume(i, 128);

                    Mix_PlayChannel(i, samples[currentCrash], 0);
                    break;
                case 's': // hi-hat
                    i = Mix_GroupAvailable(-1);
                    Mix_Volume(i, 128);
                    Mix_PlayChannel(i, samples[currentHiHat], 0);
                    break;
                case 'q': // tom
                    i = Mix_GroupAvailable(-1);
                    Mix_Volume(i, 128);
                    Mix_PlayChannel(i, samples[currentTom], 0);
                    break;
                case 'e': // ride
                    i = Mix_GroupAvailable(-1);
                    Mix_Volume(i, 128);
                    Mix_PlayChannel(i, samples[currentRide], 0);
                    break;
                case 'x': // ophat
                    i = Mix_GroupAvailable(-1);
                    Mix_Volume(i, 128);
                    Mix_PlayChannel(i, samples[currentOpHat], 0);
                    break;
                case 'i': // output sample indexes
                    std::cerr << "k " << currentKick << " s " << currentSnare << " hh "
                              << currentHiHat << " c " << currentCrash << " t " << currentTom
                              << " r " << currentRide << " oh " << currentOpHat << std::endl;
                case 'f': // randomize samples
                    currentKick = *select_randomly(kicks.begin(), kicks.end());
                    currentSnare = *select_randomly(snares.begin(), snares.end());
                    currentHiHat = *select_randomly(hihats.begin(), hihats.end());
                    currentCrash = *select_randomly(crashes.begin(), crashes.end());
                    currentTom = *select_randomly(toms.begin(), toms.end());
                    currentRide = *select_randomly(rides.begin(), rides.end());
                    currentOpHat = *select_randomly(ophats.begin(), ophats.end());
                    break;
                case SDLK_ESCAPE: // quit
                    done = true;
                    break;
                case SDLK_SPACE: // quickly fade out all channels
                    Mix_FadeOutChannel(-1, 200);
                    break;
                default:
                    break;
                }
                break;

            case SDL_QUIT:
                done = true;
                break;

            default:
                break;
            }
            if (!done)
                gotEvent = SDL_PollEvent(&Event);
        }
#ifndef WIN32
        usleep(1000);
#else
        Sleep(1);
#endif
    }

    // Free samples
    for (size_t i = 0; i < samples.size(); ++i) {
        Mix_FreeChunk((Mix_Chunk*)(samples[i]));
    }

    Mix_CloseAudio();

    SDL_DestroyTexture(tex);
    SDL_DestroyRenderer(ren);
    SDL_DestroyWindow(win);

    SDL_Quit();

    return 0;
}
