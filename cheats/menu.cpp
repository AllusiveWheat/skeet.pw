// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include <unordered_map>
#include "menu.h"
#include "../ImGui/code_editor.h"
#include "../constchars.h"
#include "../cheats/misc/logs.h"

#define ALPHA (ImGuiColorEditFlags_AlphaPreview | ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel | ImGuiColorEditFlags_AlphaBar| ImGuiColorEditFlags_InputRGB | ImGuiColorEditFlags_Float)
#define NOALPHA (ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel | ImGuiColorEditFlags_NoAlpha | ImGuiColorEditFlags_InputRGB | ImGuiColorEditFlags_Float)

std::vector <std::string> files;
std::vector <std::string> scripts;
std::string editing_script;

auto selected_script = 0;
auto loaded_editing_script = false;
static char search_skins[64] = "\0";

static auto menu_setupped = false;

IDirect3DTexture9* all_skins[36];

std::string get_wep(int id, int custom_index = -1, bool knife = true)
{
	if (custom_index > -1)
	{
		if (knife)
		{
			switch (custom_index)
			{
			case 0: return crypt_str("weapon_knife");
			case 1: return crypt_str("weapon_bayonet");
			case 2: return crypt_str("weapon_knife_css");
			case 3: return crypt_str("weapon_knife_skeleton");
			case 4: return crypt_str("weapon_knife_outdoor");
			case 5: return crypt_str("weapon_knife_cord");
			case 6: return crypt_str("weapon_knife_canis");
			case 7: return crypt_str("weapon_knife_flip");
			case 8: return crypt_str("weapon_knife_gut");
			case 9: return crypt_str("weapon_knife_karambit");
			case 10: return crypt_str("weapon_knife_m9_bayonet");
			case 11: return crypt_str("weapon_knife_tactical");
			case 12: return crypt_str("weapon_knife_falchion");
			case 13: return crypt_str("weapon_knife_survival_bowie");
			case 14: return crypt_str("weapon_knife_butterfly");
			case 15: return crypt_str("weapon_knife_push");
			case 16: return crypt_str("weapon_knife_ursus");
			case 17: return crypt_str("weapon_knife_gypsy_jackknife");
			case 18: return crypt_str("weapon_knife_stiletto");
			case 19: return crypt_str("weapon_knife_widowmaker");
			}
		}
		else
		{
			switch (custom_index)
			{
			case 0: return crypt_str("ct_gloves"); 
			case 1: return crypt_str("studded_bloodhound_gloves");
			case 2: return crypt_str("t_gloves");
			case 3: return crypt_str("ct_gloves");
			case 4: return crypt_str("sporty_gloves");
			case 5: return crypt_str("slick_gloves");
			case 6: return crypt_str("leather_handwraps");
			case 7: return crypt_str("motorcycle_gloves");
			case 8: return crypt_str("specialist_gloves");
			case 9: return crypt_str("studded_hydra_gloves");
			}
		}
	}
	else
	{
		switch (id)
		{
		case 0: return crypt_str("knife");
		case 1: return crypt_str("gloves");
		case 2: return crypt_str("weapon_ak47");
		case 3: return crypt_str("weapon_aug");
		case 4: return crypt_str("weapon_awp");
		case 5: return crypt_str("weapon_cz75a");
		case 6: return crypt_str("weapon_deagle");
		case 7: return crypt_str("weapon_elite");
		case 8: return crypt_str("weapon_famas");
		case 9: return crypt_str("weapon_fiveseven");
		case 10: return crypt_str("weapon_g3sg1");
		case 11: return crypt_str("weapon_galilar");
		case 12: return crypt_str("weapon_glock");
		case 13: return crypt_str("weapon_m249");
		case 14: return crypt_str("weapon_m4a1_silencer");
		case 15: return crypt_str("weapon_m4a1");
		case 16: return crypt_str("weapon_mac10");
		case 17: return crypt_str("weapon_mag7");
		case 18: return crypt_str("weapon_mp5sd");
		case 19: return crypt_str("weapon_mp7");
		case 20: return crypt_str("weapon_mp9");
		case 21: return crypt_str("weapon_negev");
		case 22: return crypt_str("weapon_nova");
		case 23: return crypt_str("weapon_hkp2000");
		case 24: return crypt_str("weapon_p250");
		case 25: return crypt_str("weapon_p90");
		case 26: return crypt_str("weapon_bizon");
		case 27: return crypt_str("weapon_revolver");
		case 28: return crypt_str("weapon_sawedoff");
		case 29: return crypt_str("weapon_scar20");
		case 30: return crypt_str("weapon_ssg08");
		case 31: return crypt_str("weapon_sg556");
		case 32: return crypt_str("weapon_tec9");
		case 33: return crypt_str("weapon_ump45");
		case 34: return crypt_str("weapon_usp_silencer");
		case 35: return crypt_str("weapon_xm1014");
		default: return crypt_str("unknown");
		}
	}
}

IDirect3DTexture9* get_skin_preview(const char* weapon_name, const std::string& skin_name, IDirect3DDevice9* device)
{
	IDirect3DTexture9* skin_image = nullptr;
	std::string vpk_path;

	if (strcmp(weapon_name, crypt_str("unknown")) && strcmp(weapon_name, crypt_str("knife")) && strcmp(weapon_name, crypt_str("gloves")))
	{
		if (skin_name.empty() || skin_name == crypt_str("default"))
			vpk_path = crypt_str("resource/flash/econ/weapons/base_weapons/") + std::string(weapon_name) + crypt_str(".png");
		else
			vpk_path = crypt_str("resource/flash/econ/default_generated/") + std::string(weapon_name) + crypt_str("_") + std::string(skin_name) + crypt_str("_light_large.png");
	}
	else
	{
		if (!strcmp(weapon_name, crypt_str("knife")))
			vpk_path = crypt_str("resource/flash/econ/weapons/base_weapons/weapon_knife.png");
		else if (!strcmp(weapon_name, crypt_str("gloves")))
			vpk_path = crypt_str("resource/flash/econ/weapons/base_weapons/ct_gloves.png");
		else if (!strcmp(weapon_name, crypt_str("unknown")))
			vpk_path = crypt_str("resource/flash/econ/weapons/base_weapons/weapon_snowball.png");

	}
	const auto handle = m_basefilesys()->Open(vpk_path.c_str(), crypt_str("r"), crypt_str("GAME"));
	if (handle)
	{
		int file_len = m_basefilesys()->Size(handle);
		char* image = new char[file_len];

		m_basefilesys()->Read(image, file_len, handle);
		m_basefilesys()->Close(handle);

		D3DXCreateTextureFromFileInMemory(device, image, file_len, &skin_image);
		delete[] image;
	}

	if (!skin_image)
	{
		std::string vpk_path;

		if (strstr(weapon_name, crypt_str("bloodhound")) != NULL || strstr(weapon_name, crypt_str("hydra")) != NULL)
			vpk_path = crypt_str("resource/flash/econ/weapons/base_weapons/ct_gloves.png");
		else
			vpk_path = crypt_str("resource/flash/econ/weapons/base_weapons/") + std::string(weapon_name) + crypt_str(".png");

		const auto handle = m_basefilesys()->Open(vpk_path.c_str(), crypt_str("r"), crypt_str("GAME"));

		if (handle)
		{
			int file_len = m_basefilesys()->Size(handle);
			char* image = new char[file_len];

			m_basefilesys()->Read(image, file_len, handle);
			m_basefilesys()->Close(handle);

			D3DXCreateTextureFromFileInMemory(device, image, file_len, &skin_image);
			delete[] image;
		}
	}

	return skin_image;
}

void c_menu::menu_setup(ImGuiStyle& style)
{
	ImGui::StyleColorsClassic(); // colors setup.
	ImGui::SetNextWindowSize(ImVec2(width, height), ImGuiCond_Once); // window pos setup.
	ImGui::SetNextWindowBgAlpha(min(style.Alpha, 0.94f)); // window bg alpha setup.

	styles.WindowPadding = style.WindowPadding;
	styles.WindowRounding = style.WindowRounding;
	styles.WindowMinSize = style.WindowMinSize;
	styles.ChildRounding = style.ChildRounding;
	styles.PopupRounding = style.PopupRounding;
	styles.FramePadding = style.FramePadding;
	styles.FrameRounding = style.FrameRounding;
	styles.ItemSpacing = style.ItemSpacing;
	styles.ItemInnerSpacing = style.ItemInnerSpacing;
	styles.TouchExtraPadding = style.TouchExtraPadding;
	styles.IndentSpacing = style.IndentSpacing;
	styles.ColumnsMinSpacing = style.ColumnsMinSpacing;
	styles.ScrollbarSize = style.ScrollbarSize;
	styles.ScrollbarRounding = style.ScrollbarRounding;
	styles.GrabMinSize = style.GrabMinSize;
	styles.GrabRounding = style.GrabRounding;
	styles.TabRounding = style.TabRounding;
	styles.TabMinWidthForUnselectedCloseButton = style.TabMinWidthForUnselectedCloseButton;
	styles.DisplayWindowPadding = style.DisplayWindowPadding;
	styles.DisplaySafeAreaPadding = style.DisplaySafeAreaPadding;
	styles.MouseCursorScale = style.MouseCursorScale;

	// setup skins preview
	for (auto i = 0; i < g_cfg.skins.skinChanger.size(); i++)
		if (!all_skins[i])
			all_skins[i] = get_skin_preview(get_wep(i, (i == 0 || i == 1) ? g_cfg.skins.skinChanger.at(i).definition_override_vector_index : -1, i == 0).c_str(), g_cfg.skins.skinChanger.at(i).skin_name, device); //-V810

	menu_setupped = true; // we dont want to setup menu again
}

// resize current style sizes
void c_menu::dpi_resize(float scale_factor, ImGuiStyle& style)
{
	style.WindowPadding = (styles.WindowPadding * scale_factor);
	style.WindowRounding = (styles.WindowRounding * scale_factor);
	style.WindowMinSize = (styles.WindowMinSize * scale_factor);
	style.ChildRounding = (styles.ChildRounding * scale_factor);
	style.PopupRounding = (styles.PopupRounding * scale_factor);
	style.FramePadding = (styles.FramePadding * scale_factor);
	style.FrameRounding = (styles.FrameRounding * scale_factor);
	style.ItemSpacing = (styles.ItemSpacing * scale_factor);
	style.ItemInnerSpacing = (styles.ItemInnerSpacing * scale_factor);
	style.TouchExtraPadding = (styles.TouchExtraPadding * scale_factor);
	style.IndentSpacing = (styles.IndentSpacing * scale_factor);
	style.ColumnsMinSpacing = (styles.ColumnsMinSpacing * scale_factor);
	style.ScrollbarSize = (styles.ScrollbarSize * scale_factor);
	style.ScrollbarRounding = (styles.ScrollbarRounding * scale_factor);
	style.GrabMinSize = (styles.GrabMinSize * scale_factor);
	style.GrabRounding = (styles.GrabRounding * scale_factor);
	style.TabRounding = (styles.TabRounding * scale_factor);
	if (styles.TabMinWidthForUnselectedCloseButton != FLT_MAX)
		style.TabMinWidthForUnselectedCloseButton = (styles.TabMinWidthForUnselectedCloseButton * scale_factor);
	style.DisplayWindowPadding = (styles.DisplayWindowPadding * scale_factor);
	style.DisplaySafeAreaPadding = (styles.DisplaySafeAreaPadding * scale_factor);
	style.MouseCursorScale = (styles.MouseCursorScale * scale_factor);
}


std::string get_config_dir()
{
	std::string folder;
	static TCHAR path[MAX_PATH];

	if (SUCCEEDED(SHGetFolderPath(NULL, 0x001a, NULL, NULL, path)))
		folder = std::string(path) + crypt_str("\\skeet.pw\\Configs\\");

	return folder;
}

void load_config()
{
	if (cfg_manager->files.empty())
		return;

	cfg_manager->load(cfg_manager->files.at(g_cfg.selected_config), false);
	c_lua::get().unload_all_scripts();

	for (auto& script : g_cfg.scripts.scripts)
		c_lua::get().load_script(c_lua::get().get_script_id(script));

	scripts = c_lua::get().scripts;

	if (selected_script >= scripts.size())
		selected_script = scripts.size() - 1;

	for (auto& current : scripts)
	{
		if (current.size() >= 5 && current.at(current.size() - 1) == 'c')
			current.erase(current.size() - 5, 5);
		else if (current.size() >= 4)
			current.erase(current.size() - 4, 4);
	}

	for (auto i = 0; i < g_cfg.skins.skinChanger.size(); ++i)
		all_skins[i] = nullptr;

	g_cfg.scripts.scripts.clear();

	cfg_manager->load(cfg_manager->files.at(g_cfg.selected_config), true);
	cfg_manager->config_files();

	eventlogs::get().add(crypt_str("Loaded ") + files.at(g_cfg.selected_config) + crypt_str("pw config"), false);
}

void save_config()
{
	if (cfg_manager->files.empty())
		return;

	g_cfg.scripts.scripts.clear();

	for (auto i = 0; i < c_lua::get().scripts.size(); ++i)
	{
		auto script = c_lua::get().scripts.at(i);

		if (c_lua::get().loaded.at(i))
			g_cfg.scripts.scripts.emplace_back(script);
	}

	cfg_manager->save(cfg_manager->files.at(g_cfg.selected_config));
	cfg_manager->config_files();

	eventlogs::get().add(crypt_str("Saved ") + files.at(g_cfg.selected_config) + crypt_str(" config"), false);
}

void remove_config()
{
	if (cfg_manager->files.empty())
		return;

	eventlogs::get().add(crypt_str("Removed ") + files.at(g_cfg.selected_config) + crypt_str(" config"), false);

	cfg_manager->remove(cfg_manager->files.at(g_cfg.selected_config));
	cfg_manager->config_files();

	files = cfg_manager->files;

	if (g_cfg.selected_config >= files.size())
		g_cfg.selected_config = files.size() - 1;

	for (auto& current : files)
		if (current.size() > 2)
			current.erase(current.size() - 3, 3);
}

void add_config()
{
	auto empty = true;

	for (auto current : g_cfg.new_config_name)
	{
		if (current != ' ')
		{
			empty = false;
			break;
		}
	}

	if (empty)
		g_cfg.new_config_name = crypt_str("config");

	eventlogs::get().add(crypt_str("Added ") + g_cfg.new_config_name + crypt_str(" config"), false);

	if (g_cfg.new_config_name.find(crypt_str(".pw")) == std::string::npos)
		g_cfg.new_config_name += crypt_str(".pw");

	cfg_manager->save(g_cfg.new_config_name);
	cfg_manager->config_files();

	g_cfg.selected_config = cfg_manager->files.size() - 1;
	files = cfg_manager->files;

	for (auto& current : files)
		if (current.size() > 2)
			current.erase(current.size() - 3, 3);
}

__forceinline void padding(float x, float y)
{
	ImGui::SetCursorPosX(ImGui::GetCursorPosX() + x);
	ImGui::SetCursorPosY(ImGui::GetCursorPosY() + y);
}

// title of content child.
void child_title(const char* label)
{
	ImGui::PushFont(c_menu::get().futura_large);
	ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.f, 1.f, 1.f, 1.f));

	ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 12);
	ImGui::Text(label);

	ImGui::PopStyleColor();
	ImGui::PopFont();
}

void draw_combo(const char* name, int& variable, const char* labels[], int count)
{
	ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 6);
	ImGui::Text(name);
	ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 5);
	ImGui::Combo(std::string(crypt_str("##COMBO__") + std::string(name)).c_str(), &variable, labels, count);
}

void draw_combo(const char* name, int& variable, bool (*items_getter)(void*, int, const char**), void* data, int count)
{
	ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 6);
	ImGui::Text(name);
	ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 5);
	ImGui::Combo(std::string(crypt_str("##COMBO__") + std::string(name)).c_str(), &variable, items_getter, data, count);
}

void draw_multicombo(std::string name, std::vector<int>& variable, const char* labels[], int count, std::string& preview)
{
	padding(-3, -6);
	ImGui::Text((crypt_str(" ") + name).c_str());
	padding(0, -5);

	auto hashname = crypt_str("##") + name; // we dont want to render name of combo.

	for (auto i = 0, j = 0; i < count; i++)
	{
		if (variable[i])
		{
			if (j)
				preview += crypt_str(", ") + (std::string)labels[i];
			else
				preview = labels[i];

			j++;
		}
	}

	if (ImGui::BeginCombo(hashname.c_str(), preview.c_str())) // draw start.
	{
		ImGui::Spacing();
		ImGui::BeginGroup();
		{

			for (auto i = 0; i < count; i++)
				ImGui::Selectable(labels[i], (bool*)&variable[i], ImGuiSelectableFlags_DontClosePopups);

		}
		ImGui::EndGroup();
		ImGui::Spacing();

		ImGui::EndCombo();
	}

	preview = crypt_str("None"); // reset preview to use later.
}

bool LabelClick(const char* label, bool* v, const char* unique_id)
{
	ImGuiWindow* window = ImGui::GetCurrentWindow();
	if (window->SkipItems)
		return false;

	// The concatoff/on thingies were for my weapon config system so if we're going to make that, we still need this aids.
	char Buf[64];
	_snprintf(Buf, 62, crypt_str("%s"), label);

	char getid[128];
	sprintf_s(getid, 128, crypt_str("%s%s"), label, unique_id);


	ImGuiContext& g = *GImGui;
	const ImGuiStyle& style = g.Style;
	const ImGuiID id = window->GetID(getid);
	const ImVec2 label_size = ImGui::CalcTextSize(label, NULL, true);

	const ImRect check_bb(window->DC.CursorPos, ImVec2(label_size.y + style.FramePadding.y * 2 + window->DC.CursorPos.x, window->DC.CursorPos.y + label_size.y + style.FramePadding.y * 2));
	ImGui::ItemSize(check_bb, style.FramePadding.y);

	ImRect total_bb = check_bb;

	if (label_size.x > 0)
	{
		ImGui::SameLine(0, style.ItemInnerSpacing.x);
		const ImRect text_bb(ImVec2(window->DC.CursorPos.x, window->DC.CursorPos.y + style.FramePadding.y), ImVec2(window->DC.CursorPos.x + label_size.x, window->DC.CursorPos.y + style.FramePadding.y + label_size.y));

		ImGui::ItemSize(ImVec2(text_bb.GetWidth(), check_bb.GetHeight()), style.FramePadding.y);
		total_bb = ImRect(ImMin(check_bb.Min, text_bb.Min), ImMax(check_bb.Max, text_bb.Max));
	}

	if (!ImGui::ItemAdd(total_bb, id))
		return false;

	bool hovered, held;
	bool pressed = ImGui::ButtonBehavior(total_bb, id, &hovered, &held);
	if (pressed)
		*v = !(*v);

	if (*v)
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(89 / 255.f, 119 / 255.f, 239 / 255.f, 1.f));
	if (label_size.x > 0.0f)
		ImGui::RenderText(ImVec2(check_bb.GetTL().x + 12, check_bb.GetTL().y), Buf);
	if (*v)
		ImGui::PopStyleColor();

	return pressed;

}


void draw_keybind(const char* label, key_bind* key_bind, const char* unique_id)
{
	// reset bind if we re pressing esc.
	if (key_bind->key == KEY_ESCAPE)
		key_bind->key = KEY_NONE;

	auto clicked = false;
	auto text = (std::string)m_inputsys()->ButtonCodeToString(key_bind->key);

	if (key_bind->key <= KEY_NONE || key_bind->key >= KEY_MAX)
		text = crypt_str("None");

	// if we clicked on keybind.
	if (hooks::input_shouldListen && hooks::input_receivedKeyval == &key_bind->key)
	{
		clicked = true;
		text = crypt_str("...");
	}

	auto textsize = ImGui::CalcTextSize(text.c_str()).x + 8;
	auto labelsize = ImGui::CalcTextSize(label);

	ImGui::Text(label);
	ImGui::SameLine();

	ImGui::SetCursorPosX(ImGui::GetWindowSize().x - (ImGui::GetWindowSize().x - ImGui::CalcItemWidth()) - max(50, textsize));
	ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 3);

	if (ImGui::KeybindButton(text.c_str(), unique_id, ImVec2(max(50, textsize), 23), clicked))
		clicked = true;


	if (clicked)
	{
		hooks::input_shouldListen = true;
		hooks::input_receivedKeyval = &key_bind->key;
	}

	static auto hold = false, toggle = false;

	switch (key_bind->mode)
	{
	case HOLD:
		hold = true;
		toggle = false;
		break;
	case TOGGLE:
		toggle = true;
		hold = false;
		break;
	}

	if (ImGui::BeginPopup(unique_id))
	{
		ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 6);
		ImGui::SetCursorPosX(ImGui::GetCursorPosX() + ((ImGui::GetCurrentWindow()->Size.x / 2) - (ImGui::CalcTextSize(crypt_str("Hold")).x / 2)));
		ImGui::SetCursorPosX(ImGui::GetCursorPosX() - 11);

		if (LabelClick(crypt_str("Hold"), &hold, unique_id))
		{
			if (hold)
			{
				toggle = false;
				key_bind->mode = HOLD;
			}
			else if (toggle)
			{
				hold = false;
				key_bind->mode = TOGGLE;
			}
			else
			{
				toggle = false;
				key_bind->mode = HOLD;
			}

			ImGui::CloseCurrentPopup();
		}

		ImGui::SetCursorPosX(ImGui::GetCursorPosX() + ((ImGui::GetCurrentWindow()->Size.x / 2) - (ImGui::CalcTextSize(crypt_str("Toggle")).x / 2)));
		ImGui::SetCursorPosX(ImGui::GetCursorPosX() - 11);
		ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 9);

		if (LabelClick(crypt_str("Toggle"), &toggle, unique_id))
		{
			if (toggle)
			{
				hold = false;
				key_bind->mode = TOGGLE;
			}
			else if (hold)
			{
				toggle = false;
				key_bind->mode = HOLD;
			}
			else
			{
				hold = false;
				key_bind->mode = TOGGLE;
			}

			ImGui::CloseCurrentPopup();
		}

		ImGui::EndPopup();
	}
}

void draw_semitabs(const char* labels[], int count, int& tab, ImGuiStyle& style)
{
	ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 2);

	// center of main child.
	float offset = 343.f;

	// text size padding + frame padding.
	for (int i = 0; i < count; i++)
		offset -= (ImGui::CalcTextSize(labels[i]).x) / 2 + style.FramePadding.x * 2;

	// set new padding.
	ImGui::SetCursorPosX(ImGui::GetCursorPosX() + offset);
	ImGui::BeginGroup();

	for (int i = 0; i < count; i++)
	{
		// switch current tab.
		if (ImGui::ContentTab(labels[i], tab == i))
			tab = i;

		// continue drawing on same line.
		if (i + 1 != count)
		{
			ImGui::SameLine();
			ImGui::SetCursorPosX(ImGui::GetCursorPosX() + style.ItemSpacing.x);
		}
	}

	ImGui::EndGroup();
}

__forceinline void tab_start()
{
	ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPosX() + 20, ImGui::GetCursorPosY() + 5));
}

__forceinline void tab_end()
{
	ImGui::PopStyleVar();
	ImGui::SetWindowFontScale(1.f);
}

void lua_edit(std::string window_name)
{
	std::string file_path;

	auto get_dir = [&]() -> void
	{
		static TCHAR path[MAX_PATH];

		if (SUCCEEDED(SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, NULL, path)))
			file_path = std::string(path) + crypt_str("\\skeet.pw\\Scripts\\");
		file_path += window_name + crypt_str(".lua");
	};

	get_dir();
	const char* child_name = (window_name + window_name).c_str();

	const std::string final_window_name = crypt_str("      ") + window_name + crypt_str(".lua");

	ImGui::SetNextWindowSize(ImVec2(700, 600), ImGuiCond_Once);
	ImGui::Begin(final_window_name.c_str(), nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
	ImGui::PushStyleVar(ImGuiStyleVar_ScrollbarSize, 5.f);

	static TextEditor editor;

	if (!loaded_editing_script)
	{
		static auto lang = TextEditor::LanguageDefinition::Lua();

		editor.SetLanguageDefinition(lang);
		editor.SetReadOnly(false);

		std::ifstream t(file_path);

		if (t.good()) // does while exist?
		{
			std::string str((std::istreambuf_iterator<char>(t)), std::istreambuf_iterator<char>());
			editor.SetText(str); // setup script content.
		}

		loaded_editing_script = true;
	}

	// dpi scale for font.
	// we dont need to resize it for full scale.
	ImGui::SetWindowFontScale(1.f);

	// new size depending on dpi scale.
	ImGui::SetWindowSize(ImVec2(800, 700));
	editor.Render(child_name, ImGui::GetWindowSize() - ImVec2(0, 66));

	// seperate code with buttons.
	ImGui::Separator();

	// set cursor pos to right edge of window.
	ImGui::SetCursorPosX(ImGui::GetCursorPosX() + ImGui::GetWindowSize().x - 266.f);
	ImGui::BeginGroup();

	if (ImGui::CustomButton(crypt_str("Save"), (crypt_str("Save") + window_name).c_str(), ImVec2(125, 0), true, c_menu::get().ico_bottom, crypt_str("S")))
	{
		std::ofstream out;

		out.open(file_path);
		out << editor.GetText() << std::endl;
		out.close();
	}

	ImGui::SameLine();

	// TOOD: close button will close window (return in start of function).
	if (ImGui::CustomButton(crypt_str("Close"), (crypt_str("Close") + window_name).c_str(), ImVec2(125, 0)))
	{
		g_ctx.globals.focused_on_input = false;
		loaded_editing_script = false;
		editing_script.clear();
	}

	ImGui::EndGroup();

	ImGui::PopStyleVar();
	ImGui::End();
}

void c_menu::draw_tabs_ragebot()
{
	const char* rage_sections[2] = { crypt_str("General"), crypt_str("Weapon") };
	draw_semitabs(rage_sections, 2, rage_section, ImGui::GetStyle());
}

void c_menu::draw_ragebot(int child)
{
	if (!rage_section)
	{
		if (!child)
		{
			child_title(crypt_str("Ragebot"));

			tab_start();
			ImGui::BeginChild(crypt_str("##RAGE1_FIRST"), ImVec2(317, (child_height - 35)));
			{

				ImGui::Checkbox(crypt_str("Enable"), &g_cfg.ragebot.enable);

				if (g_cfg.ragebot.enable)
					g_cfg.legitbot.enabled = false;

				ImGui::SliderInt(crypt_str("Field of view"), &g_cfg.ragebot.field_of_view, 1, 180, false, crypt_str("%d°"));
				ImGui::Checkbox(crypt_str("Silent aim"), &g_cfg.ragebot.silent_aim);
				ImGui::Checkbox(crypt_str("Automatic wall"), &g_cfg.ragebot.autowall);
				ImGui::Checkbox(crypt_str("Aimbot with zeus"), &g_cfg.ragebot.zeus_bot);
				ImGui::Checkbox(crypt_str("Aimbot with knife"), &g_cfg.ragebot.knife_bot);
				ImGui::Checkbox(crypt_str("Automatic fire"), &g_cfg.ragebot.autoshoot);
				ImGui::Checkbox(crypt_str("Automatic scope"), &g_cfg.ragebot.autoscope);
			}
			ImGui::EndChild();
			tab_end();
		}
		else
		{
			child_title(crypt_str("Anti-aim"));

			tab_start();
			ImGui::BeginChild(crypt_str("##RAGE2_FIRST"), ImVec2(317, (child_height - 35)));
			{

				static auto type = 0;

				ImGui::Checkbox(crypt_str("Enable"), &g_cfg.antiaim.enable);
				draw_combo(crypt_str("Anti-aim type"), g_cfg.antiaim.antiaim_type, antiaim_type, ARRAYSIZE(antiaim_type));

				if (g_cfg.antiaim.antiaim_type)
				{
					padding(0, 3);
					draw_combo(crypt_str("Desync"), g_cfg.antiaim.desync, desync, ARRAYSIZE(desync));

					if (g_cfg.antiaim.desync)
					{
						padding(0, 3);
						draw_combo(crypt_str("LBY type"), g_cfg.antiaim.legit_lby_type, lby_type, ARRAYSIZE(lby_type));

						if (g_cfg.antiaim.desync == 1)
						{
							draw_keybind(crypt_str("Invert desync"), &g_cfg.antiaim.flip_desync, crypt_str("##HOTKEY_INVERT_DESYNC"));
						}
					}
				}
				else
				{
					draw_combo(crypt_str("Movement type"), type, movement_type, ARRAYSIZE(movement_type));
					padding(0, 3);
					draw_combo(crypt_str("Pitch"), g_cfg.antiaim.type[type].pitch, pitch, ARRAYSIZE(pitch));
					padding(0, 3);
					draw_combo(crypt_str("Yaw"), g_cfg.antiaim.type[type].yaw, yaw, ARRAYSIZE(yaw));
					padding(0, 3);
					draw_combo(crypt_str("Base angle"), g_cfg.antiaim.type[type].base_angle, baseangle, ARRAYSIZE(baseangle));

					if (g_cfg.antiaim.type[type].yaw)
					{
						ImGui::SliderInt(g_cfg.antiaim.type[type].yaw == 1 ? crypt_str("Jitter range") : crypt_str("Spin range"), &g_cfg.antiaim.type[type].range, 1, 180);

						if (g_cfg.antiaim.type[type].yaw == 2)
							ImGui::SliderInt(crypt_str("Spin speed"), &g_cfg.antiaim.type[type].speed, 1, 15);

						padding(0, 3);
					}

					draw_combo(crypt_str("Desync"), g_cfg.antiaim.type[type].desync, desync, ARRAYSIZE(desync));

					if (g_cfg.antiaim.type[type].desync)
					{
						if (type == ANTIAIM_STAND)
						{
							padding(0, 3);
							draw_combo(crypt_str("LBY type"), g_cfg.antiaim.lby_type, lby_type, ARRAYSIZE(lby_type));
						}

						if (type != ANTIAIM_STAND || !g_cfg.antiaim.lby_type)
						{
							ImGui::SliderInt(crypt_str("Desync range"), &g_cfg.antiaim.type[type].desync_range, 1, 60);
							ImGui::SliderInt(crypt_str("Inverted desync range"), &g_cfg.antiaim.type[type].inverted_desync_range, 1, 60);
							ImGui::SliderInt(crypt_str("Body lean"), &g_cfg.antiaim.type[type].body_lean, 0, 100);
							ImGui::SliderInt(crypt_str("Inverted body lean"), &g_cfg.antiaim.type[type].inverted_body_lean, 0, 100);
						}

						if (g_cfg.antiaim.type[type].desync == 1)
						{
							draw_keybind(crypt_str("Invert desync"), &g_cfg.antiaim.flip_desync, crypt_str("##HOTKEY_INVERT_DESYNC"));
						}
					}

					draw_keybind(crypt_str("Manual back"), &g_cfg.antiaim.manual_back, crypt_str("##HOTKEY_INVERT_BACK"));

					draw_keybind(crypt_str("Manual left"), &g_cfg.antiaim.manual_left, crypt_str("##HOTKEY_INVERT_LEFT"));

					draw_keybind(crypt_str("Manual right"), &g_cfg.antiaim.manual_right, crypt_str("##HOTKEY_INVERT_RIGHT"));

					if (g_cfg.antiaim.manual_back.key > KEY_NONE && g_cfg.antiaim.manual_back.key < KEY_MAX || g_cfg.antiaim.manual_left.key > KEY_NONE && g_cfg.antiaim.manual_left.key < KEY_MAX || g_cfg.antiaim.manual_right.key > KEY_NONE && g_cfg.antiaim.manual_right.key < KEY_MAX)
					{
						ImGui::Checkbox(crypt_str("Manuals indicator"), &g_cfg.antiaim.flip_indicator);
						ImGui::SameLine();
						ImGui::ColorEdit(crypt_str("##invc"), &g_cfg.antiaim.flip_indicator_color, ALPHA);
					}
				}

			}
			ImGui::EndChild();
			tab_end();
		}
	}
	else
	{
		const char* rage_weapons[8] = { crypt_str("Revolver / Deagle"), crypt_str("Pistols"), crypt_str("SMGs"), crypt_str("Rifles"), crypt_str("Auto"), crypt_str("Scout"), crypt_str("AWP"), crypt_str("Heavy") };

		if (!child)
		{
			child_title(crypt_str("General"));
			tab_start();

			ImGui::BeginChild(crypt_str("##RAGE1_SECOND"), ImVec2(317, (child_height - 35)));
			{
				ImGui::Spacing();

				draw_combo(crypt_str("Current weapon"), hooks::rage_weapon, rage_weapons, ARRAYSIZE(rage_weapons));
				ImGui::Spacing();

				draw_combo(crypt_str("Target selection"), g_cfg.ragebot.weapon[hooks::rage_weapon].selection_type, selection, ARRAYSIZE(selection));
				padding(0, 3);
				draw_multicombo(crypt_str("Hitboxes"), g_cfg.ragebot.weapon[hooks::rage_weapon].hitboxes, hitboxes, ARRAYSIZE(hitboxes), preview);

				ImGui::Checkbox(crypt_str("Static point scale"), &g_cfg.ragebot.weapon[hooks::rage_weapon].static_point_scale);

				if (g_cfg.ragebot.weapon[hooks::rage_weapon].static_point_scale)
				{
					ImGui::SliderFloat(crypt_str("Head scale"), &g_cfg.ragebot.weapon[hooks::rage_weapon].head_scale, 0.2f, 1.0f, g_cfg.ragebot.weapon[hooks::rage_weapon].head_scale ? crypt_str("%.2f") : crypt_str("None"));
					ImGui::SliderFloat(crypt_str("Body scale"), &g_cfg.ragebot.weapon[hooks::rage_weapon].body_scale, 0.2f, 1.0f, g_cfg.ragebot.weapon[hooks::rage_weapon].body_scale ? crypt_str("%.2f") : crypt_str("None"));
				}

				ImGui::Checkbox(crypt_str("Double tap"), &g_cfg.ragebot.weapon[hooks::rage_weapon].double_tap);

				if (g_cfg.ragebot.weapon[hooks::rage_weapon].double_tap)
				{
					ImGui::SameLine();
					draw_keybind(crypt_str(""), &g_cfg.ragebot.double_tap_key, crypt_str("##HOTKEY_DT"));
					ImGui::SliderInt(crypt_str("Shift value##DT"), &g_cfg.ragebot.weapon[hooks::rage_weapon].double_tap_shift_value, 1, 16);
					ImGui::Checkbox(crypt_str("Slow teleport"), &g_cfg.ragebot.weapon[hooks::rage_weapon].slow_teleport);
				}

				ImGui::Checkbox(crypt_str("Hide shots"), &g_cfg.ragebot.weapon[hooks::rage_weapon].hide_shots);

				if (g_cfg.ragebot.weapon[hooks::rage_weapon].hide_shots)
				{
					ImGui::SameLine();
					draw_keybind(crypt_str(""), &g_cfg.ragebot.hide_shots_key, crypt_str("##HOTKEY_HS"));
					ImGui::SliderInt(crypt_str("Shift value##HS"), &g_cfg.ragebot.weapon[hooks::rage_weapon].hide_shots_shift_value, 1, 16);
				}
			}

			ImGui::EndChild();
			tab_end();
		}
		else
		{
			child_title(crypt_str("Accuracy"));
			tab_start();

			ImGui::BeginChild(crypt_str("##RAGE2_SECOND"), ImVec2(317, (child_height - 35)));
			{
				ImGui::Checkbox(crypt_str("Hitchance"), &g_cfg.ragebot.weapon[hooks::rage_weapon].hitchance);

				if (g_cfg.ragebot.weapon[hooks::rage_weapon].hitchance)
					ImGui::SliderInt(crypt_str("Hitchance amount"), &g_cfg.ragebot.weapon[hooks::rage_weapon].hitchance_amount, 1, 99);

				if (g_cfg.ragebot.weapon[hooks::rage_weapon].double_tap)
				{
					ImGui::Checkbox(crypt_str("Double tap hitchance"), &g_cfg.ragebot.weapon[hooks::rage_weapon].double_tap_hitchance);

					if (g_cfg.ragebot.weapon[hooks::rage_weapon].double_tap_hitchance)
						ImGui::SliderInt(crypt_str("Double tap hitchance amount"), &g_cfg.ragebot.weapon[hooks::rage_weapon].double_tap_hitchance_amount, 1, 100);
				}

				draw_multicombo(crypt_str("Automatic stop conditions"), g_cfg.ragebot.weapon[hooks::rage_weapon].automatic_stop_conditions, automatic_stop_conditions, ARRAYSIZE(automatic_stop_conditions), preview);
				draw_multicombo(crypt_str("Safe points conditions"), g_cfg.ragebot.weapon[hooks::rage_weapon].safe_points_conditions, safe_points_conditions, ARRAYSIZE(safe_points_conditions), preview);
				draw_multicombo(crypt_str("Body aim conditions"), g_cfg.ragebot.weapon[hooks::rage_weapon].body_aim_conditions, body_aim_conditions, ARRAYSIZE(body_aim_conditions), preview);

				draw_keybind(crypt_str("Force safe points"), &g_cfg.ragebot.safe_point_key, crypt_str("##HOKEY_FORCE_SAFE_POINTS"));
				draw_keybind(crypt_str("Force body aim"), &g_cfg.ragebot.body_aim_key, crypt_str("##HOKEY_FORCE_BODY_AIM"));

				ImGui::SliderInt(crypt_str("Minimum visible damage"), &g_cfg.ragebot.weapon[hooks::rage_weapon].minimum_visible_damage, 1, 120, true);

				if (g_cfg.ragebot.autowall)
					ImGui::SliderInt(crypt_str("Minimum wall damage"), &g_cfg.ragebot.weapon[hooks::rage_weapon].minimum_damage, 1, 120, true);

				draw_keybind(crypt_str("Damage override"), &g_cfg.ragebot.damage_override_key, crypt_str("##HOTKEY__DAMAGE_OVERRIDE"));

				if (g_cfg.ragebot.damage_override_key.key > KEY_NONE && g_cfg.ragebot.damage_override_key.key < KEY_MAX)
					ImGui::SliderInt(crypt_str("Minimum override damage"), &g_cfg.ragebot.weapon[hooks::rage_weapon].minimum_override_damage, 1, 120, true);

			}

			ImGui::EndChild();
			tab_end();
		}
	}
}

void c_menu::draw_legit(int child)
{
	const char* legit_weapons[6] = { crypt_str("Deagle"), crypt_str("Pistols"), crypt_str("Rifles"), crypt_str("SMGs"), crypt_str("Snipers"), crypt_str("Heavy") };
	const char* hitbox_legit[3] = { crypt_str("Nearest"), crypt_str("Head"), crypt_str("Body") };

	if (!child)
	{
		child_title(crypt_str("Legitbot"));

		tab_start();
		ImGui::BeginChild(crypt_str("##RAGE1_FIRST"), ImVec2(317, (child_height - 35)));
		{
			padding(0, 6);
			ImGui::Checkbox(crypt_str("Enable"), &g_cfg.legitbot.enabled);
			ImGui::SameLine();
			draw_keybind(crypt_str(""), &g_cfg.legitbot.key, crypt_str("##HOTKEY_LGBT_KEY"));

			if (g_cfg.legitbot.enabled)
				g_cfg.ragebot.enable = false;

			ImGui::Checkbox(crypt_str("Friendly fire"), &g_cfg.legitbot.friendly_fire);
			ImGui::Checkbox(crypt_str("Automatic pistols"), &g_cfg.legitbot.autopistol);

			ImGui::Checkbox(crypt_str("Automatic scope"), &g_cfg.legitbot.autoscope);

			if (g_cfg.legitbot.autoscope)
				ImGui::Checkbox(crypt_str("Unscope after shot"), &g_cfg.legitbot.unscope);

			ImGui::Checkbox(crypt_str("Snipers in zoom only"), &g_cfg.legitbot.sniper_in_zoom_only);

			ImGui::Checkbox(crypt_str("Enable in air"), &g_cfg.legitbot.do_if_local_in_air);
			ImGui::Checkbox(crypt_str("Enable if flashed"), &g_cfg.legitbot.do_if_local_flashed);
			ImGui::Checkbox(crypt_str("Enable in smoke"), &g_cfg.legitbot.do_if_enemy_in_smoke);

			draw_keybind(crypt_str("Automatic fire key"), &g_cfg.legitbot.autofire_key, crypt_str("##HOTKEY_AUTOFIRE_LGBT_KEY"));
			ImGui::SliderInt(crypt_str("Automatic fire delay"), &g_cfg.legitbot.autofire_delay, 0, 12, false, (!g_cfg.legitbot.autofire_delay ? crypt_str("None") : (g_cfg.legitbot.autofire_delay == 1 ? crypt_str("%d tick") : crypt_str("%d ticks"))));

		}
		ImGui::EndChild();


		tab_end();
	}
	else
	{
		child_title(crypt_str("Weapon settings"));

		tab_start();
		ImGui::BeginChild(crypt_str("##RAGE1_FIRST"), ImVec2(317, (child_height - 35)));
		{
			ImGui::Spacing();
			draw_combo(crypt_str("Current weapon"), hooks::legit_weapon, legit_weapons, ARRAYSIZE(legit_weapons));
			ImGui::Spacing();
			draw_combo(crypt_str("Hitbox"), g_cfg.legitbot.weapon[hooks::legit_weapon].priority, hitbox_legit, ARRAYSIZE(hitbox_legit));

			ImGui::Checkbox(crypt_str("Automatic stop"), &g_cfg.legitbot.weapon[hooks::legit_weapon].auto_stop);

			draw_combo(crypt_str("Field of view type"), g_cfg.legitbot.weapon[hooks::legit_weapon].fov_type, LegitFov, ARRAYSIZE(LegitFov));
			ImGui::SliderFloat(crypt_str("Field of view amount"), &g_cfg.legitbot.weapon[hooks::legit_weapon].fov, 0.f, 30.f, crypt_str("%.2f"));

			ImGui::Spacing();

			ImGui::SliderFloat(crypt_str("Silent field of view"), &g_cfg.legitbot.weapon[hooks::legit_weapon].silent_fov, 0.f, 30.f, (!g_cfg.legitbot.weapon[hooks::legit_weapon].silent_fov ? crypt_str("None") : crypt_str("%.2f")));

			ImGui::Spacing();

			draw_combo(crypt_str("Smooth type"), g_cfg.legitbot.weapon[hooks::legit_weapon].smooth_type, LegitSmooth, ARRAYSIZE(LegitSmooth));
			ImGui::SliderFloat(crypt_str("Smooth amount"), &g_cfg.legitbot.weapon[hooks::legit_weapon].smooth, 1.f, 12.f, crypt_str("%.1f"));

			ImGui::Spacing();

			draw_combo(crypt_str("RCS type"), g_cfg.legitbot.weapon[hooks::legit_weapon].rcs_type, RCSType, ARRAYSIZE(RCSType));
			ImGui::SliderFloat(crypt_str("RCS amount"), &g_cfg.legitbot.weapon[hooks::legit_weapon].rcs, 0.f, 100.f, crypt_str("%.0f%%"), 1.f);

			if (g_cfg.legitbot.weapon[hooks::legit_weapon].rcs > 0)
			{
				ImGui::SliderFloat(crypt_str("RCS custom field of view"), &g_cfg.legitbot.weapon[hooks::legit_weapon].custom_rcs_fov, 0.f, 30.f, (!g_cfg.legitbot.weapon[hooks::legit_weapon].custom_rcs_fov ? crypt_str("None") : crypt_str("%.2f")));
				ImGui::SliderFloat(crypt_str("RCS Custom smooth"), &g_cfg.legitbot.weapon[hooks::legit_weapon].custom_rcs_smooth, 0.f, 12.f, (!g_cfg.legitbot.weapon[hooks::legit_weapon].custom_rcs_smooth ? crypt_str("None") : crypt_str("%.1f")));
			}

			ImGui::Spacing();

			ImGui::SliderInt(crypt_str("Automatic wall damage"), &g_cfg.legitbot.weapon[hooks::legit_weapon].awall_dmg, 0, 100, false, (!g_cfg.legitbot.weapon[hooks::legit_weapon].awall_dmg ? crypt_str("None") : crypt_str("%d")));
			ImGui::SliderInt(crypt_str("Automatic fire hitchance"), &g_cfg.legitbot.weapon[hooks::legit_weapon].autofire_hitchance, 0, 100, false, (!g_cfg.legitbot.weapon[hooks::legit_weapon].autofire_hitchance ? crypt_str("None") : crypt_str("%d")));
			ImGui::SliderFloat(crypt_str("Target switch delay"), &g_cfg.legitbot.weapon[hooks::legit_weapon].target_switch_delay, 0.f, 1.f);
		}
		ImGui::EndChild();


		tab_end();
	}
}

void c_menu::draw_tabs_visuals()
{
	const char* visuals_sections[3] = { crypt_str("General"), crypt_str("Viewmodel"), crypt_str("Skinchanger") };
	draw_semitabs(visuals_sections, 3, visuals_section, ImGui::GetStyle());
}

void c_menu::draw_visuals(int child)
{
	if (visuals_section != 2)
	{

		if (!child)
		{
			if (!visuals_section)
			{
				child_title(crypt_str("General"));

				tab_start();
				ImGui::BeginChild(crypt_str("##VISUAL1_FIRST"), ImVec2(317, (child_height - 35)));
				{
					ImGui::Checkbox(crypt_str("Enable"), &g_cfg.player.enable);

					draw_multicombo(crypt_str("Indicators"), g_cfg.esp.indicators, indicators, ARRAYSIZE(indicators), preview);
					padding(0, 3);

					draw_multicombo(crypt_str("Removals"), g_cfg.esp.removals, removals, ARRAYSIZE(removals), preview);

					if (g_cfg.esp.removals[REMOVALS_ZOOM])
						ImGui::Checkbox(crypt_str("Fix zoom sensivity"), &g_cfg.esp.fix_zoom_sensivity);

					ImGui::Checkbox(crypt_str("Grenade prediction"), &g_cfg.esp.grenade_prediction);
					ImGui::SameLine();
					ImGui::ColorEdit(crypt_str("##grenpredcolor"), &g_cfg.esp.grenade_prediction_color, ALPHA);

					if (g_cfg.esp.grenade_prediction)
					{
						ImGui::Checkbox(crypt_str("On click"), &g_cfg.esp.on_click);
						ImGui::Text(crypt_str("Tracer color "));
						ImGui::SameLine();
						ImGui::ColorEdit(crypt_str("##tracergrenpredcolor"), &g_cfg.esp.grenade_prediction_tracer_color, ALPHA);
					}

					ImGui::Checkbox(crypt_str("Grenade projectiles"), &g_cfg.esp.projectiles);

					if (g_cfg.esp.projectiles)
						draw_multicombo(crypt_str("Grenade ESP"), g_cfg.esp.grenade_esp, proj_combo, ARRAYSIZE(proj_combo), preview);

					if (g_cfg.esp.grenade_esp[GRENADE_ICON] || g_cfg.esp.grenade_esp[GRENADE_TEXT])
					{
						ImGui::Text(crypt_str("Color "));
						ImGui::SameLine();
						ImGui::ColorEdit(crypt_str("##projectcolor"), &g_cfg.esp.projectiles_color, ALPHA);
					}

					if (g_cfg.esp.grenade_esp[GRENADE_BOX])
					{
						ImGui::Text(crypt_str("Box color "));
						ImGui::SameLine();
						ImGui::ColorEdit(crypt_str("##grenade_box_color"), &g_cfg.esp.grenade_box_color, ALPHA);
					}

					if (g_cfg.esp.grenade_esp[GRENADE_GLOW])
					{
						ImGui::Text(crypt_str("Glow color "));
						ImGui::SameLine();
						ImGui::ColorEdit(crypt_str("##grenade_glow_color"), &g_cfg.esp.grenade_glow_color, ALPHA);
					}

					ImGui::Checkbox(crypt_str("Fire timer"), &g_cfg.esp.molotov_timer);
					ImGui::SameLine();
					ImGui::ColorEdit(crypt_str("##molotovcolor"), &g_cfg.esp.molotov_timer_color, ALPHA);

					ImGui::Checkbox(crypt_str("Smoke timer"), &g_cfg.esp.smoke_timer);
					ImGui::SameLine();
					ImGui::ColorEdit(crypt_str("##smokecolor"), &g_cfg.esp.smoke_timer_color, ALPHA);

					ImGui::Checkbox(crypt_str("Bomb indicator"), &g_cfg.esp.bomb_timer);
					draw_multicombo(crypt_str("Weapon ESP"), g_cfg.esp.weapon, weaponesp, ARRAYSIZE(weaponesp), preview);

					if (g_cfg.esp.weapon[WEAPON_ICON] || g_cfg.esp.weapon[WEAPON_TEXT] || g_cfg.esp.weapon[WEAPON_DISTANCE])
					{
						ImGui::Text(crypt_str("Color "));
						ImGui::SameLine();
						ImGui::ColorEdit(crypt_str("##weaponcolor"), &g_cfg.esp.weapon_color, ALPHA);
					}

					if (g_cfg.esp.weapon[WEAPON_BOX])
					{
						ImGui::Text(crypt_str("Box color "));
						ImGui::SameLine();
						ImGui::ColorEdit(crypt_str("##weaponboxcolor"), &g_cfg.esp.box_color, ALPHA);
					}

					if (g_cfg.esp.weapon[WEAPON_GLOW])
					{
						ImGui::Text(crypt_str("Glow color "));
						ImGui::SameLine();
						ImGui::ColorEdit(crypt_str("##weaponglowcolor"), &g_cfg.esp.weapon_glow_color, ALPHA);
					}

					if (g_cfg.esp.weapon[WEAPON_AMMO])
					{
						ImGui::Text(crypt_str("Ammo bar color "));
						ImGui::SameLine();
						ImGui::ColorEdit(crypt_str("##weaponammocolor"), &g_cfg.esp.weapon_ammo_color, ALPHA);
					}

					ImGui::Checkbox(crypt_str("Client bullet impacts"), &g_cfg.esp.client_bullet_impacts);
					ImGui::SameLine();
					ImGui::ColorEdit(crypt_str("##clientbulletimpacts"), &g_cfg.esp.client_bullet_impacts_color, ALPHA);

					ImGui::Checkbox(crypt_str("Server bullet impacts"), &g_cfg.esp.server_bullet_impacts);
					ImGui::SameLine();
					ImGui::ColorEdit(crypt_str("##serverbulletimpacts"), &g_cfg.esp.server_bullet_impacts_color, ALPHA);

					ImGui::Checkbox(crypt_str("Local bullet tracers"), &g_cfg.esp.bullet_tracer);
					ImGui::SameLine();
					ImGui::ColorEdit(crypt_str("##bulltracecolor"), &g_cfg.esp.bullet_tracer_color, ALPHA);

					ImGui::Checkbox(crypt_str("Enemy bullet tracers"), &g_cfg.esp.enemy_bullet_tracer);
					ImGui::SameLine();

					ImGui::ColorEdit(crypt_str("##enemybulltracecolor"), &g_cfg.esp.enemy_bullet_tracer_color, ALPHA);
					draw_multicombo(crypt_str("Hit marker"), g_cfg.esp.hitmarker, hitmarkers, ARRAYSIZE(hitmarkers), preview);
					ImGui::Checkbox(crypt_str("Damage marker"), &g_cfg.esp.damage_marker);
					ImGui::Checkbox(crypt_str("Kill effect"), &g_cfg.esp.kill_effect);

					if (g_cfg.esp.kill_effect)
						ImGui::SliderFloat(crypt_str("Duration"), &g_cfg.esp.kill_effect_duration, 0.01f, 3.0f);

					draw_keybind(crypt_str("Thirdperson"), &g_cfg.misc.thirdperson_toggle, crypt_str("##TPKEY__HOTKEY"));

					ImGui::Checkbox(crypt_str("Thirdperson when spectating"), &g_cfg.misc.thirdperson_when_spectating);

					if (g_cfg.misc.thirdperson_toggle.key > KEY_NONE && g_cfg.misc.thirdperson_toggle.key < KEY_MAX)
						ImGui::SliderInt(crypt_str("Thirdperson distance"), &g_cfg.misc.thirdperson_distance, 100, 300);

					ImGui::SliderInt(crypt_str("Field of view"), &g_cfg.esp.fov, 0, 89);
					ImGui::Checkbox(crypt_str("Show spread"), &g_cfg.esp.show_spread);
					ImGui::SameLine();
					ImGui::ColorEdit(crypt_str("##spredcolor"), &g_cfg.esp.show_spread_color, ALPHA);
					ImGui::Checkbox(crypt_str("Penetration crosshair"), &g_cfg.esp.penetration_reticle);
				}

				ImGui::EndChild();
				tab_end();
			}
			else
			{
				child_title(crypt_str("General"));

				tab_start();
				ImGui::BeginChild(crypt_str("##VISUAL2_FIRST"), ImVec2(317, (child_height - 35)));
				{
					ImGui::Checkbox(crypt_str("Rare animations"), &g_cfg.skins.rare_animations);
					ImGui::SliderInt(crypt_str("Viewmodel field of view"), &g_cfg.esp.viewmodel_fov, 0, 89);
					ImGui::SliderInt(crypt_str("Viewmodel X"), &g_cfg.esp.viewmodel_x, -50, 50);
					ImGui::SliderInt(crypt_str("Viewmodel Y"), &g_cfg.esp.viewmodel_y, -50, 50);
					ImGui::SliderInt(crypt_str("Viewmodel Z"), &g_cfg.esp.viewmodel_z, -50, 50);
					ImGui::SliderInt(crypt_str("Viewmodel roll"), &g_cfg.esp.viewmodel_roll, -180, 180);
				}

				ImGui::EndChild();
				tab_end();
			}
		}
		else
		{
			if (!visuals_section)
			{
				child_title(crypt_str("World"));

				tab_start();
				ImGui::BeginChild(crypt_str("##VISUAL1_SECOND"), ImVec2(317, (child_height - 35)));
				{
					ImGui::Checkbox(crypt_str("Full bright"), &g_cfg.esp.bright);

					draw_combo(crypt_str("Skybox"), g_cfg.esp.skybox, skybox, ARRAYSIZE(skybox));

					ImGui::Text(crypt_str("Color "));
					ImGui::SameLine();
					ImGui::ColorEdit(crypt_str("##skyboxcolor"), &g_cfg.esp.skybox_color, NOALPHA);

					if (g_cfg.esp.skybox == 21)
					{
						static char sky_custom[64] = "\0";

						if (!g_cfg.esp.custom_skybox.empty())
							strcpy_s(sky_custom, sizeof(sky_custom), g_cfg.esp.custom_skybox.c_str());

						ImGui::Text(crypt_str("Name"));
						ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.f);

						if (ImGui::InputText(crypt_str("##customsky"), sky_custom, sizeof(sky_custom)))
							g_cfg.esp.custom_skybox = sky_custom;

						ImGui::PopStyleVar();
					}

					ImGui::Checkbox(crypt_str("Color modulation"), &g_cfg.esp.nightmode);

					if (g_cfg.esp.nightmode)
					{
						ImGui::Text(crypt_str("World color "));
						ImGui::SameLine();
						ImGui::ColorEdit(crypt_str("##worldcolor"), &g_cfg.esp.world_color, ALPHA);

						ImGui::Text(crypt_str("Props color "));
						ImGui::SameLine();
						ImGui::ColorEdit(crypt_str("##propscolor"), &g_cfg.esp.props_color, ALPHA);
					}

					ImGui::Checkbox(crypt_str("World modulation"), &g_cfg.esp.world_modulation);

					if (g_cfg.esp.world_modulation)
					{
						ImGui::SliderFloat(crypt_str("Bloom"), &g_cfg.esp.bloom, 0.0f, 750.0f);
						ImGui::SliderFloat(crypt_str("Exposure"), &g_cfg.esp.exposure, 0.0f, 2000.0f);
						ImGui::SliderFloat(crypt_str("Ambient"), &g_cfg.esp.ambient, 0.0f, 1500.0f);
					}

					ImGui::Checkbox(crypt_str("Fog modulation"), &g_cfg.esp.fog);

					if (g_cfg.esp.fog)
					{
						ImGui::SliderInt(crypt_str("Distance"), &g_cfg.esp.fog_distance, 0, 2500);
						ImGui::SliderInt(crypt_str("Density"), &g_cfg.esp.fog_density, 0, 100);

						ImGui::Text(crypt_str("Color "));
						ImGui::SameLine();
						ImGui::ColorEdit(crypt_str("##fogcolor"), &g_cfg.esp.fog_color, NOALPHA);
					}
				}

				ImGui::EndChild();
				tab_end();
			}
			else
			{
				child_title(crypt_str("Chams"));

				tab_start();
				ImGui::BeginChild(crypt_str("##VISUAL2_SECOND"), ImVec2(317, (child_height - 35)));
				{
					ImGui::Checkbox(crypt_str("Arms chams"), &g_cfg.esp.arms_chams);
					ImGui::SameLine();
					ImGui::ColorEdit(crypt_str("##armscolor"), &g_cfg.esp.arms_chams_color, ALPHA);


					draw_combo(crypt_str("Arms chams material"), g_cfg.esp.arms_chams_type, chamstype, ARRAYSIZE(chamstype));

					if (g_cfg.esp.arms_chams_type != 6)
					{
						ImGui::Checkbox(crypt_str("Arms double material "), &g_cfg.esp.arms_double_material);
						ImGui::SameLine();
						ImGui::ColorEdit(crypt_str("##armsdoublematerial"), &g_cfg.esp.arms_double_material_color, ALPHA);
					}

					ImGui::Checkbox(crypt_str("Arms animated material "), &g_cfg.esp.arms_animated_material);
					ImGui::SameLine();
					ImGui::ColorEdit(crypt_str("##armsanimatedmaterial"), &g_cfg.esp.arms_animated_material_color, ALPHA);

					ImGui::Spacing();

					ImGui::Checkbox(crypt_str("Weapon chams"), &g_cfg.esp.weapon_chams);
					ImGui::SameLine();
					ImGui::ColorEdit(crypt_str("##weaponchamscolors"), &g_cfg.esp.weapon_chams_color, ALPHA);

					draw_combo(crypt_str("Weapon chams material"), g_cfg.esp.weapon_chams_type, chamstype, ARRAYSIZE(chamstype));

					if (g_cfg.esp.weapon_chams_type != 6)
					{
						ImGui::Checkbox(crypt_str("Double material "), &g_cfg.esp.weapon_double_material);
						ImGui::SameLine();
						ImGui::ColorEdit(crypt_str("##weapondoublematerial"), &g_cfg.esp.weapon_double_material_color, ALPHA);
					}

					ImGui::Checkbox(crypt_str("Animated material "), &g_cfg.esp.weapon_animated_material);
					ImGui::SameLine();
					ImGui::ColorEdit(crypt_str("##weaponanimatedmaterial"), &g_cfg.esp.weapon_animated_material_color, ALPHA);

				}

				ImGui::EndChild();
				tab_end();
			}
		}
	}
	else if (child == -1)
	{
		// hey stewen, what r u doing there? he, hm heee, DRUGS
		static bool drugs = false;

		// some animation logic(switch)
		static bool active_animation = false;
		static bool preview_reverse = false;
		static float switch_alpha = 1.f;
		static int next_id = -1;
		if (active_animation)
		{
			if (preview_reverse)
			{
				if (switch_alpha == 1.f)
				{
					preview_reverse = false;
					active_animation = false;
				}

				switch_alpha = math::clamp(switch_alpha + (4.f * ImGui::GetIO().DeltaTime), 0.01f, 1.f);
			}
			else
			{
				if (switch_alpha == 0.01f)
				{
					preview_reverse = true;
				}

				switch_alpha = math::clamp(switch_alpha - (4.f * ImGui::GetIO().DeltaTime), 0.01f, 1.f);
			}
		}
		else
			switch_alpha = math::clamp(switch_alpha + (4.f * ImGui::GetIO().DeltaTime), 0.0f, 1.f);

		ImGui::BeginChild(crypt_str("##SKINCHANGER__TAB"), ImVec2(686, child_height)); // first functional child
		{
			ImGui::PushStyleVar(ImGuiStyleVar_Alpha, (1.f - preview_alpha) * public_alpha * switch_alpha);
			child_title(current_profile == -1 ? crypt_str("Skinchanger") : game_data::weapon_names[current_profile].name);
			tab_start();
			ImGui::BeginChild(crypt_str("##SKINCHANGER__CHILD"), ImVec2(686, (child_height - 35)));
			{
				// we need to count our items in 1 line
				auto same_line_counter = 0;

				// if we didnt choose any weapon
				if (current_profile == -1)
				{
					for (auto i = 0; i < g_cfg.skins.skinChanger.size(); i++)
					{
						// do we need update our preview for some reasons?
						if (!all_skins[i])
						{
							g_cfg.skins.skinChanger.at(i).update();
							all_skins[i] = get_skin_preview(get_wep(i, (i == 0 || i == 1) ? g_cfg.skins.skinChanger.at(i).definition_override_vector_index : -1, i == 0).c_str(), g_cfg.skins.skinChanger.at(i).skin_name, device); //-V810
						}

						// we licked on weapon
						if (ImGui::ImageButton(all_skins[i], ImVec2(107, 76)))
						{
							next_id = i;
							active_animation = true;
						}

						// if our animation step is half from all - switch profile
						if (active_animation && preview_reverse)
						{
							ImGui::SetScrollY(0);
							current_profile = next_id;
						}

						if (same_line_counter < 4) { // continue push same-line
							ImGui::SameLine();
							same_line_counter++;
						}
						else { // we have maximum elements in 1 line
							same_line_counter = 0;
						}
					}
				}
				else
				{
					// update skin preview bool
					static bool need_update[36];

					// we pressed crypt_str("Save & Close") button
					static bool leave;

					// update if we have nullptr texture or if we push force update
					if (!all_skins[current_profile] || need_update[current_profile])
					{
						all_skins[current_profile] = get_skin_preview(get_wep(current_profile, (current_profile == 0 || current_profile == 1) ? g_cfg.skins.skinChanger.at(current_profile).definition_override_vector_index : -1, current_profile == 0).c_str(), g_cfg.skins.skinChanger.at(current_profile).skin_name, device); //-V810
						need_update[current_profile] = false;
					}

					// get settings for selected weapon
					auto& selected_entry = g_cfg.skins.skinChanger[current_profile];
					selected_entry.itemIdIndex = current_profile;

					ImGui::BeginGroup();
					ImGui::PushItemWidth(260);

					static auto item_index = selected_entry.paint_kit_vector_index;

					if (!current_profile)
					{
						ImGui::Text(crypt_str("Knife"));
						ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 5);
						if (ImGui::Combo(crypt_str("##Knife_combo"), &selected_entry.definition_override_vector_index, [](void* data, int idx, const char** out_text)
							{
								*out_text = game_data::knife_names[idx].name;
								return true;
							}, nullptr, IM_ARRAYSIZE(game_data::knife_names)))
							need_update[current_profile] = true; // push force update
					}
					else if (current_profile == 1)
					{
						ImGui::Text(crypt_str("Gloves"));
						ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 5);
						if (ImGui::Combo(crypt_str("##Glove_combo"), &selected_entry.definition_override_vector_index, [](void* data, int idx, const char** out_text)
							{
								*out_text = game_data::glove_names[idx].name;
								return true;
							}, nullptr, IM_ARRAYSIZE(game_data::glove_names)))
						{
							item_index = 0; // set new generated paintkits element to 0;
							need_update[current_profile] = true; // push force update
						}
					}
					else
						selected_entry.definition_override_vector_index = 0;

					if (current_profile != 1)
					{
						ImGui::Text(crypt_str("Search"));
						ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.f);

						if (ImGui::InputText(crypt_str("##search"), search_skins, sizeof(search_skins)))
							item_index = -1;

						ImGui::PopStyleVar();
					}

					auto main_kits = current_profile == 1 ? SkinChanger::gloveKits : SkinChanger::skinKits;
					auto display_index = 0;

					SkinChanger::displayKits = main_kits;

					// we dont need custom gloves
					if (current_profile == 1)
					{
						for (auto i = 0; i < main_kits.size(); i++)
						{
							auto main_name = main_kits.at(i).name;

							for (auto i = 0; i < main_name.size(); i++)
								if (iswalpha((main_name.at(i))))
									main_name.at(i) = towlower(main_name.at(i));

							char search_name[64];

							if (!strcmp(game_data::glove_names[selected_entry.definition_override_vector_index].name, crypt_str("Hydra")))
								strcpy_s(search_name, sizeof(search_name), crypt_str("Bloodhound"));
							else
								strcpy_s(search_name, sizeof(search_name), game_data::glove_names[selected_entry.definition_override_vector_index].name);

							for (auto i = 0; i < sizeof(search_name); i++)
								if (iswalpha(search_name[i]))
									search_name[i] = towlower(search_name[i]);

							if (main_name.find(search_name) != std::string::npos)
							{
								SkinChanger::displayKits.at(display_index) = main_kits.at(i);
								display_index++;
							}
						}

						SkinChanger::displayKits.erase(SkinChanger::displayKits.begin() + display_index, SkinChanger::displayKits.end());
					}
					else
					{
						if (strcmp(search_skins, crypt_str("")))
						{
							for (auto i = 0; i < main_kits.size(); i++)
							{
								auto main_name = main_kits.at(i).name;

								for (auto i = 0; i < main_name.size(); i++)
									if (iswalpha(main_name.at(i)))
										main_name.at(i) = towlower(main_name.at(i));

								char search_name[64];
								strcpy_s(search_name, sizeof(search_name), search_skins);

								for (auto i = 0; i < sizeof(search_name); i++)
									if (iswalpha(search_name[i]))
										search_name[i] = towlower(search_name[i]);

								if (main_name.find(search_name) != std::string::npos)
								{
									SkinChanger::displayKits.at(display_index) = main_kits.at(i);
									display_index++;
								}
							}

							SkinChanger::displayKits.erase(SkinChanger::displayKits.begin() + display_index, SkinChanger::displayKits.end());
						}
						else
							item_index = selected_entry.paint_kit_vector_index;
					}

					ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.f);
					if (!SkinChanger::displayKits.empty())
					{
						if (ImGui::ListBox(crypt_str("##PAINTKITS"), &item_index, [](void* data, int idx, const char** out_text)
							{
								while (SkinChanger::displayKits.at(idx).name.find(crypt_str("С‘")) != std::string::npos)
									SkinChanger::displayKits.at(idx).name.replace(SkinChanger::displayKits.at(idx).name.find(crypt_str("С‘")), 2, crypt_str("Рµ"));

								*out_text = SkinChanger::displayKits.at(idx).name.c_str();
								return true;
							}, nullptr, SkinChanger::displayKits.size(), SkinChanger::displayKits.size() > 9 ? 9 : SkinChanger::displayKits.size()) || !all_skins[current_profile])
						{
							SkinChanger::scheduleHudUpdate();
							need_update[current_profile] = true;

							auto i = 0;

							while (i < main_kits.size())
							{
								if (main_kits.at(i).id == SkinChanger::displayKits.at(item_index).id)
								{
									selected_entry.paint_kit_vector_index = i;
									break;
								}

								i++;
							}

						}
					}
					ImGui::PopStyleVar();

					if (ImGui::InputInt(crypt_str("Seed"), &selected_entry.seed, 1, 100))
						SkinChanger::scheduleHudUpdate();

					if (ImGui::InputInt(crypt_str("StatTrak"), &selected_entry.stat_trak, 1, 15))
						SkinChanger::scheduleHudUpdate();

					if (ImGui::SliderFloat(crypt_str("Wear"), &selected_entry.wear, 0.0f, 1.0f))
						drugs = true;
					else if (drugs)
					{
						SkinChanger::scheduleHudUpdate();
						drugs = false;
					}

					ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 6);
					ImGui::Text(crypt_str("Quality"));
					ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 5);
					if (ImGui::Combo(crypt_str("##Quality_combo"), &selected_entry.entity_quality_vector_index, [](void* data, int idx, const char** out_text)
						{
							*out_text = game_data::quality_names[idx].name;
							return true;
						}, nullptr, IM_ARRAYSIZE(game_data::quality_names)))
						SkinChanger::scheduleHudUpdate();

						if (current_profile != 1)
						{
							if (!g_cfg.skins.custom_name_tag[current_profile].empty())
								strcpy_s(selected_entry.custom_name, sizeof(selected_entry.custom_name), g_cfg.skins.custom_name_tag[current_profile].c_str());

							ImGui::Text(crypt_str("Name Tag"));
							ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.f);

							if (ImGui::InputText(crypt_str("##nametag"), selected_entry.custom_name, sizeof(selected_entry.custom_name)))
							{
								g_cfg.skins.custom_name_tag[current_profile] = selected_entry.custom_name;
								SkinChanger::scheduleHudUpdate();
							}

							ImGui::PopStyleVar();
						}

						ImGui::PopItemWidth();

						ImGui::EndGroup();

						ImGui::SameLine();
						ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 286 - 200);

						ImGui::BeginGroup();
						if (ImGui::ImageButton(all_skins[current_profile], ImVec2(190, 155)))
						{
							// maybe i will do smth later where, who knows :/
						}

						if (ImGui::CustomButton(crypt_str("Close"), crypt_str("##CLOSE__SKING"), ImVec2(198, 26)))
						{
							// start animation
							active_animation = true;
							next_id = -1;
							leave = true;
						}
						ImGui::EndGroup();

						// update element
						selected_entry.update();

						// we need to reset profile in the end to prevent render images with massive's index == -1
						if (leave && (preview_reverse || !active_animation))
						{
							ImGui::SetScrollY(0);
							current_profile = next_id;
							leave = false;
						}

				}
			}
			ImGui::EndChild();
			tab_end();

		}
		ImGui::EndChild();
	}
}

void c_menu::draw_tabs_players()
{
	const char* players_sections[3] = { crypt_str("Enemy"), crypt_str("Team"), crypt_str("Local") };
	draw_semitabs(players_sections, 3, players_section, ImGui::GetStyle());
}

void c_menu::draw_players(int child)
{
	auto player = players_section;

	if (!child)
	{
		child_title(crypt_str("ESP"));

		tab_start();
		ImGui::BeginChild(crypt_str("##ESP1_FIRST"), ImVec2(317, (child_height - 35)));
		{

			ImGui::Checkbox(crypt_str("Enable"), &g_cfg.player.enable);

			if (player == ENEMY)
			{
				ImGui::Checkbox(crypt_str("OOF arrows"), &g_cfg.player.arrows);
				ImGui::SameLine();
				ImGui::ColorEdit(crypt_str("##arrowscolor"), &g_cfg.player.arrows_color, ALPHA);

				if (g_cfg.player.arrows)
				{
					ImGui::SliderInt(crypt_str("Arrows distance"), &g_cfg.player.distance, 1, 100);
					ImGui::SliderInt(crypt_str("Arrows size"), &g_cfg.player.size, 1, 100);
				}
			}

			ImGui::Checkbox(crypt_str("Bounding box"), &g_cfg.player.type[player].box);
			ImGui::SameLine();
			ImGui::ColorEdit(crypt_str("##boxcolor"), &g_cfg.player.type[player].box_color, ALPHA);

			ImGui::Checkbox(crypt_str("Name"), &g_cfg.player.type[player].name);
			ImGui::SameLine();
			ImGui::ColorEdit(crypt_str("##namecolor"), &g_cfg.player.type[player].name_color, ALPHA);

			ImGui::Checkbox(crypt_str("Health bar"), &g_cfg.player.type[player].health);
			ImGui::Checkbox(crypt_str("Health color"), &g_cfg.player.type[player].custom_health_color);
			ImGui::SameLine();
			ImGui::ColorEdit(crypt_str("##healthcolor"), &g_cfg.player.type[player].health_color, ALPHA);

			for (auto i = 0, j = 0; i < ARRAYSIZE(flags); i++)
			{
				if (g_cfg.player.type[player].flags[i])
				{
					if (j)
						preview += crypt_str(", ") + (std::string)flags[i];
					else
						preview = flags[i];

					j++;
				}
			}

			draw_multicombo(crypt_str("Flags"), g_cfg.player.type[player].flags, flags, ARRAYSIZE(flags), preview);
			draw_multicombo(crypt_str("Weapon"), g_cfg.player.type[player].weapon, weaponplayer, ARRAYSIZE(weaponplayer), preview);


			if (g_cfg.player.type[player].weapon[WEAPON_ICON] || g_cfg.player.type[player].weapon[WEAPON_TEXT])
			{
				ImGui::Text(crypt_str("Color "));
				ImGui::SameLine();
				ImGui::ColorEdit(crypt_str("##weapcolor"), &g_cfg.player.type[player].weapon_color, ALPHA);
			}

			ImGui::Checkbox(crypt_str("Skeleton"), &g_cfg.player.type[player].skeleton);
			ImGui::SameLine();
			ImGui::ColorEdit(crypt_str("##skeletoncolor"), &g_cfg.player.type[player].skeleton_color, ALPHA);

			ImGui::Checkbox(crypt_str("Ammo bar"), &g_cfg.player.type[player].ammo);
			ImGui::SameLine();
			ImGui::ColorEdit(crypt_str("##ammocolor"), &g_cfg.player.type[player].ammobar_color, ALPHA);

			ImGui::Checkbox(crypt_str("Footsteps"), &g_cfg.player.type[player].footsteps);
			ImGui::SameLine();
			ImGui::ColorEdit(crypt_str("##footstepscolor"), &g_cfg.player.type[player].footsteps_color, ALPHA);

			if (g_cfg.player.type[player].footsteps)
			{
				ImGui::SliderInt(crypt_str("Thickness"), &g_cfg.player.type[player].thickness, 1, 10);
				ImGui::SliderInt(crypt_str("Radius"), &g_cfg.player.type[player].radius, 50, 500);
			}

			if (player == ENEMY || player == TEAM)
			{
				ImGui::Checkbox(crypt_str("Snap lines"), &g_cfg.player.type[player].snap_lines);
				ImGui::SameLine();
				ImGui::ColorEdit(crypt_str("##snapcolor"), &g_cfg.player.type[player].snap_lines_color, ALPHA);

				if (player == ENEMY)
				{
					if (g_cfg.ragebot.enable)
					{
						ImGui::Checkbox(crypt_str("Aimbot points"), &g_cfg.player.show_multi_points);
						ImGui::SameLine();
						ImGui::ColorEdit(crypt_str("##showmultipointscolor"), &g_cfg.player.show_multi_points_color, ALPHA);
					}

					ImGui::Checkbox(crypt_str("Aimbot hitboxes"), &g_cfg.player.lag_hitbox);
					ImGui::SameLine();
					ImGui::ColorEdit(crypt_str("##lagcompcolor"), &g_cfg.player.lag_hitbox_color, ALPHA);
				}
			}
			else
			{
				draw_combo(crypt_str("Player model T"), g_cfg.player.player_model_t, player_model, ARRAYSIZE(player_model));
				padding(0, 3);
				draw_combo(crypt_str("Player model CT"), g_cfg.player.player_model_ct, player_model, ARRAYSIZE(player_model));
			}

		}
		ImGui::EndChild();
		tab_end();
	}
	else
	{
		child_title(crypt_str("Model"));

		tab_start();
		ImGui::BeginChild(crypt_str("##ESP2_SECOND"), ImVec2(317, (child_height - 35)));
		{
			ImGui::Spacing();
			if (player == LOCAL)
				draw_combo(crypt_str("Type"), g_cfg.player.local_chams_type, local_chams_type, ARRAYSIZE(local_chams_type));

			if (player != LOCAL || !g_cfg.player.local_chams_type)
				draw_multicombo(crypt_str("Chams"), g_cfg.player.type[player].chams, g_cfg.player.type[player].chams[PLAYER_CHAMS_VISIBLE] ? chamsvisact : chamsvis, g_cfg.player.type[player].chams[PLAYER_CHAMS_VISIBLE] ? ARRAYSIZE(chamsvisact) : ARRAYSIZE(chamsvis), preview);

			if (g_cfg.player.type[player].chams[PLAYER_CHAMS_VISIBLE] || player == LOCAL && g_cfg.player.local_chams_type)
			{
				if (player == LOCAL && g_cfg.player.local_chams_type)
				{
					ImGui::Checkbox(crypt_str("Enable desync chams"), &g_cfg.player.fake_chams_enable);
					ImGui::Checkbox(crypt_str("Visualize lag"), &g_cfg.player.visualize_lag);
					ImGui::Checkbox(crypt_str("Layered"), &g_cfg.player.layered);

					draw_combo(crypt_str("Player chams material"), g_cfg.player.fake_chams_type, chamstype, ARRAYSIZE(chamstype));

					ImGui::Text(crypt_str("Color "));
					ImGui::SameLine();
					ImGui::ColorEdit(crypt_str("##fakechamscolor"), &g_cfg.player.fake_chams_color, ALPHA);

					if (g_cfg.player.fake_chams_type != 6)
					{
						ImGui::Checkbox(crypt_str("Double material "), &g_cfg.player.fake_double_material);
						ImGui::SameLine();
						ImGui::ColorEdit(crypt_str("##doublematerialcolor"), &g_cfg.player.fake_double_material_color, ALPHA);
					}

					ImGui::Checkbox(crypt_str("Animated material"), &g_cfg.player.fake_animated_material);
					ImGui::SameLine();
					ImGui::ColorEdit(crypt_str("##animcolormat"), &g_cfg.player.fake_animated_material_color, ALPHA);
				}
				else
				{
					draw_combo(crypt_str("Player chams material"), g_cfg.player.type[player].chams_type, chamstype, ARRAYSIZE(chamstype));

					if (g_cfg.player.type[player].chams[PLAYER_CHAMS_VISIBLE])
					{
						ImGui::Text(crypt_str("Visible "));
						ImGui::SameLine();
						ImGui::ColorEdit(crypt_str("##chamsvisible"), &g_cfg.player.type[player].chams_color, ALPHA);
					}

					if (g_cfg.player.type[player].chams[PLAYER_CHAMS_VISIBLE] && g_cfg.player.type[player].chams[PLAYER_CHAMS_INVISIBLE])
					{
						ImGui::Text(crypt_str("Invisible "));
						ImGui::SameLine();
						ImGui::ColorEdit(crypt_str("##chamsinvisible"), &g_cfg.player.type[player].xqz_color, ALPHA);
					}

					if (g_cfg.player.type[player].chams_type != 6)
					{
						ImGui::Checkbox(crypt_str("Double material "), &g_cfg.player.type[player].double_material);
						ImGui::SameLine();
						ImGui::ColorEdit(crypt_str("##doublematerialcolor"), &g_cfg.player.type[player].double_material_color, ALPHA);
					}

					ImGui::Checkbox(crypt_str("Animated material"), &g_cfg.player.type[player].animated_material);
					ImGui::SameLine();
					ImGui::ColorEdit(crypt_str("##animcolormat"), &g_cfg.player.type[player].animated_material_color, ALPHA);

					if (player == ENEMY)
					{
						ImGui::Checkbox(crypt_str("Backtrack chams"), &g_cfg.player.backtrack_chams);

						if (g_cfg.player.backtrack_chams)
						{
							draw_combo(crypt_str("Backtrack chams material"), g_cfg.player.backtrack_chams_material, chamstype, ARRAYSIZE(chamstype));

							ImGui::Text(crypt_str("Color "));
							ImGui::SameLine();
							ImGui::ColorEdit(crypt_str("##backtrackcolor"), &g_cfg.player.backtrack_chams_color, ALPHA);
						}
					}
				}
			}

			if (player == ENEMY || player == TEAM)
			{
				ImGui::Checkbox(crypt_str("Ragdoll chams"), &g_cfg.player.type[player].ragdoll_chams);

				if (g_cfg.player.type[player].ragdoll_chams)
				{
					draw_combo(crypt_str("Ragdoll chams material"), g_cfg.player.type[player].ragdoll_chams_material, chamstype, ARRAYSIZE(chamstype));

					ImGui::Text(crypt_str("Color "));
					ImGui::SameLine();
					ImGui::ColorEdit(crypt_str("##ragdollcolor"), &g_cfg.player.type[player].ragdoll_chams_color, ALPHA);
				}
			}
			else if (!g_cfg.player.local_chams_type)
			{
				ImGui::Checkbox(crypt_str("Transparency in scope"), &g_cfg.player.transparency_in_scope);

				if (g_cfg.player.transparency_in_scope)
					ImGui::SliderFloat(crypt_str("Alpha"), &g_cfg.player.transparency_in_scope_amount, 0.0f, 1.0f);
			}

			ImGui::Spacing();

			ImGui::Checkbox(crypt_str("Glow"), &g_cfg.player.type[player].glow);

			if (g_cfg.player.type[player].glow)
			{
				draw_combo(crypt_str("Glow type"), g_cfg.player.type[player].glow_type, glowtype, ARRAYSIZE(glowtype));
				ImGui::Text(crypt_str("Color "));
				ImGui::SameLine();
				ImGui::ColorEdit(crypt_str("##glowcolor"), &g_cfg.player.type[player].glow_color, ALPHA);
			}

		}
		ImGui::EndChild();
		tab_end();
	}
}


void c_menu::draw_tabs_misc()
{
	const char* misc_sections[2] = { crypt_str("Main"), crypt_str("Additives") };
	draw_semitabs(misc_sections, 2, misc_section, ImGui::GetStyle());
}

void c_menu::draw_misc(int child)
{
	if (!child)
	{
		if (!misc_section)
		{
			child_title(crypt_str("General"));

			tab_start();
			ImGui::BeginChild(crypt_str("##MISC1_FIRST"), ImVec2(317, (child_height - 35)));
			{
				ImGui::Checkbox(crypt_str("Anti-untrusted"), &g_cfg.misc.anti_untrusted);
				ImGui::Checkbox(crypt_str("Rank reveal"), &g_cfg.misc.rank_reveal);
				ImGui::Checkbox(crypt_str("Unlock inventory access"), &g_cfg.misc.inventory_access);
				ImGui::SliderInt(crypt_str("Ragdolls X"), &g_cfg.misc.ragdolls_x, 1, 10);
				ImGui::SliderInt(crypt_str("Ragdolls Y"), &g_cfg.misc.ragdolls_y, 1, 10);
				ImGui::SliderInt(crypt_str("Ragdolls Z"), &g_cfg.misc.ragdolls_z, 1, 10);
				ImGui::Checkbox(crypt_str("Preserve killfeed"), &g_cfg.esp.preserve_killfeed);
				ImGui::Checkbox(crypt_str("Aspect ratio"), &g_cfg.misc.aspect_ratio);

				if (g_cfg.misc.aspect_ratio)
				{
					padding(0, -5);
					ImGui::SliderFloat(crypt_str("Amount"), &g_cfg.misc.aspect_ratio_amount, 1.0f, 2.0f);
				}

				ImGui::Checkbox(crypt_str("Fake lag"), &g_cfg.antiaim.fakelag);
				if (g_cfg.antiaim.fakelag)
				{
					draw_combo(crypt_str("Fake lag type"), g_cfg.antiaim.fakelag_type, fakelags, ARRAYSIZE(fakelags));
					ImGui::SliderInt(crypt_str("Limit"), &g_cfg.antiaim.fakelag_limit, 1, 16);

					draw_multicombo(crypt_str("Fake lag triggers"), g_cfg.antiaim.fakelag_enablers, lagstrigger, ARRAYSIZE(lagstrigger), preview);

					auto enabled_fakelag_triggers = false;

					for (auto i = 0; i < ARRAYSIZE(lagstrigger); i++)
						if (g_cfg.antiaim.fakelag_enablers[i])
							enabled_fakelag_triggers = true;

					if (enabled_fakelag_triggers)
						ImGui::SliderInt(crypt_str("Triggers limit"), &g_cfg.antiaim.triggers_fakelag_limit, 1, 16);
				}
			}
			ImGui::EndChild();
			tab_end();
		}
		else
		{
			child_title(crypt_str("Information"));
			tab_start();

			ImGui::BeginChild(crypt_str("##MISC1_SECOND"), ImVec2(317, (child_height - 35)));
			{

				ImGui::Checkbox(crypt_str("Watermark"), &g_cfg.menu.watermark);
				ImGui::Checkbox(crypt_str("Spectators list"), &g_cfg.misc.spectators_list);
				draw_combo(crypt_str("Hitsound"), g_cfg.esp.hitsound, sounds, ARRAYSIZE(sounds));
				ImGui::Checkbox(crypt_str("Killsound"), &g_cfg.esp.killsound);
				draw_multicombo(crypt_str("Logs"), g_cfg.misc.events_to_log, events, ARRAYSIZE(events), preview);
				padding(0, 3);
				draw_multicombo(crypt_str("Logs output"), g_cfg.misc.log_output, events_output, ARRAYSIZE(events_output), preview);

				if (g_cfg.misc.events_to_log[EVENTLOG_HIT] || g_cfg.misc.events_to_log[EVENTLOG_ITEM_PURCHASES] || g_cfg.misc.events_to_log[EVENTLOG_BOMB])
				{
					ImGui::Text(crypt_str("Color "));
					ImGui::SameLine();
					ImGui::ColorEdit(crypt_str("##logcolor"), &g_cfg.misc.log_color, ALPHA);
				}

				ImGui::Checkbox(crypt_str("Show CS:GO logs"), &g_cfg.misc.show_default_log);

			}
			ImGui::EndChild();

			tab_end();
		}
	}
	else
	{
		if (!misc_section)
		{
			child_title(crypt_str("Movement"));

			tab_start();
			ImGui::BeginChild(crypt_str("##MISC2_FIRST"), ImVec2(317, (child_height - 35)));
			{

				ImGui::Checkbox(crypt_str("Automatic jump"), &g_cfg.misc.bunnyhop);
				draw_combo(crypt_str("Automatic strafes"), g_cfg.misc.airstrafe, strafes, ARRAYSIZE(strafes));
				ImGui::Checkbox(crypt_str("Crouch in air"), &g_cfg.misc.crouch_in_air);
				ImGui::Checkbox(crypt_str("Fast stop"), &g_cfg.misc.fast_stop);
				ImGui::Checkbox(crypt_str("Slide walk"), &g_cfg.misc.slidewalk);
				ImGui::Checkbox(crypt_str("No duck cooldown"), &g_cfg.misc.noduck);

				if (g_cfg.misc.noduck)
					draw_keybind(crypt_str("Fake duck"), &g_cfg.misc.fakeduck_key, crypt_str("##FAKEDUCK__HOTKEY"));

				draw_keybind(crypt_str("Slow walk"), &g_cfg.misc.slowwalk_key, crypt_str("##SLOWWALK__HOTKEY"));
				draw_keybind(crypt_str("Auto peek"), &g_cfg.misc.automatic_peek, crypt_str("##AUTOPEEK__HOTKEY"));
				draw_keybind(crypt_str("Edge jump"), &g_cfg.misc.edge_jump, crypt_str("##EDGEJUMP__HOTKEY"));

			}
			ImGui::EndChild();

			tab_end();
		}
		else
		{
			child_title(crypt_str("Extra"));

			tab_start();
			ImGui::BeginChild(crypt_str("##MISC2_SECOND"), ImVec2(317, (child_height - 35)));
			{

				ImGui::Checkbox(crypt_str("Anti-screenshot"), &g_cfg.misc.anti_screenshot);
				ImGui::Checkbox(crypt_str("Enable buybot"), &g_cfg.misc.buybot_enable);

				if (g_cfg.misc.buybot_enable)
				{
					draw_combo(crypt_str("Snipers"), g_cfg.misc.buybot1, mainwep, ARRAYSIZE(mainwep));
					padding(0, 3);
					draw_combo(crypt_str("Pistols"), g_cfg.misc.buybot2, secwep, ARRAYSIZE(secwep));
					padding(0, 3);
					draw_multicombo(crypt_str("Other"), g_cfg.misc.buybot3, grenades, ARRAYSIZE(grenades), preview);
				}
			}

			ImGui::EndChild();
			tab_end();
		}
	}
}

void c_menu::draw_settings(int child)
{
	child_title(crypt_str("Configs"));
	tab_start();

	ImGui::BeginChild(crypt_str("##CONFIGS__FIRST"), ImVec2(686, (child_height - 35)));
	{
		ImGui::BeginChild(crypt_str("##CONFIGS__FIRST_POSHELNAHUI"), ImVec2(686, (child_height - 35)));
		ImGui::PushItemWidth(629);

		static auto should_update = true;

		if (should_update)
		{
			should_update = false;

			cfg_manager->config_files();
			files = cfg_manager->files;

			for (auto& current : files)
				if (current.size() > 2)
					current.erase(current.size() - 3, 3);
		}

		if (ImGui::CustomButton(crypt_str("Open configs folder"), crypt_str("##CONFIG__FOLDER"), ImVec2(629, 26)))
		{
			std::string folder;

			auto get_dir = [&folder]() -> void
			{
				static TCHAR path[MAX_PATH];

				if (SUCCEEDED(SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, NULL, path)))
					folder = std::string(path) + crypt_str("\\skeet.pw\\Configs\\");
			};

			get_dir();
			ShellExecute(NULL, crypt_str("open"), folder.c_str(), NULL, NULL, SW_SHOWNORMAL);
		}

		ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.f);
		ImGui::ListBoxConfigArray(crypt_str("##CONFIGS"), &g_cfg.selected_config, files, 7);
		ImGui::PopStyleVar();

		if (ImGui::CustomButton(crypt_str("Refresh configs"), crypt_str("##CONFIG__REFRESH"), ImVec2(629, 26)))
		{
			cfg_manager->config_files();
			files = cfg_manager->files;

			for (auto& current : files)
				if (current.size() > 2)
					current.erase(current.size() - 3, 3);
		}

		static char config_name[64] = "\0";

		ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.f);
		ImGui::InputText(crypt_str("##configname"), config_name, sizeof(config_name));
		ImGui::PopStyleVar();

		if (ImGui::CustomButton(crypt_str("Create config"), crypt_str("##CONFIG__CREATE"), ImVec2(629, 26)))
		{
			g_cfg.new_config_name = config_name;
			add_config();
		}

		static auto next_save = false;
		static auto prenext_save = false;
		static auto clicked_sure = false;
		static auto save_time = m_globals()->m_realtime;
		static auto save_alpha = 1.0f;

		save_alpha = math::clamp(save_alpha + (4.f * ImGui::GetIO().DeltaTime * (!prenext_save ? 1.f : -1.f)), 0.01f, 1.f);
		ImGui::PushStyleVar(ImGuiStyleVar_Alpha, save_alpha * public_alpha * (1.f - preview_alpha));

		if (!next_save)
		{
			clicked_sure = false;

			if (prenext_save && save_alpha <= 0.01f)
				next_save = true;

			if (ImGui::CustomButton(crypt_str("Save config"), crypt_str("##CONFIG__SAVE"), ImVec2(629, 26)))
			{
				save_time = m_globals()->m_realtime;
				prenext_save = true;
			}
		}
		else
		{
			if (prenext_save && save_alpha <= 0.01f)
			{
				prenext_save = false;
				next_save = !clicked_sure;
			}

			if (ImGui::CustomButton(crypt_str("Are you sure?"), crypt_str("##AREYOUSURE__SAVE"), ImVec2(629, 26)))
			{
				save_config();
				prenext_save = true;
				clicked_sure = true;
			}

			if (!clicked_sure && m_globals()->m_realtime > save_time + 1.5f)
			{
				prenext_save = true;
				clicked_sure = true;
			}
		}

		ImGui::PopStyleVar();

		if (ImGui::CustomButton(crypt_str("Load config"), crypt_str("##CONFIG__LOAD"), ImVec2(629, 26)))
			load_config();

		static auto next_delete = false;
		static auto prenext_delete = false;
		static auto clicked_sure_del = false;
		static auto delete_time = m_globals()->m_realtime;
		static auto delete_alpha = 1.0f;

		delete_alpha = math::clamp(delete_alpha + (4.f * ImGui::GetIO().DeltaTime * (!prenext_delete ? 1.f : -1.f)), 0.01f, 1.f);
		ImGui::PushStyleVar(ImGuiStyleVar_Alpha, delete_alpha * public_alpha * (1.f - preview_alpha));

		if (!next_delete)
		{
			clicked_sure_del = false;

			if (prenext_delete && delete_alpha <= 0.01f)
				next_delete = true;

			if (ImGui::CustomButton(crypt_str("Remove config"), crypt_str("##CONFIG__delete"), ImVec2(629, 26)))
			{
				delete_time = m_globals()->m_realtime;
				prenext_delete = true;
			}
		}
		else
		{
			if (prenext_delete && delete_alpha <= 0.01f)
			{
				prenext_delete = false;
				next_delete = !clicked_sure_del;
			}

			if (ImGui::CustomButton(crypt_str("Are you sure?"), crypt_str("##AREYOUSURE__delete"), ImVec2(629, 26)))
			{
				remove_config();
				prenext_delete = true;
				clicked_sure_del = true;
			}

			if (!clicked_sure_del && m_globals()->m_realtime > delete_time + 1.5f)
			{
				prenext_delete = true;
				clicked_sure_del = true;
			}
		}

		ImGui::PopStyleVar();
		ImGui::PopItemWidth();

		ImGui::EndChild();
		ImGui::SetWindowFontScale(1.f);
	}
	ImGui::EndChild();

	ImGui::PopStyleVar();
}

void c_menu::draw_lua(int child)
{
	if (!child)
	{
		child_title(crypt_str("Scripts"));
		tab_start();

		ImGui::BeginChild(crypt_str("##LUA_FIRST"), ImVec2(317, (child_height - 35)));
		{
			ImGui::PushItemWidth(291);
			static auto should_update = true;

			if (should_update)
			{
				should_update = false;
				scripts = c_lua::get().scripts;

				for (auto& current : scripts)
				{
					if (current.size() >= 5 && current.at(current.size() - 1) == 'c')
						current.erase(current.size() - 5, 5);
					else if (current.size() >= 4)
						current.erase(current.size() - 4, 4);
				}
			}

			if (ImGui::CustomButton(crypt_str("Open scripts folder"), crypt_str("##LUAS__FOLDER"), ImVec2(291, 26)))
			{
				std::string folder;

				auto get_dir = [&folder]() -> void
				{
					static TCHAR path[MAX_PATH];

					if (SUCCEEDED(SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, NULL, path)))
						folder = std::string(path) + crypt_str("\\skeet.pw\\Scripts\\");
				};

				get_dir();
				ShellExecute(NULL, crypt_str("open"), folder.c_str(), NULL, NULL, SW_SHOWNORMAL);
			}

			ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.f);

			if (scripts.empty())
				ImGui::ListBoxConfigArray(crypt_str("##LUAS"), &selected_script, scripts, 7);
			else
			{
				auto backup_scripts = scripts;

				for (auto& script : scripts)
				{
					auto script_id = c_lua::get().get_script_id(script + crypt_str(".lua"));

					if (script_id == -1)
						continue;

					if (c_lua::get().loaded.at(script_id))
						scripts.at(script_id) += crypt_str(" [loaded]");
				}

				ImGui::ListBoxConfigArray(crypt_str("##LUAS"), &selected_script, scripts, 7);
				scripts = std::move(backup_scripts);
			}

			ImGui::PopStyleVar();

			if (ImGui::CustomButton(crypt_str("Refresh scripts"), crypt_str("##LUA__REFRESH"), ImVec2(291, 26)))
			{
				c_lua::get().refresh_scripts();
				scripts = c_lua::get().scripts;

				if (selected_script >= scripts.size())
					selected_script = scripts.size() - 1;

				for (auto& current : scripts)
				{
					if (current.size() >= 5 && current.at(current.size() - 1) == 'c')
						current.erase(current.size() - 5, 5);
					else if (current.size() >= 4)
						current.erase(current.size() - 4, 4);
				}
			}

			if (ImGui::CustomButton(crypt_str("Edit script"), crypt_str("##LUA__EDIT"), ImVec2(291, 26)))
			{
				loaded_editing_script = false;
				editing_script = scripts.at(selected_script);
			}

			if (ImGui::CustomButton(crypt_str("Load script"), crypt_str("##SCRIPTS__LOAD"), ImVec2(291, 26)))
			{
				c_lua::get().load_script(selected_script);
				c_lua::get().refresh_scripts();

				scripts = c_lua::get().scripts;

				if (selected_script >= scripts.size())
					selected_script = scripts.size() - 1;

				for (auto& current : scripts)
				{
					if (current.size() >= 5 && current.at(current.size() - 1) == 'c')
						current.erase(current.size() - 5, 5);
					else if (current.size() >= 4)
						current.erase(current.size() - 4, 4);
				}

				eventlogs::get().add(crypt_str("Loaded ") + scripts.at(selected_script) + crypt_str(".lua"), false);
			}

			if (ImGui::CustomButton(crypt_str("Unload script"), crypt_str("##SCRIPTS__UNLOAD"), ImVec2(291, 26)))
			{
				c_lua::get().unload_script(selected_script);
				c_lua::get().refresh_scripts();

				scripts = c_lua::get().scripts;

				if (selected_script >= scripts.size())
					selected_script = scripts.size() - 1;

				for (auto& current : scripts)
				{
					if (current.size() >= 5 && current.at(current.size() - 1) == 'c')
						current.erase(current.size() - 5, 5);
					else if (current.size() >= 4)
						current.erase(current.size() - 4, 4);
				}

				eventlogs::get().add(crypt_str("Unloaded ") + scripts.at(selected_script) + crypt_str(".lua"), false);
			}

			if (ImGui::CustomButton(crypt_str("Reload all scripts"), crypt_str("##SCRIPTS__RELOAD"), ImVec2(291, 26)))
			{
				c_lua::get().reload_all_scripts();
				c_lua::get().refresh_scripts();

				scripts = c_lua::get().scripts;

				if (selected_script >= scripts.size())
					selected_script = scripts.size() - 1;

				for (auto& current : scripts)
				{
					if (current.size() >= 5 && current.at(current.size() - 1) == 'c')
						current.erase(current.size() - 5, 5);
					else if (current.size() >= 4)
						current.erase(current.size() - 4, 4);
				}
			}

			if (ImGui::CustomButton(crypt_str("Unload all scripts"), crypt_str("##SCRIPTS__UNLOADALL"), ImVec2(291, 26)))
			{
				c_lua::get().unload_all_scripts();
				c_lua::get().refresh_scripts();

				scripts = c_lua::get().scripts;

				if (selected_script >= scripts.size())
					selected_script = scripts.size() - 1;

				for (auto& current : scripts)
				{
					if (current.size() >= 5 && current.at(current.size() - 1) == 'c')
						current.erase(current.size() - 5, 5);
					else if (current.size() >= 4)
						current.erase(current.size() - 4, 4);
				}
			}

			ImGui::PopItemWidth();
		}

		ImGui::EndChild();
		tab_end();
	}
	else
	{
		child_title(crypt_str("Script elements"));
		tab_start();

		ImGui::BeginChild(crypt_str("##LUA_SECOND"), ImVec2(317, (child_height - 35)));
		{
			ImGui::Checkbox(crypt_str("Developer mode"), &g_cfg.scripts.developer_mode);
			ImGui::Checkbox(crypt_str("Allow HTTP requests"), &g_cfg.scripts.allow_http);
			ImGui::Checkbox(crypt_str("Allow files read or write"), &g_cfg.scripts.allow_file);
			ImGui::Spacing();

			auto previous_check_box = false;

			for (auto& current : c_lua::get().scripts)
			{
				auto& items = c_lua::get().items.at(c_lua::get().get_script_id(current));

				for (auto& item : items)
				{
					std::string item_name;

					auto first_point = false;
					auto item_str = false;

					for (auto& c : item.first)
					{
						if (c == '.')
						{
							if (first_point)
							{
								item_str = true;
								continue;
							}
							else
								first_point = true;
						}

						if (item_str)
							item_name.push_back(c);
					}

					switch (item.second.type)
					{
					case NEXT_LINE:
						previous_check_box = false;
						break;
					case CHECK_BOX:
						previous_check_box = true;
						ImGui::Checkbox(item_name.c_str(), &item.second.check_box_value);
						break;
					case COMBO_BOX:
						previous_check_box = false;
						draw_combo(item_name.c_str(), item.second.combo_box_value, [](void* data, int idx, const char** out_text)
							{
								auto labels = (std::vector <std::string>*)data;
								*out_text = labels->at(idx).c_str();
								return true;
							}, &item.second.combo_box_labels, item.second.combo_box_labels.size());
						break;
					case SLIDER_INT:
						previous_check_box = false;
						ImGui::SliderInt(item_name.c_str(), &item.second.slider_int_value, item.second.slider_int_min, item.second.slider_int_max);
						break;
					case SLIDER_FLOAT:
						previous_check_box = false;
						ImGui::SliderFloat(item_name.c_str(), &item.second.slider_float_value, item.second.slider_float_min, item.second.slider_float_max);
						break;
					case COLOR_PICKER:
						if (previous_check_box)
							previous_check_box = false;
						else
							ImGui::Text((item_name + ' ').c_str());

						ImGui::SameLine();
						ImGui::ColorEdit((crypt_str("##") + item_name).c_str(), &item.second.color_picker_value, ALPHA, true);
						break;
					}
				}
			}
		}

		ImGui::EndChild();
		tab_end();
	}
}


void c_menu::draw_radar(int child)
{
	if (!child)
	{
		child_title(crypt_str("Radar"));
		tab_start();

		ImGui::BeginChild(crypt_str("##RADAR_FIRST"), ImVec2(317, (child_height - 35)));
		{
			ImGui::Checkbox(crypt_str("Enable radar"), &g_cfg.misc.ingame_radar);
			ImGui::Checkbox(crypt_str("Render local"), &g_cfg.radar.render_local);
			ImGui::Checkbox(crypt_str("Render enemy"), &g_cfg.radar.render_enemy);
			ImGui::Checkbox(crypt_str("Render team"), &g_cfg.radar.render_team);
			ImGui::Checkbox(crypt_str("Show planted c4"), &g_cfg.radar.render_planted_c4);
			ImGui::Checkbox(crypt_str("Show dropped c4"), &g_cfg.radar.render_dropped_c4);
			ImGui::Checkbox(crypt_str("Show health"), &g_cfg.radar.render_health);
		}
		ImGui::EndChild();

		tab_end();
	}
}

void c_menu::draw_player_list(int child)
{
	auto player = players_section;

	static std::vector <Player_list_data> players;

	if (!g_cfg.player_list.refreshing)
	{
		players.clear();

		for (auto player : g_cfg.player_list.players)
			players.emplace_back(player);
	}

	static auto current_player = 0;

	if (!child)
	{
		child_title(crypt_str("Players"));
		tab_start();

		ImGui::BeginChild(crypt_str("##ESP2_FIRST"), ImVec2(317, (child_height - 35)));
		{

			if (!players.empty())
			{
				std::vector <std::string> player_names;

				for (auto player : players)
					player_names.emplace_back(player.name);

				ImGui::PushItemWidth(291);
				ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.f);
				ImGui::ListBoxConfigArray(crypt_str("##PLAYERLIST"), &current_player, player_names, 14);
				ImGui::PopStyleVar();
				ImGui::PopItemWidth();
			}

		}
		ImGui::EndChild();
		tab_end();
	}
	else
	{
		child_title(crypt_str("Settings"));

		tab_start();
		ImGui::BeginChild(crypt_str("##ESP1_SECOND"), ImVec2(317, (child_height - 35)));
		{

			if (!players.empty())
			{
				if (current_player >= players.size())
					current_player = players.size() - 1;

				ImGui::Checkbox(crypt_str("White list"), &g_cfg.player_list.white_list[players.at(current_player).i]);

				if (!g_cfg.player_list.white_list[players.at(current_player).i])
				{
					ImGui::Checkbox(crypt_str("High priority"), &g_cfg.player_list.high_priority[players.at(current_player).i]);
					ImGui::Checkbox(crypt_str("Force safe points"), &g_cfg.player_list.force_safe_points[players.at(current_player).i]);
					ImGui::Checkbox(crypt_str("Force body aim"), &g_cfg.player_list.force_body_aim[players.at(current_player).i]);
					ImGui::Checkbox(crypt_str("Low delta"), &g_cfg.player_list.low_delta[players.at(current_player).i]);
					ImGui::Checkbox(crypt_str("Disable resolver"), &g_cfg.player_list.disable_resolver[players.at(current_player).i]);
				}
			}
		}
		ImGui::EndChild();
		tab_end();
	}
}

void c_menu::draw(bool is_open)
{
	static auto w = 0, h = 0, current_h = 0;
	m_engine()->GetScreenSize(w, current_h);

	if (h != current_h)
	{
		if (h)
			update_scripts = true;

		h = current_h;
		update_dpi = true;
	}

	// animation related code.
	static float m_alpha = 0.0002f;
	m_alpha = math::clamp(m_alpha + (3.f * ImGui::GetIO().DeltaTime * (is_open ? 1.f : -1.f)), 0.0001f, 1.f);

	// set alpha in class to use later in widgets.
	public_alpha = m_alpha;

	if (m_alpha <= 0.0001f)
		return;

	// set new alpha.
	ImGui::PushStyleVar(ImGuiStyleVar_Alpha, m_alpha);

	// setup colors and some styles.
	if (!menu_setupped)
		menu_setup(ImGui::GetStyle());

	ImGui::PushStyleColor(ImGuiCol_ScrollbarGrab, ImVec4(ImGui::GetStyle().Colors[ImGuiCol_ScrollbarGrab].x, ImGui::GetStyle().Colors[ImGuiCol_ScrollbarGrab].y, ImGui::GetStyle().Colors[ImGuiCol_ScrollbarGrab].z, m_alpha));

	// default menu size.
	const int x = 850, y = 521;

	// last active tab to switch effect & reverse alpha & preview alpha.
	// IMPORTANT: DO TAB SWITCHING BY LAST_TAB!!!!!
	static int last_tab = active_tab;
	static bool preview_reverse = false;

	// start menu render.
	ImGui::Begin(crypt_str("skeet.pw"), nullptr, ImGuiWindowFlags_NoDecoration);
	{
		auto p = ImGui::GetWindowPos();

		// set padding to draw { tabs , main functional area } / close group to draw last bottom child at x-0 pos.
		ImGui::SetCursorPos(ImVec2(10, 13));
		ImGui::BeginGroup();

		ImGui::BeginChild(crypt_str("##tabs_area"), ImVec2(124, 509)); // there will be tabs.
		{
			ImGui::PushFont(futura);
			ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(8.f, 12.f));

			ImGui::Spacing();
			//  we need some more checks to prevent over-switching tabs.
			if (ImGui::MainTab(crypt_str("Rage & AA"), ico_menu, crypt_str("R"), active_tab == 0, ImVec2(124, 0)) && last_tab == active_tab && !preview_reverse) active_tab = 0;
			if (ImGui::MainTab(crypt_str("Legit"), ico_menu, crypt_str("L"), active_tab == 1, ImVec2(124, 0)) && last_tab == active_tab && !preview_reverse) active_tab = 1;
			if (ImGui::MainTab(crypt_str("Visuals"), ico_menu, crypt_str("V"), active_tab == 2, ImVec2(124, 0)) && last_tab == active_tab && !preview_reverse) active_tab = 2;
			if (ImGui::MainTab(crypt_str("Players"), ico_menu, crypt_str("P"), active_tab == 3, ImVec2(124, 0)) && last_tab == active_tab && !preview_reverse) active_tab = 3;
			if (ImGui::MainTab(crypt_str("Misc"), ico_menu, crypt_str("M"), active_tab == 4, ImVec2(124, 0)) && last_tab == active_tab && !preview_reverse) active_tab = 4;
			if (ImGui::MainTab(crypt_str("Settings"), ico_menu, crypt_str("S"), active_tab == 5, ImVec2(124, 0)) && last_tab == active_tab && !preview_reverse) active_tab = 5;
			if (ImGui::MainTab(crypt_str("Player list"), ico_bottom, crypt_str("P"), active_tab == 6, ImVec2(124, 0)) && last_tab == active_tab && !preview_reverse) active_tab = 6;
			if (ImGui::MainTab(crypt_str("Scripts"), ico_bottom, crypt_str("L"), active_tab == 7, ImVec2(124, 0)) && last_tab == active_tab && !preview_reverse) active_tab = 7;
			if (ImGui::MainTab(crypt_str("Radar"), ico_bottom, crypt_str("R"), active_tab == 8, ImVec2(124, 0)) && last_tab == active_tab && !preview_reverse) active_tab = 8;

			ImGui::PopStyleVar();
			ImGui::PopFont();
		}
		ImGui::EndChild();

		ImGui::SameLine();

		// vertical separator after tabs.
		ImGui::GetWindowDrawList()->AddRectFilled(ImGui::GetCursorPos() + p + ImVec2(3, 0), ImGui::GetCursorPos() + p + ImVec2(4, 498), ImGui::ColorConvertFloat4ToU32(ImGui::GetStyle().Colors[ImGuiCol_Border]));

		if (last_tab != active_tab || preview_reverse)
		{
			if (!preview_reverse)
			{
				if (preview_alpha == 1.f)
					preview_reverse = true;

				preview_alpha = math::clamp(preview_alpha + (4.f * ImGui::GetIO().DeltaTime), 0.01f, 1.f);
			}
			else
			{
				last_tab = active_tab;
				if (preview_alpha == 0.01f)
				{
					preview_reverse = false;
				}

				preview_alpha = math::clamp(preview_alpha - (4.f * ImGui::GetIO().DeltaTime), 0.01f, 1.f);
			}
		}
		else
			preview_alpha = math::clamp(preview_alpha - (4.f * ImGui::GetIO().DeltaTime), 0.0f, 1.f);

		ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (12));

		ImGui::BeginChild(crypt_str("##content_area"), ImVec2(686, 498)); // there wiil be main cheat functional.
		{
			ImGui::PushFont(futura);

			if (last_tab > 4)
				child_height = 490;
			else
				child_height = 454;

			// push alpha-channel adjustment.
			ImGui::PushStyleVar(ImGuiStyleVar_Alpha, (1.f - preview_alpha) * m_alpha);

			// semitabs rendering.
			switch (last_tab % 9)
			{
			case 0: draw_tabs_ragebot(); break;
			case 2: draw_tabs_visuals(); break;
			case 3: draw_tabs_players(); break;
			case 4: draw_tabs_misc(); break;
			default: break;
			}

			ImGui::PopStyleVar();

			ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 6);

			ImGui::BeginGroup();  // groupping two childs to prevent multipadding.
			{
				ImGui::PushFont(futura_small);
				if (last_tab == 2 && visuals_section == 2)
				{
					draw_visuals(-1);
				}
				else if (last_tab == 5)
				{
					ImGui::PushStyleVar(ImGuiStyleVar_Alpha, (1.f - preview_alpha) * m_alpha);
					draw_settings(0);
				}
				else
				{
					ImGui::BeginChild(crypt_str("##content_first"), ImVec2(339, child_height)); // first functional child.
					{
						ImGui::PushStyleVar(ImGuiStyleVar_Alpha, (1.f - preview_alpha) * m_alpha);

						switch (last_tab % 9)
						{
						case 0: draw_ragebot(0); break;
						case 1: draw_legit(0); break;
						case 2: draw_visuals(0); break;
						case 3: draw_players(0); break;
						case 4: draw_misc(0); break;
						case 6: draw_player_list(0); break;
						case 7: draw_lua(0); break;
						case 8: draw_radar(0); break;
						default: break;
						}

					}
					ImGui::EndChild();

					ImGui::SameLine();

					ImGui::BeginChild(crypt_str("##content_second"), ImVec2(339, child_height)); // second functional child.
					{
						ImGui::PushStyleVar(ImGuiStyleVar_Alpha, (1.f - preview_alpha) * m_alpha);

						switch (last_tab % 9)
						{
						case 0: draw_ragebot(1); break;
						case 1: draw_legit(1); break;
						case 2: draw_visuals(1); break;
						case 3: draw_players(1); break;
						case 4: draw_misc(1); break;
						case 6: draw_player_list(1); break;
						case 7: draw_lua(1); break;
						case 8: draw_radar(1); break;
						default: break;
						}

					}
					ImGui::EndChild();

					if (!editing_script.empty())
						lua_edit(editing_script);
				}

				ImGui::PopFont();

			};
			ImGui::EndGroup();

			ImGui::PopFont();
		}
		ImGui::EndChild();

		ImGui::EndGroup();
	}
	ImGui::End();

	ImGui::PopStyleColor();
	ImGui::PopStyleVar();
}