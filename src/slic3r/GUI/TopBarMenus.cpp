#include "TopBarMenus.hpp"
#include "TopBar.hpp"

#include "GUI_App.hpp"
#include "Plater.hpp"
#include "UserAccount.hpp"

#include "I18N.hpp"

using namespace Slic3r::GUI;

TopBarMenus::TopBarMenus()
{
    CreateAccountMenu();
    ApplyWorkspacesMenu();
    UpdateAccountMenu();

    BindEvtClose();
}

void TopBarMenus::AppendMenuItem(wxMenu* menu, const wxString& title)
{
    append_submenu(&main, menu, wxID_ANY, title, "cog");
}

void TopBarMenus::AppendMenuSeparaorItem()
{
    main.AppendSeparator();
}

wxString TopBarMenus::get_workspace_name(const int mode)
{
    return  mode == Slic3r::ConfigOptionMode::comSimple   ? _L("Beginner mode") :
            mode == Slic3r::ConfigOptionMode::comAdvanced ? _L("Normal mode")  : _L("Expert mode");
}

void TopBarMenus::ApplyWorkspacesMenu()
{
    wxMenuItemList& items = workspaces.GetMenuItems();
    if (!items.IsEmpty()) {
        for (int id = int(workspaces.GetMenuItemCount()) - 1; id >= 0; id--)
            workspaces.Destroy(items[id]);
    }

    for (const Slic3r::ConfigOptionMode& mode : { Slic3r::ConfigOptionMode::comSimple,
                                                  Slic3r::ConfigOptionMode::comAdvanced,
                                                  Slic3r::ConfigOptionMode::comExpert }) {
        const wxString label = get_workspace_name(mode);
        append_menu_item(&workspaces, wxID_ANY, label, label,
            [mode](wxCommandEvent&) {
                if (wxGetApp().get_mode() != mode)
                    wxGetApp().save_mode(mode);
            }, get_bmp_bundle("mode", 16, -1, wxGetApp().get_mode_btn_color(mode)));

        if (mode < Slic3r::ConfigOptionMode::comExpert)
            workspaces.AppendSeparator();
    }
}

void TopBarMenus::CreateAccountMenu()
{
    remember_me_item_id = wxWindow::NewControlId();
    append_menu_check_item(&account, remember_me_item_id, _L("Remember me"), "" , 
        [](wxCommandEvent&) { wxGetApp().plater()->get_user_account()->toggle_remember_session(); } , nullptr);

    m_login_item = append_menu_item(&account, wxID_ANY, "", "",
        [](wxCommandEvent&) {
            auto user_account = wxGetApp().plater()->get_user_account();
            if (user_account->is_logged())
                user_account->do_logout();
            else
                user_account->do_login();
        }, get_bmp_bundle("login", 16));
}

void TopBarMenus::UpdateAccountMenu(Slic3r::GUI::UserAccount* user_account)
{
    bool is_logged = user_account && user_account->is_logged();
    if (m_login_item) {
        m_login_item->SetItemLabel(is_logged ? _L("Prusa Account Log out") : _L("Prusa Account Log in"));
        m_login_item->SetBitmap(is_logged ? *get_bmp_bundle("logout", 16) : *get_bmp_bundle("login", 16));
    }
}

void TopBarMenus::Popup(TopBarItemsCtrl* popup_ctrl, wxMenu* menu, wxPoint pos)
{
    m_popup_ctrl = popup_ctrl;
    m_popup_ctrl->PopupMenu(menu, pos);
}

void TopBarMenus::BindEvtClose()
{
    auto close_fn = [this]() {
        if (m_popup_ctrl)
            m_popup_ctrl->UnselectPopupButtons();
        m_popup_ctrl = nullptr;
    };

    main.        Bind(wxEVT_MENU_CLOSE, [close_fn](wxMenuEvent&) { close_fn(); });
    workspaces.  Bind(wxEVT_MENU_CLOSE, [close_fn](wxMenuEvent&) { close_fn(); });
    account.     Bind(wxEVT_MENU_CLOSE, [close_fn](wxMenuEvent&) { close_fn(); });
}