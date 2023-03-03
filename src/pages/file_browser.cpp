#include <kernel.h>
#include <paf.h>

#include "pages/file_browser.h"
#include "sl_plugin.h"
#include "utils.h"
#include "settings.h"
#include "print.h"
#include "common.h"

using namespace paf;

FileBrowser::FileBrowser(const char *intialPath):generic::Page(page_file_browser, Plugin::PageOpenParam(), Plugin::PageCloseParam())
{
    pathText = Utils::GetChildByHash<ui::Text>(root, text_path);
    entryListView = Utils::GetChildByHash<ui::ListView>(root, list_view_entries);
    optionsButton = Utils::GetChildByHash<ui::CornerButton>(root, options_button);
    backButton = Utils::GetChildByHash<ui::CornerButton>(root, back_button);

    ui::EventCallback *optionsCallback = new ui::EventCallback();
    optionsCallback->eventHandler = OptionsCallback;

    optionsButton->RegisterEventCallback(ui::EventMain_Decide, optionsCallback);

    ui::EventCallback *backCallback = new ui::EventCallback();
    backCallback->eventHandler = BackCallback;
    backCallback->pUserData = this;

    backButton->RegisterEventCallback(ui::EventMain_Decide, backCallback);

    //Setup the list view
	entryListView->RegisterItemCallback(new ListViewCB(this));

	entryListView->SetConfigurationType(0, ui::ListView::ConfigurationType_Simple);
    entryListView->SetSegmentEnable(0, 1);
    entryListView->SetCellSize(0, &paf::Vector4(960.0f, 85.0f));

    path = intialPath;

    Display();
}

SceVoid FileBrowser::SetPath(const char *path)
{
    this->path = path;
}

SceVoid FileBrowser::AppendPath(const char *path)
{
    this->path += path;
}

SceVoid FileBrowser::PathUp()
{
    //Find /
	int slashLocation = path.length() - 2;
	for (; slashLocation > 0 && path.c_str()[slashLocation] != '/'; slashLocation--);

	//Copy all characters before it
	char *newPath = new char[slashLocation];
    print("0x%X 0x%X %d\n", newPath, path.c_str(), slashLocation);
    sce_paf_memset(newPath, 0, slashLocation);
	sce_paf_strncpy(newPath, path.c_str(), slashLocation);
	newPath[slashLocation] = 0;
	path = newPath;
	delete[] newPath;
    if(path.size() > 0)
        path += "/";
}

SceVoid FileBrowser::DirectoryCallback(SceInt32 id, paf::ui::Widget *self, SceInt32 unk, ScePVoid pUserData)
{
    FileBrowser *workBrowser = (FileBrowser *)pUserData;
    DirEnt *entry = &workBrowser->entries[self->elem.hash];
    
    workBrowser->AppendPath(entry->name.c_str());
    workBrowser->AppendPath("/");
    workBrowser->Display();
}

SceVoid FileBrowser::FileCallback(SceInt32 id, paf::ui::Widget *self, SceInt32 unk, ScePVoid pUserData)
{
    FileBrowser *workBrowser = (FileBrowser *)pUserData;
    DirEnt *entry = &workBrowser->entries[self->elem.hash];

    g_settingsLaunch = true;
    workBrowser->AppendPath(entry->name.c_str());
    Settings::GetInstance()->Open();
}

SceVoid FileBrowser::OptionsCallback(SceInt32 id, paf::ui::Widget *self, SceInt32 unk, ScePVoid pUserData)
{
    g_settingsLaunch = false;
    Settings::GetInstance()->Open();
}

SceVoid FileBrowser::BackCallback(SceInt32 id, paf::ui::Widget *self, SceInt32 unk, ScePVoid pUserData)
{
    FileBrowser *workBrowser = (FileBrowser *)pUserData;
    workBrowser->PathUp();
    workBrowser->Display();
}

string FileBrowser::GetPath()
{
    string out;
    
    common::Utf16ToUtf8(Utils::GetStringPFromIDWithNum("msg_settings_device_", Settings::GetInstance()->device), &out);
    out += path;
    return out;
}

ui::ListItem *FileBrowser::ListViewCB::Create(FileBrowser::ListViewCB::Param *param)
{
    rco::Element                searchParam;
    Plugin::TemplateOpenParam   tOpen;

    ui::ListItem        *item           = SCE_NULL;
    ui::ImageButton     *button         = SCE_NULL;
    graph::Surface     **icon           = SCE_NULL;

    ui::EventCallback   *callback       = new ui::EventCallback();
    callback->pUserData = workPage;
    
    ui::Widget *targetRoot = param->parent;
    DirEnt &workEntry = workPage->entries[param->cellIndex];

    wstring titleLabel;
    
    if(workEntry.type == DirEnt::Type::Type_Dir)
    {
        icon = &g_dirIcon;
        callback->eventHandler = DirectoryCallback;
    }
    else
    {
        // //Determine File Extension
                    
        // //Find * to dot
        const char *dot = sce_paf_strchr(workEntry.name.c_str(), '.');

        if(dot == workEntry.name.c_str() || !dot) //No Extension
            icon = &g_unkIcon;

        else if (
            sce_paf_strcmp(dot, ".self") == 0 ||
            sce_paf_strcmp(dot, ".bin") == 0 ||
            sce_paf_strcmp(dot, ".out") == 0
            )
            icon = &g_appIcon;
            
        else
            icon = &g_unkIcon;

        callback->eventHandler = FileCallback;
    }

    searchParam.hash = template_list_item_entry;
    g_appPlugin->TemplateOpen(targetRoot, &searchParam, &tOpen);
    
    item = (ui::ListItem *)targetRoot->GetChild(targetRoot->childNum - 1);
    button = Utils::GetChildByHash<ui::ImageButton>(item, button_entry);

    button->elem.hash = param->cellIndex;
    
    common::Utf8ToUtf16(workEntry.name, &titleLabel);
    button->SetLabel(&titleLabel);

    button->SetSurfaceBase(icon);
    button->RegisterEventCallback(ui::EventMain_Decide, callback);

    return item;
}

SceInt32 FileBrowser::SortFiles(const ScePVoid p1, const ScePVoid p2)
{
	DirEnt *entryA = (DirEnt *)p1;
	DirEnt *entryB = (DirEnt *)p2;

	if (entryA->type == DirEnt::Type_Dir && entryB->type != DirEnt::Type_Dir)
		return -1; //Dont swap
	else if ((entryA->type != DirEnt::Type_Dir) && (entryB->type == DirEnt::Type_Dir))
		return 1; //Swap
	else 
		return sce_paf_strcasecmp(entryA->name.c_str(), entryB->name.c_str());
}

SceVoid FileBrowser::Display()
{   
    if(entryListView->GetCellNum(0) > 0)
        entryListView->RemoveItem(0, 0, entryListView->GetCellNum(0)); //Delete all items 
    entries.clear();

    string device;
    common::Utf16ToUtf8(Utils::GetStringPFromIDWithNum("msg_settings_device_", Settings::GetInstance()->device), &device);
    
    string fullPath = device + path;
    print("%s\n", fullPath.c_str());
    wstring labelStr;
    common::Utf8ToUtf16(fullPath, &labelStr);
    pathText->SetLabel(&labelStr);

    paf::Dir dir;
    if(dir.Open(fullPath.c_str()) == SCE_OK)
    {
        DirEnt entry;
        
        while(dir.Read(&entry) >= 0)
            entries.push_back(entry);

        dir.Close();

        // I am very surprised this works
        sce_paf_qsort(entries.data(), entries.size(), sizeof(DirEnt), (int(*)(const void *, const void *))SortFiles);
        
        entryListView->AddItem(0, 0, entries.size());
    } 
    else 
    {

    }

    int slashCount = 0;
    for(int i = 0; i < path.size() + 1; i++)
        if(path.c_str()[i] == '/')
            slashCount++;

    if(slashCount >= 1)
        backButton->PlayEffect(0, effect::EffectType_Reset);
    else 
        backButton->PlayEffectReverse(0, effect::EffectType_Reset);
    
    Utils::SetPreviousPath(path.c_str());
}

FileBrowser::~FileBrowser()
{

}