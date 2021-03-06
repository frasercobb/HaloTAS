#include "plugin.h"
#include "tas_logger.h"
#include "imgui/imgui.h"
#include "halo_map_data.h"
#include "halo_constants.h"
#include <map>
#include <sstream>

using namespace halo::mapdata;

TAG_PROPERTY_TYPE plugin::get_element_type(const std::string& type_str)
{
	if (type_str == "enum16") { return TAG_PROPERTY_TYPE::ENUM16; }
	else if (type_str == "enum32") { return TAG_PROPERTY_TYPE::ENUM32; }
	else if (type_str == "bitmask8") { return TAG_PROPERTY_TYPE::BITMASK8; }
	else if (type_str == "bitmask16") { return TAG_PROPERTY_TYPE::BITMASK16; }
	else if (type_str == "bitmask32") { return TAG_PROPERTY_TYPE::BITMASK32; }
	else if (type_str == "option") { return TAG_PROPERTY_TYPE::OPTION; }
	else if (type_str == "float") { return TAG_PROPERTY_TYPE::FLOAT; }
	else if (type_str == "int8") { return TAG_PROPERTY_TYPE::INT8; }
	else if (type_str == "short") { return TAG_PROPERTY_TYPE::INT16; }
	else if (type_str == "long") { return TAG_PROPERTY_TYPE::INT32; }
	else if (type_str == "int32") { return TAG_PROPERTY_TYPE::INT32; }
	else if (type_str == "string4") { return TAG_PROPERTY_TYPE::STRING4; }
	else if (type_str == "string32") { return TAG_PROPERTY_TYPE::STRING32; }
	else if (type_str == "dependency") { return TAG_PROPERTY_TYPE::REFERENCE; }
	else if (type_str == "struct") { return TAG_PROPERTY_TYPE::REFLEXIVE; }
	else if (type_str == "index") { return TAG_PROPERTY_TYPE::INDEX; }
	else if (type_str == "colorbyte") { return TAG_PROPERTY_TYPE::COLOR_RGBA8; }
	else if (type_str == "colorRGB") { return TAG_PROPERTY_TYPE::COLOR_RGB32; }
	else if (type_str == "colorARGB") { return TAG_PROPERTY_TYPE::COLOR_ARGB32; }

	else {
		tas_logger::warning("Parsed element with unknown property type: %s", type_str.c_str());
		return TAG_PROPERTY_TYPE::UNKNOWN;
	}
}

std::string plugin::get_type_str(TAG_PROPERTY_TYPE type)
{
	switch (type) {
	case TAG_PROPERTY_TYPE::ENUM16: return "enum16";
	case TAG_PROPERTY_TYPE::ENUM32: return "enum32";
	case TAG_PROPERTY_TYPE::BITMASK8: return "bitmask8";
	case TAG_PROPERTY_TYPE::BITMASK16: return "bitmask16";
	case TAG_PROPERTY_TYPE::BITMASK32: return "bitmask32";
	case TAG_PROPERTY_TYPE::FLOAT: return "float";
	case TAG_PROPERTY_TYPE::INT8: return "int8";
	case TAG_PROPERTY_TYPE::INT16: return "short";
	case TAG_PROPERTY_TYPE::INT32: return "long";
	case TAG_PROPERTY_TYPE::STRING4: return "string4";
	case TAG_PROPERTY_TYPE::STRING32: return "string32";
	case TAG_PROPERTY_TYPE::REFERENCE: return "tag_id";
	case TAG_PROPERTY_TYPE::COLOR_RGBA8: return "colorbyte";
	case TAG_PROPERTY_TYPE::COLOR_RGB32: return "colorRGB";
	case TAG_PROPERTY_TYPE::COLOR_ARGB32: return "colorARGB";
	default:
		return "";
	}
}

std::string new_patch_string(tag_property& prop, void* addr, const std::string& valueStr) {
	std::stringstream ss;
	ss << "<patch>\r\n";

	ss << "\t<name>NEW_PATCH</name>\r\n";
	ss << "\t<optional_name>" << prop.name << "</optional_name>\r\n";
	ss << "\t<address>" << addr << "</address>\r\n";
	ss << "\t<type>" << plugin::get_type_str(prop.type) << "</type>\r\n";
	ss << "\t<value>" << valueStr << "</value>\r\n";
	
	ss << "</patch>";

	return ss.str();
}

void draw_patch_button(tag_property& prop, void* value_ptr, std::string& value_str) {
	if (ImGui::Button("patch")) {
		std::string patch_string = new_patch_string(prop, value_ptr, value_str);
		ImGui::SetClipboardText(patch_string.c_str());
	}
	ImGui::SameLine();
}

void draw_element_float(tag_property& prop, void* base_ptr) {

	float* value_ptr = (float*)((char*)base_ptr + prop.local_offset);

	std::string value_str = std::to_string(*value_ptr);
	draw_patch_button(prop, value_ptr, value_str);
	
	ImGui::PushItemWidth(200);
	ImGui::InputFloat(prop.name.c_str(), value_ptr);
	ImGui::PopItemWidth();
}

void draw_element_int8(tag_property& prop, void* base_ptr) {
	int8_t* value_ptr = (int8_t*)((char*)base_ptr + prop.local_offset);
	ImGui::PushItemWidth(200);
	ImGui::InputScalar(prop.name.c_str(), ImGuiDataType_S8, value_ptr);
	ImGui::PopItemWidth();
}

void draw_element_int16(tag_property& prop, void* base_ptr) {
	int16_t* value_ptr = (int16_t*)((char*)base_ptr + prop.local_offset);
	ImGui::PushItemWidth(200);
	ImGui::InputScalar(prop.name.c_str(), ImGuiDataType_S16, value_ptr);
	ImGui::PopItemWidth();
}

void draw_element_int32(tag_property& prop, void* base_ptr) {
	int32_t* value_ptr = (int32_t*)((char*)base_ptr + prop.local_offset);
	ImGui::PushItemWidth(200);
	ImGui::InputScalar(prop.name.c_str(), ImGuiDataType_S32, value_ptr);
	ImGui::PopItemWidth();
}

void draw_element_string(tag_property& prop, void* base_ptr, size_t string_size) {
	char* value_ptr = (char*)base_ptr + prop.local_offset;
	ImGui::InputText(prop.name.c_str(), value_ptr, string_size);
}

void draw_element_rgba8(tag_property& prop, void* base_ptr) {
	using namespace halo::mapdata;

	color_bgra8* colorData = (color_bgra8*)((char*)base_ptr + prop.local_offset);
	ImU32 igData = IM_COL32(colorData->r, colorData->g, colorData->b, colorData->a);

	ImVec4 floatData = ImGui::ColorConvertU32ToFloat4(igData);
	if (ImGui::ColorEdit4(prop.name.c_str(), (float*)& floatData, ImGuiColorEditFlags_Uint8)) {
		igData = ImGui::ColorConvertFloat4ToU32(floatData);
		uint8_t* col = (uint8_t*)& igData;
		colorData->r = col[0];
		colorData->g = col[1];
		colorData->b = col[2];
		colorData->a = col[3];
	}
}

void draw_element_rgb32(tag_property& prop, void* base_ptr) {
	color_rgb32* colorData = (color_rgb32*)((char*)base_ptr + prop.local_offset);
	ImGui::ColorEdit3(prop.name.c_str(), (float*)colorData);
}

void draw_element_reference(tag_property& prop, void* base_ptr) {

	tag_reference* refDestination = (tag_reference*)((char*)base_ptr + prop.local_offset);

	auto tagHeader = reinterpret_cast<tag_header*>(halo::addr::TAGS_BEGIN);
	tag_entry* tags = tagHeader->tag_array;

	const char* current = nullptr;
	std::string currentStr;
	std::map<std::string, tag_id> strValue;

	for (uint16_t i = 0; i < tagHeader->tag_count; i++) {
		if (refDestination->tag_class == tags[i].tag_class ||
			refDestination->tag_class == tags[i].tag_class_secondary ||
			refDestination->tag_class == tags[i].tag_class_tertiary) {

			tag_entry& t = tags[i];

			strValue[t.tag_path] = t.tag_index;

			if (refDestination->tag_index == t.tag_index) {
				currentStr = t.tag_path;
				current = currentStr.c_str();
			}
		}
	}

	if (ImGui::BeginCombo(prop.name.c_str(), current))
	{
		for (auto kv : strValue)
		{
			bool is_selected = (current == kv.first.c_str());
			if (ImGui::Selectable(kv.first.c_str(), is_selected)) {
				currentStr = kv.first.c_str();
				refDestination->tag_index = strValue[currentStr];
			}
			if (is_selected) {
				ImGui::SetItemDefaultFocus();
			}
		}
		ImGui::EndCombo();
	}
}

void draw_element_enum16(tag_property& prop, void* base_ptr) {
	int16_t* enumValue = (int16_t*)((char*)base_ptr + prop.local_offset);

	const char* current = nullptr;
	std::string currentStr;
	std::unordered_map<std::string, int16_t> strValue;
	bool currentValueInEnum = false;

	for (auto option : prop.reflexive) {
		if (option.type != TAG_PROPERTY_TYPE::OPTION)
			continue;
		strValue[option.name] = static_cast<int16_t>(option.option_value);
		if (strValue[option.name] == *enumValue) {
			currentValueInEnum = true;
			currentStr = option.name.c_str();
			current = currentStr.c_str();
		}
	}

	if (!currentValueInEnum) {
		currentStr = "CURRENT_VALUE_HAS_NO_STRING - " + std::to_string(*enumValue);
		strValue[currentStr] = *enumValue;
		current = currentStr.c_str();
	}

	bool currentChanged = false;
	if (ImGui::BeginCombo(prop.name.c_str(), current))
	{
		for (auto kv : strValue)
		{
			bool is_selected = (current == kv.first.c_str());
			if (ImGui::Selectable(kv.first.c_str(), is_selected)) {
				current = kv.first.c_str();
				currentChanged = true;
			}
			if (is_selected) {
				ImGui::SetItemDefaultFocus();
			}
		}
		ImGui::EndCombo();
	}

	if (currentChanged) {
		std::string newStr = current;
		*enumValue = strValue[newStr];
	}

}

void draw_element_bitmask(tag_property& prop, void* base_ptr, size_t bit_count) {
	if (ImGui::TreeNode(prop.name.c_str())) {
		uint32_t* bitmaskPtr = (uint32_t*)((char*)base_ptr + prop.local_offset);

		for (int i = 0; i < bit_count; i++) {
			auto it = std::find_if(prop.reflexive.begin(), prop.reflexive.end(), 
				[i,bit_count](const tag_property& tp) { if (((bit_count - 1) - tp.option_value) == i) { return true; } return false; });

			if (it != prop.reflexive.end()) {
				uint32_t bit = (bit_count - 1) - it->option_value; // mask values are in reverse bit order
				bool v = (*bitmaskPtr & (1U << bit)) > 0;
				if (ImGui::Checkbox(it->name.c_str(), &v)) {
					*bitmaskPtr ^= 1U << bit;
				}
			}
			else {
				uint32_t bit = i;
				bool v = (*bitmaskPtr & (1U << bit)) > 0;
				std::string bitString = "UNKNOWN BIT #" + std::to_string(bit);
				if (ImGui::Checkbox(bitString.c_str(), &v)) {
					*bitmaskPtr ^= 1U << bit;
				}
			}
		}

		ImGui::TreePop();
	}
}

void imgui_draw_element(tag_property& prop, void* base_ptr) {
	ImGui::PushID(prop.name.c_str());
	switch (prop.type) {
	case TAG_PROPERTY_TYPE::FLOAT:
		draw_element_float(prop, base_ptr);
		break;
	case TAG_PROPERTY_TYPE::INT8:
		draw_element_int8(prop, base_ptr);
		break;
	case TAG_PROPERTY_TYPE::INT16:
		draw_element_int16(prop, base_ptr);
		break;
	case TAG_PROPERTY_TYPE::STRING4:
		draw_element_string(prop, base_ptr, 4);
		break;
	case TAG_PROPERTY_TYPE::STRING32:
		draw_element_string(prop, base_ptr, 32);
		break;
	case TAG_PROPERTY_TYPE::INT32:
		draw_element_int32(prop, base_ptr);
		break;
	case TAG_PROPERTY_TYPE::COLOR_RGBA8:
		draw_element_rgba8(prop, base_ptr);
		break;
	case TAG_PROPERTY_TYPE::COLOR_RGB32:
		draw_element_rgb32(prop, base_ptr);
		break;
	case TAG_PROPERTY_TYPE::REFERENCE:
		draw_element_reference(prop, base_ptr);
		break;
	case TAG_PROPERTY_TYPE::REFLEXIVE:
		if (ImGui::TreeNode(prop.name.c_str())) {

			tag_reflexive* reflexive = (tag_reflexive*)((char*)base_ptr + prop.local_offset);
			if (reflexive->data_address != nullptr) {
				void* reflex_addr = reflexive->data_address;
				for (int s = 0; s < reflexive->data_count; s++) {
					ImGui::PushID(s);
					if(ImGui::TreeNode(std::to_string(s).c_str())) {
						for (int i = 0; i < prop.reflexive.size(); i++) {
							auto& sub_property = prop.reflexive[i];
							imgui_draw_element(sub_property, reflex_addr);
						}
						ImGui::TreePop();
					}
					reflex_addr = (char*)reflex_addr + prop.reflexive_size;
					ImGui::PopID();
				}
			}

			ImGui::TreePop();
		}
		break;
	case TAG_PROPERTY_TYPE::ENUM16:
		draw_element_enum16(prop, base_ptr);
		break;
	case TAG_PROPERTY_TYPE::BITMASK8:
		draw_element_bitmask(prop, base_ptr, 8);
		break;
	case TAG_PROPERTY_TYPE::BITMASK16:
		draw_element_bitmask(prop, base_ptr, 16);
		break;
	case TAG_PROPERTY_TYPE::BITMASK32:
		draw_element_bitmask(prop, base_ptr, 32);
		break;
	default:
		break;
	}
	ImGui::PopID();
}

void plugin::imgui_draw_plugin(halo::mapdata::tag_entry& tag)
{
	for (auto& prop : properties) {
		imgui_draw_element(prop, tag.tag_data);
	}
}
