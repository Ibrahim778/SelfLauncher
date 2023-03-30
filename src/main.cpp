#include <kernel.h>
#include <paf.h>
#include <apputil.h>
#include <taihen.h>

#include "settings.h"
#include "print.h"
#include "utils.h"
#include "pages/file_browser.h"
#include "sl_plugin.h"
#include "SLKernel.h"

#define WIDE2(x) L##x
#define WIDE(x) WIDE2(x)

extern "C" {
    const char			sceUserMainThreadName[] = "SL_MAIN";
    const int			sceUserMainThreadPriority = SCE_KERNEL_DEFAULT_PRIORITY_USER;
    const unsigned int	sceUserMainThreadStackSize = SCE_KERNEL_THREAD_STACK_SIZE_DEFAULT_USER_MAIN;
}

using namespace paf;

Plugin *g_appPlugin = SCE_NULL;
wchar_t *g_versionInfo = SCE_NULL;

graph::Surface *g_unkIcon = SCE_NULL;
graph::Surface *g_appIcon = SCE_NULL;
graph::Surface *g_dirIcon = SCE_NULL;

SceInt32 ExitThread(SceSize args, ScePVoid pUserData)
{
    sceKernelDelayThread(10 * 1000); // Allow the launch function to do its thing
    sceKernelExitProcess(0); //'ight see ya
    return sceKernelExitDeleteThread(0);
}

SceVoid LaunchSelfFromFile(SLKernelLaunchParam *lParam)
{
    auto f = LocalFile::Open(SLKernelParamPath, SCE_O_WRONLY | SCE_O_CREAT | SCE_O_TRUNC, 0666, SCE_NULL);
    f.get()->Write(lParam, sizeof(SLKernelLaunchParam));
    
    SceUID modid = SCE_UID_INVALID_UID;
    modid = taiLoadStartKernelModule("ux0:app/SELF00001/module/SLKernel.skprx", 0, SCE_NULL, 0);
    if(modid < 0)
    {
        print("[Error] Failed to load SLKernel: 0x%X\n", modid);
        return;
    }

    taiStopUnloadKernelModule(modid, 0, SCE_NULL, 0, SCE_NULL, SCE_NULL);
}

SceVoid Launch()
{
    FileBrowser *workBrowser = (FileBrowser *)generic::Page::GetCurrentPage();
    
    Settings *settingInstance = Settings::GetInstance();
	SLKernelLaunchParam lParam;

    print("argsEnabled = %d\n", settingInstance->args_enabled);
    
    sce_paf_memset(&lParam, 0, sizeof(lParam));
    sce_paf_strncpy(lParam.path, workBrowser->GetPath().c_str(), sizeof(lParam.path));

	if (settingInstance->launchArgs_enabled == 1)
	{
		print("Args enabled!\n");
		//Parse arguments here
		char *args = lParam.args;

		sce_paf_memset(args, 0, sizeof(args));

		if (settingInstance->bg)
			sce_paf_strcat(args, "-bg~");
		if (settingInstance->budget_enabled == 1)
		{
			print("Budget: %s\n", settingInstance->budget);
			char buff[0x20];
			sce_paf_memset(buff, 0, sizeof(buff));

			sce_paf_snprintf(buff, sizeof(buff), "-budget %s~", settingInstance->budget);

			sce_paf_strcat(args, buff);
		}

		if (settingInstance->memsize_enabled == 1)
		{
			print("Memsize: %d\n", settingInstance->memsize);
			char buff[0x40];
			sce_paf_memset(buff, 0, sizeof(buff));

			sce_paf_snprintf(buff, sizeof(buff), "-memsize %d~", settingInstance->memsize);

			sce_paf_strcat(args, buff);
		}

		if (settingInstance->la_type_enabled == 1)
		{
			print("LA_Type: %s\n", settingInstance->la_type);
			char buff[0x40];
			sce_paf_memset(buff, 0, sizeof(buff));

			sce_paf_snprintf(buff, sizeof(buff), "-la_type %s~", settingInstance->la_type);

			sce_paf_strcat(args, buff);
		}

		if (settingInstance->nonsuspendable)
			sce_paf_strcat(args, "-nonsuspendable~");
		
		if (settingInstance->titleid_enabled)
		{
			char buff[0x40];
			sce_paf_memset(buff, 0, sizeof(buff));

			sce_paf_snprintf(buff, sizeof(buff), "-titeid %s~", settingInstance->titleID);

			sce_paf_strcat(args, buff);
		}

		if (settingInstance->phmemsize_enabled)
		{
			print("Memsize: %d\n", settingInstance->memsize);
			char buff[0x40];
			sce_paf_memset(buff, 0, sizeof(buff));

			sce_paf_snprintf(buff, sizeof(buff), "-phcontmemsize %d~", settingInstance->phymemsize);

			sce_paf_strcat(args, buff);
		}
		
		if (settingInstance->la_off)
			sce_paf_strcat(args, "-livearea_off~");

		if (settingInstance->exit_to_enabled)
		{
			char buff[0x40];
			sce_paf_memset(buff, 0, sizeof(buff));

			sce_paf_snprintf(buff, sizeof(buff), "-exit_to %s~", settingInstance->exit_to);

			sce_paf_strcat(args, buff);
		}

		if (settingInstance->args_enabled)
		{
			char buff[0x100];
			sce_paf_memset(buff, 0, sizeof(buff));

			sce_paf_snprintf(buff, sizeof(buff), "-param %s~", settingInstance->args);

			sce_paf_strcat(args, buff);
		}

		if (settingInstance->usecdlg)
			sce_paf_strcat(args, "-usecdlg~");

		if (settingInstance->port_enabled)
		{
			char buff[0x40];
			sce_paf_memset(buff, 0, sizeof(buff));

			sce_paf_snprintf(buff, sizeof(buff), "-port %d~", settingInstance->port);

			sce_paf_strcat(args, buff);
		}

		print("Args: %s\n", args);
		lParam.argLength = sce_paf_strlen(args) + 1;
		for (int i = 0; i < sizeof(args) && args[i] != 0; i++)
        {
			if (args[i] == '~') args[i] = 0;
        }
		
        print("%s\n", workBrowser->GetPath().c_str());
	}

    LaunchSelfFromFile(&lParam);
    sceKernelStartThread(
        sceKernelCreateThread(
            "SL_exit_thread", 
            ExitThread, 
            SCE_KERNEL_HIGHEST_PRIORITY_USER + 10, 
            SCE_KERNEL_4KiB, 
            0, 
            SCE_KERNEL_THREAD_CPU_AFFINITY_MASK_DEFAULT, 
            SCE_NULL), 
    0, SCE_NULL); //Exit thread to kill the app
    
}

SceVoid onPluginReady(Plugin *plugin)
{
    if(plugin == SCE_NULL)
    {
        print("[MAIN_SL] Error Plugin load failed!\n");
        return;
    }

    g_appPlugin = plugin;

    //Thanks Graphene
    auto infoString = new wstring;

#ifdef _DEBUG
    *infoString = L"Type: Private Beta\n";
#else
    *infoString = L"Type: Public Release\n";
#endif

    *infoString += L"Date: " WIDE(__DATE__) L"\n";
    *infoString += L"Version: 1.0";

    print("%ls\n", infoString->data());

    g_versionInfo = (wchar_t *)infoString->data();

    rco::Element e; 
    e.hash = tex_icon_launch;
    g_appPlugin->GetTexture(&g_appIcon, g_appPlugin, &e);
    g_appIcon->AddRef(); //Prevent Deletion

	e.hash = tex_icon_unk;
	Plugin::GetTexture(&g_unkIcon, g_appPlugin, &e);
    g_unkIcon->AddRef(); //Prevent Deletion

	e.hash = tex_icon_dir;
	Plugin::GetTexture(&g_dirIcon, g_appPlugin, &e);
    g_dirIcon->AddRef(); //Prevent Deletion

    new Settings();
    print("%s\n", Utils::GetPreviousPath().c_str());
    new FileBrowser(Utils::GetPreviousPath().c_str());
}

int main()
{
#if defined(SCE_PAF_TOOL_PRX) && defined(_DEBUG) && !defined(__INTELLISENSE__)
    SCE_PAF_AUTO_TEST_SET_EXTRA_TTY(sceIoOpen("tty0:", SCE_O_WRONLY, 0));
#endif

    Framework::InitParam fwParam;
    fwParam.LoadDefaultParams();
    fwParam.applicationMode = Framework::ApplicationMode::Mode_Application;
    
    fwParam.defaultSurfacePoolSize = 6 * 1024 * 1024;
    fwParam.textSurfaceCacheSize = 2621440; //2.5MB

    Framework *fw = new Framework(fwParam);

    fw->LoadCommonResourceSync();

    SceAppUtilInitParam init;
    SceAppUtilBootParam boot;

    //Can use sce_paf_... because paf is preloaded
    sce_paf_memset(&init, 0, sizeof(SceAppUtilInitParam));
    sce_paf_memset(&boot, 0, sizeof(SceAppUtilBootParam));
    
    sceAppUtilInit(&init, &boot);

    Plugin::InitParam piParam;

    piParam.pluginName = "sl_plugin";
    piParam.resourcePath = "app0:resource/sl_plugin.rco";
    piParam.scopeName = "__main__";
#if defined(_DEBUG) && defined(SCE_PAF_TOOL_PRX)
    piParam.pluginFlags = Plugin::InitParam::PluginFlag_UseRcdDebug;
#endif
    piParam.pluginStartCB = onPluginReady;

    fw->LoadPluginAsync(piParam);

    fw->Run();

    return sceKernelExitProcess(0);
}