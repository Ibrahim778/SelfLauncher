#ifndef  _SL_FILE_BROWSER_H_
#define  _SL_FILE_BROWSER_H_

#include <paf.h>
#include <kernel.h>

#include "page.h"

class FileBrowser : generic::Page
{
public:
    class ListViewCB : public paf::ui::ListView::ItemCallback
    {
    public:
        ListViewCB(FileBrowser *workPage)
        {
            this->workPage = workPage;
        }

        ~ListViewCB()
        {

        }

        paf::ui::ListItem *Create(Param *info);

        SceVoid Start(Param *info)
        {
            info->parent->PlayEffect(0.0f, paf::effect::EffectType_Fadein1);
        }

        FileBrowser *workPage;
    };

    FileBrowser(const char *initalDir = "");
    ~FileBrowser();

    static SceInt32 SortFiles(const ScePVoid p1, const ScePVoid p2);
    static SceVoid DirectoryCallback(SceInt32 id, paf::ui::Widget *self, SceInt32 unk, ScePVoid pUserData);
    static SceVoid FileCallback(SceInt32 id, paf::ui::Widget *self, SceInt32 unk, ScePVoid pUserData);
    static SceVoid OptionsCallback(SceInt32 id, paf::ui::Widget *self, SceInt32 unk, ScePVoid pUserData);
    static SceVoid BackCallback(SceInt32 id, paf::ui::Widget *self, SceInt32 unk, ScePVoid pUserData);

    //Scan the current directory and display
    SceVoid Display(); 

    //Set the current path
    SceVoid SetPath(const char *path);
    //Append to the end of the path
    SceVoid AppendPath(const char *path);
    //Back up one level in the path
    SceVoid PathUp();
    //Get path
    paf::string GetPath();

private:
    paf::string path;
    paf::vector<paf::DirEnt> entries;

    paf::ui::Text *pathText;
    paf::ui::ListView *entryListView;
    paf::ui::CornerButton *optionsButton;
    paf::ui::CornerButton *backButton;
};

#endif