// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "HTTPRequest.hpp"
#include "Clua.h"	
#include <Windows.h>
#include <any>
#include "..\cheats\visuals\other_esp.h"
#include "..\utils\csgo.hpp"
#include "..\cheats\misc\logs.h"
#include "..\includes.hpp"


void lua_panic(sol::optional <std::string> message)
{
	if (!message)
		return;

	auto log = crypt_str("Lua error: ") + message.value_or("unknown");
	eventlogs::get().add(log, false);
}

std::string get_current_script(sol::this_state s)
{
	sol::state_view lua_state(s);
	sol::table rs = lua_state["debug"]["getinfo"](2, ("S"));
	std::string source = rs["source"];
	std::string filename = std::filesystem::path(source.substr(1)).filename().string();

	return filename;
}

int get_current_script_id(sol::this_state s)
{
	return c_lua::get().get_script_id(get_current_script(s));
}

namespace ns_client
{
	void add_callback(sol::this_state s, std::string eventname, sol::protected_function func)
	{		
		if (eventname != crypt_str("on_paint") && eventname != crypt_str("on_createmove") && eventname != crypt_str("on_shot"))
		{
			eventlogs::get().add(crypt_str("Lua error: invalid callback \"") + eventname + '\"', false);
			return;
		}

		if (c_lua::get().loaded.at(get_current_script_id(s)))//new
		c_lua::get().hooks.registerHook(eventname, get_current_script_id(s), func);
	}

	void load_script(std::string name)
	{
		c_lua::get().refresh_scripts();
		c_lua::get().load_script(c_lua::get().get_script_id(name));
	}

	void unload_script(std::string name)
	{
		c_lua::get().refresh_scripts();
		c_lua::get().unload_script(c_lua::get().get_script_id(name));
	}

	void log(std::string text)
	{
		eventlogs::get().add(text, false);
	}
}

std::vector <std::pair <std::string, menu_item>>::iterator find_item(std::vector <std::pair <std::string, menu_item>>& items, const std::string& name)
{
	for (auto it = items.begin(); it != items.end(); ++it)
		if (it->first == name)
			return it;

	return items.end();
}

menu_item find_item(std::vector <std::vector <std::pair <std::string, menu_item>>>& scripts, const std::string& name)
{
	for (auto& script : scripts)
	{
		for (auto& item : script)
		{
			std::string item_name;

			auto first_point = false;
			auto second_point = false;

			for (auto& c : item.first)
			{
				if (c == '.')
				{
					if (first_point)
					{
						second_point = true;
						continue;
					}
					else
					{
						first_point = true;
						continue;
					}
				}

				if (!second_point)
					continue;

				item_name.push_back(c);
			}

			if (item_name == name)
				return item.second;
		}
	}

	return menu_item();
}

namespace ns_menu
{
	bool get_visible()
	{
		return hooks::menu_open;
	}

	void set_visible(bool visible)
	{
		hooks::menu_open = visible;
	}

	auto next_line_counter = 0;

	void next_line(sol::this_state s)
	{
		c_lua::get().items.at(get_current_script_id(s)).emplace_back(std::make_pair(crypt_str("next_line_") + std::to_string(next_line_counter), menu_item()));
		++next_line_counter;
	}

	void add_check_box(sol::this_state s, const std::string& name)
	{
		auto script = get_current_script(s);
		auto script_id = c_lua::get().get_script_id(script);

		auto& items = c_lua::get().items.at(script_id);
		auto full_name = script + '.' + name;

		if (find_item(items, full_name) != items.end())
			return;

		items.emplace_back(std::make_pair(full_name, menu_item(false)));
	}

	void add_combo_box(sol::this_state s, std::string name, std::vector <std::string> labels) //-V813
	{
		if (labels.empty())
			return;

		auto script = get_current_script(s);
		auto script_id = c_lua::get().get_script_id(script);

		auto& items = c_lua::get().items.at(script_id);
		auto full_name = script + '.' + name;

		if (find_item(items, full_name) != items.end())
			return;

		items.emplace_back(std::make_pair(full_name, menu_item(labels, 0)));
	}

	void add_slider_int(sol::this_state s, const std::string& name, int min, int max)
	{
		auto script = get_current_script(s);
		auto script_id = c_lua::get().get_script_id(script);

		auto& items = c_lua::get().items.at(script_id);
		auto full_name = script + '.' + name;

		if (find_item(items, full_name) != items.end())
			return;

		items.emplace_back(std::make_pair(full_name, menu_item(min, max, min)));
	}

	void add_slider_float(sol::this_state s, const std::string& name, float min, float max)
	{
		auto script = get_current_script(s);
		auto script_id = c_lua::get().get_script_id(script);

		auto& items = c_lua::get().items.at(script_id);
		auto full_name = script + '.' + name;

		if (find_item(items, full_name) != items.end())
			return;

		items.emplace_back(std::make_pair(full_name, menu_item(min, max, min)));
	}

	void add_color_picker(sol::this_state s, const std::string& name)
	{
		auto script = get_current_script(s);
		auto script_id = c_lua::get().get_script_id(script);

		auto& items = c_lua::get().items.at(script_id);
		auto full_name = script + '.' + name;

		if (find_item(items, full_name) != items.end())
			return;

		items.emplace_back(std::make_pair(full_name, menu_item(Color::White)));
	}

	std::unordered_map <std::string, bool> first_update;
	std::unordered_map <std::string, menu_item> stored_values;
	std::unordered_map <std::string, void*> config_items;

	bool find_config_item(std::string name, std::string type)
	{
		if (config_items.find(name) == config_items.end())
		{
			auto found = false;

			for (auto item : cfg_manager->items)
			{
				if (item->name == name)
				{
					if (item->type != type)
					{
						eventlogs::get().add(crypt_str("Lua error: invalid config item type, must be ") + type, false);
						return false;
					}

					found = true;
					config_items[name] = item->pointer;
					break;
				}
			}

			if (!found)
			{
				eventlogs::get().add(crypt_str("Lua error: cannot find config variable \"") + name + '\"', false);
				return false;
			}
		}

		return true;
	}

	bool get_bool(std::string name)
	{
		if (first_update.find(name) == first_update.end())
			first_update[name] = false;

		if (!hooks::menu_open && first_update[name])
		{
			if (stored_values.find(name) != stored_values.end())
				return stored_values[name].check_box_value;
			else if (config_items.find(name) != config_items.end())
				return *(bool*)config_items[name];
			else
				return false;
		}

		auto& it = find_item(c_lua::get().items, name);

		if (it.type == NEXT_LINE)
		{
			if (find_config_item(name, crypt_str("bool")))
				return *(bool*)config_items[name];

			eventlogs::get().add(crypt_str("Lua error: cannot find menu variable \"") + name + '\"', false);
			return false;
		}

		first_update[name] = true;
		stored_values[name] = it;

		return it.check_box_value;
	}

	int get_int(std::string name)
	{
		if (first_update.find(name) == first_update.end())
			first_update[name] = false;

		if (!hooks::menu_open && first_update[name])
		{
			if (stored_values.find(name) != stored_values.end())
				return stored_values[name].type == COMBO_BOX ? stored_values[name].combo_box_value : stored_values[name].slider_int_value;
			else if (config_items.find(name) != config_items.end())
				return *(int*)config_items[name];
			else
				return 0;
		}

		auto& it = find_item(c_lua::get().items, name);

		if (it.type == NEXT_LINE)
		{
			if (find_config_item(name, crypt_str("int")))
				return *(int*)config_items[name];

			eventlogs::get().add(crypt_str("Lua error: cannot find menu variable \"") + name + '\"', false);
			return 0;
		}

		first_update[name] = true;
		stored_values[name] = it;

		return it.type == COMBO_BOX ? it.combo_box_value : it.slider_int_value;
	}

	float get_float(std::string name)
	{
		if (first_update.find(name) == first_update.end())
			first_update[name] = false;

		if (!hooks::menu_open && first_update[name])
		{
			if (stored_values.find(name) != stored_values.end())
				return stored_values[name].slider_float_value;
			else if (config_items.find(name) != config_items.end())
				return *(float*)config_items[name];
			else
				return 0.0f;
		}

		auto& it = find_item(c_lua::get().items, name);

		if (it.type == NEXT_LINE)
		{
			if (find_config_item(name, crypt_str("float")))
				return *(float*)config_items[name];

			eventlogs::get().add(crypt_str("Lua error: cannot find menu variable \"") + name + '\"', false);
			return 0.0f;
		}

		first_update[name] = true;
		stored_values[name] = it;

		return it.slider_float_value;
	}

	Color get_color(std::string name)
	{
		if (first_update.find(name) == first_update.end())
			first_update[name] = false;

		if (!hooks::menu_open && first_update[name])
		{
			if (stored_values.find(name) != stored_values.end())
				return stored_values[name].color_picker_value;
			else if (config_items.find(name) != config_items.end())
				return *(Color*)config_items[name];
			else
				return Color::White;
		}

		auto& it = find_item(c_lua::get().items, name);

		if (it.type == NEXT_LINE)
		{
			if (find_config_item(name, crypt_str("Color")))
				return *(Color*)config_items[name];

			eventlogs::get().add(crypt_str("Lua error: cannot find menu variable \"") + name + '\"', false);
			return Color::White;
		}

		first_update[name] = true;
		stored_values[name] = it;

		return it.color_picker_value;
	}

	bool get_key_bind_state(int key_bind)
	{
		return key_binds::get().get_key_bind_state_lua(key_bind);
	}

	int get_key_bind_mode(int key_bind)
	{
		return key_binds::get().get_key_bind_mode(key_bind);
	}
	
	void set_bool(std::string name, bool value)
	{
		if (!find_config_item(name, crypt_str("bool")))
			return;

		*(bool*)config_items[name] = value;
	}

	void set_int(std::string name, int value)
	{
		if (!find_config_item(name, crypt_str("int")))
			return;

		*(int*)config_items[name] = value;
	}

	void set_float(std::string name, float value)
	{
		if (!find_config_item(name, crypt_str("float")))
			return;

		*(float*)config_items[name] = value;
	}

	void set_color(std::string name, Color value)
	{
		if (!find_config_item(name, crypt_str("Color")))
			return;

		*(Color*)config_items[name] = value;
	}
}

namespace ns_globals
{
	int get_framerate()
	{
		return g_ctx.globals.framerate;
	}

	int get_ping()
	{
		return g_ctx.globals.ping;
	}

	std::string get_server_address()
	{
		if (!m_engine()->IsInGame())
			return "Unknown";

		auto nci = m_engine()->GetNetChannelInfo();

		if (!nci)
			return "Unknown";

		auto server = nci->GetAddress();

		if (!strcmp(server, "loopback"))
			server = "Local server";
		else if (m_gamerules()->m_bIsValveDS())
			server = "Valve server";

		return server;
	}

	std::string get_time()
	{
		return g_ctx.globals.time;
	}

	std::string get_username()
	{
		return g_ctx.username;
	}

	float get_realtime()
	{
		return m_globals()->m_realtime;
	}

	float get_curtime()
	{
		return m_globals()->m_curtime;
	}

	float get_frametime()
	{
		return m_globals()->m_frametime;
	}

	int get_tickcount()
	{
		return m_globals()->m_tickcount;
	}

	int get_framecount()
	{
		return m_globals()->m_framecount;
	}

	float get_intervalpertick()
	{
		return m_globals()->m_intervalpertick;
	}

	int get_maxclients()
	{
		return m_globals()->m_maxclients;
	}
}

namespace ns_engine
{
	static int width, height;

	int get_screen_width()
	{
		m_engine()->GetScreenSize(width, height);
		return width;
	}

	int get_screen_height()
	{
		m_engine()->GetScreenSize(width, height);
		return height;
	}

	player_info_t get_player_info(int i)
	{
		player_info_t player_info;
		m_engine()->GetPlayerInfo(i, &player_info);

		return player_info;
	}

	int get_player_for_user_id(int i) 
	{
		return m_engine()->GetPlayerForUserID(i);
	}

	int get_local_player_index() 
	{
		return m_engine()->GetLocalPlayer();
	}

	Vector get_view_angles() 
	{
		Vector view_angles;
		m_engine()->GetViewAngles(view_angles);

		return view_angles;
	}

	void set_view_angles(Vector view_angles)
	{
		math::normalize_angles(view_angles);
		m_engine()->SetViewAngles(view_angles);
	}

	bool is_in_game() 
	{
		return m_engine()->IsInGame();
	}

	bool is_connected() 
	{
		return m_engine()->IsConnected();
	}

	std::string get_level_name() 
	{
		return m_engine()->GetLevelName();
	}

	std::string get_level_name_short() 
	{
		return m_engine()->GetLevelNameShort();
	}

	std::string get_map_group_name() 
	{
		return m_engine()->GetMapGroupName();
	}

	bool is_playing_demo()
	{
		return m_engine()->IsPlayingDemo();
	}

	bool is_recording_demo()
	{
		return m_engine()->IsRecordingDemo();
	}

	bool is_paused() 
	{
		return m_engine()->IsPaused();
	}

	bool is_taking_screenshot() 
	{
		return m_engine()->IsTakingScreenshot();
	}

	bool is_hltv() 
	{
		return m_engine()->IsHLTV();
	}
}

namespace ns_render
{
	Vector world_to_screen(const Vector& world)
	{
		Vector screen;

		if (!math::world_to_screen(world, screen))
			return ZERO;

		return screen;
	}

	int get_text_width(vgui::HFont font, const std::string& text)
	{
		return render::get().text_width(font, text.c_str());
	}

	vgui::HFont create_font(const std::string& name, float size, float weight, std::optional <bool> antialias, std::optional <bool> dropshadow, std::optional <bool> outline)
	{
		DWORD flags = FONTFLAG_NONE;

		if (antialias.value_or(false))
			flags |= FONTFLAG_ANTIALIAS;

		if (dropshadow.value_or(false))
			flags |= FONTFLAG_DROPSHADOW;

		if (outline.value_or(false))
			flags |= FONTFLAG_OUTLINE;

		g_ctx.last_font_name = name;

		auto font = m_surface()->FontCreate();
		m_surface()->SetFontGlyphSet(font, name.c_str(), (int)size, (int)weight, 0, 0, flags);

		return font;
	}

	void draw_text(vgui::HFont font, float x, float y, Color color, const std::string& text)
	{
		render::get().text(font, (int)x, (int)y, color, HFONT_CENTERED_NONE, text.c_str());
	}

	void draw_text_centered(vgui::HFont font, float x, float y, Color color, bool centered_x, bool centered_y, const std::string& text)
	{
		DWORD centered_flags = HFONT_CENTERED_NONE;

		if (centered_x)
		{
			centered_flags &= ~HFONT_CENTERED_NONE; //-V753
			centered_flags |= HFONT_CENTERED_X;
		}

		if (centered_y)
		{
			centered_flags &= ~HFONT_CENTERED_NONE;
			centered_flags |= HFONT_CENTERED_Y;
		}

		render::get().text(font, (int)x, (int)y, color, centered_flags, text.c_str());
	}

	void draw_line(float x, float y, float x2, float y2, Color color)
	{
		render::get().line((int)x, (int)y, (int)x2, (int)y2, color);
	}

	void draw_rect(float x, float y, float w, float h, Color color)
	{
		render::get().rect((int)x, (int)y, (int)w, (int)h, color);
	}

	void draw_rect_filled(float x, float y, float w, float h, Color color)
	{
		render::get().rect_filled((int)x, (int)y, (int)w, (int)h, color);
	}

	void draw_rect_filled_gradient(float x, float y, float w, float h, Color color, Color color2, int gradient_type)
	{
		gradient_type = math::clamp(gradient_type, 0, 1);
		render::get().gradient((int)x, (int)y, (int)w, (int)h, color, color2, (GradientType)gradient_type);
	}

	void draw_circle(float x, float y, float points, float radius, Color color)
	{
		render::get().circle((int)x, (int)y, (int)points, (int)radius, color);
	}

	void draw_circle_filled(float x, float y, float points, float radius, Color color)
	{
		render::get().circle_filled((int)x, (int)y, (int)points, (int)radius, color);
	}

	void draw_triangle(float x, float y, float x2, float y2, float x3, float y3, Color color)
	{
		render::get().triangle(Vector2D(x, y), Vector2D(x2, y2), Vector2D(x3, y3), color);
	}
}
namespace ns_indicator
{
	void add_indicator(std::string& text, Color color)
	{
		if (!g_ctx.available())
			return;
		if (!g_ctx.local()->is_alive())
			return;
  

		if (otheresp::get().m_indicators.empty())
		{
			otheresp::get().m_indicators.insert(otheresp::get().m_indicators.begin(), m_indicator(text, color));
		}
		else {
			otheresp::get().m_indicators.insert(otheresp::get().m_indicators.begin(), m_indicator(text, color));
		}


	}
	void add_indicator_with_pos(std::string& text, Color color, int pos)
	{
		if (!g_ctx.available())
			return;
		if (!g_ctx.local()->is_alive())
			return;
		if (otheresp::get().m_indicators.empty())
		{
			otheresp::get().m_indicators.insert(otheresp::get().m_indicators.begin(), m_indicator(text, color));
		}
		else 
		{
			auto iter1 = otheresp::get().m_indicators.cbegin();
			otheresp::get().m_indicators.insert(iter1 + (pos - 1), m_indicator(text, color));
		}
	}
}
namespace ns_console
{
	void execute(std::string& command)
	{
		if (command.empty())
			return;

		m_engine()->ExecuteClientCmd(command.c_str());
	}

	std::unordered_map <std::string, ConVar*> convars;

	bool get_bool(const std::string& convar_name)
	{
		if (convars.find(convar_name) == convars.end())
		{
			convars[convar_name] = m_cvar()->FindVar(convar_name.c_str());

			if (!convars[convar_name])
			{
				eventlogs::get().add(crypt_str("Lua error: cannot find ConVar \"") + convar_name + '\"', false);
				return false;
			}
		}

		if (!convars[convar_name])
			return false;

		return convars[convar_name]->GetBool();
	}

	int get_int(const std::string& convar_name)
	{
		if (convars.find(convar_name) == convars.end())
		{
			convars[convar_name] = m_cvar()->FindVar(convar_name.c_str());

			if (!convars[convar_name])
			{
				eventlogs::get().add(crypt_str("Lua error: cannot find ConVar \"") + convar_name + '\"', false);
				return 0;
			}
		}

		if (!convars[convar_name])
			return 0;

		return convars[convar_name]->GetInt();
	}

	float get_float(const std::string& convar_name)
	{
		if (convars.find(convar_name) == convars.end())
		{
			convars[convar_name] = m_cvar()->FindVar(convar_name.c_str());

			if (!convars[convar_name])
			{
				eventlogs::get().add(crypt_str("Lua error: cannot find ConVar \"") + convar_name + '\"', false);
				return 0.0f;
			}
		}

		if (!convars[convar_name])
			return 0.0f;

		return convars[convar_name]->GetFloat();
	}

	std::string get_string(const std::string& convar_name)
	{
		if (convars.find(convar_name) == convars.end())
		{
			convars[convar_name] = m_cvar()->FindVar(convar_name.c_str());

			if (!convars[convar_name])
			{
				eventlogs::get().add(crypt_str("Lua error: cannot find ConVar \"") + convar_name + '\"', false);
				return crypt_str("");
			}
		}

		if (!convars[convar_name])
			return "";

		return convars[convar_name]->GetString();
	}

	void set_bool(const std::string& convar_name, bool value)
	{
		if (convars.find(convar_name) == convars.end())
		{
			convars[convar_name] = m_cvar()->FindVar(convar_name.c_str());

			if (!convars[convar_name])
			{
				eventlogs::get().add(crypt_str("Lua error: cannot find ConVar \"") + convar_name + '\"', false);
				return;
			}
		}

		if (!convars[convar_name])
			return;

		if (convars[convar_name]->GetBool() != value)
			convars[convar_name]->SetValue(value);
	}

	void set_int(const std::string& convar_name, int value)
	{
		if (convars.find(convar_name) == convars.end())
		{
			convars[convar_name] = m_cvar()->FindVar(convar_name.c_str());

			if (!convars[convar_name])
			{
				eventlogs::get().add(crypt_str("Lua error: cannot find ConVar \"") + convar_name + '\"', false);
				return;
			}
		}

		if (!convars[convar_name])
			return;

		if (convars[convar_name]->GetInt() != value)
			convars[convar_name]->SetValue(value);
	}

	void set_float(const std::string& convar_name, float value)
	{
		if (convars.find(convar_name) == convars.end())
		{
			convars[convar_name] = m_cvar()->FindVar(convar_name.c_str());

			if (!convars[convar_name])
			{
				eventlogs::get().add(crypt_str("Lua error: cannot find ConVar \"") + convar_name + '\"', false);
				return;
			}
		}

		if (!convars[convar_name])
			return;

		if (convars[convar_name]->GetFloat() != value)
			convars[convar_name]->SetValue(value);
	}

	void set_string(const std::string& convar_name, const std::string& value)
	{
		if (convars.find(convar_name) == convars.end())
		{
			convars[convar_name] = m_cvar()->FindVar(convar_name.c_str());

			if (!convars[convar_name])
			{
				eventlogs::get().add(crypt_str("Lua error: cannot find ConVar \"") + convar_name + '\"', false);
				return;
			}
		}

		if (!convars[convar_name])
			return;

		if (convars[convar_name]->GetString() != value)
			convars[convar_name]->SetValue(value.c_str());
	}
}

namespace ns_events
{
	void register_event(sol::this_state s, std::string event_name, sol::protected_function function)
	{
		if (std::find(g_ctx.globals.events.begin(), g_ctx.globals.events.end(), event_name) == g_ctx.globals.events.end())
		{
			m_eventmanager()->AddListener(&hooks::hooked_events, event_name.c_str(), false);
			g_ctx.globals.events.emplace_back(event_name);
		}

		c_lua::get().events[get_current_script_id(s)][event_name] = function;
	}
}

namespace ns_entitylist
{
	sol::optional <player_t*> get_local_player()
	{
		if (!m_engine()->IsInGame())
			return sol::optional <player_t*> (sol::nullopt);

		return (player_t*)m_entitylist()->GetClientEntity(m_engine()->GetLocalPlayer());
	}

	sol::optional <player_t*> get_player_by_index(int i)
	{
		if (!m_engine()->IsInGame())
			return sol::optional <player_t*> (sol::nullopt);

		return (player_t*)m_entitylist()->GetClientEntity(i);
	}

	sol::optional <weapon_t*> get_weapon_by_player(sol::optional <player_t*> player)
	{
		if (!m_engine()->IsInGame())
			return sol::optional <weapon_t*> (sol::nullopt);

		if (!player.has_value())
			return sol::optional <weapon_t*>(sol::nullopt);

		return player.value()->m_hActiveWeapon().Get();
	}

}

namespace ns_cmd
{
	bool get_send_packet()
	{
		if (!g_ctx.get_command())
			return true;

		return g_ctx.send_packet;
	}

	void set_send_packet(bool send_packet)
	{
		if (!g_ctx.get_command())
			return;

		g_ctx.send_packet = send_packet;
	}

	int get_choke()
	{
		if (!g_ctx.get_command())
			return 0;

		return m_clientstate()->iChokedCommands;
	}

	bool get_button_state(int button)
	{
		if (!g_ctx.get_command())
			return false;

		return g_ctx.get_command()->m_buttons & button;
	}

	void set_button_state(int button, bool state)
	{
		if (!g_ctx.get_command())
			return;

		if (state)
			g_ctx.get_command()->m_buttons |= button;
		else
			g_ctx.get_command()->m_buttons &= ~button;
	}
}

namespace ns_utils 
{
	uint64_t find_signature(const std::string& szModule, const std::string& szSignature)
	{
		return util::FindSignature(szModule.c_str(), szSignature.c_str());
	}
}

namespace ns_http //new
{
	std::string get(sol::this_state s, std::string& link)
	{
		if (!g_cfg.scripts.allow_http)
		{
			c_lua::get().unload_script(get_current_script_id(s));
			eventlogs::get().add(crypt_str("Please, allow HTTP requests"), false);
			return "";
		}

		try
		{		
			http::Request request(link);

			const http::Response response = request.send(crypt_str("GET"));
			return std::string(response.body.begin(), response.body.end()); 
		}
		catch (const std::exception& e)
		{
			eventlogs::get().add(crypt_str("Request failed, error: ") + std::string(e.what()), false);
			return "";
		}
	}

	std::string post(sol::this_state s, std::string& link, std::string& params)
	{
		if (!g_cfg.scripts.allow_http)
		{
			c_lua::get().unload_script(get_current_script_id(s));
			eventlogs::get().add(crypt_str("Please, allow HTTP requests"), false);
			return "";
		}
		try
		{
			http::Request request(link);
			const http::Response response = request.send(crypt_str("POST"), params, {crypt_str("Content-Type: application/x-www-form-urlencoded")});
			return std::string(response.body.begin(), response.body.end()); 
		}
		catch (const std::exception& e)
		{
			eventlogs::get().add(crypt_str("Request failed, error: ") + std::string(e.what()), false);
			return "";
		}
	}
}

namespace ns_file //new
{
	void append(sol::this_state s, std::string& path, std::string& data)
	{
		if (!g_cfg.scripts.allow_file)
		{
			c_lua::get().unload_script(get_current_script_id(s));
			eventlogs::get().add(crypt_str("Please, allow files read or write"), false);
			return;
		}

		std::ofstream out(path, std::ios::app | std::ios::binary);

		if (out.is_open())
			out << data;
		else
			eventlogs::get().add(crypt_str("Can't append to file: ") + path, false);
		
		out.close();
	}
	void write(sol::this_state s, std::string& path, std::string& data)
	{
		if (!g_cfg.scripts.allow_file)
		{
			c_lua::get().unload_script(get_current_script_id(s));
			eventlogs::get().add(crypt_str("Please, allow files read or write"), false);
			return;
		}

		std::ofstream out(path, std::ios::binary);

		if (out.is_open())
			out << data;

		else
			eventlogs::get().add(crypt_str("Can't write to file: ") + path, false);

		out.close();
	}
	std::string read(sol::this_state s, std::string& path)
	{
		if (!g_cfg.scripts.allow_file)
		{
			c_lua::get().unload_script(get_current_script_id(s));
			eventlogs::get().add(crypt_str("Please, allow files read or write"), false);
			return "";
		}

		std::ifstream file(path, std::ios::binary);

		if (file.is_open())
		{
			std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
			file.close();
			return content;
		}
		else
		{
			eventlogs::get().add(crypt_str("Can't read file: ") + path, false);
			file.close();
			return "";
		}
	}
}

sol::state lua;
void c_lua::initialize()
{
	lua = sol::state(sol::c_call<decltype(&lua_panic), &lua_panic>);
	lua.open_libraries(sol::lib::base, sol::lib::string, sol::lib::math, sol::lib::table, sol::lib::debug, sol::lib::package);

	lua[crypt_str("collectgarbage")] = sol::nil;
	lua[crypt_str("dofilsse")] = sol::nil;
	lua[crypt_str("load")] = sol::nil;
	lua[crypt_str("loadfile")] = sol::nil;
	lua[crypt_str("pcall")] = sol::nil;
	lua[crypt_str("print")] = sol::nil;
	lua[crypt_str("xpcall")] = sol::nil;
	lua[crypt_str("getmetatable")] = sol::nil;
	lua[crypt_str("setmetatable")] = sol::nil;
	lua[crypt_str("__nil_callback")] = [](){};

	lua.new_enum(crypt_str("key_binds"),
		crypt_str("legit_automatic_fire"), 0,
		crypt_str("legit_enable"), 1,
		crypt_str("double_tap"), 2,
		crypt_str("safe_points"), 3,
		crypt_str("damage_override"), 4,
		crypt_str("hide_shots"), 12,
		crypt_str("manual_back"), 13,
		crypt_str("manual_left"), 14,
		crypt_str("manual_right"), 15,
		crypt_str("flip_desync"), 16,
		crypt_str("thirdperson"), 17,
		crypt_str("automatic_peek"), 18,
		crypt_str("edge_jump"), 19,
		crypt_str("fakeduck"), 20,
		crypt_str("slowwalk"), 21,
		crypt_str("body_aim"), 22
	);

	lua.new_enum(crypt_str("key_bind_mode"),
		crypt_str("hold"), 0,
		crypt_str("toggle"), 1
	);

	lua.new_usertype<entity_t>(crypt_str("entity"), // new
		(std::string)crypt_str("get_prop_int"), &entity_t::GetPropInt,
		(std::string)crypt_str("get_prop_float"), &entity_t::GetPropFloat,
		(std::string)crypt_str("get_prop_bool"), &entity_t::GetPropBool,
		(std::string)crypt_str("get_prop_string"), &entity_t::GetPropString,
		(std::string)crypt_str("set_prop_int"), &entity_t::SetPropInt,
		(std::string)crypt_str("set_prop_float"), &entity_t::SetPropFloat,
		(std::string)crypt_str("set_prop_bool"), &entity_t::SetPropBool		
	);

	lua.new_usertype <Color> (crypt_str("color"), sol::constructors <Color(), Color(int, int, int), Color(int, int, int, int)> (), 
		(std::string)crypt_str("r"), &Color::r, 
		(std::string)crypt_str("g"), &Color::g,
		(std::string)crypt_str("b"), &Color::b, 
		(std::string)crypt_str("a"), &Color::a
	);

	lua.new_usertype <Vector> (crypt_str("vector"), sol::constructors <Vector(), Vector(float, float, float)> (),
		(std::string)crypt_str("x"), &Vector::x,
		(std::string)crypt_str("y"), &Vector::y,
		(std::string)crypt_str("z"), &Vector::z,
		(std::string)crypt_str("length"), &Vector::Length,
		(std::string)crypt_str("length_sqr"), &Vector::LengthSqr,
		(std::string)crypt_str("length_2d"), &Vector::Length2D,
		(std::string)crypt_str("length_2d_sqr"), &Vector::Length2DSqr,
		(std::string)crypt_str("is_zero"), &Vector::IsZero,
		(std::string)crypt_str("is_valid"), &Vector::IsValid,
		(std::string)crypt_str("zero"), &Vector::Zero,
		(std::string)crypt_str("dist_to"), &Vector::DistTo,
		(std::string)crypt_str("dist_to_sqr"), &Vector::DistToSqr,
		(std::string)crypt_str("cross_product"), &Vector::Cross,
		(std::string)crypt_str("normalize"), &Vector::Normalize
	);

	lua.new_usertype <player_info_t> (crypt_str("player_info"),
		(std::string)crypt_str("bot"), &player_info_t::fakeplayer,
		(std::string)crypt_str("name"), &player_info_t::szName,
		(std::string)crypt_str("steam_id"), &player_info_t::szSteamID

	);

	lua.new_usertype <IGameEvent> (crypt_str("game_event"),
		(std::string)crypt_str("get_bool"), &IGameEvent::GetBool,
		(std::string)crypt_str("get_int"), &IGameEvent::GetInt,
		(std::string)crypt_str("get_float"), &IGameEvent::GetFloat,
		(std::string)crypt_str("get_string"), &IGameEvent::GetString,
		(std::string)crypt_str("set_bool"), &IGameEvent::SetBool,
		(std::string)crypt_str("set_int"), &IGameEvent::SetInt,
		(std::string)crypt_str("set_float"), &IGameEvent::SetFloat,
		(std::string)crypt_str("set_string"), &IGameEvent::SetString
	);

	lua.new_enum(crypt_str("hitboxes"),
		crypt_str("head"), HITBOX_HEAD,
		crypt_str("neck"), HITBOX_NECK,
		crypt_str("pelvis"), HITBOX_PELVIS,
		crypt_str("stomach"), HITBOX_STOMACH,
		crypt_str("lower_chest"), HITBOX_LOWER_CHEST,
		crypt_str("chest"), HITBOX_CHEST,
		crypt_str("upper_chest"), HITBOX_UPPER_CHEST,
		crypt_str("right_thigh"), HITBOX_RIGHT_THIGH,
		crypt_str("left_thigh"), HITBOX_LEFT_THIGH,
		crypt_str("right_calf"), HITBOX_RIGHT_CALF,
		crypt_str("left_calf"), HITBOX_LEFT_CALF,
		crypt_str("right_foot"), HITBOX_RIGHT_FOOT,
		crypt_str("left_foot"), HITBOX_LEFT_FOOT,
		crypt_str("right_hand"), HITBOX_RIGHT_HAND,
		crypt_str("left_hand"), HITBOX_LEFT_HAND,
		crypt_str("right_upper_arm"), HITBOX_RIGHT_UPPER_ARM,
		crypt_str("right_forearm"), HITBOX_RIGHT_FOREARM,
		crypt_str("left_upper_arm"), HITBOX_LEFT_UPPER_ARM,
		crypt_str("left_forearm"), HITBOX_LEFT_FOREARM
	);

	lua.new_usertype <player_t> (crypt_str("player"), sol::base_classes, sol::bases<entity_t>(), //new
		(std::string)crypt_str("get_index"), &player_t::EntIndex,
		(std::string)crypt_str("get_dormant"), &player_t::IsDormant,
		(std::string)crypt_str("get_team"), &player_t::m_iTeamNum,
		(std::string)crypt_str("get_alive"), &player_t::is_alive,
		(std::string)crypt_str("get_velocity"), &player_t::m_vecVelocity,
		(std::string)crypt_str("get_origin"), &player_t::GetAbsOrigin,
		(std::string)crypt_str("get_angles"), &player_t::m_angEyeAngles,
		(std::string)crypt_str("get_hitbox_position"), &player_t::hitbox_position,
		(std::string)crypt_str("has_helmet"), &player_t::m_bHasHelmet,
		(std::string)crypt_str("has_heavy_armor"), &player_t::m_bHasHeavyArmor,
		(std::string)crypt_str("is_scoped"), &player_t::m_bIsScoped,
		(std::string)crypt_str("get_health"), &player_t::m_iHealth
	);

	lua.new_usertype <weapon_t> (crypt_str("weapon"), sol::base_classes, sol::bases<entity_t>(),
		(std::string)crypt_str("is_empty"), &weapon_t::is_empty,
		(std::string)crypt_str("can_fire"), &weapon_t::can_fire,
		(std::string)crypt_str("is_non_aim"), &weapon_t::is_non_aim,
		(std::string)crypt_str("can_double_tap"), &weapon_t::can_double_tap,
		(std::string)crypt_str("get_name"), &weapon_t::get_name,
		(std::string)crypt_str("get_inaccuracy"), &weapon_t::get_inaccuracy,
		(std::string)crypt_str("get_spread"), &weapon_t::get_spread
	);

	lua.new_enum(crypt_str("buttons"),
		crypt_str("in_attack"), IN_ATTACK,
		crypt_str("in_jump"), IN_JUMP,
		crypt_str("in_duck"), IN_DUCK,
		crypt_str("in_forward"), IN_FORWARD,
		crypt_str("in_back"), IN_BACK,
		crypt_str("in_use"), IN_USE,
		crypt_str("in_cancel"), IN_CANCEL,
		crypt_str("in_left"), IN_LEFT,
		crypt_str("in_right"), IN_RIGHT,
		crypt_str("in_moveleft"), IN_MOVELEFT,
		crypt_str("in_moveright"), IN_MOVERIGHT,
		crypt_str("in_attack2"), IN_ATTACK2,
		crypt_str("in_run"), IN_RUN,
		crypt_str("in_reload"), IN_RELOAD,
		crypt_str("in_alt1"), IN_ALT1,
		crypt_str("in_alt2"), IN_ALT2,
		crypt_str("in_score"), IN_SCORE,
		crypt_str("in_speed"), IN_SPEED,
		crypt_str("in_walk"), IN_WALK,
		crypt_str("in_zoom"), IN_ZOOM,
		crypt_str("in_weapon1"), IN_WEAPON1,
		crypt_str("in_weapon2"), IN_WEAPON2,
		crypt_str("in_bullrush"), IN_BULLRUSH,
		crypt_str("in_grenade1"), IN_GRENADE1,
		crypt_str("in_grenade2"), IN_GRENADE2,
		crypt_str("in_lookspin"), IN_LOOKSPIN
	);

	lua.new_usertype <shot_info> (crypt_str("shot_info"), sol::constructors <> (), 
		(std::string)crypt_str("target_name"), &shot_info::target_name, 
		(std::string)crypt_str("result"), &shot_info::result, 
		(std::string)crypt_str("client_hitbox"), &shot_info::client_hitbox, 
		(std::string)crypt_str("server_hitbox"), &shot_info::server_hitbox,
		(std::string)crypt_str("client_damage"), &shot_info::client_damage,
		(std::string)crypt_str("server_damage"), &shot_info::server_damage,
		(std::string)crypt_str("hitchance"), &shot_info::hitchance,
		(std::string)crypt_str("backtrack_ticks"), &shot_info::backtrack_ticks,
		(std::string)crypt_str("aim_point"), &shot_info::aim_point
	);

	auto client = lua.create_table();
	client[crypt_str("add_callback")] = ns_client::add_callback;
	client[crypt_str("load_script")] = ns_client::load_script;
	client[crypt_str("unload_script")] = ns_client::unload_script;
	client[crypt_str("log")] = ns_client::log;

	auto menu = lua.create_table();
	menu[crypt_str("next_line")] = ns_menu::next_line;
	menu[crypt_str("add_check_box")] = ns_menu::add_check_box;
	menu[crypt_str("add_combo_box")] = ns_menu::add_combo_box;
	menu[crypt_str("add_slider_int")] = ns_menu::add_slider_int;
	menu[crypt_str("add_slider_float")] = ns_menu::add_slider_float;
	menu[crypt_str("add_color_picker")] = ns_menu::add_color_picker;
	menu[crypt_str("get_bool")] = ns_menu::get_bool;
	menu[crypt_str("get_int")] = ns_menu::get_int;
	menu[crypt_str("get_float")] = ns_menu::get_float;
	menu[crypt_str("get_color")] = ns_menu::get_color;
	menu[crypt_str("get_key_bind_state")] = ns_menu::get_key_bind_state;
	menu[crypt_str("get_key_bind_mode")] = ns_menu::get_key_bind_mode;
	menu[crypt_str("set_bool")] = ns_menu::set_bool;
	menu[crypt_str("set_int")] = ns_menu::set_int;
	menu[crypt_str("set_float")] = ns_menu::set_float;
	menu[crypt_str("set_color")] = ns_menu::set_color;

	auto globals = lua.create_table();
	globals[crypt_str("get_framerate")] = ns_globals::get_framerate;
	globals[crypt_str("get_ping")] = ns_globals::get_ping;
	globals[crypt_str("get_server_address")] = ns_globals::get_server_address;
	globals[crypt_str("get_time")] = ns_globals::get_time;
	globals[crypt_str("get_username")] = ns_globals::get_username;
	globals[crypt_str("get_realtime")] = ns_globals::get_realtime;
	globals[crypt_str("get_curtime")] = ns_globals::get_curtime;
	globals[crypt_str("get_frametime")] = ns_globals::get_frametime;
	globals[crypt_str("get_tickcount")] = ns_globals::get_tickcount;
	globals[crypt_str("get_framecount")] = ns_globals::get_framecount;
	globals[crypt_str("get_intervalpertick")] = ns_globals::get_intervalpertick;
	globals[crypt_str("get_maxclients")] = ns_globals::get_maxclients;

	auto engine = lua.create_table();
	engine[crypt_str("get_screen_width")] = ns_engine::get_screen_width;
	engine[crypt_str("get_screen_height")] = ns_engine::get_screen_height;
	engine[crypt_str("get_level_name")] = ns_engine::get_level_name;
	engine[crypt_str("get_level_name_short")] = ns_engine::get_level_name_short;
	engine[crypt_str("get_local_player_index")] = ns_engine::get_local_player_index;
	engine[crypt_str("get_map_group_name")] = ns_engine::get_map_group_name;
	engine[crypt_str("get_player_for_user_id")] = ns_engine::get_player_for_user_id;
	engine[crypt_str("get_player_info")] = ns_engine::get_player_info;
	engine[crypt_str("get_view_angles")] = ns_engine::get_view_angles;
	engine[crypt_str("is_connected")] = ns_engine::is_connected;
	engine[crypt_str("is_hltv")] = ns_engine::is_hltv;
	engine[crypt_str("is_in_game")] = ns_engine::is_in_game;
	engine[crypt_str("is_paused")] = ns_engine::is_paused;
	engine[crypt_str("is_playing_demo")] = ns_engine::is_playing_demo;
	engine[crypt_str("is_recording_demo")] = ns_engine::is_recording_demo;
	engine[crypt_str("is_taking_screenshot")] = ns_engine::is_taking_screenshot;
	engine[crypt_str("set_view_angles")] = ns_engine::set_view_angles;

	auto render = lua.create_table();
	render[crypt_str("world_to_screen")] = ns_render::world_to_screen;
	render[crypt_str("get_text_width")] = ns_render::get_text_width;
	render[crypt_str("create_font")] = ns_render::create_font;
	render[crypt_str("draw_text")] = ns_render::draw_text;
	render[crypt_str("draw_text_centered")] = ns_render::draw_text_centered;
	render[crypt_str("draw_line")] = ns_render::draw_line;
	render[crypt_str("draw_rect")] = ns_render::draw_rect;
	render[crypt_str("draw_rect_filled")] = ns_render::draw_rect_filled;
	render[crypt_str("draw_rect_filled_gradient")] = ns_render::draw_rect_filled_gradient;
	render[crypt_str("draw_circle")] = ns_render::draw_circle;
	render[crypt_str("draw_circle_filled")] = ns_render::draw_circle_filled;
	render[crypt_str("draw_triangle")] = ns_render::draw_triangle;

	auto console = lua.create_table();
	console[crypt_str("execute")] = ns_console::execute;
	console[crypt_str("get_int")] = ns_console::get_int;
	console[crypt_str("get_float")] = ns_console::get_float;
	console[crypt_str("get_string")] = ns_console::get_string;
	console[crypt_str("set_int")] = ns_console::set_int;
	console[crypt_str("set_float")] = ns_console::set_float;
	console[crypt_str("set_string")] = ns_console::set_string;

	auto events = lua.create_table();
	events[crypt_str("register_event")] = ns_events::register_event;

	auto entitylist = lua.create_table();
	entitylist[crypt_str("get_local_player")] = ns_entitylist::get_local_player;
	entitylist[crypt_str("get_player_by_index")] = ns_entitylist::get_player_by_index;
	entitylist[crypt_str("get_weapon_by_player")] = ns_entitylist::get_weapon_by_player;

	auto cmd = lua.create_table();
	cmd[crypt_str("get_send_packet")] = ns_cmd::get_send_packet;
	cmd[crypt_str("set_send_packet")] = ns_cmd::set_send_packet;
	cmd[crypt_str("get_choke")] = ns_cmd::get_choke;
	cmd[crypt_str("get_button_state")] = ns_cmd::get_button_state;
	cmd[crypt_str("set_button_state")] = ns_cmd::set_button_state;

	auto utils = lua.create_table();//new
	utils[crypt_str("find_signature")] = ns_utils::find_signature;//new

	auto indicators = lua.create_table();//new
	indicators[crypt_str("add")] = ns_indicator::add_indicator;//new
	indicators[crypt_str("add_position")] = ns_indicator::add_indicator_with_pos;//new

	auto http = lua.create_table();//new
	http[crypt_str("get")] = ns_http::get;//new
	http[crypt_str("post")] = ns_http::post;//new

	auto file = lua.create_table();//new
	file[crypt_str("append")] = ns_file::append; //new
	file[crypt_str("write")] = ns_file::write;//new
	file[crypt_str("read")] = ns_file::read;//new

	lua[crypt_str("client")] = client;
	lua[crypt_str("menu")] = menu;
	lua[crypt_str("globals")] = globals;
	lua[crypt_str("engine")] = engine;
	lua[crypt_str("render")] = render;
	lua[crypt_str("console")] = console;
	lua[crypt_str("events")] = events;
	lua[crypt_str("entitylist")] = entitylist;
	lua[crypt_str("cmd")] = cmd;
	lua[crypt_str("utils")] = utils;//new
	lua[crypt_str("indicators")] = indicators;//new
	lua[crypt_str("http")] = http;//new
	lua[crypt_str("file")] = file;//new

	refresh_scripts();
}

int c_lua::get_script_id(const std::string& name)
{
	for (auto i = 0; i < scripts.size(); i++)
		if (scripts.at(i) == name)
			return i;

	return -1;
}

int c_lua::get_script_id_by_path(const std::string& path)
{
	for (auto i = 0; i < pathes.size(); i++)
		if (pathes.at(i).string() == path)
			return i;

	return -1;
}

void c_lua::refresh_scripts()
{
	auto oldLoaded = loaded;
	auto oldScripts = scripts;

	loaded.clear();
	pathes.clear();
	scripts.clear();
	ns_console::convars.clear();

	std::string folder;
	static TCHAR path[MAX_PATH];

	if (SUCCEEDED(SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, NULL, path)))
	{
		folder = std::string(path) + crypt_str("\\skeet.pw\\Scripts\\");

		auto i = 0;

		for (auto& entry : std::filesystem::directory_iterator(folder))
		{
			if (entry.path().extension() == crypt_str(".lua") || entry.path().extension() == crypt_str(".luac"))
			{
				auto path = entry.path();
				auto filename = path.filename().string();

				auto didPut = false;

				for (auto i = 0; i < oldScripts.size(); i++)
				{
					if (filename == oldScripts.at(i))
					{
						loaded.emplace_back(oldLoaded.at(i));
						didPut = true;
					}
				}

				if (!didPut)
					loaded.emplace_back(false);

				pathes.emplace_back(path);
				scripts.emplace_back(filename);

				items.emplace_back(std::vector <std::pair <std::string, menu_item>> ());
				++i;
			}
		}
	}
}

void c_lua::load_script(int id)
{
	if (id == -1)
		return;

	if (loaded.at(id))
		return;

	auto path = get_script_path(id);

	if (path == crypt_str(""))
		return;

	auto error_load = false;
	loaded.at(id) = true;
	lua.script_file(path, 
		[&error_load](lua_State*, sol::protected_function_result result)
		{
			if (!result.valid())
			{
				sol::error error = result;
				auto log = crypt_str("Lua error: ") + (std::string)error.what();

				eventlogs::get().add(log, false);
				error_load = true;
				
			}

			return result;
		}
	);

	if (error_load | loaded.at(id) == false)
	{
		loaded.at(id) = false;
		return;
	}
		

	
	g_ctx.globals.loaded_script = true;
}

void c_lua::unload_script(int id)
{
	if (id == -1)
		return;

	if (!loaded.at(id))
		return;

	items.at(id).clear();

	if (c_lua::get().events.find(id) != c_lua::get().events.end())
		c_lua::get().events.at(id).clear();

	hooks.unregisterHooks(id);
	loaded.at(id) = false;
}

void c_lua::reload_all_scripts()
{
	for (auto current : scripts)
	{
		if (!loaded.at(get_script_id(current)))
			continue;

		unload_script(get_script_id(current));
		load_script(get_script_id(current));
	}
}

void c_lua::unload_all_scripts()
{
	for (auto s : scripts)
		unload_script(get_script_id(s));
}

std::string c_lua::get_script_path(const std::string& name)
{
	return get_script_path(get_script_id(name));
}

std::string c_lua::get_script_path(int id)
{
	if (id == -1)
		return crypt_str("");

	return pathes.at(id).string();
}
