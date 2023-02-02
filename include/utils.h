#ifndef SL_UTILS_H
#define SL_UTILS_H

#include <paf.h>

namespace Utils
{
    SceVoid GetStringFromID(const char *id, paf::string *out);
    SceVoid GetfStringFromID(const char *id, paf::string *out);
    wchar_t *GetStringPFromID(const char *id);
    wchar_t *GetStringPFromIDWithNum(const char *id, int num);
    SceUInt32 GetHashById(const char *id);    
    SceInt32 PlayEffect(paf::ui::Widget *widget, SceFloat32 param, paf::effect::EffectType animId, paf::ui::EventCallback::EventHandler animCB = (paf::ui::EventCallback::EventHandler)SCE_NULL, ScePVoid pUserData = SCE_NULL);
    SceInt32 PlayEffectReverse(paf::ui::Widget *widget, SceFloat32 param, paf::effect::EffectType animId, paf::ui::EventCallback::EventHandler animCB = (paf::ui::EventCallback::EventHandler)SCE_NULL, ScePVoid pUserData = SCE_NULL);
    paf::string GetPreviousPath();
    SceVoid SetPreviousPath(const char *path);
    template<class T = paf::ui::Widget>
    T *GetChildByHash(paf::ui::Widget *parent, SceUInt32 hash)
    {
        paf::rco::Element search;
        search.hash = hash;
        return (T*)parent->GetChild(&search, 0);
    }
    
    SceInt32 DownloadFile(const char *url, const char *destination, void *callingPage, paf::ui::ProgressBar *progressBar = NULL);
    SceInt32 SetWidgetLabel(paf::ui::Widget *widget, const char *text);
    SceInt32 SetWidgetLabel(paf::ui::Widget *widget, paf::string *text);

#ifdef _DEBUG
    SceVoid PrintAllChildren(paf::ui::Widget *widget, int offset = 0);
#endif

};
#endif