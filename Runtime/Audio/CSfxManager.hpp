#ifndef __URDE_CSFXMANAGER_HPP__
#define __URDE_CSFXMANAGER_HPP__

#include <vector>
#include "../RetroTypes.hpp"
#include "zeus/CVector3f.hpp"
#include "CAudioSys.hpp"

namespace urde
{

class CSfxManager
{
    static std::vector<s16>* mpSfxTranslationTable;
public:
    CSfxManager();

    enum class ESfxChannels
    {
        Zero,
        One
    };

    enum class ESfxAudibility
    {
        Aud0,
        Aud1,
        Aud2,
        Aud3
    };

    class CSfxChannel
    {
    };

    class CBaseSfxWrapper;
    using CSfxHandle = std::shared_ptr<CBaseSfxWrapper>;

    class CBaseSfxWrapper : public std::enable_shared_from_this<CBaseSfxWrapper>
    {
        float x4_ = 15.f;
        s16 x8_rank = 0;
        s16 xa_prio;
        //CSfxHandle xc_handle;
        TAreaId x10_area;
        union
        {
            struct
            {
                bool x14_24_isActive:1;
                bool x14_25_isPlaying:1;
                bool x14_26_looped:1;
                bool x14_27_inArea:1;
                bool x14_28_available:1;
                bool x14_29_useAcoustics:1;
            };
            u16 _dummy = 0;
        };
    public:
        virtual ~CBaseSfxWrapper() = default;
        virtual void SetActive(bool v) { x14_24_isActive = v; }
        virtual void SetPlaying(bool v) { x14_25_isPlaying = v; }
        virtual void SetRank(short v) { x8_rank = v; }
        virtual void SetInArea(bool v) { x14_27_inArea = v; }
        virtual bool IsInArea() const { return x14_27_inArea; }
        virtual bool IsPlaying() const { return x14_25_isPlaying; }
        virtual bool UseAcoustics() const { return x14_29_useAcoustics; }
        virtual bool IsLooped() const { return x14_26_looped; }
        virtual bool IsActive() const { return x14_24_isActive; }
        virtual s16 GetRank() const { return x8_rank; }
        virtual s16 GetPriority() const { return xa_prio; }
        virtual TAreaId GetArea() const { return x10_area; }
        virtual CSfxHandle GetSfxHandle() { return shared_from_this(); }
        virtual void Play()=0;
        virtual void Stop()=0;
        virtual bool Ready()=0;
        virtual ESfxAudibility GetAudible(const zeus::CVector3f&)=0;
        virtual const std::shared_ptr<amuse::Voice>& GetVoice() const=0;

        void Release() { x14_28_available = true; }
        bool Available() const { return x14_28_available; }

        CBaseSfxWrapper(bool looped, s16 prio, /*const CSfxHandle& handle,*/ bool useAcoustics, TAreaId area)
        : x8_rank(0), xa_prio(prio), /*xc_handle(handle),*/ x10_area(area), x14_24_isActive(true), x14_25_isPlaying(false),
          x14_26_looped(looped), x14_27_inArea(true), x14_28_available(false), x14_29_useAcoustics(useAcoustics) {}
    };

    class CSfxEmitterWrapper : public CBaseSfxWrapper
    {
        float x1a_reverb;
        CAudioSys::C3DEmitterParmData x24_parmData;
        std::shared_ptr<amuse::Emitter> x50_emitterHandle;
        bool x54_ready = true;
    public:
        bool IsPlaying() const;
        void Play();
        void Stop();
        bool Ready();
        ESfxAudibility GetAudible(const zeus::CVector3f&);
        const std::shared_ptr<amuse::Voice>& GetVoice() const { return x50_emitterHandle->getVoice(); }

        const std::shared_ptr<amuse::Emitter>& GetHandle() const { return x50_emitterHandle; }

        CSfxEmitterWrapper(bool looped, s16 prio, const CAudioSys::C3DEmitterParmData& data,
                           /*const CSfxHandle& handle,*/ bool useAcoustics, TAreaId area)
        : CBaseSfxWrapper(looped, prio, /*handle,*/ useAcoustics, area), x24_parmData(data) {}
    };

    class CSfxWrapper : public CBaseSfxWrapper
    {
        u16 x18_sfxId;
        std::shared_ptr<amuse::Voice> x1c_voiceHandle;
        float x20_vol;
        float x22_pan;
        bool x24_ready = true;
    public:
        bool IsPlaying() const;
        void Play();
        void Stop();
        bool Ready();
        ESfxAudibility GetAudible(const zeus::CVector3f&) { return ESfxAudibility::Aud3; }
        const std::shared_ptr<amuse::Voice>& GetVoice() const { return x1c_voiceHandle; }

        void SetVolume(s16 vol) { x20_vol = vol; }

        CSfxWrapper(bool looped, s16 prio, u16 sfxId, float vol, float pan,
                    /*const CSfxHandle& handle,*/ bool useAcoustics, TAreaId area)
        : CBaseSfxWrapper(looped, prio, /*handle,*/ useAcoustics, area),
          x18_sfxId(sfxId), x20_vol(vol), x22_pan(pan) {}
    };

    static CSfxChannel m_channels[4];
    static rstl::reserved_vector<std::shared_ptr<CSfxEmitterWrapper>, 128> m_emitterWrapperPool;
    static rstl::reserved_vector<std::shared_ptr<CSfxWrapper>, 128> m_wrapperPool;
    static ESfxChannels m_currentChannel;
    static bool m_doUpdate;
    static void* m_usedSounds;
    static bool m_muted;
    static bool m_auxProcessingEnabled;
    static float m_reverbAmount;

    static u16 kMaxPriority;
    static u16 kMedPriority;
    static u16 kInternalInvalidSfxId;
    static u32 kAllAreas;

    static bool LoadTranslationTable(CSimplePool* pool, const SObjectTag* tag);
    static bool IsAuxProcessingEnabled() { return m_auxProcessingEnabled; }
    static void SetChannel(ESfxChannels) {}
    static void KillAll(ESfxChannels) {}
    static void TurnOnChannel(ESfxChannels) {}
    static ESfxChannels GetCurrentChannel() {return m_currentChannel;}
    static void AddListener(ESfxChannels,
                            const zeus::CVector3f& pos, const zeus::CVector3f& dir,
                            const zeus::CVector3f& heading, const zeus::CVector3f& up,
                            float frontRadius, float surroundRadius, float soundSpeed,
                            u32 flags /* 0x1 for doppler */, u8 vol);
    static void UpdateListener(const zeus::CVector3f& pos, const zeus::CVector3f& dir,
                               const zeus::CVector3f& heading, const zeus::CVector3f& up,
                               u8 vol);

    static void RemoveEmitter(const CSfxHandle&) {}
    static void PitchBend(const CSfxHandle&, s32) {}
    static u16 TranslateSFXID(u16);
    static void SfxStop(const CSfxHandle& handle);
    static CSfxHandle SfxStart(u16 id, float vol, float pan, bool useAcoustics, s16 prio, bool looped, s32 areaId);
    static void StopAndRemoveAllEmitters();
    static void DisableAuxCallbacks();

    static void Update();
    static void Shutdown();

private:
    static std::shared_ptr<CSfxWrapper>* AllocateCSfxWrapper();
};

using CSfxHandle = CSfxManager::CSfxHandle;

}

#endif // __URDE_CSFXMANAGER_HPP__
