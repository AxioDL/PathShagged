#ifndef __URDE_CGUIMODEL_HPP__
#define __URDE_CGUIMODEL_HPP__

#include "CGuiWidget.hpp"
#include "CToken.hpp"
#include "Graphics/CModel.hpp"

namespace urde
{
class CSimplePool;

class CGuiModel : public CGuiWidget
{
    TLockedToken<CModel> xf8_model;
    ResId x108_modelId;
    u32 x10c_lightMask;
public:
    CGuiModel(const CGuiWidgetParms& parms, CSimplePool* sp, ResId modelId, u32 lightMask, bool flag);
    FourCC GetWidgetTypeID() const {return FOURCC('MODL');}

    std::vector<ResId> GetModelAssets() const;
    bool GetIsFinishedLoadingWidgetSpecific() const;
    void Touch() const;
    void Draw(const CGuiWidgetDrawParms& parms) const;

    static std::shared_ptr<CGuiWidget> Create(CGuiFrame* frame, CInputStream& in, CSimplePool* sp);
};

}

#endif // __URDE_CGUIMODEL_HPP__
