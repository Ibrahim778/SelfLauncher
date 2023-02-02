#ifndef _SL_PAGE_H_
#define _SL_PAGE_H_

#include <paf.h>

namespace generic
{
    class Page
    {
    public:
        Page(SceInt32 hash, paf::Plugin::PageOpenParam openParam, paf::Plugin::PageCloseParam closeParam);
		virtual ~Page();

        SceUInt64 GetHash();

        paf::ui::Scene *root;

        // virtual void OnRedisplay();
        // virtual void OnDelete();

        static SceVoid DeleteCurrentPage();
        static generic::Page *GetCurrentPage();

        static SceVoid DefaultBackButtonCB(SceInt32 eventID, paf::ui::Widget *self, SceInt32 unk, ScePVoid pUserData);

    private:

        paf::Plugin::PageCloseParam closeParam;
        SceInt32 hash;
    };
}

#endif