#ifndef SETTINGS_H
#define SETTINGS_H

#include <kernel.h>
#include <app_settings.h>

class Settings
{
public:
    enum Hash
    {
        Hash_Budget = 0x81A4C14,
        Hash_La_Type = 0x10451287,
        Hash_TitleID = 0x6AB8271B,
        Hash_Memsize = 0xBF070427,
        Hash_Phymemsize = 0x5947ED00,
        Hash_Args = 0xC051E3DA,
        Hash_Exit_To = 0xF5121C7F,
        Hash_Port = 0xE29C67A5,

        Hash_Cdlg = 0xF127BA36,
        Hash_Bg = 0xF40D741,
        Hash_Nonsuspendable = 0x7AB0D3B8,
        Hash_La_off = 0x91CABF24,
        Hash_Budget_Enabled = 0xD97E265A,
        Hash_La_Type_Enabled = 0x48D91DE0,
        Hash_Memsize_Enabled = 0xDF4A8FD9,
        Hash_TitleID_Enabled = 0x5CBBBD28,
        Hash_Phmemsize_Enabled = 0x7F90C46F,
        Hash_Args_Enabled = 0x858844FD,
        Hash_Exit_To_Enabled = 0xE2F551F1,
        Hash_Port_Enabled = 0x7E5D0200,

        Hash_LaunchArgs_Enabled = 0xE30D69A5,
        Hash_OpenButton = 0x93B02D5E
    };

    Settings();
    ~Settings();

    static Settings *GetInstance();
    static sce::AppSettings *GetAppSettings();
    SceVoid Open();

    SceInt32 launchArgs_enabled;

    paf::string budget;
    paf::string la_type;
    paf::string args;
    SceInt32 memsize;
    SceInt32 phymemsize;
    char titleID[10];
    char exit_to[10];
    SceInt32 port;

    SceInt32 device;
    SceInt32 usecdlg;
    SceInt32 bg;
    SceInt32 nonsuspendable;
    SceInt32 la_off;
    SceInt32 budget_enabled;
    SceInt32 la_type_enabled;
    SceInt32 memsize_enabled;
    SceInt32 titleid_enabled;
    SceInt32 phmemsize_enabled;
    SceInt32 args_enabled;
    SceInt32 exit_to_enabled;
    SceInt32 port_enabled;

private:
    static sce::AppSettings *appSettings;

    static SceVoid CBListChange(const char *elementId, SceInt32 type);

    static SceVoid CBListForwardChange(const char *elementId, SceInt32 type);

    static SceVoid CBListBackChange(const char *elementId, SceInt32 type);

    static SceInt32 CBIsVisible(const char *elementId, SceBool *pIsVisible);

    static SceInt32 CBElemInit(const char *elementId, sce::AppSettings::Element *element);

    static SceInt32 CBElemAdd(const char *elementId, paf::ui::Widget *widget);

    static SceInt32 CBValueChange(const char *elementId, const char *newValue);

    static SceInt32 CBValueChange2(const char *elementId, const char *newValue);

    static SceVoid CBTerm(SceInt32 result);

    static wchar_t *CBGetString(const char *elementId);

    static SceInt32 CBGetTex(paf::graph::Surface **tex, const char *elementId);

    const int d_settingsVersion = 1;
    const char *d_budget = "big";
    const char *d_la_type = "B";
    const char *d_titleID = "ABCD12345";
    const int d_memsize = 0;
    const int d_phymemsize = 0;
    const char *d_args = "";
    const char *d_exit_to = "ABCD12345";
    const int d_port = 1;

    const SceInt32 d_device = 0;
    const SceInt32 d_launchArgs_enabled = 0;
    const SceInt32 d_usecdlg = 0;
    const SceInt32 d_bg = 0;
    const SceInt32 d_nonsuspendable = 0;
    const SceInt32 d_la_off = 0;
    const SceInt32 d_budget_enabled = 0;
    const SceInt32 d_la_type_enabled = 0;
    const SceInt32 d_memsize_enabled = 0;
    const SceInt32 d_titleid_enabled = 0;
    const SceInt32 d_phmemsize_enabled = 0;
    const SceInt32 d_args_enabled = 0;
    const SceInt32 d_exit_to_enabled = 0;
    const SceInt32 d_port_enabled = 0;
};

#endif // !SETTINGS_H
