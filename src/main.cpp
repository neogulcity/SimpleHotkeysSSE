// Global variables stored EquipSet Data / EquipSet Data 전역 변수
std::vector<RE::BSFixedString> SH_EquipSetName;
std::vector<bool> SH_IsCycleEquipSet;
std::vector<int32_t> SH_Hotkey;
std::vector<uint32_t> SH_Shift;
std::vector<uint32_t> SH_Ctrl;
std::vector<uint32_t> SH_Alt;
std::vector<uint32_t> SH_EquipSound;
std::vector<uint32_t> SH_EqUeq;
std::vector<uint32_t> SH_Req;
std::vector<uint32_t> SH_CyclePersist;
std::vector<float> SH_CycleTimeout;
std::vector<uint32_t> SH_UeqRight;
std::vector<uint32_t> SH_UeqLeft;
std::vector<uint32_t> SH_UeqShout;
std::vector<uint32_t> SH_IsSetRighthand;
std::vector<RE::TESForm*> SH_Righthand;
std::vector<uint32_t> SH_IsSetLefthand;
std::vector<RE::TESForm*> SH_Lefthand;
std::vector<uint32_t> SH_IsSetShout;
std::vector<RE::TESForm*> SH_Shout;
std::vector<uint32_t> SH_ItemsAddedCount;
std::vector<uint32_t> SH_CycleEquipSetAddedCount;
std::vector<std::vector<RE::TESForm*>> SH_Items;
std::vector<std::vector<RE::BSFixedString>> SH_EquipSets;
std::vector<int32_t> SH_Input_Keycode;
std::vector<uint32_t> SH_CurCycle;
std::vector<int32_t> SH_CycleStoredIndex;
std::vector<RE::ExtraDataList*> SH_Righthand_ExtraList;
std::vector<RE::ExtraDataList*> SH_Lefthand_ExtraList;
std::vector<std::vector<RE::ExtraDataList*>> SH_Items_ExtraList;
//=========================================================
RE::TESForm* DummyDagger;
RE::TESForm* DummyShout;


std::vector<bool> SH_CycleLock;
std::vector<float> SH_CycleTCount;
bool CloseThread = false;

bool IsMenuOpen(RE::UI* UI)
{
	bool result = false;
	if (!UI)
		return result;

	if (UI->IsMenuOpen("InventoryMenu")) result = true;
	else if (UI->IsMenuOpen("ContainerMenu")) result = true;
	else if (UI->IsMenuOpen("MagicMenu")) result = true;
	else if (UI->IsMenuOpen("MapMenu")) result = true;
	else if (UI->IsMenuOpen("StatsMenu")) result = true;
	else if (UI->IsMenuOpen("TweenMenu")) result = true;
	else if (UI->IsMenuOpen("Quantity Menu")) result = true;
	else if (UI->IsMenuOpen("Console")) result = true;
	else if (UI->IsMenuOpen("BarterMenu")) result = true;
	else if (UI->IsMenuOpen("Book Menu")) result = true;
	else if (UI->IsMenuOpen("Journal Menu")) result = true;
	else if (UI->IsMenuOpen("LevelUp Menu")) result = true;
	else if (UI->IsMenuOpen("Training Menu")) result = true;
	
	return result;
}

class Timer
{
public:
	void CreateTimer(int32_t id, float input);
};

void Timer_EndFunction(int32_t id)
{
	SH_CurCycle[id] = 0;
	SH_CycleStoredIndex[id] = id;
	SH_CycleTCount[id] = 0.0;
	SH_CycleLock[id] = false;
}

int32_t Timer_Function(int32_t id, int32_t input)
{
	auto UI = RE::UI::GetSingleton();
	if (!UI)
		return 0;

	float MAX = float(input);
	while (!CloseThread) {
		if (SH_CycleTCount[id] < MAX) {
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
			if (!CloseThread && !IsMenuOpen(UI))
				SH_CycleTCount[id] += 100.0;

		} else
			break;
	}

	if (!CloseThread)
		Timer_EndFunction(id);

	return 0;
}

void Timer::CreateTimer(int32_t id, float input)
{
	std::thread t1(Timer_Function, id, int32_t(input * 1000));
	t1.detach();
}

void CloseAllThread()
{
	CloseThread = true;
	std::this_thread::sleep_for(std::chrono::milliseconds(200));
	CloseThread = false;
}

//Get BGSEquipSlot EitherHand. It's currently not used. // 현재 사용되지 않음.
RE::BGSEquipSlot* GetEitherHandSlot()
{
	using func_t = decltype(GetEitherHandSlot);
	REL::Relocation<func_t> func{ REL::ID(23152) };
	return func();
}

//Get BGSEquipSlot RightHand
RE::BGSEquipSlot* GetRightHandSlot()
{
	using func_t = decltype(GetRightHandSlot);
	REL::Relocation<func_t> func{ REL::ID(23151) };
	return func();
}

//Get BGSEquipSlot LeftHand
RE::BGSEquipSlot* GetLeftHandSlot()
{
	using func_t = decltype(GetLeftHandSlot);
	REL::Relocation<func_t> func{ REL::ID(23150) };
	return func();
}

//Return all items player equipped except Spell, Shout // Spell, Shout을 제외한 플레이어가 착용한 모든 아이템을 리턴.
std::vector<RE::TESForm*> GetAllEquippedItems()
{
	std::vector<RE::TESForm*> result;

	auto playerref = RE::PlayerCharacter::GetSingleton();
	if (!playerref)
		return result;

	auto inv = playerref->GetInventory();
	for (const auto& [item, data] : inv) {
		if (item->Is(RE::FormType::LeveledItem)) {
			continue;
		}
		const auto& [count, entry] = data;
		if (count > 0 && entry->IsWorn()) {
			result.push_back(item);
		}
	}

	return result;
}

//Return true if Player has item // 플레이어의 아이템 소지 여부 반환.
bool HasItem(RE::TESForm* a_form, RE::ExtraDataList* a_extralist)
{
	bool result = false;

	if (!a_form)
		return result;

	if (a_form == DummyDagger)
		return true;

	auto playerref = RE::PlayerCharacter::GetSingleton();
	if (!playerref)
		return result;

	auto inv = playerref->GetInventory();
	for (const auto& [item, data] : inv) {
		if (item->Is(RE::FormType::LeveledItem)) {
			continue;
		}
		const auto& [count, entry] = data;
		if (count > 0 && entry->IsFavorited()) {
			if (item->GetName() == a_form->GetName() && entry->extraLists->front() == a_extralist) {
				result = true;
				break;
			}
		}
	}

	return result;
}

//Return name of all forms in form array // 배열에 있는 모든 form 의 이름을 리턴.
std::vector<std::string> GetStringArrayFromFormArray(std::vector<RE::TESForm*> a_form)
{
	std::vector<std::string> result;
	for (int i = 0; i < a_form.size(); ++i)
		result.push_back(a_form[i]->GetName());

	return result;
}

//Return is magic favorited or not // Spell 이 즐겨찾기되어 있는지 아닌지 bool 값 리턴.
bool IsMagicFavorited(RE::TESForm* a_form)
{
	if (!a_form)
		return false;

	if (a_form->Is(RE::FormType::Spell, RE::FormType::Shout)) {
		const auto magicFavorites = RE::MagicFavorites::GetSingleton();
		if (magicFavorites && std::ranges::find(magicFavorites->spells, a_form) != magicFavorites->spells.end()) {
			return true;
		}
	}
	return false;
}

//Equip Item
void EquipItem(RE::TESForm* a_form, RE::BGSEquipSlot* a_slot, bool a_sound, RE::ExtraDataList* a_extralist, bool a_queue = true, bool a_force = false)
{
	if (!a_form)
		return;

	RE::ActorEquipManager* equipManager = RE::ActorEquipManager::GetSingleton();
	if (!equipManager)
		return;

	auto playerref = RE::PlayerCharacter::GetSingleton();
	if (!playerref)
		return;

	//Check FormType Spell
	if (a_form->Is(RE::FormType::Spell)) {
		RE::SpellItem* a_spell = a_form->As<RE::SpellItem>();

		equipManager->EquipSpell(playerref, a_spell, a_slot);

		//Check FormType Shout
	} else if (a_form->Is(RE::FormType::Shout)) {
		RE::TESShout* a_shout = a_form->As<RE::TESShout>();

		equipManager->EquipShout(playerref, a_shout);

		//Items
	} else {
		if (a_form->GetFormType() == RE::FormType::Light) {
			RE::TESBoundObject* a_object = a_form->As<RE::TESBoundObject>();

			equipManager->EquipObject(playerref, a_object, nullptr, 1U, a_slot, a_queue, a_force, a_sound, false);
		} else if (HasItem(a_form, a_extralist)) {
			RE::TESBoundObject* a_object = a_form->As<RE::TESBoundObject>();

			equipManager->EquipObject(playerref, a_object, a_extralist, 1U, a_slot, a_queue, a_force, a_sound, false);
		}
	}
}

//Unequip Item
void UnequipItem(RE::TESForm* a_form, RE::BGSEquipSlot* a_slot, bool a_sound, RE::ExtraDataList* a_list, bool a_queue = false, bool a_force = false)
{
	if (!a_form)
		return;

	RE::ActorEquipManager* equipManager = RE::ActorEquipManager::GetSingleton();
	if (!equipManager)
		return;

	auto playerref = RE::PlayerCharacter::GetSingleton();
	if (!playerref)
		return;

	if (a_form->Is(RE::FormType::Shout)) {
		//Need to figure out Unequip Shout // Shout Unequip 은 아직 확인이 더 필요.

	} else {
		RE::TESBoundObject* a_object = a_form->As<RE::TESBoundObject>();
		equipManager->UnequipObject(playerref, a_object, a_list, 1U, a_slot, a_queue, a_force, a_sound, false, nullptr);
	}
}

//Get Favorited Items exclude weapons and spells // weapons, spells 을 제외하고 즐겨찾기 되어있는 Items 을 리턴.
std::vector<RE::TESForm*> SH_GetFavItems(RE::StaticFunctionTag*)
{
	/*	
	*	Armor
	*	Ammo
	*	AlchemyItem
	*	Scroll
	*	Ingredient
	*	Misc
	*	SoulGem
	*	ETC
	*/
	std::vector<RE::TESForm*> result;
	std::vector<std::pair<std::string, RE::TESForm*>> temp_armor, temp_ammo, temp_alchemy, temp_scroll, temp_ingredient, temp_misc, temp_soulgem, temp_etc;
	uint32_t armor_count = 0, ammo_count = 0, alchemy_count = 0, scroll_count = 0, ingredient_count = 0, misc_count = 0, soulgem_count = 0, etc_count = 0;

	auto playerref = RE::PlayerCharacter::GetSingleton();
	if (!playerref)
		return result;

	//Get Favorited Inventory Items form and name then pair it to temp variable. // Inventory 에서 즐겨찾기 되어 있는 items 의 form 과 이름을 합쳐서 임시 변수에 선언
	auto inv = playerref->GetInventory();
	for (const auto& [item, data] : inv) {
		if (item->Is(RE::FormType::LeveledItem, RE::FormType::Weapon)) {
			continue;
		}
		const auto& [count, entry] = data;
		if (count > 0 && entry->IsFavorited()) {
			if (item->Is(RE::FormType::Armor)) {
				++armor_count;
				temp_armor.push_back(std::pair<std::string, RE::TESForm*>(item->GetName(), item));
			} else if (item->Is(RE::FormType::Ammo)) {
				++ammo_count;
				temp_ammo.push_back(std::pair<std::string, RE::TESForm*>(item->GetName(), item));
			} else if (item->Is(RE::FormType::AlchemyItem)) {
				++alchemy_count;
				temp_alchemy.push_back(std::pair<std::string, RE::TESForm*>(item->GetName(), item));
			} else if (item->Is(RE::FormType::Scroll)) {
				++scroll_count;
				temp_scroll.push_back(std::pair<std::string, RE::TESForm*>(item->GetName(), item));
			} else if (item->Is(RE::FormType::Ingredient)) {
				++ingredient_count;
				temp_ingredient.push_back(std::pair<std::string, RE::TESForm*>(item->GetName(), item));
			} else if (item->Is(RE::FormType::Misc)) {
				++misc_count;
				temp_misc.push_back(std::pair<std::string, RE::TESForm*>(item->GetName(), item));
			} else if (item->Is(RE::FormType::SoulGem)) {
				++soulgem_count;
				temp_soulgem.push_back(std::pair<std::string, RE::TESForm*>(item->GetName(), item));
			} else {
				++etc_count;
				temp_etc.push_back(std::pair<std::string, RE::TESForm*>(item->GetName(), item));
			}
		}
	}

	// Sort by name
	sort(temp_armor.begin(), temp_armor.end());
	sort(temp_ammo.begin(), temp_ammo.end());
	sort(temp_alchemy.begin(), temp_alchemy.end());
	sort(temp_scroll.begin(), temp_scroll.end());
	sort(temp_ingredient.begin(), temp_ingredient.end());
	sort(temp_misc.begin(), temp_misc.end());
	sort(temp_soulgem.begin(), temp_soulgem.end());
	sort(temp_etc.begin(), temp_etc.end());

	// push back to result
	for (uint32_t i = 0; i < armor_count; ++i) {
		result.push_back(temp_armor[i].second);
	}

	for (uint32_t i = 0; i < ammo_count; ++i) {
		result.push_back(temp_ammo[i].second);
	}

	for (uint32_t i = 0; i < alchemy_count; ++i) {
		result.push_back(temp_alchemy[i].second);
	}

	for (uint32_t i = 0; i < scroll_count; ++i) {
		result.push_back(temp_scroll[i].second);
	}

	for (uint32_t i = 0; i < ingredient_count; ++i) {
		result.push_back(temp_ingredient[i].second);
	}

	for (uint32_t i = 0; i < misc_count; ++i) {
		result.push_back(temp_misc[i].second);
	}

	for (uint32_t i = 0; i < soulgem_count; ++i) {
		result.push_back(temp_soulgem[i].second);
	}

	for (uint32_t i = 0; i < etc_count; ++i) {
		result.push_back(temp_etc[i].second);
	}

	return result;
}

//Get Favorited Weapons and Spells // 즐겨찾기 되어 있는 weapons, spells 리턴.
std::vector<RE::TESForm*> SH_GetFavWeapSpells(RE::StaticFunctionTag*)
{
	std::vector<RE::TESForm*> result;
	uint32_t weapcount = 0, spellcount = 0;
	std::vector<std::pair<std::string, RE::TESForm*>> temp_weaps, temp_spells;

	auto playerref = RE::PlayerCharacter::GetSingleton();
	if (!playerref)
		return result;

	auto inv = playerref->GetInventory();

	for (const auto& [item, data] : inv) {
		if (item->Is(RE::FormType::LeveledItem)) {
			continue;
		}
		const auto& [count, entry] = data;
		if (count > 0 && item->Is(RE::FormType::Weapon) && entry->IsFavorited()) {
			++weapcount;
			temp_weaps.push_back(std::pair<std::string, RE::TESForm*>(item->GetName(), item));
			//logger::info("Favorited Item : {}", entry->GetDisplayName());
		}
	}

	sort(temp_weaps.begin(), temp_weaps.end());

	uint32_t a_num = playerref->GetSpellCount(playerref);
	//logger::info("GetSpellCount : {}", a_num);

	for (uint32_t i = 0; i < a_num; ++i) {
		RE::SpellItem* a_spell = playerref->GetNthSpell(playerref, i);
		RE::TESForm* a_form = a_spell->As<RE::TESForm>();
		if (!a_form)
			return result;

		if (IsMagicFavorited(a_form)) {
			++spellcount;
			temp_spells.push_back(std::pair<std::string, RE::TESForm*>(a_form->GetName(), a_form));
			//logger::info("Favorited Spell : {}", a_spell->As<RE::TESForm>()->GetName());
		}
	}

	sort(temp_spells.begin(), temp_spells.end());

	for (uint32_t i = 0; i < weapcount; ++i) {
		result.push_back(temp_weaps[i].second);
		//logger::info("Weapon : {}", temp_weaps[i].first);
	}

	for (uint32_t i = 0; i < spellcount; ++i) {
		result.push_back(temp_spells[i].second);
		//logger::info("Spell : {}", temp_spells[i].first);
	}

	return result;
}

//Get Equipped Shout Papyrus Function
RE::TESForm* SH_GetEquippedShout(RE::StaticFunctionTag*)
{
	auto playerref = RE::PlayerCharacter::GetSingleton();
	if (!playerref)
		return nullptr;

	RE::TESForm* a_shout = nullptr;
	a_shout = playerref->GetEquippedShout(playerref);
	if (!a_shout)
		return nullptr;

	//logger::info("Equipped Shout : {}", a_shout->GetName());

	return a_shout;
}

//Get Equipped Shout
RE::TESForm* GetEquippedShout()
{
	auto playerref = RE::PlayerCharacter::GetSingleton();
	if (!playerref)
		return nullptr;

	RE::TESForm* a_shout = nullptr;
	a_shout = playerref->GetEquippedShout(playerref);
	if (!a_shout)
		return nullptr;

	//logger::info("Equipped Shout : {}", a_shout->GetName());

	return a_shout;
}

//Return bool false <-> true
bool SH_ReturnToggleBool(RE::StaticFunctionTag*, bool a_bool)
{
	return a_bool ? false : true;
}

//Return all name of forms in array. // form 배열에 있는 모든 이름을 리턴.
std::vector<RE::BSFixedString> SH_GetFormsName(RE::StaticFunctionTag*, std::vector<RE::TESForm*> a_form)
{
	std::vector<RE::BSFixedString> result;

	for (uint32_t i = 0; i < a_form.size(); ++i)
		result.push_back(a_form[i]->GetName());

	return result;
}

//Return string array size Papyrus Function // String 배열 크기 리턴.
uint32_t SH_GetStringArraySize(RE::StaticFunctionTag*, std::vector<RE::BSFixedString> a_string)
{
	return (uint32_t)a_string.size();
}

//Return form array size Papyrus Function // Form 배열 크기 리턴.
uint32_t SH_GetFormArraySize(RE::StaticFunctionTag*, std::vector<RE::TESForm*> a_form)
{
	return (uint32_t)a_form.size();
}

//Push form last in form array. // form 배열의 마지막에 추가.
std::vector<RE::TESForm*> SH_PushLastInArray(RE::StaticFunctionTag*, RE::TESForm* src_form, std::vector<RE::TESForm*> desc_form)
{
	std::vector<RE::TESForm*> result;
	for (uint32_t i = 0; i < desc_form.size(); ++i) {
		result.push_back(desc_form[i]);
	}

	result.push_back(src_form);

	return result;
}

//Push string first in string array. // String 배열의 첫 번째에 추가.
std::vector<RE::BSFixedString> SH_PushFirstInArray(RE::StaticFunctionTag*, RE::BSFixedString src_string, std::vector<RE::BSFixedString> desc_string)
{
	std::vector<RE::BSFixedString> result;

	result.push_back(src_string);

	for (uint32_t i = 0; i < desc_string.size(); ++i)
		result.push_back(desc_string[i]);

	return result;
}

//Return index of matched string in array. // String 배열에서 일치하는 String 의 index 를 리턴.
uint32_t SH_FindIndexInStringArray(RE::StaticFunctionTag*, RE::BSFixedString a_string, std::vector<RE::BSFixedString> b_string)
{
	uint32_t result = 0;

	for (uint32_t i = 0; i < b_string.size(); ++i) {
		if (a_string == b_string[i]) {
			return result;
		} else {
			++result;
		}
	}

	return (uint32_t)-1;
}

//Check EquipSet name conflict, blank or '.' // EquipSet 이름이 겹치는지 공란인지 '.' 을 포함하는지 확인
uint32_t SH_CheckString(RE::StaticFunctionTag*, RE::BSFixedString a_string, std::vector<RE::BSFixedString> b_string)
{
	for (uint32_t i = 0; i < b_string.size(); ++i) {
		//std::string test_string = static_cast<std::string>(b_string[i]);
		//logger::info("EquipSetNameList{} : {}", i, test_string);
		if (a_string == b_string[i]) {
			return (uint32_t)1;  // Result = 1 : EquipSet name conflicts with other EquipSet. // Result = 1 : EquipSet 이름이 다른 EquipSet과 겹침
		}
	}
	std::string ab_string = static_cast<std::string>(a_string);

	std::string temp_string = ab_string;
	temp_string.erase(remove(temp_string.begin(), temp_string.end(), ' '), temp_string.end());

	if (temp_string == "")
		return (uint32_t)2;  // Result = 2 : EquipSet named blank // Result = 2 : EquipSet 이름이 공란임

	else if (ab_string.find(".") != std::string::npos)
		return (uint32_t)3;  // Result = 3 : EquipSet named with '.' : Json path solve using '.' so, EquipSet should not be named with '.' // Result = 3 : EquipSet 이름이 '.' 을 포함. Json 에서 '.' 을 이용하기 때문에 '.' 을 포함하면 안됨.

	else if (ab_string.find("[") != std::string::npos || ab_string.find("]") != std::string::npos)
		return (uint32_t)4;  // Result = 4 : EquipSet named with '[' or ']' // Result = 4 : EquipSet 이름이 '[' 또는 ']'을 포함.

	else
		(uint32_t)0;

	return (uint32_t)0;
}

std::vector<RE::BSFixedString> SH_ArraySort(RE::StaticFunctionTag*, std::vector<RE::BSFixedString> src_string, uint32_t Count, uint32_t Type = 3)
{
	std::vector<std::string> temp_string;
	uint32_t MAX = uint32_t(src_string.size()) - Count;

	for (int i = 0; i < src_string.size(); ++i)
		temp_string.push_back(static_cast<std::string>(src_string[i]));

	for (uint32_t i = 0; i < MAX; ++i) {
		temp_string.pop_back();
	}

	// Ascending order
	if (Type == 1) {
		sort(temp_string.begin(), temp_string.end());
	}

	// Descending order
	else if (Type == 2) {
		sort(temp_string.begin(), temp_string.end(), std::greater<std::string>());
	}

	// Creation Descending order
	else if (Type == 4) {
		reverse(temp_string.begin(), temp_string.end());
	}

	// Creation Ascending order
	else {

	}

	std::vector<RE::BSFixedString> result;
	for (int i = 0; i < temp_string.size(); ++i)
		result.push_back(static_cast<RE::BSFixedString>(temp_string[i]));
	
	return result;
}

//Initialize EquipSet Global variables
void SH_InitializeSKSE_EquipSetData(RE::StaticFunctionTag*)
{
	std::vector<RE::BSFixedString> a_SH_EquipSetName;
	std::vector<bool> a_SH_IsCycleEquipSet;
	std::vector<int32_t> a_SH_Hotkey;
	std::vector<uint32_t> a_SH_Shift;
	std::vector<uint32_t> a_SH_Ctrl;
	std::vector<uint32_t> a_SH_Alt;
	std::vector<uint32_t> a_SH_EquipSound;
	std::vector<uint32_t> a_SH_EqUeq;
	std::vector<uint32_t> a_SH_Req;
	std::vector<uint32_t> a_SH_UeqRight;
	std::vector<uint32_t> a_SH_UeqLeft;
	std::vector<uint32_t> a_SH_UeqShout;
	std::vector<uint32_t> a_SH_CyclePersist;
	std::vector<float> a_SH_CycleTimeout;
	std::vector<uint32_t> a_SH_IsSetRighthand;
	std::vector<RE::TESForm*> a_SH_Righthand;
	std::vector<uint32_t> a_SH_IsSetLefthand;
	std::vector<RE::TESForm*> a_SH_Lefthand;
	std::vector<uint32_t> a_SH_IsSetShout;
	std::vector<RE::TESForm*> a_SH_Shout;
	std::vector<uint32_t> a_SH_ItemsAddedCount;
	std::vector<uint32_t> a_SH_CycleEquipSetAddedCount;
	std::vector<std::vector<RE::TESForm*>> a_SH_Items;
	std::vector<std::vector<RE::BSFixedString>> a_SH_EquipSets;
	std::vector<int32_t> a_SH_Input_Keycode;
	std::vector<uint32_t> a_SH_CurCycle;
	std::vector<int32_t> a_SH_CycleStoredIndex;
	std::vector<RE::ExtraDataList*> a_SH_Righthand_ExtraList;
	std::vector<RE::ExtraDataList*> a_SH_Lefthand_ExtraList;
	std::vector<std::vector<RE::ExtraDataList*>> a_SH_Items_ExtraList;
	std::vector<bool> a_SH_CycleLock;
	std::vector<float> a_SH_CycleTCount;

	SH_EquipSetName = a_SH_EquipSetName;
	SH_IsCycleEquipSet = a_SH_IsCycleEquipSet;
	SH_Hotkey = a_SH_Hotkey;
	SH_Shift = a_SH_Shift;
	SH_Ctrl = a_SH_Ctrl;
	SH_Alt = a_SH_Alt;
	SH_EquipSound = a_SH_EquipSound;
	SH_EqUeq = a_SH_EqUeq;
	SH_Req = a_SH_Req;
	SH_UeqRight = a_SH_UeqRight;
	SH_UeqLeft = a_SH_UeqLeft;
	SH_UeqShout = a_SH_UeqShout;
	SH_CyclePersist = a_SH_CyclePersist;
	SH_CycleTimeout = a_SH_CycleTimeout;
	SH_IsSetRighthand = a_SH_IsSetRighthand;
	SH_Righthand = a_SH_Righthand;
	SH_IsSetLefthand = a_SH_IsSetLefthand;
	SH_Lefthand = a_SH_Lefthand;
	SH_IsSetShout = a_SH_IsSetShout;
	SH_Shout = a_SH_Shout;
	SH_ItemsAddedCount = a_SH_ItemsAddedCount;
	SH_CycleEquipSetAddedCount = a_SH_CycleEquipSetAddedCount;
	SH_Items = a_SH_Items;
	SH_EquipSets = a_SH_EquipSets;
	SH_Input_Keycode = a_SH_Input_Keycode;
	SH_CurCycle = a_SH_CurCycle;
	SH_CycleStoredIndex = a_SH_CycleStoredIndex;
	SH_Righthand_ExtraList = a_SH_Righthand_ExtraList;
	SH_Lefthand_ExtraList = a_SH_Lefthand_ExtraList;
	SH_Items_ExtraList = a_SH_Items_ExtraList;
	SH_CycleLock = a_SH_CycleLock;
	SH_CycleTCount = a_SH_CycleTCount;

	std::thread t2(CloseAllThread);
	t2.detach();
}

//Get EquipSet Data form Papyrus
void SH_EquipSetDataToSKSE(RE::StaticFunctionTag*, bool IsCycle, RE::BSFixedString a_EquipSetName, int32_t a_Hotkey, uint32_t a_Shift, uint32_t a_Ctrl, uint32_t a_Alt, uint32_t a_EquipSound, uint32_t a_EqUeq, uint32_t a_Req, uint32_t a_CyclePersist, float a_CycleTimeout, uint32_t a_UeqRight, uint32_t a_UeqLeft, uint32_t a_UeqShout, uint32_t a_IsSetRighthand, RE::TESForm* a_Righthand, uint32_t a_IsSetLefthand, RE::TESForm* a_Lefthand, uint32_t a_IsSetShout, RE::TESForm* a_Shout, uint32_t a_ItemsAddedCount, std::vector<RE::TESForm*> a_Items, uint32_t a_CycleEquipSetAddedCount, std::vector<RE::BSFixedString> a_EquipSets)
{
	SH_EquipSetName.push_back(a_EquipSetName);
	SH_Hotkey.push_back(a_Hotkey);
	SH_Shift.push_back(a_Shift);
	SH_Ctrl.push_back(a_Ctrl);
	SH_Alt.push_back(a_Alt);
	SH_EquipSound.push_back(a_EquipSound);
	SH_EqUeq.push_back(a_EqUeq);
	SH_Req.push_back(a_Req);
	SH_UeqRight.push_back(a_UeqRight);
	SH_UeqLeft.push_back(a_UeqLeft);
	SH_UeqShout.push_back(a_UeqShout);
	SH_CyclePersist.push_back(a_CyclePersist);
	SH_CycleTimeout.push_back(a_CycleTimeout);
	SH_IsSetRighthand.push_back(a_IsSetRighthand);
	SH_Righthand.push_back(a_Righthand);
	SH_IsSetLefthand.push_back(a_IsSetLefthand);
	SH_Lefthand.push_back(a_Lefthand);
	SH_IsSetShout.push_back(a_IsSetShout);
	SH_Shout.push_back(a_Shout);
	SH_ItemsAddedCount.push_back(a_ItemsAddedCount);
	SH_Items.push_back(a_Items);

	SH_CycleEquipSetAddedCount.push_back(a_CycleEquipSetAddedCount);
	SH_EquipSets.push_back(a_EquipSets);

	if (IsCycle)
		SH_IsCycleEquipSet.push_back(true);
	else
		SH_IsCycleEquipSet.push_back(false);

	SH_CurCycle.push_back((uint32_t)0);
	SH_CycleStoredIndex.push_back((int32_t)-1);

	SH_CycleLock.push_back(false);
	SH_CycleTCount.push_back(0.0);

	auto playerref = RE::PlayerCharacter::GetSingleton();
	auto inv = playerref->GetInventory();

	//Righthand ExtraDataList
	bool righthandset = false;
	if (a_IsSetRighthand == 1) {
		for (const auto& [item, data] : inv) {
			//if (item->Is(RE::FormType::LeveledItem, RE::FormType::Weapon)) {
			//	continue;
			//}
			const auto& [count, entry] = data;

			if (count > 0 && entry->IsFavorited()) {
				if (item->GetName() == a_Righthand->GetName() && !righthandset) {
					righthandset = true;
					SH_Righthand_ExtraList.push_back(entry->extraLists->front());
					break;
				}
			}
		}
	}

	if (!righthandset)
		SH_Righthand_ExtraList.push_back(nullptr);

	//Lefthand ExtraDataList
	bool lefthandset = false;
	if (a_IsSetLefthand == 1) {
		for (const auto& [item, data] : inv) {
			//if (item->Is(RE::FormType::LeveledItem, RE::FormType::Weapon)) {
			//	continue;
			//}
			const auto& [count, entry] = data;

			if (count > 0 && entry->IsFavorited()) {
				if (item->GetName() == a_Lefthand->GetName() && !lefthandset) {
					lefthandset = true;
					SH_Lefthand_ExtraList.push_back(entry->extraLists->front());
					break;
				}
			}
		}
	}

	if (!lefthandset)
		SH_Lefthand_ExtraList.push_back(nullptr);

	//Items ExtraDataList
	std::vector<RE::ExtraDataList*> temp_extralist;
	for (uint32_t i = 0; i < a_ItemsAddedCount; ++i) {
		bool itemsset = false;
		for (const auto& [item, data] : inv) {
			//if (item->Is(RE::FormType::LeveledItem, RE::FormType::Weapon)) {
			//	continue;
			//}
			const auto& [count, entry] = data;

			if (count > 0 && entry->IsFavorited()) {
				if (item->GetName() == a_Items[i]->GetName() && !itemsset) {
					itemsset = true;
					temp_extralist.push_back(entry->extraLists->front());
					break;
				}
			}
		}

		if (!itemsset)
			temp_extralist.push_back(nullptr);
	}

	SH_Items_ExtraList.push_back(temp_extralist);

	/*
	bool righthandset = false, lefthandset = false;
	std::vector<RE::ExtraDataList*> temp_extralist;
	for (const auto& [item, data] : inv) {
		//if (item->Is(RE::FormType::LeveledItem, RE::FormType::Weapon)) {
		//	continue;
		//}
		const auto& [count, entry] = data;
		
		if (count > 0 && entry->IsFavorited()) {
			if (item->GetName() == a_Righthand->GetName() && !righthandset) {
				righthandset = true;
				SH_Righthand_ExtraList.push_back(entry->extraLists->front());
			}

			if (item->GetName() == a_Lefthand->GetName() && !lefthandset) {
				lefthandset = true;
				SH_Lefthand_ExtraList.push_back(entry->extraLists->front());
			}

			for (int i = 0; i < a_ItemsAddedCount; ++i) {
				if (item->GetName() == a_Items[i]->GetName()) {
					temp_extralist.push_back(entry->extraLists->front());
				}
			}
		}
	}
	SH_Items_ExtraList.push_back(temp_extralist);
	*/

	//Modify keycode with Shift(1000), Ctrl(2000), Alt(4000) so, "Shift + Ctrl + Y" = 3021
	if (a_Hotkey != -1) {
		int32_t result = a_Hotkey;

		if (a_Shift == 1)
			result = result + (int32_t)1000;

		if (a_Ctrl == 1)
			result = result + (int32_t)2000;

		if (a_Alt == 1)
			result = result + (int32_t)4000;

		SH_Input_Keycode.push_back(result);
	} else {
		SH_Input_Keycode.push_back((int32_t)-1);
	}

	/*
	int a_int = (int)SH_EquipSetName.size() - 1;
	
	if (IsCycle) {
		
		logger::info("==================================");
		logger::info("EquipSetName : {}", SH_EquipSetName[a_int]);
		logger::info("Hotkey : {}", SH_Hotkey[a_int]);
		logger::info("Shift : {}", SH_Shift[a_int]);
		logger::info("Ctrl : {}", SH_Ctrl[a_int]);
		logger::info("Alt : {}", SH_Alt[a_int]);
		logger::info("CycleEquipSetsAddedCount : {}", SH_CycleEquipSetAddedCount[a_int]);

		for (uint32_t i = 0; i < SH_CycleEquipSetAddedCount[a_int]; ++i) {
			logger::info("EquipSets{} : {}", i, SH_EquipSets[a_int][i]);
		}

		logger::info("CycleTimeout : {}", SH_CycleTimeout[a_int]);
		
	} else {
		logger::info("==================================");
		logger::info("EquipSetName : {}", SH_EquipSetName[a_int]);
		logger::info("Hotkey : {}", SH_Hotkey[a_int]);
		logger::info("Shift : {}", SH_Shift[a_int]);
		logger::info("Ctrl : {}", SH_Ctrl[a_int]);
		logger::info("Alt : {}", SH_Alt[a_int]);
		logger::info("EquipSound : {}", SH_EquipSound[a_int]);
		logger::info("EqUeq : {}", SH_EqUeq[a_int]);
		logger::info("UeqRight : {}", SH_UeqRight[a_int]);
		logger::info("UeqLeft : {}", SH_UeqLeft[a_int]);
		logger::info("UeqShout : {}", SH_UeqShout[a_int]);
		logger::info("IsSetRighthand : {}", SH_IsSetRighthand[a_int]);
		logger::info("IsSetLefthand : {}", SH_IsSetLefthand[a_int]);
		logger::info("IsSetShout : {}", SH_IsSetShout[a_int]);
		logger::info("ItemsAddedCount : {}", SH_ItemsAddedCount[a_int]);

		for (uint32_t i = 0; i < SH_ItemsAddedCount[a_int]; ++i) {
			logger::info("Items{} : {}", i, SH_Items[a_int][i]->GetName());
		}
	}
	*/
}

//Execute SKSE Equip Function
void ExecEquip(uint32_t a_index)
{
	auto playerref = RE::PlayerCharacter::GetSingleton();

	if (!playerref)
		return;

	bool EquipRight = false, EquipLeft = false, EquipShout = false;
	bool UnequipRight = false, UnequipLeft = false, UnequipShout = false;
	std::vector<bool> EquipItems, UnequipItems;

	for (uint32_t i = 0; i < SH_ItemsAddedCount[a_index]; ++i) {
		EquipItems.push_back(false);
		UnequipItems.push_back(false);
	}

	// Unequip right option On
	if (SH_UeqRight[a_index] == 1) {
		UnequipRight = true;
	}
	// Unequip left option On
	if (SH_UeqLeft[a_index] == 1) {
		UnequipLeft = true;
	}
	// Unequip shout option On
	if (SH_UeqShout[a_index] == 1) {
		UnequipShout = true;
	}

	RE::TESForm* right_form = playerref->GetEquippedObject(false);
	RE::TESForm* left_form = playerref->GetEquippedObject(true);
	RE::TESForm* shout_form = GetEquippedShout();

	// Toggle equip/unequip option Off & Re equip option On
	if (SH_EqUeq[a_index] == 0 && SH_Req[a_index] == 1) {
		EquipRight = true;
		EquipLeft = true;
		EquipShout = true;
		UnequipRight = true;
		UnequipLeft = true;
		UnequipShout = true;
		for (uint32_t i = 0; i < SH_ItemsAddedCount[a_index]; ++i) {
			EquipItems[i] = true;
			UnequipItems[i] = true;
		}
	}

	else {
		// Toggle equip/unequip option Off & Re equip option Off
		if (SH_IsSetRighthand[a_index] == 1) {
			if (!right_form)
				EquipRight = true;

			else if (right_form->GetName() != SH_Righthand[a_index]->GetName())
				EquipRight = true;
		}
		if (SH_IsSetLefthand[a_index] == 1) {
			if (!left_form)
				EquipLeft = true;

			else if (left_form->GetName() != SH_Lefthand[a_index]->GetName())
				EquipLeft = true;
		}
		if (SH_IsSetShout[a_index] == 1) {
			if (!shout_form)
				EquipShout = true;

			else if (shout_form->GetName() != SH_Shout[a_index]->GetName())
				EquipShout = true;
		}

		uint32_t a_count = 0;
		if (SH_ItemsAddedCount[a_index] > 0) {
			std::vector<RE::TESForm*> a_form = GetAllEquippedItems();
			std::vector<std::string> a_string = static_cast<std::vector<std::string>>(GetStringArrayFromFormArray(a_form));
			std::vector<bool> a_equipped;

			for (uint32_t i = 0; i < SH_ItemsAddedCount[a_index]; ++i)
				a_equipped.push_back(false);

			for (uint32_t i = 0; i < SH_ItemsAddedCount[a_index]; ++i) {
				for (int j = 0; j < a_form.size(); ++j) {
					if (a_string[j].compare(SH_Items[a_index][i]->GetName()) == 0) {
						a_equipped[i] = true;
						++a_count;
					}
				}
			}

			for (uint32_t i = 0; i < SH_ItemsAddedCount[a_index]; ++i) {
				if (!a_equipped[i]) {
					EquipItems[i] = true;
				}
			}
		}

		if (SH_EqUeq[a_index] == 1) {
			// Toggle equip/unequip option On & Re equip option Off
			if (!EquipRight && !EquipLeft && !EquipShout && a_count == SH_ItemsAddedCount[a_index]) {
				UnequipRight = true;
				UnequipLeft = true;
				UnequipShout = true;
				for (uint32_t i = 0; i < SH_ItemsAddedCount[a_index]; ++i)
					UnequipItems[i] = true;
			}

			// Toggle equip/unequip option On & Re equip option On
			else if (SH_Req[a_index] == 1) {
				EquipRight = true;
				EquipLeft = true;
				EquipShout = true;
				UnequipRight = true;
				UnequipLeft = true;
				UnequipShout = true;
				for (uint32_t i = 0; i < SH_ItemsAddedCount[a_index]; ++i) {
					EquipItems[i] = true;
					UnequipItems[i] = true;
				}
			}
		}
	}
	if (UnequipRight) {
		if (DummyDagger) {
			EquipItem(DummyDagger, GetRightHandSlot(), false, nullptr, false, true);
			UnequipItem(DummyDagger, GetRightHandSlot(), false, nullptr, false, true);
		}
	}
	if (UnequipLeft) {
		if (DummyDagger) {
			EquipItem(DummyDagger, GetLeftHandSlot(), false, nullptr, false, true);
			UnequipItem(DummyDagger, GetLeftHandSlot(), false, nullptr, false, true);
		}
	}
	if (UnequipShout) {
		if (DummyShout) {
			EquipItem(DummyShout, nullptr, false, nullptr, false, true);
			UnequipItem(DummyShout, nullptr, false, nullptr, false, true);
		}
	}

	for (uint32_t i = 0; i < SH_ItemsAddedCount[a_index]; ++i) {
		if (UnequipItems[i])
			UnequipItem(SH_Items[a_index][i], nullptr, SH_EquipSound[a_index] == (uint32_t)1 ? true : false, SH_Items_ExtraList[a_index][i]);
	}

	if (EquipRight)
		SH_IsSetRighthand[a_index] == 1 ? EquipItem(SH_Righthand[a_index], GetRightHandSlot(), SH_EquipSound[a_index] == (uint32_t)1 ? true : false, SH_Righthand_ExtraList[a_index]) : void(0);

	if (EquipLeft)
		SH_IsSetLefthand[a_index] == 1 ? EquipItem(SH_Lefthand[a_index], GetLeftHandSlot(), SH_EquipSound[a_index] == (uint32_t)1 ? true : false, SH_Lefthand_ExtraList[a_index]) : void(0);
	
	if (EquipShout)
		SH_IsSetShout[a_index] == 1 ? EquipItem(SH_Shout[a_index], nullptr, SH_EquipSound[a_index] == (uint32_t)1 ? true : false, nullptr) : void(0);

	for (uint32_t i = 0; i < SH_ItemsAddedCount[a_index]; ++i) {
		if (EquipItems[i]) {
			if (SH_Items[a_index][i]->GetFormType() == RE::FormType::Light) {
				RE::TESForm* a_form = playerref->GetEquippedObject(false);

				if (a_form) {
					RE::WEAPON_TYPE a_type = a_form->As<RE::TESObjectWEAP>()->GetWeaponType();
					if (a_type == RE::WEAPON_TYPE::kTwoHandSword || a_type == RE::WEAPON_TYPE::kTwoHandAxe || a_type == RE::WEAPON_TYPE::kBow || a_type == RE::WEAPON_TYPE::kCrossbow) {
						EquipItem(DummyDagger, GetRightHandSlot(), false, nullptr, false, true);
						UnequipItem(DummyDagger, GetRightHandSlot(), false, nullptr, false, true);
					}
				}
			}

			EquipItem(SH_Items[a_index][i], nullptr, SH_EquipSound[a_index] == (uint32_t)1 ? true : false, SH_Items_ExtraList[a_index][i]);
		}
	}
}

//Execute EquipSet Papyrus Function
void SH_ExecEquip(RE::StaticFunctionTag*, uint32_t keycode)
{
	int a_index = -1;
	for (int i = 0; i < SH_EquipSetName.size(); ++i) {
		if (SH_Input_Keycode[i] == (int32_t)keycode) {
			a_index = i;
		}
	}

	if (a_index != -1) {
		//If not Cycle EquipSet, then execute equip Function
		if (SH_IsCycleEquipSet[a_index] != 1) {
			ExecEquip((uint32_t)a_index);

		} else {
			int b_index = -1;

			//Cycle persist option On and it's not first time to execute cycle EquipSet
			if (SH_CyclePersist[a_index] == 1 && SH_CycleStoredIndex[a_index] != -1) {
				bool IsChanged = false;
				std::vector<RE::TESForm*> a_form = GetAllEquippedItems();
				std::vector<std::string> a_string = static_cast<std::vector<std::string>>(GetStringArrayFromFormArray(a_form));

				if (SH_IsSetRighthand[SH_CycleStoredIndex[a_index]] == 1) {
					auto playerref = RE::PlayerCharacter::GetSingleton();
					if (!playerref)
						return;

					RE::TESForm* b_form = playerref->GetEquippedObject(false);

					if (!b_form)
						IsChanged = true;

					else if (b_form->GetName() != SH_Righthand[SH_CycleStoredIndex[a_index]]->GetName())
						IsChanged = true;
				}

				if (SH_IsSetLefthand[SH_CycleStoredIndex[a_index]] == 1 && !IsChanged) {
					auto playerref = RE::PlayerCharacter::GetSingleton();
					if (!playerref)
						return;

					RE::TESForm* b_form = playerref->GetEquippedObject(true);

					if (!b_form)
						IsChanged = true;

					else if (b_form->GetName() != SH_Lefthand[SH_CycleStoredIndex[a_index]]->GetName())
						IsChanged = true;
				}

				if (SH_IsSetShout[SH_CycleStoredIndex[a_index]] == 1 && !IsChanged) {
					RE::TESForm* b_form = GetEquippedShout();

					if (!b_form)
						IsChanged = true;

					else if (b_form->GetName() != SH_Shout[SH_CycleStoredIndex[a_index]]->GetName())
						IsChanged = true;
				}

				if (!IsChanged) {
					for (uint32_t i = 0; i < SH_ItemsAddedCount[SH_CycleStoredIndex[a_index]]; ++i) {
						int a_count = 0;

						for (int j = 0; j < a_form.size(); ++j) {
							if (a_string[j].compare(SH_Items[SH_CycleStoredIndex[a_index]][i]->GetName()) == 0) {
								++a_count;
							}
						}
						if (a_count == 0) {
							IsChanged = true;
							break;
						}
					}
				}

				// Equipped weapons, spells, shout, items Changed so, Turn cycle back.
				if (IsChanged) {
					SH_CurCycle[a_index] <= 0 ? SH_CurCycle[a_index] = SH_CycleEquipSetAddedCount[a_index] - (uint32_t)1 : --SH_CurCycle[a_index];
					for (int i = 0; i < SH_EquipSetName.size(); ++i) {
						if (SH_EquipSets[a_index][SH_CurCycle[a_index]] == SH_EquipSetName[i]) {
							b_index = i;
							SH_CurCycle[a_index] >= SH_CycleEquipSetAddedCount[a_index] - (uint32_t)1 ? SH_CurCycle[a_index] = 0 : ++SH_CurCycle[a_index];
							SH_CycleStoredIndex[a_index] = b_index;
							break;
						}
					}
				}

				// Equipped weapons, spells, shout, items are not changed so, Go forward.
				else {
					for (int i = 0; i < SH_EquipSetName.size(); ++i) {
						if (SH_EquipSets[a_index][SH_CurCycle[a_index]] == SH_EquipSetName[i]) {
							b_index = i;
							SH_CurCycle[a_index] >= SH_CycleEquipSetAddedCount[a_index] - (uint32_t)1 ? SH_CurCycle[a_index] = 0 : ++SH_CurCycle[a_index];
							SH_CycleStoredIndex[a_index] = b_index;
							break;
						}
					}
				}
			}
			//Cycle persist option Off or it's first time to execute cycle EquipSet
			else {
				for (int i = 0; i < SH_EquipSetName.size(); ++i) {
					if (SH_EquipSets[a_index][SH_CurCycle[a_index]] == SH_EquipSetName[i]) {
						b_index = i;
						SH_CurCycle[a_index] >= SH_CycleEquipSetAddedCount[a_index] - (uint32_t)1 ? SH_CurCycle[a_index] = 0 : ++SH_CurCycle[a_index];
						SH_CycleStoredIndex[a_index] = b_index;
						break;
					}
				}
			}
			//Execute cycle EquipSet
			if (b_index != -1) {
				ExecEquip((uint32_t)b_index);
				if (SH_CycleTimeout[a_index] != 0.0) {
					if (!SH_CycleLock[a_index]) {
						SH_CycleLock[a_index] = true;
						Timer a_timer;
						a_timer.CreateTimer(a_index, SH_CycleTimeout[a_index]);

					} else
						SH_CycleTCount[a_index] = 0.0;

				}
			}

			//Somehow can not find EquipSet. maybe EquipSet deleted so, just let cycle goes on.
			else
				SH_CurCycle[a_index] >= SH_CycleEquipSetAddedCount[a_index] - (uint32_t)1 ? SH_CurCycle[a_index] = 0 : ++SH_CurCycle[a_index];
		}
	}
}

bool SH_IsPlayerBeast(RE::StaticFunctionTag*)
{
	auto a_menu = RE::MenuControls::GetSingleton();
	return a_menu->InBeastForm();
}

void SH_DummyDaggerToSKSE(RE::StaticFunctionTag*, RE::TESForm* a_form)
{
	if (!a_form)
		return;

	DummyDagger = a_form;
}

void SH_DummyShoutToSKSE(RE::StaticFunctionTag*, RE::TESForm* a_form)
{
	if (!a_form)
		return;

	DummyShout = a_form;
}

RE::BSFixedString SH_GetVersion(RE::StaticFunctionTag*)
{
	return Version::NAME;
}

bool RegisterFuncs(RE::BSScript::Internal::VirtualMachine* a_vm)
{
	a_vm->RegisterFunction("SH_GetFavItems", "_SimpleHotkeys_MCM", SH_GetFavItems);
	a_vm->RegisterFunction("SH_GetFavWeapSpells", "_SimpleHotkeys_MCM", SH_GetFavWeapSpells);
	a_vm->RegisterFunction("SH_GetEquippedShout", "_SimpleHotkeys_MCM", SH_GetEquippedShout);
	a_vm->RegisterFunction("SH_ReturnToggleBool", "_SimpleHotkeys_MCM", SH_ReturnToggleBool);
	a_vm->RegisterFunction("SH_GetFormsName", "_SimpleHotkeys_MCM", SH_GetFormsName);
	a_vm->RegisterFunction("SH_GetStringArraySize", "_SimpleHotkeys_MCM", SH_GetStringArraySize);
	a_vm->RegisterFunction("SH_GetFormArraySize", "_SimpleHotkeys_MCM", SH_GetFormArraySize);
	a_vm->RegisterFunction("SH_PushFirstInArray", "_SimpleHotkeys_MCM", SH_PushFirstInArray);
	a_vm->RegisterFunction("SH_PushLastInArray", "_SimpleHotkeys_MCM", SH_PushLastInArray);
	a_vm->RegisterFunction("SH_FindIndexInStringArray", "_SimpleHotkeys_MCM", SH_FindIndexInStringArray);
	a_vm->RegisterFunction("SH_ArraySort", "_SimpleHotkeys_MCM", SH_ArraySort);
	a_vm->RegisterFunction("SH_CheckString", "_SimpleHotkeys_MCM", SH_CheckString);
	a_vm->RegisterFunction("SH_InitializeSKSE_EquipSetData", "_SimpleHotkeys_MCM", SH_InitializeSKSE_EquipSetData);
	a_vm->RegisterFunction("SH_EquipSetDataToSKSE", "_SimpleHotkeys_MCM", SH_EquipSetDataToSKSE);
	a_vm->RegisterFunction("SH_ExecEquip", "_SimpleHotkeys_MCM", SH_ExecEquip);
	a_vm->RegisterFunction("SH_IsPlayerBeast", "_SimpleHotkeys_MCM", SH_IsPlayerBeast);
	a_vm->RegisterFunction("SH_GetVersion", "_SimpleHotkeys_MCM", SH_GetVersion);
	a_vm->RegisterFunction("SH_DummyDaggerToSKSE", "_SimpleHotkeys_MCM", SH_DummyDaggerToSKSE);
	a_vm->RegisterFunction("SH_DummyShoutToSKSE", "_SimpleHotkeys_MCM", SH_DummyShoutToSKSE);

	return true;
}

extern "C" DLLEXPORT bool SKSEAPI SKSEPlugin_Query(const SKSE::QueryInterface* a_skse, SKSE::PluginInfo* a_info)
{
#ifndef NDEBUG
	auto sink = std::make_shared<spdlog::sinks::msvc_sink_mt>();
#else
	auto path = logger::log_directory();
	if (!path) {
		return false;
	}

	*path /= Version::PROJECT;
	*path += ".log"sv;
	auto sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(path->string(), true);
#endif

	auto log = std::make_shared<spdlog::logger>("global log"s, std::move(sink));

#ifndef NDEBUG
	log->set_level(spdlog::level::trace);
#else
	log->set_level(spdlog::level::info);
	log->flush_on(spdlog::level::warn);
#endif

	spdlog::set_default_logger(std::move(log));
	spdlog::set_pattern("%g(%#): [%^%l%$] %v"s);

	logger::info(FMT_STRING("{} v{}"), Version::PROJECT, Version::NAME);

	a_info->infoVersion = SKSE::PluginInfo::kVersion;
	a_info->name = Version::PROJECT.data();
	a_info->version = Version::MAJOR;

	if (a_skse->IsEditor()) {
		logger::critical("Loaded in editor, marking as incompatible"sv);
		return false;
	}

	const auto ver = a_skse->RuntimeVersion();
	if (ver < SKSE::RUNTIME_1_5_39) {
		logger::critical(FMT_STRING("Unsupported runtime version {}"), ver.string());
		return false;
	}

	return true;
}

extern "C" DLLEXPORT bool SKSEAPI SKSEPlugin_Load(const SKSE::LoadInterface* a_skse)
{
	logger::info("loaded");

	SKSE::Init(a_skse);

	auto papyrus = SKSE::GetPapyrusInterface();

	if (!papyrus->Register(RegisterFuncs)) {
		return false;
	}

	/*
	REL::IDDatabase* a_id = &REL::IDDatabase::get();

	uint64_t b_id = a_id->offset2id(0x00331650);
	logger::info("GetEitherHandSlot : {}", b_id);

	b_id = a_id->offset2id(0x00331620);
	logger::info("GetRightHandSlot : {}", b_id);

	b_id = a_id->offset2id(0x003315F0);
	logger::info("GetLeftHandSlot : {}", b_id);
	*/

	return true;
}
