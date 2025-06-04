#include <fmod.hpp>
#include <iostream>
#include <unordered_map>
#include <vector>
#include <string>

#include "Container/Array.h"
#include "UObject/NameTypes.h"

class FSoundManager {
public:
    static FSoundManager& GetInstance() {
        static FSoundManager instance;
        return instance;
    }

    bool Initialize() {
        FMOD_RESULT result = FMOD::System_Create(&system);
        if (result != FMOD_OK) {
            std::cerr << "FMOD System_Create failed!" << std::endl;
            return false;
        }

        result = system->init(1024, FMOD_INIT_NORMAL, nullptr);
        if (result != FMOD_OK) {
            std::cerr << "FMOD system init failed!" << std::endl;
            return false;
        }

        return true;
    }

    void Shutdown() {
        for (auto& pair : soundMap) {
            pair.second->release();
        }
        soundMap.clear();

        if (system) {
            system->close();
            system->release();
        }
    }

    bool LoadSound(const std::string& name, const std::string& filePath, bool loop = false) {
        if (soundMap.find(name) != soundMap.end()) {
            return true;
        }

        FMOD::Sound* sound = nullptr;
        FMOD_MODE mode = loop ? FMOD_DEFAULT | FMOD_LOOP_NORMAL | FMOD_CREATECOMPRESSEDSAMPLE :
            FMOD_DEFAULT | FMOD_LOOP_OFF | FMOD_CREATECOMPRESSEDSAMPLE;

        if (system->createSound(filePath.c_str(), mode, nullptr, &sound) != FMOD_OK) {
            std::cerr << "Failed to load sound: " << filePath << std::endl;
            return false;
        }
        soundMap[name] = sound;
        return true;
    }


    void PlaySound(const std::string& name, unsigned int delayMs = 0) {
        auto it = soundMap.find(name);
        if (it != soundMap.end()) {
            FMOD::Channel* newChannel = nullptr;
            system->playSound(it->second, nullptr, false, &newChannel);
            if (newChannel) {
                if (delayMs > 0) {
                    unsigned long long dspClock = 0;
                    FMOD::ChannelGroup* masterGroup = nullptr;
                    if (system->getMasterChannelGroup(&masterGroup) == FMOD_OK && masterGroup) {
                        masterGroup->getDSPClock(&dspClock, nullptr);
                        int sampleRate = 0;
                        system->getSoftwareFormat(&sampleRate, nullptr, nullptr);
                        unsigned long long delayDSPClock = dspClock + (delayMs * sampleRate) / 1000;
                        newChannel->setDelay(delayDSPClock, 0, false);
                    }
                }
                activeChannels.push_back(newChannel);
            }
        }
    }

    void Update() {
        system->update();
        activeChannels.erase(
    std::remove_if(activeChannels.begin(), activeChannels.end(),
        [](FMOD::Channel* channel) {
            bool isPlaying = false;
            if (channel) {
                FMOD_RESULT result = channel->isPlaying(&isPlaying);
                if (result != FMOD_OK) {
                    return true; 
                }
            }
            return !isPlaying;
        }),
    activeChannels.end()
);

    }

    void StopAllSounds()
    {
        // 1) 개별 채널을 멈추기
        for (FMOD::Channel* channel : activeChannels)
        {
            if (channel)
            {
                channel->stop();
            }
        }
        activeChannels.clear();

        // 2) 시스템 전체(마스터 채널 그룹) 정지 (Optional)
        FMOD::ChannelGroup* masterGroup = nullptr;
        if (system->getMasterChannelGroup(&masterGroup) == FMOD_OK && masterGroup)
        {
            masterGroup->stop();
        }
    }


    TArray<std::string> GetAllSoundNames() const {
        TArray<std::string> names;
        for (const auto& pair : soundMap)
        {
            names.Add(pair.first);
        }
        return names;
    }

    void StopSound(const std::string& name) {
        auto it = soundMap.find(name);
        if (it == soundMap.end())
            return;

        FMOD::Sound* targetSound = it->second;

        for (auto channelIt = activeChannels.begin(); channelIt != activeChannels.end(); /*증가 없음*/)
        {
            FMOD::Channel* channel = *channelIt;

            // 1) channel이 null이면 바로 제거
            if (!channel)
            {
                channelIt = activeChannels.erase(channelIt);
                continue;
            }

            // 2) FMOD 시스템 업데이트 (매 프레임 한 번만 하면 되지만, 안전을 위해 여기서도 호출)
            system->update();

            // 3) 먼저 isPlaying 검사
            bool isPlaying = false;
            FMOD_RESULT r1 = channel->isPlaying(&isPlaying);
            if (r1 != FMOD_OK || !isPlaying)
            {
                // 이미 재생 끝났거나, 에러 발생 → 채널 정리
                channel->stop();
                channelIt = activeChannels.erase(channelIt);
                continue;
            }

            // 4) 이제 getCurrentSound 시도
            FMOD::Sound* currentSound = nullptr;
            FMOD_RESULT r2 = channel->getCurrentSound(&currentSound);
            if (r2 != FMOD_OK || !currentSound)
            {
                // 소리 정보를 불러오지 못하면 → 채널 정리
                channel->stop();
                channelIt = activeChannels.erase(channelIt);
                continue;
            }

            // 5) 채널의 currentSound가 우리가 멈추려는 targetSound와 동일한지 비교
            if (currentSound == targetSound)
            {
                channel->stop();
                channelIt = activeChannels.erase(channelIt);
            }
            else
            {
                // 다른 사운드면 그냥 다음 채널로
                ++channelIt;
            }
        }
    }

    FMOD::Channel* GetChannelByName(const std::string& name) const {
        for (FMOD::Channel* channel : activeChannels) {
            FMOD::Sound* currentSound = nullptr;
            if (channel->getCurrentSound(&currentSound) == FMOD_OK && currentSound) {
                if (GetSound(name) == currentSound)
                    return channel;
            }
        }
        return nullptr; // 찾지 못한 경우
    }

    FMOD::Sound* GetSound(const std::string& name) const {
        auto it = soundMap.find(name);
        if (it != soundMap.end()) {
            return it->second; // 해당 이름의 사운드 반환
        }
        return nullptr; // 찾지 못한 경우
    }

    FMOD::System* GetSystem() const {
        return system; // FMOD 시스템 반환
    }

    
private:
    FSoundManager() : system(nullptr) {}
    ~FSoundManager() { Shutdown(); }
    FSoundManager(const FSoundManager&) = delete;
    FSoundManager& operator=(const FSoundManager&) = delete;

    FMOD::System* system;
    std::unordered_map<std::string, FMOD::Sound*> soundMap;
    std::vector<FMOD::Channel*> activeChannels;
};
