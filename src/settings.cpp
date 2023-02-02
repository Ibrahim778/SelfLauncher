#include "settings.h"
#include "utils.h"
#include "common.h"
#include "pages/file_browser.h"
#include "print.h"
#include "sl_settings.h"
#include "sl_locale.h"

#include <libsysmodule.h>

using namespace paf;
using namespace sce;

static Settings *currentSettingsInstance = SCE_NULL;
sce::AppSettings *Settings::appSettings = SCE_NULL;

bool g_settingsLaunch = false;

SceVoid Launch();

Settings::Settings()
{
	if (currentSettingsInstance != SCE_NULL)
	{
		print("Error another settings instance exists! ABORT!\n");
		sceKernelExitProcess(0);
	}

	SceInt32 ret = 0;
    SceSize fileSize = 0;
    const char *mimeType = SCE_NULL;
	Plugin::InitParam pInit;
	AppSettings::InitParam sInit;

	sceSysmoduleLoadModuleInternal(SCE_SYSMODULE_INTERNAL_BXCE);
	sceSysmoduleLoadModuleInternal(SCE_SYSMODULE_INTERNAL_INI_FILE_PROCESSOR);
	sceSysmoduleLoadModuleInternal(SCE_SYSMODULE_INTERNAL_COMMON_GUI_DIALOG);

	pInit.pluginName = "app_settings_plugin";
	pInit.resourcePath = "vs0:vsh/common/app_settings_plugin.rco";
	pInit.scopeName = "__main__";

	pInit.pluginSetParamCB = AppSettings::PluginCreateCB;
	pInit.pluginInitCB = AppSettings::PluginInitCB;
	pInit.pluginStartCB = AppSettings::PluginStartCB;
	pInit.pluginStopCB = AppSettings::PluginStopCB;
	pInit.pluginExitCB = AppSettings::PluginExitCB;
	pInit.pluginPath = "vs0:vsh/common/app_settings.suprx";
	pInit.unk_58 = 0x96;

	Framework::GetInstance()->LoadPlugin(&pInit);

    sInit.xmlFile = g_appPlugin->resource->GetFile(Utils::GetHashById("file_sl_settings"), &fileSize, &mimeType);
	
    sInit.allocCB = sce_paf_malloc;
	sInit.freeCB = sce_paf_free;
	sInit.reallocCB = sce_paf_realloc;
	sInit.safeMemoryOffset = 0;
	sInit.safeMemorySize = 0x400;

	AppSettings::GetInstance(&sInit, &appSettings);

	sce_paf_memset(titleID, 0, sizeof(titleID));

	ret = -1;
	appSettings->GetInt("settings_version", &ret, 0);
#ifdef _DEBUG
    ret = -1; //Force reset settings on debug builds
#endif // _DEBUG	
    if (ret != d_settingsVersion) //Need to setup default values
	{
		ret = appSettings->Initialize();

		appSettings->SetInt("settings_version", d_settingsVersion);
		appSettings->SetString("budget", d_budget);
		appSettings->SetString("la_type", d_la_type);
		appSettings->SetString("titleid", d_titleID);
		appSettings->SetInt("memsize", d_memsize);
		appSettings->SetInt("phymemsize", d_phymemsize);
		appSettings->SetString("args", d_args);
		appSettings->SetString("exit_to", d_exit_to);
		appSettings->SetInt("port", d_port);

		appSettings->SetInt("use_cdlg", d_usecdlg);
		appSettings->SetInt("bg", d_bg);
		appSettings->SetInt("nonsuspendable", d_nonsuspendable);
		appSettings->SetInt("la_off", d_la_off);
		appSettings->SetInt("enable_budget", d_budget_enabled);
		appSettings->SetInt("enable_la_type", d_la_type_enabled);
		appSettings->SetInt("enable_memsize", d_memsize_enabled);
		appSettings->SetInt("enable_titleid", d_titleid_enabled);
		appSettings->SetInt("enable_phymemsize", d_phmemsize_enabled);
		appSettings->SetInt("enable_args", d_args_enabled);
		appSettings->SetInt("enable_exit_to", d_exit_to_enabled);
		appSettings->SetInt("enable_port", d_port_enabled);

		appSettings->SetInt("launchargs_enabled", d_launchArgs_enabled);
        appSettings->SetInt("device", d_device);
	}

	char buff[0x40];

	sce_paf_memset(buff, 0, sizeof(buff));
	appSettings->GetString("budget", buff, sizeof(buff), d_budget);
	budget = buff;

	sce_paf_memset(buff, 0, sizeof(buff));
	appSettings->GetString("la_type", buff, sizeof(buff), d_la_type);
	la_type = buff;

	sce_paf_memset(buff, 0, sizeof(buff));
	appSettings->GetString("args", buff, sizeof(buff), d_args);
	args = buff;

	appSettings->GetString("titleid", titleID, sizeof(titleID), d_titleID);
	appSettings->GetInt("memsize", &memsize, d_memsize);
	appSettings->GetInt("phymemsize", &phymemsize, d_phymemsize);
	appSettings->GetString("exit_to", exit_to, sizeof(exit_to), d_exit_to);
	appSettings->GetInt("port", &port, d_port);

	appSettings->GetInt("use_cdlg", &usecdlg, d_usecdlg);
	appSettings->GetInt("bg", &bg, d_bg);
	appSettings->GetInt("nonsuspendable", &nonsuspendable, d_nonsuspendable);
	appSettings->GetInt("la_off", &la_off, d_la_off);
	appSettings->GetInt("enable_budget", &budget_enabled, d_budget_enabled);
	appSettings->GetInt("enable_la_type", &la_type_enabled, d_la_type_enabled);
	appSettings->GetInt("enable_memsize", &memsize_enabled, d_memsize_enabled);
	appSettings->GetInt("enable_titleid", &titleid_enabled, d_titleid_enabled);
	appSettings->GetInt("enable_phymemsize", &phmemsize_enabled, d_phmemsize_enabled);
	appSettings->GetInt("enable_args", &args_enabled, d_args_enabled);
	appSettings->GetInt("enable_exit_to", &exit_to_enabled, d_exit_to_enabled);
	appSettings->GetInt("enable_port", &port_enabled, d_port_enabled);
    appSettings->GetInt("device", &device, d_device);

	appSettings->GetInt("launchargs_enabled", &launchArgs_enabled, d_launchArgs_enabled);

	currentSettingsInstance = this;
}

Settings::~Settings()
{
	print("Not allowed! ABORT!\n");
	sceKernelExitProcess(0);
}

Settings *Settings::GetInstance()
{
	return currentSettingsInstance;
}

sce::AppSettings *Settings::GetAppSettings()
{
    return appSettings;
}

SceVoid Settings::Open()
{
	AppSettings::InterfaceCallbacks ifCb;

	ifCb.listChangeCb = CBListChange;
	ifCb.listForwardChangeCb = CBListForwardChange;
	ifCb.listBackChangeCb = CBListBackChange;
	ifCb.isVisibleCb = CBIsVisible;
	ifCb.elemInitCb = CBElemInit;
	ifCb.elemAddCb = CBElemAdd;
	ifCb.valueChangeCb = CBValueChange;
	ifCb.valueChangeCb2 = CBValueChange2;
	ifCb.termCb = CBTerm;
	ifCb.getStringCb = CBGetString;
	ifCb.getTexCb = CBGetTex;

	Plugin *appSetPlug = paf::Plugin::Find("app_settings_plugin");
	AppSettings::Interface *appSetIf = (sce::AppSettings::Interface *)appSetPlug->GetInterface(1);

	appSetIf->Show(&ifCb);
}

SceVoid Settings::CBListChange(const char *elementId)
{

}

SceVoid Settings::CBListForwardChange(const char *elementId)
{

}

SceVoid Settings::CBListBackChange(const char *elementId)
{

}

SceInt32 Settings::CBIsVisible(const char *elementId, SceBool *pIsVisible)
{
    if(!g_settingsLaunch)
    {
        if(!sce_paf_strncmp(elementId, "list_device", 20))
            *pIsVisible = SCE_TRUE;
        else *pIsVisible = SCE_FALSE;
    }
    else
    {
        if(!sce_paf_strncmp(elementId, "list_device", 20))
            *pIsVisible = SCE_FALSE;
        else *pIsVisible = SCE_TRUE;
    }
	return SCE_OK;
}

SceInt32 Settings::CBElemInit(const char *elementId)
{
	return SCE_OK;
}

SceInt32 Settings::CBElemAdd(const char *elementId, paf::ui::Widget *widget)
{
	return SCE_OK;
}

SceInt32 Settings::CBValueChange(const char *elementId, const char *newValue)
{
	SceInt32 ret = SCE_OK;
	SceUInt32 elementHash = Utils::GetHashById(elementId);
	SceInt64 value = sce_paf_strtol(newValue, NULL, 10);
    string *text8 = SCE_NULL;
	wstring label16;
	string label8;
	print("Element with id: %s (0x%X) called CBValueChange! newValue = %s\n", elementId, elementHash, newValue);

	switch (elementHash)
	{
	case list_budget:
		GetInstance()->budget = newValue;
		break;

	case list_la_type:
		GetInstance()->la_type = newValue;
		break;

	case text_field_titleid:
		if (sce_paf_strlen(newValue) < 9)
		{
			ret = 0x80001002;
			break;
		}

		sce_paf_memset(GetInstance()->titleID, 0, sizeof(GetInstance()->titleID));
		sce_paf_strcpy(GetInstance()->titleID, newValue);

		break;

	case text_field_memsize:
		GetInstance()->memsize = value;
		break;

	case text_field_phymemsize:
		GetInstance()->phymemsize = value;
		break;

	case text_field_args:
		GetInstance()->args = newValue;
		break;

	case text_field_exit_to:
		if (sce_paf_strlen(newValue) < 9)
		{
			ret = 0x80001002;
			break;
		}

		sce_paf_memset(GetInstance()->exit_to, 0, sizeof(GetInstance()->exit_to));
		sce_paf_strcpy(GetInstance()->exit_to, newValue);

		break;

	case text_field_port:
		GetInstance()->port = value;
		break;

	case list_use_cdlg:
		GetInstance()->usecdlg = value;
		break;

	case list_bg:
		GetInstance()->bg = value;
		break;

	case list_nonsuspendable:
		GetInstance()->nonsuspendable = value;
		break;

	case list_la_off:
		GetInstance()->la_off = value;
		break;
	
	case toggle_budget:
		GetInstance()->budget_enabled = value;
		break;

	case toggle_la_type:
		GetInstance()->la_type_enabled = value;
		break;

	case toggle_memsize:
		GetInstance()->memsize_enabled = value;
		break;
	
	case toggle_titleid:
		GetInstance()->titleid_enabled = value;
		break;

	case toggle_phymemsize:
		GetInstance()->phmemsize_enabled = value;
		break;
	
	case toggle_args:
		GetInstance()->args_enabled = value;
		break;

	case toggle_exit_to:
		GetInstance()->exit_to_enabled = value;
		break;

	case toggle_port:
		GetInstance()->port_enabled = value;
		break;

	case toggle_launchargs_enabled:
		GetInstance()->launchArgs_enabled = value;
		break;

	case button_open:
        Launch();
		break;
    
    case list_device:
		text8 = ccc::UTF16toUTF8WithAlloc(Utils::GetStringPFromIDWithNum("msg_settings_device_", value));
		if (LocalFile::Exists(text8->c_str())) {
			GetInstance()->device = value;
            FileBrowser *page = (FileBrowser *)generic::Page::GetCurrentPage();
            page->SetPath("");
            page->Display();
		}
		else
			ret = SCE_ERROR_ERRNO_ENOENT;
        break;
        
	default:
		break;
	}

	return ret;
}

SceInt32 Settings::CBValueChange2(const char *elementId, const char *newValue)
{
	return SCE_OK;
}

SceVoid Settings::CBTerm()
{
    if(g_settingsLaunch)
    {
        FileBrowser *page = (FileBrowser *)generic::Page::GetCurrentPage();
        page->PathUp();
    }
}

wchar_t *Settings::CBGetString(const char *elementId)
{
	rco::Element searchParam;
	searchParam.hash = Utils::GetHashById(elementId);
    if(searchParam.hash == 0xed3441bf /*msg_settings*/)
    {
        if(g_settingsLaunch)
            searchParam.hash = msg_launch_title;
        else
            searchParam.hash = msg_options;
    }
    return g_appPlugin->GetWString(&searchParam);
}

SceInt32 Settings::CBGetTex(graph::Surface **tex, const char *elementId)
{
	return SCE_OK;
}

