#include <kernel.h>
#include <audioout.h>
#include <ShellAudio.h>
#include <paf.h>
#include <power.h>
#include <libsysmodule.h>
#include <appmgr.h>
#include <message_dialog.h>
#include <taihen.h>

#include "utils.h"
#include "print.h"
#include "common.h"

using namespace paf;

SceUInt32 Utils::GetHashById(const char *id)
{
    rco::Element searchReq;
    rco::Element searchRes;
    
    searchReq.id = id;
    searchRes.hash = searchRes.GetHash(&searchReq.id);

    return searchRes.hash;
}

SceVoid Utils::GetStringFromID(const char *id, paf::string *out)
{
    rco::Element e;
    e.hash = Utils::GetHashById(id);
    
    wchar_t *wstr = g_appPlugin->GetWString(&e);
    common::Utf16ToUtf8((const wchar_t *)wstr, out);
}

SceVoid Utils::GetfStringFromID(const char *id, paf::string *out)
{
    paf::string *str = new paf::string;
    Utils::GetStringFromID(id, str);

    int slashNum = 0;
    int strlen = str->length();
    const char *strptr = str->data();
    for(int i = 0; i < strlen + 1 && strptr[i] != '\0'; i++)
        if(strptr[i] == '\\') slashNum++;

    int buffSize = (strlen + 1) - slashNum;
    char *buff = new char[buffSize];
    sce_paf_memset(buff, 0, buffSize);

    for(char *buffPtr = buff, *strPtr = (char *)strptr; *strPtr != '\0'; strPtr++, buffPtr++)
    {
        if(*strPtr == '\\')
        {
            switch(*(strPtr + sizeof(char)))
            {
                case 'n':
                    *buffPtr = '\n';
                    break;
                case 'a':
                    *buffPtr = '\a';
                    break;
                case 'b':
                    *buffPtr = '\b';
                    break;
                case 'e':
                    *buffPtr = '\e';
                    break;
                case 'f':
                    *buffPtr = '\f';
                    break;
                case 'r':
                    *buffPtr = '\r';
                    break;
                case 'v':
                    *buffPtr = '\v';
                    break;
                case '\\':
                    *buffPtr = '\\';
                    break;
                case '\'':
                    *buffPtr = '\'';
                    break;
                case '\"':
                    *buffPtr = '\"';
                    break;
                case '?':
                    *buffPtr = '\?';
                    break;
                case 't':
                    *buffPtr = '\t';
                    break;
            }
            strPtr++;
        }
        else *buffPtr = *strPtr;
    }

    *out = buff;

    delete str;
    delete[] buff;
}

SceInt32 Utils::SetWidgetLabel(paf::ui::Widget *widget, const char *text)
{
    paf::wstring wstr;
    common::Utf8ToUtf16(text, &wstr);    
    return widget->SetLabel(&wstr);
}

SceInt32 Utils::SetWidgetLabel(paf::ui::Widget *widget, paf::string *text)
{
    paf::wstring wstr;

    common::Utf8ToUtf16(text->c_str(), &wstr);

    return widget->SetLabel(&wstr);
}

wchar_t *Utils::GetStringPFromID(const char *id)
{
    rco::Element e;
    e.hash = Utils::GetHashById(id);

    return g_appPlugin->GetWString(&e);
}

wchar_t *Utils::GetStringPFromIDWithNum(const char *id, int num)
{
    string       str;
    rco::Element e;
    common::string_util::setf(str, "%s%d", id, num);

    e.hash = Utils::GetHashById(str.c_str());

    return g_appPlugin->GetWString(&e);
}

SceInt32 Utils::PlayEffect(paf::ui::Widget *widget, SceFloat32 param, paf::effect::EffectType type, paf::ui::EventCallback::EventHandler animCB, ScePVoid pUserData)
{
    widget->PlayEffect(param, type, animCB, pUserData);
    if(widget->animationStatus & 0x80)
        widget->animationStatus &= ~0x80;
}

SceInt32 Utils::PlayEffectReverse(paf::ui::Widget *widget, SceFloat32 param, paf::effect::EffectType type, paf::ui::EventCallback::EventHandler animCB, ScePVoid pUserData)
{
    widget->PlayEffectReverse(param, type, animCB, pUserData);
    if(widget->animationStatus & 0x80)
        widget->animationStatus &= ~0x80;
}

#define PREVIOUS_PATH_LOCATION "ur0:data/sl_prevPath"
paf::string Utils::GetPreviousPath()
{
    SceInt32 err = SCE_OK;
    auto file = LocalFile::Open(PREVIOUS_PATH_LOCATION, SCE_O_RDONLY, 0, &err);
    if(err != SCE_OK)
        return paf::string("");
    
    SceSize size = file.get()->GetFileSize() + 1;
    char *buff = new char[size];
    sce_paf_memset(buff, 0, size);
    file.get()->Read(buff, size - 1);
    paf::string ret(buff);
    delete[] buff;
    return ret;
}

SceVoid Utils::SetPreviousPath(const char *path)
{
    SceInt32 err = SCE_OK;
    auto file = LocalFile::Open(PREVIOUS_PATH_LOCATION, SCE_O_WRONLY | SCE_O_CREAT | SCE_O_TRUNC, 0666, &err);
    if(err != SCE_OK)
    {
        print("[NON_FATAL] Error opening %s for writing 0x%X\n", PREVIOUS_PATH_LOCATION, err);
        return;
    }
    file.get()->Write(path, sce_paf_strlen(path));
}

#ifdef _DEBUG

SceVoid Utils::PrintAllChildren(paf::ui::Widget *widget, int offset)
{
    for (int i = 0; i < widget->childNum; i++)
    {
        for (int i = 0; i < offset; i++) print(" ");
        wstring wstr;
        widget->GetChild(i)->GetLabel(&wstr);
        print(" %d 0x%X (%s, \"%ls\")\n", i, widget->GetChild(i)->elem.hash, widget->GetChild(i)->name(), wstr.data());
        Utils::PrintAllChildren(widget->GetChild(i), offset + 4);
    }
}

#endif